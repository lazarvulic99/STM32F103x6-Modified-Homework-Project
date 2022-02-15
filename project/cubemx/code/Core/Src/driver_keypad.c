/*
 * driver_keypad.c
 *
 *  Created on: Jan 27, 2022
 *      Author: Lenovo
 */

#include "driver_keypad.h"
#include "FreeRTOS.h"
#include "task.h"
#include "gpio.h"
#include "driver_lcd.h"
#include "driver_motor.h"
#include "timers.h"
#include "adc.h"

static uint8_t KEY_PreviousReleased = 1;
static TimerHandle_t KEY_TimerHandle;
static TaskHandle_t KEY_TaskHandle;
static char KEY_Map[4][3] =
{
{ '1', '2', '3' },
{ '4', '5', '6' },
{ '7', '8', '9' },
{ '*', '0', '#' } };

static void KEY_Task(void *parameters)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (KEY_PreviousReleased == 1)
		{
			for (uint8_t row = 0; row < 4; row++)
			{
				GPIOB->ODR = 0x01 << row;

				// Cekanje na stabilizaciju signala

				uint32_t state = ((GPIOB->IDR >> 4) & 0x07);

				for (uint8_t column = 0; column < 3; column++)
				{
					if (state & (0x01 << column))
					{
						KEY_PreviousReleased = 0;
						xTimerStart(KEY_TimerHandle, portMAX_DELAY);
						LCD_CommandEnqueue(LCD_INSTRUCTION,
								LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x05);
						LCD_CommandEnqueue(LCD_DATA, KEY_Map[row][column]);
					}
				}
			}
			GPIOB->ODR = 0x0F;
		}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_7)
	{
		BaseType_t woken = pdFALSE;
		vTaskNotifyGiveFromISR(KEY_TaskHandle, &woken);
		portYIELD_FROM_ISR(woken);
	}
	else if (GPIO_Pin == GPIO_PIN_11)
	{
		HAL_ADC_Start_IT(&hadc1);
		startMotor();
	}
	else if (GPIO_Pin == GPIO_PIN_12)
	{
		HAL_ADC_Stop(&hadc1);
		stopMotor();
	}
}

static void KEY_TimerCallback(TimerHandle_t KEY_TimerHandle)
{
	if (!KEY_PreviousReleased)
	{
		if (((GPIOB->IDR >> 4) & 0x07) == 0)
		{
			KEY_PreviousReleased = 1;
		}
		else
		{
			xTimerStart(KEY_TimerHandle, portMAX_DELAY);
		}
	}
}

void KEY_Init()
{
	GPIOB->ODR = 0x0F;
	KEY_TimerHandle = xTimerCreate("KEY_TimerHandle", pdMS_TO_TICKS(10),
	pdFALSE, NULL, KEY_TimerCallback);
	xTaskCreate(KEY_Task, "KEY_Task", 64, NULL, 22, &KEY_TaskHandle);
}
