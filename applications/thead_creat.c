#include "thead_creat.h"

#include <rtdevice.h>
#include <thead_creat.h>

#include <motrol.h>
#include <motrol_dir.h>
#include "pid.h"
#include "hard_encoder.h"

#define LOG_TAG "thead_creat.c"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>
/*命令缓冲区
 * [0] [0/1/-1] [speed][保留]
 * [0]:代表是速度控制命令
 * [0/1/-1] 0 停止  1 前进 -1 后退
 * 完整命令实例：  0 1 0 0
 *
 * [1] [pwm1] [pwm1] [speed]
 * [1]:代表是方向控制命令
 * [pwm1] 占空比的第一个数
 * [pwm2] 占空比的第二个数
 * [speed] 代表速度 1 代表10%的速度 2 20% 0 100%
 *完整命令实例：1 50 1
 * */

//移动命令缓冲区
char command_move_pool[4];
//方向命令缓冲区
char command_dir_pool[4];

//目标PID速度
int pid_aim_speed;
//PID目标速度最大速度
int  MAX_SPEED=16;
//信号量
rt_sem_t rx_sem,move_sem,dir_sem,pid_aimspeed_sem,pid_set_sem;

/***********************************************************************************************************************************/
/*线程1 入口 接收上层命令 传输到对应的命令缓冲区*/
static void uart_re_entry(void *parameter){

    char buffer[4];
    #define SAMPLE_UART_NAME "uart2" /* 串 口 设 备 名 称 */
    static rt_device_t serial; /* 串 口 设 备 句 柄 */
    rt_err_t ret,result;

/*接收回调函数 接收4个字节发送信号*/
    rt_err_t uart_input(rt_device_t dev, rt_size_t size) {
        if(size==4)
        {
            rt_sem_release(rx_sem);
        };
        return RT_EOK;
    }
    /*找到uart2*/
    serial = rt_device_find(SAMPLE_UART_NAME);
    /*打开 uart2*/
    ret = rt_device_open(serial, RT_DEVICE_FLAG_INT_RX|RT_DEVICE_FLAG_RDWR);//中断接收 读写打开
    if(ret<0)
    {
       rt_kprintf("open uart2 file.....\n");
    }
     rt_kprintf("open uart2 success...\n");
     /*创建接收信号量*/
     rx_sem = rt_sem_create("uart_rx_sem",0,RT_IPC_FLAG_FIFO);
     move_sem = rt_sem_create("move_command_sem",0,RT_IPC_FLAG_FIFO);
     dir_sem = rt_sem_create("dir_command_sem",0,RT_IPC_FLAG_FIFO);
     /*设置回调函数*/
     rt_device_set_rx_indicate(serial, uart_input);

     int i =0;
     while(1)
     {   /*等待串口接收到数据的信号量*/
         result = rt_sem_take(rx_sem, RT_WAITING_FOREVER);
         if (result != RT_EOK) {
         rt_kprintf("take a dynamic re_sem semaphore, failed.\n");
         rt_sem_delete(rx_sem);
         return; }
         else//接收到串口收到消息的信号量   接收命令 判断命令 发送缓冲区
         {
             char * tmp;
             /*接收4个字节的信息*/
             rt_device_read(serial,0,buffer,4);
             /*判断命令类型*/
             if(buffer[0]=='0')//判断命令类型
                 {tmp =command_move_pool;}
             else if (buffer[0]=='1')
                 {
                     tmp =command_dir_pool;
                 }
             else {//错误的命令类型
                 tmp=NULL;
            }

             if(tmp!=NULL){//命令格式正确 tmp 不是null
                      /*将四个字节转移到命令缓冲区*/
                 for(i=0;i<4;i++){
                 tmp[i]=buffer[i];
                 }
             }
             else{
                 rt_kprintf("错误的命令类型.....\n");
             }


             /*接收完一个命令复位i*/
             if(i==4){
                 /*根据命令类型发送不同的信号量给控制进程*/
                 if(tmp==command_move_pool){
                     rt_sem_release(move_sem);
                 }
                 else if (tmp==command_dir_pool)
                 {
                     rt_sem_release(dir_sem);
                 }
                i=0;//复位i
             }
         }
    }
};



