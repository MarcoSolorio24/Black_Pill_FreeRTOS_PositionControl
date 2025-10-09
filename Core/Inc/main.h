
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stm32f4xx_hal.h>
#include <cstdlib>
#include <FreeRTOS.h>
#include <task.h>
#include "stm32f4xx_hal_tim.h"
#include <cstring>
#include <cstdio>
#include <cmath>
#include <timers.h>
#include <semphr.h>
#include <queue.h>


#define Ts 0.001

#define HIGH GPIO_PIN_SET
#define LOW GPIO_PIN_RESET
#define Led_Builtin GPIO_PIN_13
#define DEBUG_PIN GPIO_PIN_3
#define DEBUG_GPIO GPIOB

void GPIO_Init(void);
void Clock_Init(void);
void UART1_Init(void);
void System_Init(void);
void TIM1_Init(void);
void TIM2_Init(void);

void blinkTimer(TimerHandle_t);
void command_task(void *);
void control_task(void *);
void sample_task(void *);
void transmitData(const char*,uint8_t);

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
