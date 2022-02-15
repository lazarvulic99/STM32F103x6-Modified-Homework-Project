

#include "homework.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdlib.h>
#include <string.h>

#include "driver_lcd.h"
#include "driver_uart.h"
#include "driver_motor.h"
#include "driver_temp.h"
#include "driver_keypad.h"

#include "gpio.h"
#include "timers.h"

//FanState fanState = TURNED_OFF;
uint32_t fanState = 0;

static TimerHandle_t timerToggler;
static uint32_t tempValue;
static char tempText[4];
static char ispis[10];
static uint8_t posebanIspis = 0;
static void homeworkTask(void *parameters)
{
	char podeoci[16] = "Broj podeoka: ";
	UART_AsyncTransmitString(podeoci);
	uint8_t unetiPodeoci = UART_BlockReceiveDecimal();
	UART_AsyncTransmitCharacter('\r');
	UART_AsyncTransmitString("Broj podeoka: ");
	char textPodeoci[4];
	itoa(unetiPodeoci, textPodeoci, 10);
	UART_AsyncTransmitString(textPodeoci);
	UART_AsyncTransmitCharacter('\r');
	redefineSteps(unetiPodeoci);
	char message[16] = "Temperatura: ";
	if (posebanIspis == 0)
	{
		LCD_CommandEnqueue(LCD_INSTRUCTION,
		LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x00);
		for (uint32_t i = 0; i < 16; i++)
		{
			LCD_CommandEnqueue(LCD_DATA, message[i]);
			UART_AsyncTransmitCharacter(message[i]);
		}
	}
	LCD_CommandEnqueue(LCD_INSTRUCTION,
	LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x0D);

	while (1)
	{
		tempValue = TEMP_GetCurrentValue();
		itoa(tempValue, tempText, 10);

		// control fan speed
		//FanState fanStateTarget;
		uint32_t fanStateTarget = 0;
		if (tempValue < 30)
		{
			posebanIspis = 0;
			if (tempValue < 15)
			{
				posebanIspis = 1;
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
				UART_AsyncTransmitString("LEDENO JE");
				ispis[0] = 'L';
				ispis[1] = 'E';
				ispis[2] = 'D';
				ispis[3] = 'E';
				ispis[4] = 'N';
				ispis[5] = 'O';
				ispis[6] = ' ';
				ispis[7] = 'J';
				ispis[8] = 'E';
				ispis[9] = '\0';
			}
			else
			{
				posebanIspis = 0;
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
			}
			//fanStateTarget = TURNED_OFF;
		}
		else if (tempValue < 35)
		{
			posebanIspis = 0;
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
			//fanStateTarget = SLOW;
		}
		else
		{
			if (tempValue > 40)
			{
				posebanIspis = 1;
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
				UART_AsyncTransmitString("TOPIM SE");
				ispis[0] = 'T';
				ispis[1] = 'O';
				ispis[2] = 'P';
				ispis[3] = 'I';
				ispis[4] = 'M';
				ispis[5] = ' ';
				ispis[6] = 'S';
				ispis[7] = 'E';
				ispis[8] = '\0';
			}
			else
			{
				posebanIspis = 0;
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
			}
			//fanStateTarget = FAST;
		}
		uint32_t korakTemp = 60 / unetiPodeoci;
		for(uint32_t i = 0; i < unetiPodeoci; i++)
		{
			if((tempValue >= i*korakTemp) && (tempValue < (i+1)*korakTemp))
			{
				fanStateTarget = i;
			}
		}
		if(tempValue >= 60)
		{
			fanStateTarget = unetiPodeoci;
		}
		if(tempValue == 0)
		{
			fanStateTarget = 0;
		}
		for (uint32_t i = 0; i < abs(fanStateTarget - fanState); i++)
		{
			if (fanStateTarget > fanState)
			{
				MOTOR_SpeedIncrease();
			}
			else
			{
				MOTOR_SpeedDecrease();
			}
		}
		fanState = fanStateTarget;

		// write temperature value
		LCD_CommandEnqueue(LCD_INSTRUCTION,
		LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x0D);

		for (uint32_t i = 0; i < strlen(tempText); i++)
		{
			LCD_CommandEnqueue(LCD_DATA, tempText[i]);
		}
		if (posebanIspis == 0)
		{
			LCD_CommandEnqueue(LCD_INSTRUCTION,
			LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x0D);
			for (uint32_t i = 0; i < strlen(tempText); i++)
			{
				LCD_CommandEnqueue(LCD_DATA, tempText[i]);
				UART_AsyncTransmitCharacter(tempText[i]);
			}

			LCD_CommandEnqueue(LCD_INSTRUCTION,
			LCD_SET_CG_RAM_ADDRESS_INSTRUCTION | 0x08);

			LCD_CommandEnqueue(LCD_DATA, 0x13); // * _ _ * * 13
			LCD_CommandEnqueue(LCD_DATA, 0x04); // _ _ * _ _ 04
			LCD_CommandEnqueue(LCD_DATA, 0x08); // _ * _ _ _ 08
			LCD_CommandEnqueue(LCD_DATA, 0x08); // _ * _ _ _ 08
			LCD_CommandEnqueue(LCD_DATA, 0x08); // _ * _ _ _ 08
			LCD_CommandEnqueue(LCD_DATA, 0x04); // _ _ * _ _ 04
			LCD_CommandEnqueue(LCD_DATA, 0x03); // _ _ _ * * 03

			LCD_CommandEnqueue(LCD_INSTRUCTION,
			LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x0F);
			LCD_CommandEnqueue(LCD_DATA, 0x01);
		}

		vTaskDelay(pdMS_TO_TICKS(200));

		// clear temperature value
		if (posebanIspis == 0)
		{
			LCD_CommandEnqueue(LCD_INSTRUCTION,
			LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x0D);
			for (uint32_t i = 0; i < strlen(tempText); i++)
			{
				LCD_CommandEnqueue(LCD_DATA, ' ');
				UART_AsyncTransmitCharacter('\b');
			}
		}
		else
		{
			LCD_CommandEnqueue(LCD_INSTRUCTION,
			LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x0D);
			for (uint32_t i = 0; i < strlen(ispis); i++)
			{
				LCD_CommandEnqueue(LCD_DATA, ' ');
				UART_AsyncTransmitCharacter('\b');
			}
		}
	}
}

void timerTogglerCallback(TimerHandle_t timerToggler)
{
	if(tempValue >= 20)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
	}
	if(tempValue <= 35)
	{
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
	}
}

void homeworkInit()
{
	timerToggler = xTimerCreate("timerToggler", pdMS_TO_TICKS(1000), pdTRUE, NULL, timerTogglerCallback);
	xTimerStart(timerToggler, portMAX_DELAY);
	LCD_Init();
	UART_Init();
	MOTOR_Init();
	TEMP_Init();
	xTaskCreate(homeworkTask, "homeworkTask", 64, NULL, 5, NULL);
	KEY_Init();
}

