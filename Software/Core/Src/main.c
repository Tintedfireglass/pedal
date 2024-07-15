/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADC1IN5 0
#define ADC1IN6 1
#define BRAKEPRESSURE 2
//#define ADC_VREF 0
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

CAN_HandleTypeDef hcan1;

DAC_HandleTypeDef hdac1;
DMA_HandleTypeDef hdma_dac_ch1;

TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/***************ADC VARIABLES***************/
uint32_t rawValues[2];//array to store values from the adc
uint16_t adcVREF;//variable for vrefint channel
uint32_t adcBuffer[3];//buffer for adcinput
uint32_t brakePressure;

/***************DAC VARIABLES***************/
uint32_t dac_val;//function to write value to the dac
uint32_t getDac;//debugging integer for dac

/***************COMMONDATA VARIABLES***************/
bmsData bmsDataObj = {0};
commonData commonDataObj = {0};

/***************CAN VARIABLES***************/
uint16_t data[8];

/***************ERROR CODES***************/
char* adcNcalib = "ADC calibration failure!\r\n";
char* potShortPedal = "Potentiometers have shorted!\r\n";
char* deviationCheckFail = "Deviation is above 10%!\r\n";
char* death = "STM die!\r\n";

/***************DATALOGGING***************/
char msg[100];//debugging array for uart

/***************FLAGS***************/
uint8_t runON = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CAN1_Init(void);
static void MX_TIM6_Init(void);
static void MX_DAC1_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */
uint32_t dacInput(uint16_t adcinput1, uint16_t adcinput2);

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	dac_val = dacInput(rawValues[0], rawValues[1]);

	if(brakePressure > 800){
		HAL_GPIO_WritePin(BrakeLight_GPIO_Port, BrakeLight_Pin, GPIO_PIN_SET);
	}else{
		HAL_GPIO_WritePin(BrakeLight_GPIO_Port, BrakeLight_Pin, GPIO_PIN_RESET);
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){
	rawValues[0] = adcBuffer[ADC1IN5];
	rawValues[1] = adcBuffer[ADC1IN6];
	brakePressure = adcBuffer[BRAKEPRESSURE];
}

void ErrorLedBlink(){
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
	HAL_Delay(500);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(RUNON_Pin){
		runON = 1;
	}
//	else if(B1_Pin){
//
//	}

	//for future interrupts
//	else{
//
//	}
}



/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_CAN1_Init();
  MX_TIM6_Init();
  MX_DAC1_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  uint32_t calibrationValue = HAL_ADCEx_Calibration_GetValue(&hadc1,ADC_SINGLE_ENDED);
  sprintf(msg, "%lu\r\n", calibrationValue);
  LOGS((uint8_t* )msg, strlen(msg));

  HAL_ADCEx_Calibration_SetValue(&hadc1, ADC_SINGLE_ENDED, calibrationValue);

  if(HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED)!=HAL_OK){
	  LOGS((uint8_t*)adcNcalib,strlen(adcNcalib));
	  ErrorLedBlink();
	  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  }

  HAL_CAN_Start(&hcan1);
  if(canNotification()!=0){

  }

  uint8_t test[] = {0x0f,0x93,0,0,0,0,0,0};

  HAL_ADC_Start_DMA(&hadc1, adcBuffer, 3);
  HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, &dac_val, 1, DAC_ALIGN_12B_R);
  HAL_TIM_Base_Start_IT(&htim6);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  canTransmit(test);
//	  sprintf(msg,"adc1 val: %hu\r\nadc2 val: %hu\r\ndac val: %hu\r\n",(uint16_t)rawValues[0],(uint16_t)rawValues[1],(uint16_t)HAL_DAC_GetValue(&hdac1, DAC_CHANNEL_1));
//	  LOGS((uint8_t*)msg,strlen(msg));
//
//	  sprintf(msg,"deviation: %hu\r\n",data[2]);
//	  LOGS((uint8_t*)msg,strlen(msg));
//
//	  sprintf(msg,"%hu %hu\r\n",data[0],data[1]);
//	  LOGS((uint8_t*)msg,strlen(msg));
//
//	  getDac = HAL_DAC_GetValue(&hdac1, DAC_CHANNEL_1);
//	  sprintf(msg,"dac:%lu\r\n",getDac);
//	  LOGS((uint8_t*)msg,strlen(msg));
//	  getDac = HAL_DAC_GetValue(&hdac1, DAC_CHANNEL_1);

//	  sprintf(msg,"99 %hu\n",(uint16_t)getDac);