/*命令缓冲区
 * [0] [0/1/2] [speed_bit1][speed_bit2]
 * [0]:代表是速度控制命令
 * [0/1/2] 0 停止  1 前进 2 后退
 * 完整命令实例：  0 1 5 1
 * */
/*线程2 入口  解析move命令缓冲区命令执行命令*/
static void total_con_move_entry(void *parameter){
    rt_err_t ret;
    pid_aimspeed_sem = rt_sem_create("pid_aimspeed_sem",1,RT_IPC_FLAG_FIFO);
              while(1)
              {
                 /*等待移动命令缓冲区信号量*/
                 ret = rt_sem_take(move_sem, RT_WAITING_FOREVER);
                if (ret != RT_EOK) {
                rt_kprintf("take a dynamic move semaphore, failed.\n");
                rt_sem_delete(move_sem);
                return; }
                else//解析命令执行动作
                {   //获取pid目标速度信号量
                    ret = rt_sem_take(pid_aimspeed_sem, RT_WAITING_FOREVER);
                    if (ret != RT_EOK) {
                    rt_kprintf("take a dynamic pid_aimspeed_sem semaphore, failed.\n");
                    rt_sem_delete(pid_aimspeed_sem);
                    return; }
                    else{//获取到目标速度 信号 设置PID 目标速度
                        if(command_move_pool[1]=='1'){
                            //前进命令
                            //解析速度
                            int tmp1=0,tmp2=0;
                            int speed_tmp3=0;
                            tmp1=command_move_pool[2]-'0';
                            tmp2=command_move_pool[3]-'0';
                            speed_tmp3=tmp1*10+tmp2;
                            //rt_kprintf("%d\n",speed_tmp3);
                            speed_tmp3=speed_tmp3*MAX_SPEED/100;//最大速度16r/s ag: speed_tmp3=80, 80/100*MAX_SPEED
                           // rt_kprintf("%d\n",speed_tmp3);
                             //设置目标pid速度
                            pid_aim_speed=speed_tmp3;
                        };
                        if(command_move_pool[1]=='0'){
                            //停止
                            pid_aim_speed=0;
                        };
                        if(command_move_pool[1]=='2'){
                            //后退
                            int tmp1=0,tmp2=0;
                            int speed_tmp3=0;
                            tmp1=command_move_pool[2]-'0';
                            tmp2=command_move_pool[3]-'0';
                            speed_tmp3=tmp1*10+tmp2;
                            //rt_kprintf("%d\n",speed_tmp3);

                            speed_tmp3=speed_tmp3*MAX_SPEED/100;
                            //rt_kprintf("%d\n",speed_tmp3);
                            //设置目标pid速度
                            pid_aim_speed=-speed_tmp3;
                        };
                      }
                     rt_sem_release(pid_aimspeed_sem); //目标速度设置完成 释放信号量    pid_aimspeed_sem
                     //rt_sem_release(move_sem);//释放命令缓冲区信号量
                  }
                rt_thread_mdelay(3);
               }
}
/*
 *[1] [pwm1] [pwm1] [speed]
 * [1]:代表是方向控制命令
 * [pwm1] 占空比的第一个数
 * [pwm2] 占空比的第二个数
 * [speed] 代表速度 1 代表10%的速度 2 20% 0 100%
 *完整命令实例：1 50 1
    线程3 入口  解析dir命令缓冲区命令执行命令*/
