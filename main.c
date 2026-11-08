#include <stdint.h>
#include <stm32f10x.h>

void delay(uint32_t ticks) {
	for (int i=0; i<ticks; i++) {
		__NOP();
	}
}


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

const uint8_t digits[10][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, 
    {0x00, 0x42, 0x7F, 0x40, 0x00}, 
    {0x42, 0x61, 0x51, 0x49, 0x46}, 
    {0x21, 0x41, 0x45, 0x4B, 0x31}, 
    {0x18, 0x14, 0x12, 0x7F, 0x10}, 
    {0x27, 0x45, 0x45, 0x45, 0x39}, 
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, 
    {0x01, 0x71, 0x09, 0x05, 0x03}, 
    {0x36, 0x49, 0x49, 0x49, 0x36}, 
    {0x06, 0x49, 0x49, 0x29, 0x1E}  
};

volatile uint8_t seconds_counter = 0;

void DrawDigit(uint8_t num) {
    if (num > 9) num = 0;

    for (uint8_t p = 0; p < 8; p++) {
        display_cmd(0xB0 | p);
        display_cmd(0x00);   
        display_cmd(0x10);
        for (uint8_t i = 0; i < 128; i++) {
            display_data(0x00); 
        }
    }

    display_cmd(0xB0 | 3);
    display_cmd(0x00 | (60 & 0x0F));
    display_cmd(0x10 | (60 >> 4));

    for (uint8_t i = 0; i < 5; i++) {
        display_data(digits[num][i]);
    }
    display_data(0x00);
}

void TIM1_Init_1Hz(void) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    TIM1->PSC = 7199;  
    TIM1->ARR = 19999; 
    
    TIM1->DIER |= TIM_DIER_UIE; 
    
    NVIC_ClearPendingIRQ(TIM1_UP_IRQn);
    NVIC_EnableIRQ(TIM1_UP_IRQn); 
    
    TIM1->CR1 |= TIM_CR1_CEN; 
}

void TIM1_UP_IRQHandler(void) {
    if (TIM1->SR & TIM_SR_UIF) {
        TIM1->SR &= ~TIM_SR_UIF; 
        GPIOC->ODR ^= (1U << 13);
        seconds_counter++;
        if (seconds_counter > 9) {
            seconds_counter = 0;
        }
        
        DrawDigit(seconds_counter);
    }
}

int __attribute((noreturn)) main(void) {
    SPI1_Init();
    SSD1306_Init();
    DrawDigit(0);
    TIM1_Init_1Hz();
	while (1) {}
}

