/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32l4xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_it.h"
#include "es_wifi_io.h"
#include "main.h"
#include "stm32l475xx.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_spi.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void) {
  while (1) {
  }
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void) {
  while (1) {
  }
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void) {
  while (1) {
  }
}

/**
 * @brief This function handles Prefetch fault, memory access fault.
 */
void BusFault_Handler(void) {
  while (1) {
  }
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void) {
  while (1) {
  }
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void) {}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void) {}

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void) {}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void) { HAL_IncTick(); }

/******************************************************************************/
/* STM32L4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l4xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles SPI3 interrupts
 */
void SPI3_IRQHandler(void) { HAL_SPI_IRQHandler(&hspi); }

/**
 * @brief This function handles EXTI line[1] interrupt
 */
void EXTI1_IRQHandler(void) {
  HAL_GPIO_EXTI_IRQHandler(ISM43362_DRDY_EXTI1_Pin);
  SPI_WIFI_ISR();
}

/**
 * @brief This function handles EXTI line[9:5] interrupts.
 */
void EXTI9_5_IRQHandler(void) {
  HAL_GPIO_EXTI_IRQHandler(SPSGRF_915_GPIO3_EXTI5_Pin);
  HAL_GPIO_EXTI_IRQHandler(SPBTLE_RF_IRQ_EXTI6_Pin);
  HAL_GPIO_EXTI_IRQHandler(VL53L0X_GPIO1_EXTI7_Pin);
  HAL_GPIO_EXTI_IRQHandler(LSM3MDL_DRDY_EXTI8_Pin);
  HAL_GPIO_EXTI_IRQHandler(LED3_WIFI__LED4_BLE_Pin);
}

/**
 * @brief This function handles EXTI line[15:10] interrupts.
 */
void EXTI15_10_IRQHandler(void) {
  HAL_GPIO_EXTI_IRQHandler(LPS22HB_INT_DRDY_EXTI0_Pin);
  HAL_GPIO_EXTI_IRQHandler(LSM6DSL_INT1_EXTI11_Pin);
  HAL_GPIO_EXTI_IRQHandler(BUTTON_EXTI13_Pin);
  HAL_GPIO_EXTI_IRQHandler(HTS221_DRDY_EXTI15_Pin);
}
