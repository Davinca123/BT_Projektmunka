/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "display_driver.h"
#include "display_module.h"
#include "ugui.h"
#include "step_counter.h"
#include "accelerometer_driver.h"
#include "ir_sensor.h"
#include "pulse_meter.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BTN_RESET_PORT  GPIOC
#define BTN_RESET_PIN   GPIO_PIN_0
#define BTN_SWITCH_PORT GPIOC
#define BTN_SWITCH_PIN  GPIO_PIN_1

#define FLAG_MEASURE_TICK  0x0001U

#define DISPLAY_REFRESH_MS    1000u  // Redraw period without new messages (clock)
#define BTN_LOCKOUT_TICKS     20u    // 20 * 10 ms = 200 ms debounce after a press
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MeasureManagerT */
osThreadId_t MeasureManagerTHandle;
const osThreadAttr_t MeasureManagerT_attributes = {
  .name = "MeasureManagerT",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for DisplayTask */
osThreadId_t DisplayTaskHandle;
const osThreadAttr_t DisplayTask_attributes = {
  .name = "DisplayTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for displayQueue */
osMessageQueueId_t displayQueueHandle;
const osMessageQueueAttr_t displayQueue_attributes = {
  .name = "displayQueue"
};
/* Definitions for mutexI2c */
osMutexId_t mutexI2cHandle;
const osMutexAttr_t mutexI2c_attributes = {
  .name = "mutexI2c"
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM6_Init(void);
static void MX_I2C1_Init(void);
void StartDefaultTask(void *argument);
void StartMeasureManagerTask(void *argument);
void StartDisplayTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*******************************************************************************
 ********************** PLATFORM SPECIFIC PORT FUNCTIONS ***********************
 ******************************************************************************/
int port_i2c_transmit(uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_addr_size, uint8_t* pdata, uint16_t size, uint32_t timeout)
{
  // I2C1 is shared between tasks (display, accelerometer, IR sensor) -> serialize access
  if (osMutexAcquire(mutexI2cHandle, osWaitForever) != osOK) return HAL_BUSY;
  int ret = HAL_I2C_Mem_Write(&hi2c1, dev_addr, mem_addr, mem_addr_size, pdata, size, timeout);
  osMutexRelease(mutexI2cHandle);
  return ret;
}

int port_i2c_receive(uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_addr_size, uint8_t* pdata, uint16_t size, uint32_t timeout)
{
  // I2C1 is shared between tasks (display, accelerometer, IR sensor) -> serialize access
  if (osMutexAcquire(mutexI2cHandle, osWaitForever) != osOK) return HAL_BUSY;
  int ret = HAL_I2C_Mem_Read(&hi2c1, dev_addr, mem_addr, mem_addr_size, pdata, size, timeout);
  osMutexRelease(mutexI2cHandle);
  return ret;
}

void port_time_delay(uint32_t delay_ms)
{
//  HAL_Delay(delay_ms);
  osDelay(delay_ms);
}

uint32_t get_systick(void)
{
  return HAL_GetTick();
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM6_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim6);
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();
  /* Create the mutex(es) */
  /* creation of mutexI2c */
  mutexI2cHandle = osMutexNew(&mutexI2c_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of displayQueue */
  displayQueueHandle = osMessageQueueNew (8, sizeof(s_disp_msg_t), &displayQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of MeasureManagerT */
  MeasureManagerTHandle = osThreadNew(StartMeasureManagerTask, NULL, &MeasureManagerT_attributes);

  /* creation of DisplayTask */
  DisplayTaskHandle = osThreadNew(StartDisplayTask, NULL, &DisplayTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 64;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00F12981;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 7999;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 99;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : LD4_Pin */
  GPIO_InitStruct.Pin = LD4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD4_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartMeasureManagerTask */
/**
* @brief Function implementing the MeasureManagerT thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMeasureManagerTask */
void StartMeasureManagerTask(void *argument)
{
  /* USER CODE BEGIN StartMeasureManagerTask */
  s_step_counter_handle_t step_hnd;
  init_step_counter(&step_hnd);
  init_accelerometer();

  s_pulse_meter_handle_t pulse_hnd;
  init_pulse_meter(&pulse_hnd);
  init_ir_sensor();

  uint32_t last_steps = UINT32_MAX;  // Force sending the first values to the display
  uint32_t last_bpm = UINT32_MAX;
  GPIO_PinState btn_switch_prev = GPIO_PIN_SET;
  uint8_t btn_switch_lockout = 0;
  s_disp_msg_t msg;
  /* Infinite loop */
  for(;;)
  {
    // Sleep until TIM6 signals the next 10 ms tick (flag is cleared automatically)
    osThreadFlagsWait(FLAG_MEASURE_TICK, osFlagsWaitAny, osWaitForever);

    calc_step(&step_hnd);
    calc_bpm(&pulse_hnd);

    // Reset step_count if the button is pressed
    //@todo refactor
    if (HAL_GPIO_ReadPin(BTN_RESET_PORT, BTN_RESET_PIN) == GPIO_PIN_RESET)
    {
      set_step_count(&step_hnd, 0);
    }

    // Send values to the display only if they changed (timeout 0: never block the measurement)
    uint32_t steps = get_step_count(&step_hnd);
    if (steps != last_steps)
    {
      msg.type = DISP_MSG_STEPS;
      msg.value = steps;
      if (osMessageQueuePut(displayQueueHandle, &msg, 0, 0) == osOK) last_steps = steps; // retry next tick if full
    }

    uint32_t bpm = get_bpm(&pulse_hnd);
    if (bpm != last_bpm)
    {
      msg.type = DISP_MSG_BPM;
      msg.value = bpm;
      if (osMessageQueuePut(displayQueueHandle, &msg, 0, 0) == osOK) last_bpm = bpm;
    }

    // Display mode switch: react on the press edge only, then ignore the button for 200 ms (debounce)
    //@todo move to EXTI when the dedicated button is wired
    GPIO_PinState btn_switch_now = HAL_GPIO_ReadPin(BTN_SWITCH_PORT, BTN_SWITCH_PIN);
    if (btn_switch_lockout > 0)
    {
      btn_switch_lockout--;
    }
    else if ((btn_switch_prev == GPIO_PIN_SET) && (btn_switch_now == GPIO_PIN_RESET))
    {
      msg.type = DISP_MSG_MODE_NEXT;
      msg.value = 0;
      osMessageQueuePut(displayQueueHandle, &msg, 0, 0);
      btn_switch_lockout = BTN_LOCKOUT_TICKS;
    }
    btn_switch_prev = btn_switch_now;
  }
  /* USER CODE END StartMeasureManagerTask */
}

/* USER CODE BEGIN Header_StartDisplayTask */
/**
* @brief Function implementing the DisplayTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDisplayTask */
void StartDisplayTask(void *argument)
{
  /* USER CODE BEGIN StartDisplayTask */

  gme12864_init();
  display_graphics_init();
  e_disp_mode_t display_mode = DISP_MODE_PULSE_METER;
  uint32_t step_count = 0;  // Latest values received from the measure task
  uint8_t bpm = 0;
  s_disp_msg_t msg;
  /* Infinite loop */
  for(;;)
  {
    switch (display_mode)
    {
      case DISP_MODE_CLOCK:
        render_clock_frame();
        break;

      case DISP_MODE_STEP_COUNTER:
        render_step_counter_frame(step_count);
        break;

      case DISP_MODE_PULSE_METER:
        render_pulse_meter_frame(bpm);
        break;

      default:
        break;
    }

    // Sleep until a message arrives (or the refresh period elapses, e.g. for the clock),
    // then process every pending message before the next redraw
    uint32_t timeout = DISPLAY_REFRESH_MS;
    while (osMessageQueueGet(displayQueueHandle, &msg, NULL, timeout) == osOK)
    {
      switch (msg.type)
      {
        case DISP_MSG_STEPS:
          step_count = msg.value;
          break;

        case DISP_MSG_BPM:
          bpm = (uint8_t) msg.value;
          break;

        case DISP_MSG_MODE_NEXT:
          display_mode = (display_mode + 1) % DISP_MODE_NUMBER;
          break;

        default:
          break;
      }
      timeout = 0; // Only drain the rest of the queue, do not wait again
    }
  }
  /* USER CODE END StartDisplayTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
  if (htim->Instance == TIM6) {
    osThreadFlagsSet(MeasureManagerTHandle, FLAG_MEASURE_TICK);
  }
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
