#include "iic.h"
#include "tim.h"

/**
 * @brief       IIC接口延时函数，用于控制IIC读写速度
 * @param       无
 * @retval      无
 */
static inline void atk_ms6050_iic_delay(void)
{
    delay_us(2);
}


/**
 * @brief       产生IIC起始信号
 * @param       无
 * @retval      无
 */
void atk_ms6050_iic_start(void)
{
    ATK_MS6050_IIC_SDA(1);
    ATK_MS6050_IIC_SCL(1);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SDA(0);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SCL(0);
    atk_ms6050_iic_delay();
}

/**
 * @brief       产生IIC停止信号
 * @param       无
 * @retval      无
 */
void atk_ms6050_iic_stop(void)
{
    ATK_MS6050_IIC_SDA(0);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SCL(1);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SDA(1);
    atk_ms6050_iic_delay();
}

/**
 * @brief       等待IIC应答信号
 * @param       无
 * @retval      0: 应答信号接收成功
 *              1: 应答信号接收失败
 */
uint8_t atk_ms6050_iic_wait_ack(void)
{
    uint8_t waittime = 0;
    uint8_t rack = 0;
    
    ATK_MS6050_IIC_SDA(1);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SCL(1);
    atk_ms6050_iic_delay();
    
    while (ATK_MS6050_IIC_READ_SDA())
    {
        waittime++;
        
        if (waittime > 250)
        {
            atk_ms6050_iic_stop();
            rack = 1;
            break;
        }
    }
    
    ATK_MS6050_IIC_SCL(0);
    atk_ms6050_iic_delay();
    
    return rack;
}

/**
 * @brief       产生ACK应答信号
 * @param       无
 * @retval      无
 */
void atk_ms6050_iic_ack(void)
{
    ATK_MS6050_IIC_SDA(0);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SCL(1);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SCL(0);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SDA(1);
    atk_ms6050_iic_delay();
}

/**
 * @brief       不产生ACK应答信号
 * @param       无
 * @retval      无
 */
void atk_ms6050_iic_nack(void)
{
    ATK_MS6050_IIC_SDA(1);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SCL(1);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SCL(0);
    atk_ms6050_iic_delay();
}

/**
 * @brief       IIC发送一个字节
 * @param       dat: 要发送的数据
 * @retval      无
 */
void atk_ms6050_iic_send_byte(uint8_t dat)
{
    uint8_t t;
    
    for (t=0; t<8; t++)
    {
        ATK_MS6050_IIC_SDA((dat & 0x80) >> 7);
        atk_ms6050_iic_delay();
        ATK_MS6050_IIC_SCL(1);
        atk_ms6050_iic_delay();
        ATK_MS6050_IIC_SCL(0);
        dat <<= 1;
    }
    ATK_MS6050_IIC_SDA(1);
}

/**
 * @brief       IIC接收一个字节
 * @param       ack: ack=1时，发送ack; ack=0时，发送nack
 * @retval      接收到的数据
 */
uint8_t atk_ms6050_iic_read_byte(uint8_t ack)
{
    uint8_t i;
    uint8_t dat = 0;
    
    for (i = 0; i < 8; i++ )
    {
        dat <<= 1;
        ATK_MS6050_IIC_SCL(1);
        atk_ms6050_iic_delay();
        
        if (ATK_MS6050_IIC_READ_SDA())
        {
            dat++;
        }
        
        ATK_MS6050_IIC_SCL(0);
        atk_ms6050_iic_delay();
    }
    
    if (ack == 0)
    {
        atk_ms6050_iic_nack();
    }
    else
    {
        atk_ms6050_iic_ack();
    }

    return dat;
}
