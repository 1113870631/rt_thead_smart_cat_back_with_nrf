/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-04     111       the first version
 */

#include <motrol_dir.h>
#include <rtdevice.h>
#include <rtthread.h>
#include <thead_creat.h>
#include <motrol.h>
/*
 * rt_err_t rt_pwm_set(struct rt_device_pwm *device,
int channel,
rt_uint32_t period,
rt_uint32_t pulse);
 * */


/*设置方向 设置pwm占空比
 * @per 角度值
 * @speed 速度
 * @device pwm设备
 *
 *0.5 ms 0度
 *1.5 ms 90度
 *2.5 ms 180度
 *
 *500000~1500000~2500000
 * */
int ch_dir(double per ,int speed,void* device){

    int period=20000000;
    int goal;
    /*500000~1500000~2500000
     * */

    /*per占总角度的比例
     * 0%   0
     * 50%  90
     * 100% 180
     * 2500000-500000=2000000*/
    goal=500000+(per/100)*2000000;
    rt_pwm_set(device,1,period, goal);
    return 0;
};


/*方向复位*/
int dir_init(void* device)
{
    int period=20000000,pulse=1500000;
    rt_pwm_set(device,1,period, pulse);
    return 0;
};
/*获取方向*/
int get_dir(void)
{


    return 0;
};
