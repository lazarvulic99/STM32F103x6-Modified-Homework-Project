

#include "driver_uart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <string.h>

#include "usart.h"

// TRANSMIT
// -----------------------------------------------------------------------------

static TaskHandle_t UART_TransmitTaskHandle;
static QueueHandle_t UART_TransmitQueueHandle;
static SemaphoreHandle_t UART_TransmitMutexHandle;

static void UART_TransmitTask(void *parameters)
{
	uint8_t buffer;
	while (1)
	{
		xQueueReceive(UART_TransmitQueueHandle, &buffer, portMAX_DELAY);
		HAL_UART_Transmit_IT(&huart1, &buffer, sizeof(uint8_t));
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == huart1.Instance)
	{
		BaseType_t woken = pdFALSE;
		vTaskNotifyGiveFromISR(UART_TransmitTaskHandle, &woken);
		portYIELD_FROM_ISR(woken);
	}
}

// RECEIVE
// -----------------------------------------------------------------------------

static TaskHandle_t UART_ReceiveTaskHandle;
static QueueHandle_t UART_ReceiveQueueHandle;
static SemaphoreHandle_t UART_ReceiveMutexHandle;

static void UART_ReceiveTask(void *parameters)
{
	uint8_t buffer;
	while (1)
	{
		HAL_UART_Receive_IT(&huart1, &buffer, sizeof(uint8_t));
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		xQueueSendToBack(UART_ReceiveQueueHandle, &buffer, portMAX_DELAY);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == huart1.Instance)
	{
		BaseType_t woken = pdFALSE;
		vTaskNotifyGiveFromISR(UART_ReceiveTaskHandle, &woken);
		portYIELD_FROM_ISR(woken);
	}
}

// GENERAL
// -----------------------------------------------------------------------------

void UART_Init()
{
	xTaskCreate(UART_TransmitTask, "transmitTask", 64, NULL, 4,
			&UART_TransmitTaskHandle);
	UART_TransmitQueueHandle = xQueueCreate(64, sizeof(uint8_t));
	UART_TransmitMutexHandle = xSemaphoreCreateMutex();

	xTaskCreate(UART_ReceiveTask, "receiveTask", 64, NULL, 20,
			&UART_ReceiveTaskHandle);
	UART_ReceiveQueueHandle = xQueueCreate(64, sizeof(uint8_t));
	UART_ReceiveMutexHandle = xSemaphoreCreateMutex();
}

// TRANSMIT UTIL
// -----------------------------------------------------------------------------

void UART_AsyncTransmitCharacter(char character)
{
	xSemaphoreTake(UART_TransmitMutexHandle, portMAX_DELAY);

	xQueueSendToBack(UART_TransmitQueueHandle, &character, portMAX_DELAY);

	xSemaphoreGive(UART_TransmitMutexHandle);
}

void UART_AsyncTransmitString(char const *string)
{
	if (string != NULL)
	{
		xSemaphoreTake(UART_TransmitMutexHandle, portMAX_DELAY);

		for (uint32_t i = 0; i < strlen(string); i++)
		{
			xQueueSendToBack(UART_TransmitQueueHandle, string + i,
					portMAX_DELAY);
		}

		xSemaphoreGive(UART_TransmitMutexHandle);
	}
}

void UART_AsyncTransmitDecimal(uint32_t decimal)
{
	xSemaphoreTake(UART_TransmitMutexHandle, portMAX_DELAY);

	char digits[32];
	uint32_t index = 32;
	while (index >= 0 && decimal != 0)
	{
		digits[--index] = '0' + decimal % 10;
		decimal /= 10;
	}

	for (uint32_t i = index; i < 32; i++)
	{
		xQueueSendToBack(UART_TransmitQueueHandle, digits + i, portMAX_DELAY);
	}

	xSemaphoreGive(UART_TransmitMutexHandle);
}

// RECEIVE UTIL
// -----------------------------------------------------------------------------

char UART_BlockReceiveCharacter()
{
	xSemaphoreTake(UART_ReceiveMutexHandle, portMAX_DELAY);

	char character;
	xQueueReceive(UART_ReceiveQueueHandle, &character, portMAX_DELAY);

	xSemaphoreGive(UART_ReceiveMutexHandle);

	return character;
}

char* UART_BlockReceiveString()
{
	xSemaphoreTake(UART_ReceiveMutexHandle, portMAX_DELAY);

	char *string = pvPortMalloc(64);

	if (string != NULL)
	{
		uint32_t index = 0;

		char character = '\0';
		while (character != '\r' && index < 64)
		{
			xQueueReceive(UART_ReceiveQueueHandle, &character, portMAX_DELAY);
			string[index++] = character;
		}

		string[--index] = '\0';
	}

	xSemaphoreGive(UART_ReceiveMutexHandle);

	return string;
}

uint32_t UART_BlockReceiveDecimal()
{
	xSemaphoreTake(UART_ReceiveMutexHandle, portMAX_DELAY);

	uint32_t decimal = 0;

	char character = '\0';
	while (character != '\r')
	{
		xQueueReceive(UART_ReceiveQueueHandle, &character, portMAX_DELAY);
		if (character >= '0' && character <= '9')
		{
			decimal = decimal * 10 + (character - '0');
		}
	}

	xSemaphoreGive(UART_ReceiveMutexHandle);

	return decimal;
}
