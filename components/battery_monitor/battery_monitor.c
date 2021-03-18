/*
 * This battery monitoring code is based on reading the voltage
 * after after a voltage divider and checking the level on an analog pin
 * Based on the adc example from Espressif
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 * Copyright 2018 Gal Zaidenstein.
 */

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "keyboard_config.h"

#include "battery_monitor.h"

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   500          //Multisampling

//static const adc_channel_t channel = BATT_PIN;
//reference voltage
uint16_t vref = 1100;
//check battery level

uint8_t get_battery_level(void) {

	uint32_t adc_reading = 0;
	//Multisampling

	for (int i = 0; i < NO_OF_SAMPLES; i++) {
		adc_reading += adc1_get_raw(ADC1_CHANNEL_6);
	}
	adc_reading /= NO_OF_SAMPLES;

	float battery_voltage = ((float)adc_reading / 4095.0)*2.0 *3.3 * (vref / 1000.0);

	float battery_percent = ((battery_voltage - Vin_min) * 100 / (Vin_max - Vin_min));

	ESP_LOGI("battery_monitor", "raw %u, Voltage: %fV percentage %f, as int %u", adc_reading, battery_voltage, battery_percent, (uint8_t) battery_percent);
	if ((uint8_t) battery_percent > 100){ 
		return 100;
	}else{
		return (uint8_t) battery_percent ;
	}
}

//initialize battery monitor pin
void init_batt_monitor(void) {

	gpio_pad_select_gpio(GPIO_NUM_14);
	gpio_set_direction(GPIO_NUM_14, GPIO_MODE_OUTPUT);
	gpio_set_level(GPIO_NUM_14, 1);

	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(BATT_PIN, ADC_ATTEN_DB_11);

	esp_adc_cal_characteristics_t adc_chars;
	esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
	if(val_type == ESP_ADC_CAL_VAL_EFUSE_VREF){
		ESP_LOGI("battery_monitor", "eFuse Vref:%umV", adc_chars.vref);
		vref = adc_chars.vref;
	} else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP){
		ESP_LOGI("battery_monitor", "Two Point --> coeff_a %umV, coeff_b %umV", adc_chars.coeff_a, adc_chars.coeff_b);
	} else {
		ESP_LOGI("batery monitor", "default Vref: 1100mV");
	}
	// adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	// esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF,
	// 		adc_chars);

}

