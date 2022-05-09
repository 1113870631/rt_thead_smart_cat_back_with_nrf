/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-05-09     王浩       the first version
 */
#ifndef DRIVERS_MY_DRV_HARD_ENCODER_HARD_ENCODER_H_
#define DRIVERS_MY_DRV_HARD_ENCODER_HARD_ENCODER_H_

#include <rtdevice.h>
#include <rtthread.h>
#include <board.h>
#include <drv_common.h>


void Encoder_Get_inf(int pos,int* count);
void Encoder_Init_TIM2(void);
void Encoder_Init_TIM3(void);

#endif /* DRIVERS_MY_DRV_HARD_ENCODER_HARD_ENCODER_H_ */