//	  LOGS((uint8_t*)msg,strlen(msg));

	  if(runON==1){
		  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
		  runON = 0;
	  }else{
		  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
	  }

	  HAL_Delay(1000);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 20;
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
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T6_TRGO;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISINGFALLING;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */
	CAN_FilterTypeDef sFilterConfig;

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 10;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */
  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;

  if(HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK){
      /* Filter configuration Error */
      Error_Handler();
    }

  sFilterConfig.FilterBank = 1;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO1;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;

    if(HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK){
        /* Filter configuration Error */
        Error_Handler();
      }
  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */

  /** DAC Initialization
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */

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
  htim6.Init.Prescaler = 0;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 65535;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

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
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SD_O_GPIO_Port, SD_O_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, BUZZER_Pin|BrakeLight_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SD_O_Pin */
  GPIO_InitStruct.Pin = SD_O_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SD_O_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RUNON_Pin */
  GPIO_InitStruct.Pin = RUNON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(RUNON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BUZZER_Pin BrakeLight_Pin */
  GPIO_InitStruct.Pin = BUZZER_Pin|BrakeLight_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

//uint32_t dacInput(uint16_t adcinput1, uint16_t adcinput2, uint16_t vref){
//	static uint16_t counter = 0;
//	static uint32_t rawcheck1 = 0;
//	static uint32_t rawcheck2 = 0;
//
//	uint16_t vrefint_cal = *((uint16_t*)VREFINT_CAL_ADDR);
//
//	uint32_t out = 0;
//
//	uint16_t vrefInt;
//	if (vref == 0){
//		vrefInt = 4096;
//	}else{
//		vrefInt = vref;
//	}
//
//	uint32_t raw1 = (3000*(uint32_t)vrefint_cal*adcinput1)/(vrefInt*4096);
//	uint32_t raw2 = (9/8)*(3000*(uint32_t)vrefint_cal*adcinput2)/(vrefInt*4096);
//
//	uint32_t deviationCheck = (abs(raw1-raw2)/4095)*100;
//
//	data[2] = deviationCheck;
//
//	//check if both values are same for 1 second
//	if((raw1!=0) && (raw2!=0) && raw1 == raw2){
//		//check if raw1 is continuously the same
//		if((raw1 != rawcheck1) && (raw2 != rawcheck2)){
//			counter = 0;
//		}
//		//set static value to be compared later
//		rawcheck1 = raw1;
//		rawcheck2 = raw2;
//
//		counter++;
//
//		if(counter > 999){
//			counter = 0;
//			//compare to see if they are still the same
//			if(rawcheck1 == raw1 && rawcheck2 == raw2){
//				out = 0;
//				LOGS(potShortPedal,strlen(potShortPedal));
////				Error_Handler();
//			}
//		}
//	}
//	//check deviation
//	else if(deviationCheck>0.1){
//		out = 0;
////		LOGS(deviationCheckFail,strlen(deviationCheckFail));
////		Error_Handler();
//	}
//	//regular output function(average value)
//	else{
//		raw1 = (raw1)*4096/3300;
//		raw2 = (raw2)*4096/3300;
//		out = ((raw1+raw2)/2);
//	}
//
//	data[0] = raw1;
//	data[1] = raw2;
//
//	return out;
//}

uint32_t dacInput(uint16_t adcinput1, uint16_t adcinput2){
	static uint16_t counter = 0;
	static uint32_t rawcheck1 = 0;
	static uint32_t rawcheck2 = 0;

	uint32_t out = 0;

	uint32_t raw1 = 0.72*((1.208)*adcinput1)+748;
	uint32_t raw2 = adcinput2;

	data[0] = raw1;
	data[1] = raw2;

	uint8_t deviationCheck = ((abs((float) raw1 - (float) raw2) / 4095.00))* 100;

	data[2] = (uint16_t) deviationCheck;

	//check if both values are same for 1 second
	if ((raw1 != 0) && (raw2 != 0) && raw1 == raw2) {
		//check if raw1 is continuously the same
		if ((raw1 != rawcheck1) && (raw2 != rawcheck2)) {
			counter = 0;
		}
		//set static value to be compared later
		rawcheck1 = raw1;
		rawcheck2 = raw2;

		counter++;

		if (counter > 999) {
		//compare to see if they are still the same
			if (rawcheck1 == raw1 && rawcheck2 == raw2) {
				out = 0;
				//LOGS((uint8_t*)potShortPedal,strlen(potShortPedal));
				//Error_Handler();
			}
			counter = 0;
			}
		}
	//check deviation
	else if (deviationCheck > 10) {
		HAL_GPIO_WritePin(SD_O_GPIO_Port, SD_O_Pin, GPIO_PIN_SET);
		out = 0;
		//LOGS((uint8_t*)deviationCheckFail,strlen(deviationCheckFail));
		//Error_Handler();
	}

	//regular output function(average value)
	else {
		out = ((raw1 + raw2) / 2);
		uint16_t out_16 = out;

		commonDataObj.torqueHigh = (uint8_t)(out_16 & 0xFF00);
		commonDataObj.torqueLow = (uint8_t)(out_16 & 0x00FF);
	}
	return out;

}



/* USER CODE END 4 */

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
	  ErrorLedBlink();
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
