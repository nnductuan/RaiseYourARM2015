/*
 * Button.c
 *
 *  Created on: Jul 6, 2015
 *      Author: NHH
 */

#include "../include.h"
#include "Button.h"

static void ButtonsISR(void);
static void (*Button_right_callback)(), (*Button_left_callback)();
static void ButtonDebounceCallback(void);
static void button_Stoptimeout(void);
static void button_Runtimeout(TIMER_CALLBACK_FUNC TimeoutCallback, uint32_t msTime);


static uint8_t Button_pressed = 0;
static TIMER_ID button_TimerID;

void Button_init(void)
{
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_AFSEL) &= ~0x01;

    ROM_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
    ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD_WPU);
    ROM_GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_FALLING_EDGE);
	GPIOIntRegister(GPIO_PORTF_BASE, &ButtonsISR);
	ROM_IntEnable(INT_GPIOF);
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
}

bool ButtonRegisterCallback(BUTTON_TYPE ButtonSelect, void (*ButtonCallback)())
{
	if (ButtonSelect == BUTTON_RIGHT)
	{
		Button_right_callback = ButtonCallback;
		return true;
	}
	else if (ButtonSelect == BUTTON_LEFT)
	{
		Button_left_callback = ButtonCallback;
		return true;
	}
	return false;
}

static void ButtonsISR(void)
{
	uint32_t ui32_IntStatus;
	ui32_IntStatus = GPIOIntStatus(GPIO_PORTF_BASE, true);
	GPIOIntClear(GPIO_PORTF_BASE, ui32_IntStatus);

	if (ui32_IntStatus & GPIO_PIN_0)
	{
		Button_pressed |= 0x01 << BUTTON_RIGHT;
	}
	if (ui32_IntStatus & GPIO_PIN_4)
	{
		Button_pressed |= 0x01 << BUTTON_LEFT;
	}
	button_Runtimeout(&ButtonDebounceCallback, BUTTON_DEBOUNCE_MS);
}

static void ButtonDebounceCallback(void)
{
	if ((Button_pressed & (0x01 << BUTTON_RIGHT)) && (Button_right_callback != NULL))
	{
		if (ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0) == 0)
		{
			Button_right_callback();
		}
	}
	if ((Button_pressed & (0x01 << BUTTON_LEFT)) && (Button_left_callback != NULL))
	{
		if (ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == 0)
		{
			Button_left_callback();
		}
	}
	Button_pressed = 0;
}

static void button_Stoptimeout(void)
{
	if (button_TimerID != INVALID_TIMER_ID)
		TIMER_UnregisterEvent(button_TimerID);
	button_TimerID = INVALID_TIMER_ID;
}

static void button_Runtimeout(TIMER_CALLBACK_FUNC TimeoutCallback, uint32_t msTime)
{
	button_Stoptimeout();
	button_TimerID = TIMER_RegisterEvent(TimeoutCallback, msTime);
}
