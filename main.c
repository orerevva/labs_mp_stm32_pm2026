#include <stdint.h>
#include <stm32f10x.h>

void delay(uint32_t ticks) {
	for (int i=0; i<ticks; i++) {
		__NOP();
	}
}

//#define HALF_PERIOD_MIN (1000000U / 64U)
//#define HALF_PERIOD_MAX (1000000U * 64U)

#define CS_LOW()   (GPIOA->ODR &= ~(1U << 4))
#define CS_HIGH()  (GPIOA->ODR |=  (1U << 4))
#define DC_CMD()   (GPIOA->ODR &= ~(1U << 1))
#define DC_DATA()  (GPIOA->ODR |=  (1U << 1))
#define RES_LOW()  (GPIOA->ODR &= ~(1U << 2))
#define RES_HIGH() (GPIOA->ODR |=  (1U << 2))

void SPI1_Init(void) 
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    GPIOA->CRL &= ~(GPIO_CRL_MODE5 | GPIO_CRL_CNF5 |
                    GPIO_CRL_MODE7 | GPIO_CRL_CNF7);
    GPIOA->CRL |= (GPIO_CRL_MODE5_0 | GPIO_CRL_MODE5_1 | GPIO_CRL_CNF5_1);
    GPIOA->CRL |= (GPIO_CRL_MODE7_0 | GPIO_CRL_MODE7_1 | GPIO_CRL_CNF7_1);

    GPIOA->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1 |
                    GPIO_CRL_MODE2 | GPIO_CRL_CNF2 |
                    GPIO_CRL_MODE4 | GPIO_CRL_CNF4);
    GPIOA->CRL |= (GPIO_CRL_MODE1_1 | GPIO_CRL_MODE2_1 | GPIO_CRL_MODE4_1);

    CS_HIGH();
    DC_CMD();
    RES_HIGH();

    SPI1->CR1 |= SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_MSTR |
                 SPI_CR1_BR | SPI_CR1_SSM | SPI_CR1_SPE;
    SPI1->CR1 |= SPI_CR1_SSI;
}

void SPI1_Write(uint8_t data) 
{
    while (!(SPI1->SR & SPI_SR_TXE));
    SPI1->DR = data;
}

uint8_t SPI1_Read(void) 
{
    SPI1->DR = 0;
    while (!(SPI1->SR & SPI_SR_RXNE));
    return SPI1->DR;
}

void display_cmd(uint8_t cmd)
{
    CS_LOW();
    DC_CMD();
    SPI1_Write(cmd);
    CS_HIGH();
}

void display_data(uint8_t dat) {
    CS_LOW();
    DC_DATA();
    SPI1_Write(dat);
    CS_HIGH();
}

void SSD1306_Init(void) 
{
    RES_LOW();
    delay(100000);
    RES_HIGH();
    delay(100000);

    display_cmd(0xAE); 
    display_cmd(0x20); display_cmd(0x00); 
    display_cmd(0xB0); 
    display_cmd(0xC8); 
    display_cmd(0x00); 
    display_cmd(0x10); 
    display_cmd(0x40); 
    display_cmd(0x81); display_cmd(0x7F); 
    display_cmd(0xA1); 
    display_cmd(0xA6); 
    display_cmd(0xA8); display_cmd(0x3F); 
    display_cmd(0xA4); 
    display_cmd(0xD3); display_cmd(0x00); 
    display_cmd(0xD5); display_cmd(0x80); 
    display_cmd(0xD9); display_cmd(0xF1); 
    display_cmd(0xDA); display_cmd(0x12); 
    display_cmd(0xDB); display_cmd(0x40); 
    display_cmd(0x8D); display_cmd(0x14); 
    display_cmd(0xAF); 
}

void draw_chessboard(void) 
{
    for (uint8_t page = 0; page < 8; page++) {
        display_cmd(0xB0 | page); 
        display_cmd(0x00);       
        display_cmd(0x10);       
        for (uint8_t col = 0; col < 128; col++) {
            uint8_t byte = 0;
            for (uint8_t bit = 0; bit < 8; bit++) {
                uint8_t y = page * 8 + bit;
                if (((col / 8) + (y / 8)) & 1)
                    byte |= (1U << bit);
            }
            display_data(byte);
        }
    }
}

int __attribute((noreturn)) main(void) {
	SPI1_Init();
    SSD1306_Init();
    draw_chessboard();
	while (1) {

    }

	// // Enable clock for AFIO
	// RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	// // Enable clock for GPIOC
	// RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	// RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	// // Enable PC13 push-pull mode
	// GPIOC->CRH &= ~GPIO_CRH_CNF13; //clear cnf bits
	// GPIOC->CRH |= GPIO_CRH_MODE13_0; //Max speed = 10Mhz

	// GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);
	// GPIOA->CRL |= GPIO_CRL_CNF0_1;
	// GPIOA->ODR |= (1U << 0);

	// GPIOA->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1);
	// GPIOA->CRL |= GPIO_CRL_CNF1_1;
	// GPIOA->ODR |= (1U << 1);

	// uint32_t half_period = 1000000U;

	// uint8_t a_was_pressed = 0;
	// uint8_t c_was_pressed = 0;

    // while (1) {
	// 	if(!(GPIOA->IDR & (1U << 0))){
	// 		if(!a_was_pressed){
	// 			if(half_period > HALF_PERIOD_MIN){
	// 				half_period /= 2;
	// 				if (half_period < HALF_PERIOD_MIN)
	// 					half_period = HALF_PERIOD_MIN;
	// 			}
	// 			a_was_pressed = 1;
	// 		}
	// 	}
	// 	else{
	// 		a_was_pressed = 0;
	// 	}

	// 	if(!(GPIOA->IDR & (1U << 1))){
	// 		if(!c_was_pressed){
	// 			if(half_period < HALF_PERIOD_MAX){
	// 				half_period *= 2;
	// 				if(half_period > HALF_PERIOD_MAX)
	// 					half_period = HALF_PERIOD_MAX;
	// 			}
	// 			c_was_pressed = 1;
	// 		}
	// 	}
	// 	else{
	// 		c_was_pressed = 0;
	// 	}

	//     GPIOC->ODR |= (1U<<13U); //U -- unsigned suffix (to avoid syntax warnings in IDE)
	// 	delay(half_period);
	//     GPIOC->ODR &= ~(1U<<13U);
	//     delay(half_period);
    // }
}
