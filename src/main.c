// /*
//  * Copyright (c) 2016 Intel Corporation
//  *
//  * SPDX-License-Identifier: Apache-2.0
//  */

#include <src/main.h>
#include <src/sensor_hub_service.h>
#include <zephyr/types.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <soc.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/settings/settings.h>
#include <dk_buttons_and_leds.h>

LOG_MODULE_REGISTER(NT53_05, LOG_LEVEL_DBG);

// define the macros for getting the device name and length of the device name
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

// macro for the company code (code of nordic semiconductors, https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Assigned_Numbers/out/en/Assigned_Numbers.pdf?v=1726749632991 page 210 and following)
#define COMPANY_ID_CODE 0x0059

// time in standby
#define SLEEP_TIME_MS   300

// /* getting the device tree node via the aliases */
#define LEDR_NODE DT_ALIAS(led0)
#define LEDG_NODE DT_ALIAS(led1)
#define LEDB_NODE DT_ALIAS(led2)
#define BT0_NODE DT_ALIAS(sw0)

// define data structure for custom data
typedef struct adv_mfg_data {
	uint16_t number_press;      /* Number of times Button 1 is pressed*/
} adv_mfg_data_type;

// // create a new instance of custom data with the id of nordic and 0 because the button has not been pressed yet
static adv_mfg_data_type adv_mfg_data = {0x00};

// advertisement data, sending the name (see prj.conf)
static const struct bt_data advertData[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
	BT_DATA(BT_DATA_MANUFACTURER_DATA,(unsigned char *)&adv_mfg_data, sizeof(adv_mfg_data))
};

// https address of nordic academy
static unsigned char url_data[] ={0x17,'/','/','a','c','a','d','e','m','y','.',
                                 'n','o','r','d','i','c','s','e','m','i','.',
                                 'c','o','m'};

// response date for bluetooth advertisement (sending the nordic semi URL)
static const struct bt_data responseData[] = {
	BT_DATA(BT_DATA_URI, url_data,sizeof(url_data))
};

//parametrize advetisement
static struct bt_le_adv_param *adv_param =
	BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_IDENTITY,
		BT_GAP_ADV_FAST_INT_MIN_1, /* 0x30 units, 48 units, 30ms */
		BT_GAP_ADV_FAST_INT_MAX_1, /* 0x60 units, 96 units, 60ms */
		NULL);	// peer address for directed advertising

// connected and disconnected callbacks
struct bt_conn *my_conn = NULL;
struct bt_conn_cb connection_callbacks = {
    .connected              = on_connected,
    .disconnected           = on_disconnected,
};

// // get the io specifications by using their nodes
static const struct gpio_dt_spec ledR = GPIO_DT_SPEC_GET(LEDR_NODE, gpios);
static const struct gpio_dt_spec ledG = GPIO_DT_SPEC_GET(LEDG_NODE, gpios);
static const struct gpio_dt_spec ledB = GPIO_DT_SPEC_GET(LEDB_NODE, gpios);
static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(BT0_NODE, gpios);

// sensors
static const struct device *bme688SensorDev = DEVICE_DT_GET_ONE(bosch_bme680);

// status variables
bool isConnected = false;

int main(void)
{
	LOG_INF("Start application...\n");
	int err;
	
	// initialize the IOs and bluetooth
	err = initButtonsLEDs();
	err = initBluetooth();

	// register callbacks for connected and disconnected
	bt_conn_cb_register(&connection_callbacks);

	LOG_INF("Advertising successfully started\n");

	while (true) {
		// read data
		sample_and_update_all_sensor_values(bme688SensorDev);

		// let the red LED blink
		if (isConnected == false)
		{
			err = gpio_pin_toggle_dt(&ledR);
		}
		else
		{
			err = gpio_pin_set_dt(&ledR, 0);
		}
		
		if (err < 0) 
		{
			return err;
		}

		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}

// if the button is pressed, increase the counter and send it over BLE
void button_changed(uint32_t button_state, uint32_t has_changed)
{
	// if (has_changed & button_state & USER_BUTTON) {	// if the button has been pressed
	// 	adv_mfg_data.number_press += 1;	// increase the counter
	// 	bt_le_adv_update_data(advertData, ARRAY_SIZE(advertData),	// send the new data over BLE
	// 		      responseData, ARRAY_SIZE(responseData));
	// }
}

int initButtonsLEDs()
{
	int result;

	// init Button and leds
	result = gpio_pin_configure_dt(&button0, GPIO_INPUT);
	result = dk_buttons_init(NULL);
	if (result < 0) {
		return result;
	}

	result = dk_leds_init();
	if (result) {
		LOG_ERR("LEDs init failed (err %d)\n", result);
		return result;
	}

	result = gpio_pin_set_dt(&ledR, 0);
	result = gpio_pin_set_dt(&ledG, 0);
	result = gpio_pin_set_dt(&ledB, 0);
	return result;
}

// Initialize Bluetooth
int initBluetooth()
{
	int err;
	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return err;
	}
	LOG_INF("Bluetooth initialized\n");

	// start
	err = bt_le_adv_start(adv_param, advertData, ARRAY_SIZE(advertData),
			      responseData, ARRAY_SIZE(responseData));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return err;
	}
	return 0;
}

