/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-05-09     王浩       the first version
 */
#include "motrol.h"
#include "hard_encoder.h"

#include <rtdevice.h>
#include <rtthread.h>
#include <board.h>
#include <drv_common.h>

/*编码器初始化
 *
 * */
void  Encoder_Init_TIM2(void){

    TIM_HandleTypeDef htim2;

      /* USER CODE BEGIN TIM2_Init 0 */

      /* USER CODE END TIM2_Init 0 */

      TIM_Encoder_InitTypeDef sConfig = {0};
      TIM_MasterConfigTypeDef sMasterConfig = {0};

      /* USER CODE BEGIN TIM2_Init 1 */

      /* USER CODE END TIM2_Init 1 */
      htim2.Instance = TIM2;
      htim2.Init.Prescaler = 0;
      htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
      htim2.Init.Period = 65535;
      htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
      htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
      sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
      sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
      sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
      sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
      sConfig.IC1Filter = 0;
      sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
      sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
      sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
      sConfig.IC2Filter = 0;
      if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK)
      {
        Error_Handler();
      }
      sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
      sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
      if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
      {
        Error_Handler();
      }
      /* USER CODE BEGIN TIM2_Init 2 */

      /* USER CODE END TIM2_Init 2 */

    HAL_StatusTypeDef ret=HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
    if(ret==HAL_OK)
        rt_kprintf("encoder TIM2 start ok!\n");
    else
        rt_kprintf("encoder TIM2 start fail!\n");


};

void  Encoder_Init_TIM3(void){
    TIM_HandleTypeDef htim3;

    TIM_Encoder_InitTypeDef sConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

     /* USER CODE BEGIN TIM3_Init 1 */

     /* USER CODE END TIM3_Init 1 */
     htim3.Instance = TIM3;
     htim3.Init.Prescaler = 0;
     htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
     htim3.Init.Period = 65535;
     htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
     htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
     sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
     sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
     sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
     sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
     sConfig.IC1Filter = 0;
     sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
     sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
     sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
     sConfig.IC2Filter = 0;
     if (HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK)
     {
       Error_Handler();
     }
     sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
     sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
     if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
     {
       Error_Handler();
     }

     HAL_StatusTypeDef ret=HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
     if(ret==HAL_OK)
         rt_kprintf("encoder TIM3 start ok!\n");
     else
         rt_kprintf("encoder TIM3 start fail!\n");

};

void Encoder_Get_inf(int pos,int* count)
{
   if(pos==0){
       if(TIM3->CR1&TIM_CR1_DIR){
           *count=-(int)TIM3->CNT;
       }
       else{
           *count=(int)TIM3->CNT;
       }
   }
   if(pos==1){
       if(TIM2->CR1&TIM_CR1_DIR){
           *count=-(int)TIM2->CNT;
       }
       else{
           *count=(int)TIM2->CNT;
       }
   }
}
