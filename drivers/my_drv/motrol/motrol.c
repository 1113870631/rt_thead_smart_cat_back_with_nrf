/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-03     111       the first version
 */
#include "motrol.h"
#include <rtdevice.h>
#include <drv_common.h>
extern int MAX_SPEED;
/*获取控制引脚编号*/
#define IN1_PIN GET_PIN(B,1)
#define IN2_PIN GET_PIN(B,2)
#define IN3_PIN GET_PIN(C,7)
#define IN4_PIN GET_PIN(C,8)
/*获取pwm引脚编号*/
#define PWM1_PIN GET_PIN(F,9)
#define PWM2_PIN GET_PIN(F,9)

/*电机控制引脚初识化*/
void set_motrol_pin(void){
    /* 设置为输出模式*/
    rt_pin_mode( IN1_PIN,  PIN_MODE_OUTPUT);
    rt_pin_mode( IN2_PIN,  PIN_MODE_OUTPUT);
    rt_pin_mode( IN3_PIN,  PIN_MODE_OUTPUT);
    rt_pin_mode( IN4_PIN,  PIN_MODE_OUTPUT);
    /*设置默认输出为低电平*/
    rt_pin_write( IN1_PIN,  PIN_LOW);
    rt_pin_write( IN2_PIN,  PIN_LOW);
    rt_pin_write( IN3_PIN,  PIN_LOW);
    rt_pin_write( IN4_PIN,  PIN_LOW);
}


int motrol_con(int pos,float dir,float speed,void* device){

    switch (pos) {
        case 1://电机1
            if(dir>0){//设置前进引脚
                 rt_pin_write( IN1_PIN,  PIN_LOW);
                 rt_pin_write( IN2_PIN,  PIN_HIGH);
             }
            if(dir<0){ //设置后退引脚
                 rt_pin_write( IN1_PIN,  PIN_HIGH);
                 rt_pin_write( IN2_PIN,  PIN_LOW);
            }
            if(dir==0){//设置停止引脚
                rt_pin_write( IN1_PIN,  PIN_LOW);
                rt_pin_write( IN2_PIN,  PIN_LOW);
             }
            break;
        case 2:
            //电机2
            if(dir>0){//设置前进引脚
                 rt_pin_write( IN3_PIN,  PIN_LOW);
                 rt_pin_write( IN4_PIN,  PIN_HIGH);
             }
            if(dir<0){//设置后退引脚
                 rt_pin_write( IN3_PIN,  PIN_HIGH);
                 rt_pin_write( IN4_PIN,  PIN_LOW);
            }
            if(dir==0){//设置停止引脚
                rt_pin_write( IN3_PIN,  PIN_LOW);
                rt_pin_write( IN4_PIN,  PIN_LOW);
             }
            break;
        default:
            break;
    }
    //处理pwm占空比
     int period=20000000;
     int goal;
     if(speed<0)
         speed=-speed;
     if(speed>1)
         speed=1;
     rt_err_t ret=rt_pwm_set((struct rt_device_pwm *)device,1,period, speed*20000000);
     if(ret<0)
         rt_kprintf("set_pwm err!!\n");
     return ret;
};