// If the BLE is connected, enable the green LED
void on_connected(struct bt_conn *conn, uint8_t err)
{
	int internalError;
    if (err) {
        LOG_ERR("Connection error %d", err);
        return;
    }
    LOG_INF("Connected");
    my_conn = bt_conn_ref(conn);
	isConnected = true;
	internalError = gpio_pin_set_dt(&ledG, 1);
	if (internalError < 0)
	{
		LOG_ERR("LED Error %d", internalError);
	}
}

// If the BLE is connected, disable the green LED
void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected. Reason %d", reason);
    bt_conn_unref(my_conn);
	isConnected = false;
	int internalError;
	internalError = gpio_pin_set_dt(&ledG, 0);
	if (internalError < 0)
	{
		LOG_ERR("LED Error %d", internalError);
	}
}

int sample_and_update_all_sensor_values(const struct device *bme688Dev)
{
    int err;
    struct sensor_value temperature_value;
    struct sensor_value pressure_value;
    struct sensor_value humidity_value;
	LOG_DBG("Reading sensor values");
     
    //Trigger sampling of BME688 sensor
    err = sensor_sample_fetch(bme688Dev);
    if(err < 0)
    {
		LOG_ERR("Failed to collect samples from bme688. Error value: %d\n", err);
        return err;
    }
 
    //Collect temperature sample and update characteristic
    err = sensor_channel_get(bme688Dev, SENSOR_CHAN_AMBIENT_TEMP, &temperature_value);
    if(err<0)
    {
		LOG_ERR("Failed to fetch temperature sample. Error value: %d\n", err);
        return err;
    }
	// double fTemperature = (double)temperature_value.val1 + (double)temperature_value.val1*1E-6;
	double fTemperature = temperature_value.val1 + temperature_value.val2 * 1E-6;
	LOG_DBG("Temperature: %f", fTemperature);
	if (my_conn)
		sensor_hub_update_temperature(my_conn, (uint8_t*)(&temperature_value.val1), sizeof(temperature_value.val1));
	
 
    //Collect pressure sample and update characteristic
    err = sensor_channel_get(bme688Dev, SENSOR_CHAN_PRESS, &pressure_value);
    if(err<0)
    {
        LOG_ERR("Failed to fetch pressure sample. Error value:  %d\n", err);
        return err;
    }
	double fPressure = pressure_value.val1 + pressure_value.val2 * 1E-6;
	LOG_DBG("Pressure: %f", fPressure);
	if (my_conn)
    	sensor_hub_update_pressure(my_conn, (uint8_t*)(&pressure_value.val1), sizeof(pressure_value.val1));
 
    //Collect humidity sample and update characteristic
    err = sensor_channel_get(bme688Dev, SENSOR_CHAN_HUMIDITY, &humidity_value);
    if(err)
    {
        LOG_ERR("Failed to fetch humidity sample. Error value: %d\n", err);
        return err;
    }
	double fHumidity = humidity_value.val1 + humidity_value.val2 * 1E-6;
	LOG_DBG("Humidity: %f", fHumidity);
    if (my_conn)
		sensor_hub_update_humidity(my_conn, (uint8_t*)(&humidity_value.val1), sizeof(humidity_value.val1));
 
    return err;
}