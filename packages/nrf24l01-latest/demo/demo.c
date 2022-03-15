#include <nrf24l01.h>
#include <rtconfig.h>
#include <rtdevice.h>
#include "board.h"
#include "drv_common.h"
#if (PKG_NRF24L01_DEMO_CE_PIN < 0)
#error Please specify a valid pin
#endif

#ifdef PKG_NRF24L01_DEMO_ROLE_PTX
    #define NRF24_DEMO_ROLE     ROLE_PTX
    #define NRF24_DEMO_SEND_INTERVAL        PKG_NRF24L01_DEMO_INTERVAL_SEND
#else
    #define NRF24_DEMO_ROLE     ROLE_PRX
#endif

#define NRF24_DEMO_SPI_DEV_NAME         PKG_NRF24L01_DEMO_SPI_DEV_NAME
#define NRF24_DEMO_CE_PIN               PKG_NRF24L01_DEMO_CE_PIN
#define NRF24_DEMO_IRQ_PIN              PKG_NRF24L01_DEMO_IRQ_PIN


extern rt_sem_t move_sem,dir_sem;
extern char command_move_pool[4];
extern char command_dir_pool[4];
const static char *ROLE_TABLE[] = {"PTX", "PRX"};

static void rx_ind(nrf24_t nrf24, uint8_t *data, uint8_t len, int pipe)
{
    /*! Don't need to care the pipe if the role is ROLE_PTX */
    //读取数据  判断命令类型   转移数据 发送信号量（遥控方发送完整8字的信号）
    //判断命令类型
    char *tmp1 ,*tmp2;
    if(data[0]=='1'||data[4]=='0')//判断命令类型
       {
           tmp1 =command_dir_pool;
           tmp2 =command_move_pool;
       }
    else {//错误的命令类型
           tmp1=NULL;
           tmp2=NULL;
         }

      if(tmp1!=NULL||tmp2!=NULL){//命令格式正确 tmp 不是null
          /*将四个字节转移到命令缓冲区*/
         for(int i=0;i<4;i++){
            tmp1[i]=data[i];
            tmp2[i]=data[i+4];
          }
        }
     else{
          rt_kprintf("错误的命令类型.....\n");
          }

    rt_sem_release(move_sem);
    rt_sem_release(dir_sem);
    rt_kprintf("(p%d): ", pipe);
    rt_kprintf((char *)data);
    rt_kprintf("\n");
}

static void tx_done(nrf24_t nrf24, int pipe)
{

}

const static struct nrf24_callback _cb = {
    .rx_ind = rx_ind,
    .tx_done = tx_done,
};

static void thread_entry(void *param)
{
    nrf24_t nrf24;

    rt_kprintf("[nrf24/demo] Version:%s\n", PKG_NRF24L01_VERSION);

    nrf24 = nrf24_default_create(NRF24_DEMO_SPI_DEV_NAME, NRF24_DEMO_CE_PIN, NRF24_DEMO_IRQ_PIN, &_cb, NRF24_DEMO_ROLE);

    if (nrf24 == RT_NULL)
    {
        rt_kprintf("\n[nrf24/demo] Failed to create nrf24. stop!\n");
        for(;;) rt_thread_mdelay(10000);
    }
    else
    {
        rt_kprintf("[nrf24/demo] running.");
    }

    //nrf24_send_data(nrf24, "Hi\n", 3, NRF24_DEFAULT_PIPE);

    while (1)
    {
        nrf24_run(nrf24);

        if(!nrf24->flags.using_irq)
            rt_thread_mdelay(10);
    }
    
}

static int nrf24l01_sample_init(void)
{
    rt_thread_t thread;
    rt_thread_delay(5000);
    rt_hw_spi_device_attach("spi2",NRF24_DEMO_SPI_DEV_NAME,GPIOG, GPIO_PIN_7);
    rt_kprintf("delay ok \n");
    thread = rt_thread_create("nrfDemo", thread_entry, RT_NULL, 1024, 26, 20);
    rt_thread_startup(thread);

    return RT_EOK;
}

INIT_APP_EXPORT(nrf24l01_sample_init);