static void total_con_dir_entry(void *parameter){
    rt_err_t ret;
    // per 收到的角度百分比  per_per上一次的角度百分比
    double per,per_per=50;
    double dir_err_num=0;
    rt_device_t  pwm_dev;
    /*打开pwm3*/
    pwm_dev = rt_device_find("pwm3");
    /*使能pwm3*/
    rt_pwm_enable((struct rt_device_pwm *)pwm_dev, 1);
    /*初始化方向*/
    dir_init(pwm_dev);
       while(1)
       {
           /*等待信号量*/
            ret = rt_sem_take(dir_sem, RT_WAITING_FOREVER);
                   if (ret != RT_EOK) {
                   rt_kprintf("take a dynamic dir semaphore, failed.\n");
                   rt_sem_delete(dir_sem);
                   return; }
                   else//解析命令执行动作
                   {
                     /*获取角度*/
                       int shi,ge;
                       shi=command_dir_pool[1]-'0';
                       ge =command_dir_pool[2]-'0';

                     per=(shi*10+ge)+dir_err_num;
                     if(per==per_per){}//此次与上次方向相同
                     else{//此次与上次方向不同
                         ch_dir(per, 100, pwm_dev);
                         per_per=per;
                     }
                   }
                   rt_sem_release(dir_sem);
                   rt_thread_mdelay(3);
       }
}



/*线程6  pid 定时器 入口*/
rt_device_t  pwm1_dev,pwm2_dev;
pid pid_speed_r,pid_speed_l;
static void timeout1(void *parameter);
static void pid_timer( void *parameter){

     //初始化电机引脚
     set_motrol_pin();
     /*打开pwm1 pwm2*/
     pwm1_dev = rt_device_find("pwm1");
     pwm2_dev = rt_device_find("pwm2");
     /*使能pwm1 pwm2*/
     rt_pwm_enable((struct rt_device_pwm *)pwm1_dev, 1);
     rt_pwm_enable((struct rt_device_pwm *)pwm2_dev, 1);

     //初始化 硬件编码器
     Encoder_Init_TIM2();
     Encoder_Init_TIM3();
    //初始化PID参数
    pid_speed_l.kp=0.08;
    pid_speed_l.ki=0.01;
    pid_speed_l.kd=0;
    pid_speed_r.kp=0.08;
    pid_speed_r.ki=0.01;
    pid_speed_r.kd=0;
    /* 创 建 PID 定 时 器 1 周 期 定 时 器 */
    static rt_timer_t timer1;
    timer1 = rt_timer_create("pid_timer", timeout1,
    RT_NULL, 10,
    RT_TIMER_FLAG_PERIODIC|RT_TIMER_FLAG_SOFT_TIMER
    );
    /* 启 动 定 时 器 1 */
    if (timer1 != RT_NULL) rt_timer_start(timer1);
    else rt_kprintf("启动PID定时器失败!\n");
};


static void timeout1(void *parameter) {
    //rt_kprintf("periodic timer is timeout %d\n");
   rt_err_t  ret = rt_sem_take(pid_aimspeed_sem,100);//获取目标速度信号量
   if (ret == -RT_ERROR) {//获取信号量出错
                   rt_kprintf("pid inerrupt take a dynamic dir semaphore, failed.\n");
                   rt_sem_delete(pid_aimspeed_sem);
                   return; }
    else if(ret==RT_EOK){//得到信号量
        //获得目标速度 进行PID 进行PID运算
        Encoder_Get_inf(0,&pid_speed_r.count);
        Encoder_Get_inf(1,&pid_speed_l.count);
        pid_speed_r.aim_speed=pid_aim_speed;
        pid_speed_l.aim_speed=pid_aim_speed;
        //释放目标速度信号量
         rt_sem_release(pid_aimspeed_sem);
        //进行PID运算
       float Pid_out1 =pid_compute(&pid_speed_l);
       float Pid_out2 =pid_compute(&pid_speed_r);
       //设置PWM占空比
        motrol_con(MOTRO_L,Pid_out1,Pid_out1,pwm1_dev);
        motrol_con(MOTRO_R,Pid_out2,Pid_out2,pwm2_dev);
       //打印信息
       rt_kprintf("%f,%f,%f,%f\n",pid_speed_l.aim_speed,pid_speed_l.speed,pid_speed_r.aim_speed,pid_speed_r.speed);

    }
    else if (ret==-RT_ETIMEOUT){ //没有在规定时间内得到目标速度 保持原速度
        //更新信息
        Encoder_Get_inf(0,&pid_speed_r.count);
        Encoder_Get_inf(1,&pid_speed_l.count);}
}

