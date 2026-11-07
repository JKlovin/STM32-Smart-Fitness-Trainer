#include "force.h"
#include "tim.h"
#include "usart.h"


#define HX711_SCK_GPIO_PORT GPIOB 
#define HX711_SCK_PIN GPIO_PIN_3
#define HX711_DOUT_GPIO_PORT GPIOA
#define HX711_DOUT_PIN GPIO_PIN_15

#define SAMPLES 100 
#define NOISE_THRESHOLD 10

uint32_t First_value = 0;
int Strength_value = 0;
unsigned long current_value = 0;
int strength_history[SAMPLES] = {0};
uint8_t filter_index = 0;

uint32_t HX711_Read(void) {
    uint32_t data = 0;
    uint8_t i;

    while (HAL_GPIO_ReadPin(HX711_DOUT_GPIO_PORT, HX711_DOUT_PIN) == GPIO_PIN_SET);

    for (i = 0; i < 24; i++) {
        HAL_GPIO_WritePin(HX711_SCK_GPIO_PORT, HX711_SCK_PIN, GPIO_PIN_SET);
        delay_us(1); // ??,??????
        data = (data << 1) | HAL_GPIO_ReadPin(HX711_DOUT_GPIO_PORT, HX711_DOUT_PIN);
        HAL_GPIO_WritePin(HX711_SCK_GPIO_PORT, HX711_SCK_PIN, GPIO_PIN_RESET);
        delay_us(1); // ??
    }

    HAL_GPIO_WritePin(HX711_SCK_GPIO_PORT, HX711_SCK_PIN, GPIO_PIN_SET);
    delay_us(1);
    HAL_GPIO_WritePin(HX711_SCK_GPIO_PORT, HX711_SCK_PIN, GPIO_PIN_RESET);
    delay_us(1);

    if (data & 0x800000) {
        data |= 0xFF000000;
    }

    return data;
}

void Hx711_zero_calibration(void) {

 int sum = 0;
    int max_value = 0;
    int min_value = 0xFFFFFF; 
    int valid_samples = 0;

    for (int i = 0; i < SAMPLES; i++) {
        strength_history[i] = HX711_Read();
        int value = strength_history[i];

        if (value > min_value && value < max_value) {
            sum += value;
            valid_samples++;
        }

        if (value > max_value) {
            max_value = value;
        }
        if (value < min_value) {
            min_value = value;
        }

        delay_us(100);
    }

    if (valid_samples > 0) {
        First_value = sum / valid_samples; 
    } else {
        First_value = 0; 
    }

    if (First_value < 0) {
        First_value = -First_value; 
    }

}

int Get_Strength(void) {
    current_value = HX711_Read();
    Strength_value = First_value - current_value + 20000;

    if (Strength_value < NOISE_THRESHOLD && Strength_value > -NOISE_THRESHOLD) {
        Strength_value = 0;
    }

    return Strength_value;
}

void Hx711_run_set(void){
	Hx711_zero_calibration();
}
