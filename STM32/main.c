#include<stdint.h>
#define RCC_AHB1ENR    (*(volatile uint32_t*)0x40023830)
#define GPIOA_MODER (*(volatile uint32_t*)0x40020000)
#define GPIOA_ODR (*(volatile uint32_t*)0x40020014)
#define STACK_SIZE 128
#define SYSTICK_LOAD  (*(volatile uint32_t*)0xE000E014)
#define SYSTICK_VAL   (*(volatile uint32_t*)0xE000E018)
#define SYSTICK_CTRL  (*(volatile uint32_t*)0xE000E010)

typedef struct
{
  uint32_t *psp_value;
  uint32_t delay_count;
}TCB_t;

uint32_t task1_stack[STACK_SIZE];
uint32_t task2_stack[STACK_SIZE];
TCB_t task1_tcb;
TCB_t task2_tcb;
TCB_t *current_tcb;

volatile uint32_t global_tick = 0;

void GPIO_Init(void)
{
	RCC_AHB1ENR |=(1<<0);
	GPIOA_MODER &=~(3<<10);//PA5 CLEAR
	GPIOA_MODER |=(1<<10);//PA5 SET
	GPIOA_MODER &=~(3<<12);//PA6 CLEAR
	GPIOA_MODER |=(1<<12);//PA6 SET
}
void task_delay(volatile TCB_t *tcb, uint32_t ticks)
{
	tcb->delay_count = ticks;          // set sleep count
	while(tcb->delay_count != 0);
}
//------------------------TASK1----------------------//
void Task1(void)
{
	while(1)
	{
		GPIOA_ODR |=(1<<5);            // LED ON
		task_delay(&task1_tcb, 2);     // sleep 2 ticks
        GPIOA_ODR &=~(1<<5);          // LED OFF
        task_delay(&task1_tcb, 2);     // sleep 2 ticks
	}
}
//-----------TASK2---------------------//
void Task2(void)
{
	while(1)
	{
		GPIOA_ODR |=(1<<6);            // LED ON
		task_delay(&task2_tcb, 4);     // sleep 4 ticks
        GPIOA_ODR &=~(1<<6);          // LED OFF
        task_delay(&task2_tcb, 4);     // sleep 4 ticks
	}
}
//------------PSP STACK---------------//
void Init_task_stack(TCB_t*tcb,uint32_t*stack,void(*task)(void))
{
	uint32_t*psp=0;
	psp = stack+STACK_SIZE;
//---------------stack frame 1------------------------//
	*(--psp)=0x01000000;                        //xpsr
	*(--psp)=(uint32_t)task;                    //pc
	*(--psp)= 0xFFFFFFFD;                       //LR
     *(--psp)=0;                                //r12
	 *(--psp)=0;                                //r3
	 *(--psp)=0;                                //r2
	 *(--psp)=0;                                //r1
	 *(--psp)=0;                                //r0
//---------------stack frame 2 ------------------------//
	 *(--psp)=0;                                //r11
	 *(--psp)=0;                                //r10
	 *(--psp)=0;                                //r9
	 *(--psp)=0;                                //r8
	 *(--psp)=0;                                //r7
	 *(--psp)=0;                                //r6
	 *(--psp)=0;                                //r5
	 *(--psp)=0;                                //r4
	 tcb->psp_value = psp;
	 tcb->delay_count = 0;
}
//---------------------------psp setup-----------------//
void set_psp(uint32_t psp_val)
{
	__asm volatile (
		"MSR PSP,%0"
		:
		:"r"(psp_val)
	);
}
//-------------- switch MSP ---TO----PSP------------------//
void Switch_to_psp(void)
{
	__asm volatile(
	"MOV R0,#0x02  \n"
	"MSR CONTROL ,R0 \n"
	"ISB   \n"
);
}
//-----------SYSTICK INIT------------------------------//
void SysTick_Init(void)
{
	SYSTICK_CTRL = 0;
	SYSTICK_LOAD = 16000000 - 1;
	SYSTICK_VAL  = 0;
	SYSTICK_CTRL = 0x07;
}
//-----------SCHEDULER------------------------------//
void schedule(void)
{
	// decrement delay counters
	if(task1_tcb.delay_count != 0)
		task1_tcb.delay_count--;
	if(task2_tcb.delay_count != 0)
		task2_tcb.delay_count--;

	// pick next ready task (round robin among tasks with delay_count == 0)
	if(current_tcb == &task1_tcb)
	{
		// try task2 next
		if(task2_tcb.delay_count == 0)
			current_tcb = &task2_tcb;
		else if(task1_tcb.delay_count == 0)
			current_tcb = &task1_tcb;
		// else both sleeping — stay on task1 (idle)
	}
	else
	{
		// try task1 next
		if(task1_tcb.delay_count == 0)
			current_tcb = &task1_tcb;
		else if(task2_tcb.delay_count == 0)
			current_tcb = &task2_tcb;
		// else both sleeping — stay on task2 (idle)
	}
}
//-----------CONTEXT SWITCH (full asm)-----------------//
__attribute__((naked)) void SysTick_Handler(void)
{
	__asm volatile(
		"PUSH {LR}                \n"

		// 1. SAVE current task context
		"MRS R0, PSP              \n"
		"STMDB R0!, {R4-R11}      \n"

		// 2. Save PSP into current TCB
		"LDR R1, =current_tcb     \n"
		"LDR R2, [R1]             \n"
		"STR R0, [R2]             \n"

		// 3. Call scheduler (updates delay_count + picks next task)
		"BL schedule              \n"

		// 4. RESTORE next task context
		"LDR R1, =current_tcb     \n"
		"LDR R2, [R1]             \n"
		"LDR R0, [R2]             \n"
		"LDMIA R0!, {R4-R11}      \n"
		"MSR PSP, R0              \n"

		// 5. Return
		"POP {LR}                 \n"
		"BX LR                    \n"
	);
}
//-----------MAIN-----------------------------------------//
int main(void)
{
	GPIO_Init();
	    Init_task_stack(&task1_tcb,task1_stack,Task1);
	    Init_task_stack(&task2_tcb,task2_stack,Task2);
	current_tcb = &task1_tcb;
	set_psp((uint32_t)task1_tcb.psp_value);
	Switch_to_psp();
	SysTick_Init();
	Task1();
	while(1);
}
