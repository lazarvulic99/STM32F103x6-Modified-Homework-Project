

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "adc.h"

// MAX VOLTAGE = 5V
#define MAX_VOLTAGE 5.0
// ADC RESOLUTION = 2^12 = 4096 (12-bit resolution)
#define RESOLUTION 4096.0

static TaskHandle_t TEMP_TaskHandle;
static QueueHandle_t TEMP_MailboxHandle;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	BaseType_t woken = pdFALSE;
	vTaskNotifyGiveFromISR(TEMP_TaskHandle, &woken);
	portYIELD_FROM_ISR(woken);
}

static void TEMP_Task(void *parameter)
{
	while (1)
	{
		HAL_ADC_Start_IT(&hadc1);
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		float value = HAL_ADC_GetValue(&hadc1); // get raw conversion result
		value *= MAX_VOLTAGE / RESOLUTION; // translate into the voltage value
		value *= 100; // translate into temperature value (problem specific)

		xQueueOverwrite(TEMP_MailboxHandle, &value);
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}

float TEMP_GetCurrentValue()
{
	float result = 0.0;
	xQueuePeek(TEMP_MailboxHandle, &result, portMAX_DELAY);
	return result;
}

void TEMP_Init()
{
	TEMP_MailboxHandle = xQueueCreate(1, sizeof(float));
	xTaskCreate(TEMP_Task, "TEMP_Task", 64, NULL, 2, &TEMP_TaskHandle);
}
