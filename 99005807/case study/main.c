/*
 * main.c
 *
 *  Created on: 20-Sep-2021
 *      Author: Hp dv 6000
 */
#include"stm32f407xx.h"
#include"stm32f407xx_gpio_drivers.h"



#define LED_PIN_GREEN       GPIO_PIN_NO_12
#define LED_PIN_ORANGE      GPIO_PIN_NO_13
#define LED_PIN_RED         GPIO_PIN_NO_14
#define LED_PIN_BLUE        GPIO_PIN_NO_15



/**
 * States of the system
 *
 */
#define STATE_START 0
#define STATE_IDLE 1
#define STATE_IGNITION 2
#define STATE_NO_SEATBELT 3
#define STATE_CRASH_NO_SEATBELT 4
#define STATE_CRASH_SEATBELT 5

/**
 *Functions to handle leds
 *
 */
void GPIO_Led_Initialize(GPIO_Handle_t* GpioLed,uint8_t LedPin){
	GpioLed->pGPIOx = GPIOD;
		GpioLed->GPIO_PinConfig.GPIO_PinNumber = LedPin;
		GpioLed->GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
		GpioLed->GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
		GpioLed->GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
		GpioLed->GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
}

void Toggle_All_LED(){

	GPIO_ToggleOutputPin(GPIOD,LED_PIN_ORANGE);
	GPIO_ToggleOutputPin(GPIOD,LED_PIN_BLUE);
	GPIO_ToggleOutputPin(GPIOD,LED_PIN_GREEN);
	GPIO_ToggleOutputPin(GPIOD,LED_PIN_RED);
}

void Reset_All_LED(){

	GPIO_WriteToOutputPin(GPIOD,LED_PIN_ORANGE,RESET);
	GPIO_WriteToOutputPin(GPIOD,LED_PIN_RED,RESET);
	GPIO_WriteToOutputPin(GPIOD,LED_PIN_GREEN,RESET);
	GPIO_WriteToOutputPin(GPIOD,LED_PIN_BLUE,RESET);

}
void Led_State_Machine(uint8_t* State,uint8_t* led_timer){
	 switch (*State)
	    {
	    // just change the state to ignintion
	    case STATE_START:
	        *State = STATE_IGNITION;
	        break;

	    case STATE_IGNITION:
	        GPIO_ToggleOutputPin(GPIOD,LED_PIN_GREEN);
	        delay(1000);
	        *(led_timer)= *(led_timer) +1;
	        if (*(led_timer) > 5){
	            *State = STATE_IDLE;
	            Reset_All_LED();
	        }
	        break;
	    case STATE_NO_SEATBELT:
	        delay(1000);
	        Toggle_All_LED();
	        break;

	    case STATE_CRASH_SEATBELT:
	        GPIO_ToggleOutputPin(GPIOD,LED_PIN_BLUE);
	        delay(1000);
	        break;

	    case STATE_CRASH_NO_SEATBELT:

	        GPIO_ToggleOutputPin(GPIOD,LED_PIN_RED);
	        delay(1000);
	        *(led_timer)= *(led_timer) +1;
	        if (*(led_timer) > 15){
	            *State = STATE_IDLE;
	            Reset_All_LED();
	        }
	        break;

	    case STATE_IDLE:
	        // Empty state does nothing
	        break;


	    default:
	        break;
	    }
}



volatile uint8_t Interrupt_flag = RESET;
volatile uint8_t Interrupt_count = 0;

int main(void)
{

	uint8_t State = STATE_START;		/*Store the state of the system*/
	uint8_t led_timer = 0;				/*To take care of timing of states*/
	GPIO_Handle_t GPIOBtn;				/* Button handler*/
    GPIO_Handle_t GpioLedRed;
    GPIO_Handle_t GpioLedGreen;
    GPIO_Handle_t GpioLedBlue;
    GPIO_Handle_t GpioLedOrange;

	//*Initialize all leds*/
	GPIO_Led_Initialize(&GpioLedRed,LED_PIN_RED);
	GPIO_Led_Initialize(&GpioLedGreen,LED_PIN_GREEN);
	GPIO_Led_Initialize(&GpioLedBlue,LED_PIN_BLUE);
	GPIO_Led_Initialize(&GpioLedOrange,LED_PIN_ORANGE);
	GPIO_PeriClockControl(GPIOD,ENABLE);

	/*Initilize the GPIOs*/
	GPIO_Init(&GpioLedRed);
	GPIO_Init(&GpioLedGreen);
	GPIO_Init(&GpioLedBlue);
	GPIO_Init(&GpioLedOrange);

	/*Configure the Button*/
	GPIOBtn.pGPIOx = GPIOA;
	GPIOBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	GPIOBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IT_FT;
	GPIOBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GPIOBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;

	GPIO_PeriClockControl(GPIOA,ENABLE);

	GPIO_Init(&GPIOBtn);

	// GPIO_WriteToOutputPin(GPIOD,GPIO_PIN_NO_12,GPIO_PIN_RESET);
	//IRQ configurations
	GPIO_IRQPriorityConfig(IRQ_NO_EXTI0,NVIC_IRQ_PRI0);
	GPIO_IRQInterruptConfig(IRQ_NO_EXTI0,ENABLE);


    while(1){
		Led_State_Machine(&State ,&led_timer);
		if (Interrupt_flag == SET && Interrupt_count == 0){
			led_timer = 0;
			State = STATE_NO_SEATBELT;
			Reset_All_LED();


		}
		if (Interrupt_flag == SET && Interrupt_count == 1)
		{
			led_timer = 0;
			State = STATE_CRASH_NO_SEATBELT;
			Reset_All_LED();

		}
		if (Interrupt_flag == SET && Interrupt_count == 2)
		{
			led_timer = 0;
			State = STATE_CRASH_SEATBELT;
			Reset_All_LED();
		}
		Interrupt_flag =RESET;
		Interrupt_count =RESET;
	}

}


void EXTI0_IRQHandler(void)
{
   delay(50); //200ms . wait till button de-bouncing gets over
	GPIO_IRQHandling(GPIO_PIN_NO_0); //clear the pending event from exti line
	if (Interrupt_flag == SET){
		Interrupt_count = Interrupt_count + 1;
	}
	Interrupt_flag = SET;
}
