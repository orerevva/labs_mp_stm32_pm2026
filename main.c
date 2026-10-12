#include <stdint.h>
#include <stm32f10x.h>

void delay(uint32_t ticks) {
	for (int i=0; i<ticks; i++) {
		__NOP();
	}
}

#define HALF_PERIOD_MIN (1000000U / 64U)
#define HALF_PERIOD_MAX (1000000U * 64U)

int __attribute((noreturn)) main(void) {
	// Enable clock for AFIO
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	// Enable clock for GPIOC
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	// Enable PC13 push-pull mode
	GPIOC->CRH &= ~GPIO_CRH_CNF13; //clear cnf bits
	GPIOC->CRH |= GPIO_CRH_MODE13_0; //Max speed = 10Mhz

	GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
	GPIOA->CRL |= GPIO_CRL_CNF0_1;
	GPIOA->ODR |= (1U << 0);

	GPIOA->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1);
	GPIOA->CRL |= GPIO_CRL_CNF1_1;
	GPIOA->ODR |= (1U << 1);

	uint32_t half_period = 1000000U;

	uint8_t a_was_pressed = 0;
	uint8_t c_was_pressed = 0;

    while (1) {
		if(!(GPIOA->IDR & (1U << 0))){
			if(!a_was_pressed){
				if(half_period > HALF_PERIOD_MIN){
					half_period /= 2;
					if (half_period < HALF_PERIOD_MIN)
						half_period = HALF_PERIOD_MIN;
				}
				a_was_pressed = 1;
			}
		}
		else{
			a_was_pressed = 0;
		}

		if(!(GPIOA->IDR & (1U << 1))){
			if(!c_was_pressed){
				if(half_period < HALF_PERIOD_MAX){
					half_period *= 2;
					if(half_period > HALF_PERIOD_MAX)
						half_period = HALF_PERIOD_MAX;
				}
				c_was_pressed = 1;
			}
		}
		else{
			c_was_pressed = 0;
		}

	    GPIOC->ODR |= (1U<<13U); //U -- unsigned suffix (to avoid syntax warnings in IDE)
		delay(half_period);
	    GPIOC->ODR &= ~(1U<<13U);
	    delay(half_period);
    }
}
