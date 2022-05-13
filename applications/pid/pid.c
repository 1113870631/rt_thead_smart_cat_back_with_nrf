#include "pid.h"
#include "hard_encoder.h"
void pid_Setpeed(pid pid_test,float aim_speed){
    pid_test.aim_speed=aim_speed;
}
double pid_compute( pid *pid_test){
    float out=0;
    //计算速度
    if(pid_test->count<0&&pid_test->count_per<0){
        pid_test->speed=(float)(pid_test->count-pid_test->count_per)*100/216;
    }
    else if(pid_test->count>0&&pid_test->count_per>0) {
        pid_test->speed=-(float)(pid_test->count-pid_test->count_per)*100/216;
    }
    if(pid_test->speed>20||pid_test->speed<-20)
        pid_test->speed=pid_test->speed_per;

    //rt_kprintf(" speed %.2f\n",pid_test->speed);
    pid_test->count_per=pid_test->count;
    //PID运算
    /*比例*/
    pid_test->Err=pid_test->speed-pid_test->aim_speed;
    //rt_kprintf("err %f\n",pid_test->Err);

    /*积分*/
    pid_test->Err_Sum+=pid_test->Err;
    //rt_kprintf("err_sum %f\n",pid_test->Err_Sum);

    /*微分*/
    pid_test->Err_change=pid_test->Err-pid_test->Err_per;
    pid_test->Err_per=pid_test->Err;

    out=-(pid_test->kp*pid_test->Err+pid_test->ki*pid_test->Err_Sum+pid_test->kd*pid_test->Err_change);
    //rt_kprintf("out %.2f\n\n",out);

    //限幅

    return out;
};
