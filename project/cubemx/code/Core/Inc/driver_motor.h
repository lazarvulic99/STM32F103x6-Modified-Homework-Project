

#ifndef CORE_INC_DRIVER_MOTOR_H_
#define CORE_INC_DRIVER_MOTOR_H_

extern void MOTOR_SpeedIncrease();
extern void MOTOR_SpeedDecrease();
extern void stopMotor();
extern void startMotor();
extern void redefineSteps(uint32_t newSteps);

extern void MOTOR_Init();

#endif /* CORE_INC_DRIVER_MOTOR_H_ */