static void set_pid(int argc, char**argv){
    if (argc < 2)
    {
     rt_kprintf("Please input r/l kp ki kd \n");
    return; }

    int  tmp = atoi(argv[1]);
    switch(tmp){
    case 1:
        pid_speed_l.kp=atof(argv[2]);
        pid_speed_l.ki=atof(argv[3]);
        pid_speed_l.kd=atof(argv[4]);
        rt_kprintf("kp_l:%f \n ki_l:%f \n kd_l:%f\n",pid_speed_l.kp,pid_speed_l.ki,pid_speed_l.kd);
        break;
    case 2:
        pid_speed_r.kp=atof(argv[2]);
        pid_speed_r.ki=atof(argv[3]);
        pid_speed_r.kd=atof(argv[4]);
        rt_kprintf("kp_r:%f \n ki_r:%f \n kd_r:%f \n",pid_speed_r.kp,pid_speed_r.ki,pid_speed_r.kd);
        break;
    default:
        rt_kprintf("error!\n");
        break;
    }



}
MSH_CMD_EXPORT(set_pid, set_pid:r/l kp ki kd);


/***********************************************************************************************************************************/
static int thead1(void){
    rt_thread_t tid1;
    /* 创 建 线 程 1， 名 称 是 uart2_re， 入 口 是 uart2_re*/
    tid1 = rt_thread_create("uart2_re",
    uart_re_entry, RT_NULL,
    THREAD_STACK_SIZE,
    THREAD_PRIORITY, THREAD_TIMESLICE);
    /* 如 果 获 得 线 程 控 制 块， 启 动 这 个 线 程 */
    if (tid1 != RT_NULL)
    rt_thread_startup(tid1);
    rt_kprintf("thead1 ok\n");
    return 0;
}
static int thead2(void){
    rt_thread_t tid2;
    /* 创 建 线 程 2， 名 称 是 total_con， 入 口 是 total_con_entry*/
    tid2 = rt_thread_create("con_move",
    total_con_move_entry, RT_NULL,
    THREAD_STACK_SIZE,
    26, THREAD_TIMESLICE);
    /* 如 果 获 得 线 程 控 制 块， 启 动 这 个 线 程 */
    if (tid2 != RT_NULL)
    rt_thread_startup(tid2);\
    rt_kprintf("thead2 ok\n");
    return 0;
}
static int thead3(void){
    rt_thread_t tid3;
    /* 创 建 线 程 3， 名 称 是 con_dir， 入 口 是 con_dir*/
    tid3 = rt_thread_create("con_dir",
    total_con_dir_entry, RT_NULL,
    THREAD_STACK_SIZE,
    26, THREAD_TIMESLICE);
    /* 如 果 获 得 线 程 控 制 块， 启 动 这 个 线 程 */
    if (tid3 != RT_NULL)
    rt_thread_startup(tid3);\
    rt_kprintf("thead3 ok\n");
    return 0;
}

static int thead6(void){
    rt_thread_t tid6;
    /* 创 建 线 程 5， 名 称 是 tid5， 入 口 是 hard_encoder*/
    tid6 = rt_thread_create("pid_timer",
    pid_timer, RT_NULL,
    THREAD_STACK_SIZE,
    26, THREAD_TIMESLICE);
    /* 如 果 获 得 线 程 控 制 块， 启 动 这 个 线 程 */
    if (tid6 != RT_NULL)
    rt_thread_startup(tid6);\
    rt_kprintf("thead6 ok\n");
    return 0;
}


/***********************************************************************************************************************************/
int thead_creat(void){
    thead1();//启动线程1 uart 命令接收与转移到缓冲区
     // nrf命令接收 转移到缓冲区
    thead2();//启动线程2 move 缓冲区 命令解析与执行
    thead3();//启动线程3 dir  缓冲区命令解析与执行
    thead6();//启动PID定时器进程
    return 0;
};
