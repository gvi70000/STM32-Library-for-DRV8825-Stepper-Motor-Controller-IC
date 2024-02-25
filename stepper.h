#ifndef __stepper_H
#define __stepper_H

#ifdef __cplusplus
extern "C" {
#endif
#include "gpio.h"
//MODE2		MODE1		MODE0		STEP MODE
//0 			0 			0 			Full step (2-phase excitation) with 71% current
//0 			0 			1 			1/2 step (1-2 phase excitation)
//0 			1 			0 			1/4 step (W1-2 phase excitation)
//0 			1 			1 			8 microsteps/step
//1 			0 			0 			16 microsteps/step
//1 			0 			1 			32 microsteps/step
//1 			1 			0 			32 microsteps/step
//1 			1 			1 			32 microsteps/step
	
// Below values are valid for stepper 1 connections to DRV8825
//M0 connected to PA1
//M1 connected to PA0
//M2 connected to PC3	
// Below values are valid for stepper 2 connections to DRV8825
//M0 connected to PC7
//M1 connected to PC6
//M2 connected to PC12	
#define STEPPERS_COUNT	2
#define GearRatio				405.0
#define AccRatio				0.2
#define MaxSpeedEnd			0.8
#define MaxStepAngle		0.9
#define STEPS_TO_BREAK 		900
//Maximum steps per second
//for simple calculation keep it as multiple of 50 -> 50*36=1800steps/sec for microstepping of 1
//#define MAX_SPS1	(36 * SPS_Update)
//#define MAX_SPS2	(2 * MAX_SPS1)
//#define MAX_SPS4	(4 * MAX_SPS1)
//#define MAX_SPS8	(8 * MAX_SPS1)
//#define MAX_SPS16	(16 * MAX_SPS1)
//#define MAX_SPS32	(32 * MAX_SPS1)

extern volatile uint8_t Stepper_Fault[2];
extern volatile uint32_t Stepper_Pos[2];

typedef enum {
    SS_STOPPED         = 0,
    SS_STARTING        = 1,
		SS_FULLSPEED       = 2,
    SS_BREAKING        = 3,
    SS_BREAKCORRECTION = 4
} stepper_status;

typedef struct {
	uint8_t Id;	
	stepper_status Status;
	volatile uint8_t Fault; //handle by interrupt
	volatile int32_t CrtPos; //handle by interrupt
	uint8_t Direction;
	uint8_t Gear;
	uint16_t SPS;//Steps per second. For uStep of 32 we have max. 57600SPS
	uint32_t StepsDec;//When To start decelerating
	uint32_t StepsToTarget;//Steps to target
} stepper_params;

void enableMotor(uint8_t motorNumber, uint8_t isOnOff);
void setGear(uint8_t motorNumber, uint8_t gear);
void directionMotor(uint8_t motorNumber, uint8_t dir);
void resetMotor(uint8_t motorNumber, uint8_t rstOnOff);
void setMotorSpeed(uint8_t motorNumber, uint16_t freq);
void Stepper_UpdateAll(void);
void Stepper_FaultUpdate(uint8_t myStepper, uint8_t value);
void Stepper_PulseTimerUpdate(uint8_t myStepper);
void Stepper_Refresh(stepper_params * crtStepper);
void Steppers_Init(void);
void Stepper_Move(uint8_t myStepper, double targetPos);

#ifdef __cplusplus
}
#endif

#endif /* __stepper_H */

