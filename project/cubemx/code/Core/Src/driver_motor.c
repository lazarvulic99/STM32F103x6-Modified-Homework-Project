
#include "tim.h"

#define ARR 9
uint32_t STEP = 2;
#define INCREMENT ((ARR + 1) / STEP)

static uint32_t volatile compareRegisterValue = 0;

void redefineSteps(uint32_t newSteps)
{
	STEP = newSteps;
}

void stopMotor()
{
	htim3.Instance->CCR1 = 0;
}
void startMotor()
{
	htim3.Instance->CCR1 = compareRegisterValue;
}

void MOTOR_SpeedIncrease()
{
	if (compareRegisterValue < ARR + 1)
	{
		compareRegisterValue += INCREMENT;
	}
	htim3.Instance->CCR1 = compareRegisterValue;
}

void MOTOR_SpeedDecrease()
{
	if (compareRegisterValue > 0)
	{
		compareRegisterValue -= INCREMENT;
	}
	htim3.Instance->CCR1 = compareRegisterValue;
}

void MOTOR_Init()
{
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}
