#include "stepper.h"
#include "tim.h"
//angle per step table
static stepper_params S_AZ, S_ALT;
static const uint16_t MaxSPS[7] = {0, 1800, 3600, 7200, 14400, 28800, 57600};
//static const double Angle[7] = {0.0, 0.9, 45, 0.225, 0.1125, 0.05625, 0.028125};
//how many steps we add every milisecond (timer 14 tick) depending on the gear selected
static const uint8_t StepIncrement[7] = {0, 2, 4, 8, 16, 32, 64};
//Timer 2 Channel 3 is controlling the Azimuth/RA/Horizontal motor, or motor 0
//Timer 5 Channel 4 is controlling the Altitude/DEC/Vertical motor, or motor 1
void enableMotor(uint8_t motorNumber, uint8_t isOnOff){
		switch(motorNumber){
			case 0:
				if(isOnOff){
					Stepper1_On;}
				else{
					Stepper1_Off;}
				break;
			case 1:
				if(isOnOff){
					Stepper2_On;}
				else{
					Stepper2_Off;}
				break;
		}	
}

void setGear(uint8_t motorNumber, uint8_t gear){
		switch(motorNumber){
			case 0:
				switch(gear){
					case 0:
						enableMotor(0, 0);
					break;
					case 1:
						Stepper1_M0_Off;
						Stepper1_M1_Off;
						Stepper1_M2_Off;
					break;
					case 2:
						Stepper1_M0_On;
						Stepper1_M1_Off;
						Stepper1_M2_Off;
					break;
					case 4:
						Stepper1_M0_Off;
						Stepper1_M1_On;
						Stepper1_M2_Off;
					break;
					case 8:
						Stepper1_M0_On;
						Stepper1_M1_On;
						Stepper1_M2_Off;
					break;
					case 16:
						Stepper1_M0_Off;
						Stepper1_M1_Off;
						Stepper1_M2_On;
					break;
					case 32:
						Stepper1_M0_On;
						Stepper1_M1_Off;
						Stepper1_M2_On;
					break;
				}
			break;
			case 1:
				switch(gear){
					case 0:
						enableMotor(1, 0);
					break;
					case 1:
						Stepper2_M0_Off;
						Stepper2_M1_Off;
						Stepper2_M2_Off;
					break;
					case 2:
						Stepper2_M0_On;
						Stepper2_M1_Off;
						Stepper2_M2_Off;
					break;
					case 4:
						Stepper2_M0_Off;
						Stepper2_M1_On;
						Stepper2_M2_Off;
					break;
					case 8:
						Stepper2_M0_On;
						Stepper2_M1_On;
						Stepper2_M2_Off;
					break;
					case 16:
						Stepper2_M0_Off;
						Stepper2_M1_Off;
						Stepper2_M2_On;
					break;
					case 32:
						Stepper2_M0_On;
						Stepper2_M1_Off;
						Stepper2_M2_On;
					break;
				}
				break;
		}	
}

void directionMotor(uint8_t motorNumber, uint8_t dir){
		switch(motorNumber){
			case 0:
				if(dir){
					Stepper1_DirA;}
				else{
					Stepper1_DirB;}
				break;
			case 1:
				if(dir){
					Stepper2_DirA;}
				else{
					Stepper2_DirB;}
				break;
		}	
}

void resetMotor(uint8_t motorNumber, uint8_t rstOnOff){
		switch(motorNumber){
			case 0:
				if(rstOnOff){
					Stepper1_RST_On;}
				else{
					Stepper1_RST_Off;}
				break;
			case 1:
				if(rstOnOff){
					Stepper2_RST_On;}
				else{
					Stepper2_RST_Off;}
				break;
		}	
}

void setMotorSpeed(uint8_t motorNumber, uint16_t freq){
    uint32_t NewTimPeriod = ((TimTickFreq/freq) - 1);
		switch(motorNumber){
			case 0:
					htim2.Instance->ARR = NewTimPeriod;
					htim2.Instance->CNT = 0;
				break;
			case 1:
					htim5.Instance->ARR = NewTimPeriod;
					htim5.Instance->CNT = 0;
				break;
		}	
}

static stepper_params * GetStepper(uint8_t idx) {
	if(idx)
		return &S_ALT;
	else
		return &S_AZ;
}  

static void IncrementSPS(stepper_params * Stepper){
	if (Stepper->SPS < MaxSPS[Stepper->Gear]) {
	Stepper->SPS +=StepIncrement[Stepper->Gear];
	setMotorSpeed(Stepper->Id, Stepper->SPS);
	} else {
	Stepper->Status = SS_FULLSPEED;
	}
	
}

