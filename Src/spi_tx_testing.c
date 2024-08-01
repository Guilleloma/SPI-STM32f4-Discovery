#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "stm32f407xx.h"
#include "stm32f407xx_gpio_driver.h"
#include "stm32f407xx_spi_driver.h"

// Inicialización del GPIO para el LED (PD12)
void LED_GPIOInit(void) {
    GPIO_Handle_t LEDPin;

    // Configurar el pin PD12
    LEDPin.pGPIOx = GPIOD;
    LEDPin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12; // Pin PD12
    LEDPin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUTPUT; // Modo de salida
    LEDPin.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW; // Velocidad baja
    LEDPin.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP; // Salida push-pull
    LEDPin.GPIO_PinConfig.GPIO_PinPUPDControl = GPIO_NO_PUPD; // Sin pull-up/pull-down

    GPIO_Init(&LEDPin);

    // Configurar el pin PD13 (LED Naranja)

    LEDPin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
    GPIO_Init(&LEDPin);

    // Configurar el pin PD14 (LED Rojo)
    LEDPin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
    GPIO_Init(&LEDPin);

    // Configurar el pin PD15 (LED Azul)
    LEDPin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
    GPIO_Init(&LEDPin);
}


void LED_On(uint8_t PinNumber) {
    GPIO_WriteToOutputPin(GPIOD, PinNumber, GPIO_PIN_SET);
}

void LED_Off(uint8_t PinNumber) {
    GPIO_WriteToOutputPin(GPIOD, PinNumber, GPIO_PIN_RESET);
}

// Función para alternar el estado del LED
void toggle_LED(void) {
    GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_NO_12);
}

// Función de retardo simple
void delay(void) {
    for (uint32_t i = 0; i < 800000; i++); // Ajustar según la velocidad del reloj
}

void SPI1_GPIOInits(void)
{
    GPIO_Handle_t SPIPins;

    SPIPins.pGPIOx = GPIOA;
    SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 5; //AF5 para SPI1
    SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    SPIPins.GPIO_PinConfig.GPIO_PinPUPDControl = GPIO_NO_PUPD;
    SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;

    // SCLK
    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5;
    GPIO_Init(&SPIPins);

    // MOSI
    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_7;
    GPIO_Init(&SPIPins);

    // MISO
    // SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
    // GPIO_Init(&SPIPins);

    // NSS
     SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_4;
     GPIO_Init(&SPIPins);
}

void SPI1_Inits(void)
{
    SPI_Handle_t SPI1handle;

    SPI1handle.pSPIx = SPI1;
    SPI1handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
    SPI1handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
    SPI1handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV2; // genera sclk de 8MHz
    SPI1handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS;
    SPI1handle.SPIConfig.SPI_CPOL = SPI_CPOL_HIGH;
    SPI1handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;
    //SPI1handle.SPIConfig.SPI_SSM = SPI_SSM_EN; // gestión de esclavo por software habilitada para pin NSS
    SPI1handle.SPIConfig.SPI_SSM = SPI_SSM_DI;
    SPI_Init(&SPI1handle);



}

int main(void) {
    char user_data[] = "Hello world";

    // Inicializar el GPIO para el LED
    LED_GPIOInit();
    // Encender cada LED de forma secuencial para prueba
    LED_On(GPIO_PIN_NO_13); // LED Naranja
    delay();
    LED_Off(GPIO_PIN_NO_13);

    LED_On(GPIO_PIN_NO_14); // LED Rojo
    delay();
    LED_Off(GPIO_PIN_NO_14);

    LED_On(GPIO_PIN_NO_15); // LED Azul
    delay();
    LED_Off(GPIO_PIN_NO_15);

    // Inicializar los pines GPIO para comportarse como pines SPI1
    SPI1_GPIOInits();

    // Inicializar el periférico SPI1
    SPI1_Inits();

    // Configurar NSS a alto inicialmente (deseleccionar esclavo)
        GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_4, GPIO_PIN_SET);

        // Esperar un breve momento antes de comenzar la transmisión
            delay();

            // Configurar NSS a bajo para seleccionar el esclavo
            GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_4, GPIO_PIN_RESET);


    // Hacer NSS internamente alto y evitar error MODF (problema del maestro)
    //SPI_SSIConfig(SPI1, ENABLE);

    // Habilitar el periférico SPI
    SPI_PeripheralControl(SPI1, ENABLE);

    // Verificar registro RCC->APB2ENR
    volatile uint32_t apb2enr = RCC->APB2ENR;

    // Verificar registros CR1 y CR2 de SPI1
    volatile uint32_t cr1 = SPI1->CR1;
    volatile uint32_t cr2 = SPI1->CR2;

    // Encender el LED Naranja si el SPI está habilitado (SPE bit en CR1)
    if (SPI1->CR1 & (1 << SPI_CR1_SPE)) {
         LED_On(GPIO_PIN_NO_13); // LED Naranja encendido
    }


    // Verifica CR1 y SR antes de enviar datos
    volatile uint32_t debug_CR1_before = SPI1->CR1;
    volatile uint32_t debug_SR_before = SPI1->SR;

   // Enviar datos a través del SPI y encender LED Rojo si TXE (Transmitter Empty) está configurado
    if (SPI1->SR & (1 << SPI_SR_TXE)) {
        LED_On(GPIO_PIN_NO_14); // LED Rojo encendido
    }

    // Enviar datos a través del SPI
    SPI_Send(SPI1, (uint8_t*)user_data, strlen(user_data));

    // Verifica CR1 y SR después de enviar datos
    volatile uint32_t debug_CR1_after = SPI1->CR1;
    volatile uint32_t debug_SR_after = SPI1->SR;

    // Apagar el LED Rojo y encender el LED Azul si BSY (Busy) está desactivado
        if (!(SPI1->SR & (1 << SPI_SR_BSY))) {
            LED_Off(GPIO_PIN_NO_14); // Apagar LED Rojo
            LED_On(GPIO_PIN_NO_15);  // Encender LED Azul
        }

        // Configurar NSS a alto para deseleccionar el esclavo
            GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_4, GPIO_PIN_SET);

    while(1) {
        // Alternar el estado del LED
        toggle_LED();
        // Esperar 100 ms
        delay();
    }

    return 0;
}
