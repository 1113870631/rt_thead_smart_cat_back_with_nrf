/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-04-28     王浩       the first version
 */
#ifndef APPLICATIONS_PID_PID_H_
#define APPLICATIONS_PID_PID_H_
typedef struct
{
    float speed;
    float speed_per;

    float Err;
    float Err_per;
    float Err_Sum;
    float Err_change;
    int count;
    int count_per;
    int dir;
    int dir_per;
    float kp;
    float ki;
    float kd;
    float aim_speed;
}pid;
double pid_compute(pid *pid_test);
void pid_Setpeed(pid pid_test,float aim_speed);
#endif /* APPLICATIONS_PID_PID_H_ */