static void DecrementSPS(stepper_params * Stepper){
	Stepper->SPS -=StepIncrement[Stepper->Gear];
	//we have roll-over
	if (Stepper->SPS > MaxSPS[Stepper->Gear]){
		Stepper->SPS = 1;	
	}
	setMotorSpeed(Stepper->Id, Stepper->SPS);
}

void Stepper_UpdateAll(void){
	for(uint8_t i = 0; i < STEPPERS_COUNT; i++) {
		Stepper_Refresh(GetStepper(i));
	}
}

void Stepper_FaultUpdate(uint8_t myStepper, uint8_t value) {
  stepper_params * CrtStepper = GetStepper(myStepper);
	CrtStepper->Fault = value;
}

void Stepper_PulseTimerUpdate(uint8_t myStepper) {
  stepper_params * CrtStepper = GetStepper(myStepper);
	//stepper_status CrtStatus = CrtStepper->Status;
  switch (CrtStepper->Status){
		case SS_STOPPED:
			return;
		case SS_STARTING: 
			CrtStepper->CrtPos++; //increase the curent step
			//the condition to exit SS_STARTING is handled in IncrementSPS function
		break;	
		case SS_FULLSPEED: //Motor is in full speed interval
			CrtStepper->CrtPos++; //increase the curent step
			if (CrtStepper->CrtPos > CrtStepper->StepsDec){// start decelerate
				CrtStepper->Status = SS_BREAKING;
			}
		break;	
		case SS_BREAKING: //Motor is in deceleration interval
			CrtStepper->CrtPos++; //increase the curent step
			if(CrtStepper->CrtPos == CrtStepper->StepsToTarget){
				CrtStepper->Status = SS_STOPPED;
				enableMotor(CrtStepper->Id, 0);//stop the motor
			} else if(CrtStepper->CrtPos > CrtStepper->StepsToTarget){
				//Motor exceeded the limit
				CrtStepper->Status = SS_BREAKCORRECTION;
				CrtStepper->SPS = 64;
				setMotorSpeed(CrtStepper->Id, CrtStepper->SPS);
			}
		break;	
		case SS_BREAKCORRECTION: //Motor is correcting the position
			if (CrtStepper->CrtPos > CrtStepper->StepsToTarget){
				CrtStepper->CrtPos--;
			} else {
				CrtStepper->Status = SS_STOPPED;
				enableMotor(CrtStepper->Id, 0);//stop the motor
			}
		break;
  }
}

//this one is called once every 1ms
void Stepper_Refresh(stepper_params * crtStepper) {
	switch (crtStepper->Status){
	case SS_STARTING:
		IncrementSPS(crtStepper);
	break;
	case SS_BREAKING:
		DecrementSPS(crtStepper);
	break;
	}
}

void Steppers_Init(void) {
	//init the structures
		for(uint32_t i = 0; i < 2; i++) {
			stepper_params * crtStepper = GetStepper(i);
			crtStepper->Id = i;	
			crtStepper->Status = SS_STOPPED;
			crtStepper->Fault = 0; //handle by interrupt
			crtStepper->CrtPos = 0; //handle by interrupt
			crtStepper->Direction = 1;
			crtStepper->Gear = 16;
			crtStepper->SPS = 57600;//Steps per second (actual frequency of the timer) this is why 32 bit timer is nice
			crtStepper->StepsDec = 2;//When To start decelerating
			crtStepper->StepsToTarget = 0;//Steps to target
			resetMotor(i, 0);
			setGear(i, crtStepper->Gear);
		}
	__HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
  __HAL_TIM_ENABLE_IT(&htim5, TIM_IT_UPDATE);
	//Motor 0 PWM
	//HAL_TIM_Base_Start(&htim2);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
	//Motor 1 PWM
	//HAL_TIM_Base_Start(&htim5);
	HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_4);
}

//how much we have to move the output shaft. This takes in to accout the gear ratio
void Stepper_Move(uint8_t myStepper, double targetPos) {
	stepper_params * crtStepper = GetStepper(myStepper);
	//if we try move the motor while motor is doing something else
	if(crtStepper->Status)
		return;
	//the acceleration/deceleration is 20% + 20% of the total time. 60% we move at full speed
	crtStepper->StepsToTarget = (((targetPos * GearRatio) / (MaxStepAngle/(double)crtStepper->Gear)) + 0.5);//here we get the rounded steps to target
	//printf("S2T %d\r\n", crtStepper->StepsToTarget);
	crtStepper->StepsDec = crtStepper->StepsToTarget - STEPS_TO_BREAK;	
	//printf("DEC %d\r\n", crtStepper->StepsDec);
	setMotorSpeed(myStepper, crtStepper->SPS);
	crtStepper->CrtPos = 0;
	crtStepper->Status = SS_STARTING;
	enableMotor(myStepper, 1);
}

