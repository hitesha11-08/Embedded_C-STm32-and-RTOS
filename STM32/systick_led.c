#include <stdint.h>
#define RCC_AHB1ENR   (*(volatile uint32_t*)0x40023830)
#define GPIOA_MODER   (*(volatile uint32_t*)0x40020000)
#define GPIOA_ODR    (*(volatile uint32_t*)0x40020014)
#define SYST_CSR      (*(volatile uint32_t*)0xE000E010)//systick load
#define SYST_RVR      (*(volatile uint32_t*)0xE000E014)//systicl value
#define SYST_CVR      (*(volatile uint32_t*)0xE000E018)//systick control
volatile uint32_t g_tick = 0;
void delay(){
	for(int i=0;i<=20000;i++);
}
void led_task(void){ // led task
	static uint32_t end =0;
	if(g_tick-end>=1000){
		GPIOA_ODR|=(1<<5);
		delay();
		GPIOA_ODR &=~(1<<5);
		delay();
		end=g_tick;

	}
}
void SysTick_Handler(void){
	g_tick++;
}
int main(void){
	RCC_AHB1ENR|=(1<<0);
	GPIOA_MODER &= ~(3<<10);
	GPIOA_MODER|=(1<<10);
	SYST_RVR = 16000 - 1;//reload value
	SYST_CVR = 0; //current count
	SYST_CSR = 0x07;//enable systick and clk timer use 111
while(1){
	led_task();
  }
}

