#include<stdint.h>

#define RCC_AHB1ENR  (*(volatile uint32_t*)0x40023830)
#define RCC_APB1ENR  (*(volatile uint32_t*)0x40023840)

#define GPIOB_MODER   (*(volatile uint32_t*)0x40020400)
#define GPIOA_MODER   (*(volatile uint32_t*)0x40020000)

#define GPIOB_AFRH    (*(volatile uint32_t*)0x40020424)
#define GPIOA_ODR     (*(volatile uint32_t*)0x40020014)

#define GPIOB_OTYPER  (*(volatile uint32_t*)0x40020404)
#define GPIOB_PUPDR   (*(volatile uint32_t*)0x4002040C)

//I2C REGISTERS

#define I2C_CR1   (*(volatile uint32_t*)0x40005400)
#define I2C_CR2   (*(volatile uint32_t*)0x40005404)
#define I2C_DR    (*(volatile uint32_t*)0x40005410)
#define I2C_SR1   (*(volatile uint32_t*)0x40005414)
#define I2C_SR2   (*(volatile uint32_t*)0x40005418)
#define I2C_CCR   (*(volatile uint32_t*)0x4000541C)
#define I2C_TRISE (*(volatile uint32_t*)0x40005420)

void delay(void){
	int i;
	for(i=0;i<5000000;i++);
}


void gpio_int(void){

    RCC_AHB1ENR  |=(1<<0);
    RCC_AHB1ENR  |=(1<<1);

    GPIOA_MODER &= ~(3<<10);
    GPIOA_MODER |= (1<<10);

    GPIOB_MODER &= ~(3<<16);
    GPIOB_MODER &= ~(3<<18);

    GPIOB_MODER |= (2<<16);
    GPIOB_MODER |= (2<<18);

    GPIOB_AFRH &=~(0xFF);
    GPIOB_AFRH |= (4<<0);
    GPIOB_AFRH |= (4<<4);

    GPIOB_OTYPER |=(1<<8);
    GPIOB_OTYPER |=(1<<9);

    GPIOB_PUPDR &= ~(3<<16);
    GPIOB_PUPDR &= ~(3<<18);

    GPIOB_PUPDR |= (1<<16);
    GPIOB_PUPDR |= (1<<18);


}

void i2c_int(void){

	RCC_APB1ENR |=(1<<21);

	I2C_CR1 &= ~(1<<0);
	I2C_CR2 =16;
	I2C_CCR =80;
	I2C_TRISE =17;
	I2C_CR1 |=(1<<0);


}

void i2c1start (void){

	I2C_CR1 |=(1<<8);

	while(!(I2C_SR1 & (1<<0)));

}


void i2c1address(uint8_t address){

	I2C_DR = (address << 1);

	while(!(I2C_SR1 & (1<<1)));

	volatile uint32_t temp;

		temp = I2C_SR1;
		temp = I2C_SR2;

}


void i2c1write (uint8_t data){

	while(!(I2C_SR1 & (1<<7)));

	I2C_DR = data;

	while(!(I2C_SR1 & (1<<2)));

}

void i2c1stop (void){

	I2C_CR1 |=(1<<9);
}
void led_blink(void){

	GPIOA_ODR |= (1<<5);
	delay();

	GPIOA_ODR &= ~(1<<5);
	delay();
}
void string(char *str){

	while(*str){

		i2c1write(*str++);
	}

		while(!(I2C_SR1 & (1<<2)));
	}



int main(void){

	char dd[]="Aravind samy Embedded engineer";

	gpio_int();
    i2c_int();

	while(1){
		led_blink();
		i2c1start();

		i2c1address(0x27);

		string(dd);

		i2c1stop ();

		delay();


	}

}
