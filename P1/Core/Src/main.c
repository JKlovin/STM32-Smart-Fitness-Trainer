/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "pwr.h"
#include "force.h"
#include "mpu6050.h"
#include "inv_mpu.h"
#include "oled.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint8_t Res=0;
uint8_t Key_flag=0;
uint8_t BLE_flag=0;
uint32_t LL_value = 0;
uint8_t Page=1;
int16_t Sleep_flag=1;
float Update_value = 0;
float DC_value = 0;
int oled_value=0;
extern uint32_t First_value;
extern int Strength_value;
extern unsigned long current_value;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
    uint8_t ret;
    float pit, rol, yaw;
	
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
  MX_USART3_UART_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

/*串口*/
//	HAL_UART_Receive_IT(&huart3,&Res,1);
//	UsartPrintf(&huart3,"P1 TEST\r\n");
	
/*外设电源开关 HX711 蓝牙 OLED*/

	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET);//1开0关

/*ADC 电量检测*/
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_RESET);//0开1关

/*定时器  250ms*/
	HAL_TIM_Base_Start_IT(&htim3);
	
/*HX711*/
	Hx711_run_set();
	
/*OLED*/
	OLED_Init();
	Page=1;
	OLED_ShowString(1, 1, "Dev P1 TEST");


/*MPU6050*/
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);
	ret = atk_ms6050_init();
  if (ret != 0)
  {
		UsartPrintf(&huart3,"ATK-MS6050 init failed!\r\n");
		OLED_ShowString(2, 1, "6050 init error");
		while (1){};
  }else	
	{
		UsartPrintf(&huart3,"ATK-MS6050 init OK!\r\n");
		OLED_ShowString(2, 1, "6050 init OK");
	}

  /* 初始化ATK-MS6050 DMP */
  ret = atk_ms6050_dmp_init();
  if (ret != 0)
  {
		UsartPrintf(&huart3,"ATK-MS6050 DMP init failed!\r\n");
		OLED_ShowString(3, 1, "DMP  init error");
//		while (1){};
  }else	
	{
		UsartPrintf(&huart3,"ATK-MS6050 DMP init OK!\r\n");
		OLED_ShowString(3, 1, "DMP  init OK");
	}

	delay_ms(1000);
	OLED_Clear();
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {	
		Key_read();
		if(Key_flag==1){
			Page++;
			Sleep_flag=0;
			delay_ms(100);
			Hx711_run_set();
			OLED_Clear();
			if(Page>=4)
				Page=1;
		}else if(Key_flag==2){
			Page--;
			Sleep_flag=0;
			delay_ms(100);
			Hx711_run_set();
			if(Page<=0)
				Page=3;
			OLED_Clear();
		}
		
		Sleep_flag++;
		if(Sleep_flag>=2000)
		{
			OLED_Clear();
			Page=9;
		}
//		UsartPrintf(&huart3,"Sleep_flag:%d\r\n",Sleep_flag);
		
		Key_flag=0;
		BLE_flag=HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2);
		
			if(BLE_flag==1){
					if(Page==1){
						UsartPrintf(&huart3,"Page:    %d\r\n",Page);
						if(ret>=80) ret=0;
						OLED_ShowString(1, 1, "Dev P1 Work");
						OLED_ShowString(2, 1, "Power (%):");
						DC_value=ADC_value_get()*200.0/4.2;
						OLED_ShowFNum(2,10,DC_value, 3, 1);
						if(DC_value<70) 
							OLED_ShowString(3, 1, "Power no enough");
						delay_ms(200); 
					}else if(Page==2){
						LL_value=Get_Strength()+100;
						Update_value=LL_value/50.0;
						LL_value=(int)Update_value;
						Update_value=(Update_value-1783.3)/5.62;
						UsartPrintf(&huart3,"val:%.2f\r\n",Update_value); 
						OLED_ShowString(1, 1, "Val (N):");
						OLED_ShowFNum(1,10, (float)(int)LL_value, 5, 1);
						OLED_ShowProgressBar((int)LL_value/100);
						delay_ms(100); 
					}else if(Page==3){ 
						UsartPrintf(&huart3,"Page:    %d\r\n",Page);
						UsartPrintf(&huart3,"--------------------\r\n");
						ret  = atk_ms6050_dmp_get_data(&pit, &rol, &yaw);							
						if (ret == 0)
						{
							OLED_Clear();
							if((rol>115)|(rol<98))
							{
								UsartPrintf(&huart3,"姿势有误，请纠正\r\n",rol);
								OLED_ShowString(1, 1, "ERROR  ERROR  ERROR");
								OLED_ShowString(2, 1, "ERROR  ERROR  ERROR");
								OLED_ShowString(3, 1, "ERROR  ERROR  ERROR");
								OLED_ShowString(4, 1, "ERROR  ERROR  ERROR");
							}else
							{
								UsartPrintf(&huart3,"Rol:     %.2f\r\n",rol);
								OLED_ShowString(2, 1, "Rol:");
								OLED_ShowFNum(2,6,rol, 4, 1);
							}
						}
						delay_ms(100); 
					}else if(Page==9){
						OLED_ShowString(1, 1, "DEV Sleep");
						delay_ms(200); 
					}
				}

		
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
