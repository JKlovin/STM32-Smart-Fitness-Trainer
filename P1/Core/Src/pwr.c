#include "pwr.h"
#include "main.h"
#include "adc.h"
#include "tim.h"

float ADC_value_get(void)
{
	uint32_t adcValue = 0;
	float voltage = 0.0;
	
	HAL_ADC_Start(&hadc1);
	delay_ms(1);
	if(HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK) {
					adcValue = HAL_ADC_GetValue(&hadc1);
					voltage = (adcValue * 3.3) / 4095.0;  
	}
	HAL_ADC_Stop(&hadc1);
	
	return voltage;
}

