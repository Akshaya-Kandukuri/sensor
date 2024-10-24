// sensor_hub_service.c
 
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <soc.h>
 
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/addr.h>
#include <zephyr/bluetooth/gatt.h>
 
#include "sensor_hub_service.h"

// environmental sensor data
#define BT_UUID_SENSOR_ENV              BT_UUID_DECLARE_128(SENSOR_HUB_ENV_SERVICE_UUID)
#define BT_UUID_SENSOR_ENV_TEMP         BT_UUID_DECLARE_128(TEMP_CHARACTERISTIC_UUID)
#define BT_UUID_SENSOR_ENV_PRESSURE     BT_UUID_DECLARE_128(PRESSURE_CHARACTERISTIC_UUID)
#define BT_UUID_SENSOR_ENV_HUMIDITY     BT_UUID_DECLARE_128(HUMIDITY_CHARACTERISTIC_UUID)

// positioning sensor data
#define BT_UUID_SENSOR_POS              BT_UUID_DECLARE_128(SENSOR_HUB_POS_SERVICE_UUID)
#define BT_UUID_SENSOR_POS_XACC         BT_UUID_DECLARE_128(XACCELERATION_CHARACTERISTIC_UUID)
#define BT_UUID_SENSOR_POS_YACC         BT_UUID_DECLARE_128(YACCELERATION_CHARACTERISTIC_UUID)
#define BT_UUID_SENSOR_POS_ZACC         BT_UUID_DECLARE_128(ZACCELERATION_CHARACTERISTIC_UUID)
 
/*This function is called whenever the Client Characteristic Control Descriptor (CCCD) has been
changed by the GATT client, for each of the characteristics*/
void on_cccd_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);
    switch(value)
    {
        case BT_GATT_CCC_NOTIFY:
            // Start sending stuff!
            break;
         
        case 0:
            // Stop sending stuff
            break;
 
        default:
            printk("Error, CCCD has been set to an invalid value");    
    }
}
 
//Sensor hub Service Declaration and Registration
BT_GATT_SERVICE_DEFINE(sensor_env,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_SENSOR_ENV),
     
    BT_GATT_CHARACTERISTIC(BT_UUID_SENSOR_ENV_TEMP,
                    BT_GATT_CHRC_NOTIFY,
                    BT_GATT_PERM_READ,
                    NULL, NULL, NULL),
    BT_GATT_CCC(on_cccd_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
     
    BT_GATT_CHARACTERISTIC(BT_UUID_SENSOR_ENV_PRESSURE,
                    BT_GATT_CHRC_NOTIFY,
                    BT_GATT_PERM_READ,
                    NULL, NULL, NULL),
    BT_GATT_CCC(on_cccd_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
 
    BT_GATT_CHARACTERISTIC(BT_UUID_SENSOR_ENV_HUMIDITY,
                    BT_GATT_CHRC_NOTIFY,
                    BT_GATT_PERM_READ,
                    NULL, NULL, NULL),
    BT_GATT_CCC(on_cccd_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
);

BT_GATT_SERVICE_DEFINE(sensor_pos,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_SENSOR_POS),
     
    BT_GATT_CHARACTERISTIC(BT_UUID_SENSOR_POS_XACC,
                    BT_GATT_CHRC_NOTIFY,
                    BT_GATT_PERM_READ,
                    NULL, NULL, NULL),
    BT_GATT_CCC(on_cccd_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
     
    BT_GATT_CHARACTERISTIC(BT_UUID_SENSOR_POS_YACC,
                    BT_GATT_CHRC_NOTIFY,
                    BT_GATT_PERM_READ,
                    NULL, NULL, NULL),
    BT_GATT_CCC(on_cccd_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
 
    BT_GATT_CHARACTERISTIC(BT_UUID_SENSOR_POS_ZACC,
                    BT_GATT_CHRC_NOTIFY,
                    BT_GATT_PERM_READ,
                    NULL, NULL, NULL),
    BT_GATT_CCC(on_cccd_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
);
 
/* The below functions send a notification to a GATT client with the provided data,
given that the CCCD has been set to Notify (0x1) */
void sensor_hub_update_temperature(struct bt_conn *conn, const double *data, uint16_t len)
{
    const struct bt_gatt_attr *attr = &sensor_env.attrs[2];
 
    struct bt_gatt_notify_params params =
    {
        .uuid   = BT_UUID_SENSOR_ENV_TEMP,
        .attr   = attr,
        .data   = data,
        .len    = len,
        .func   = NULL
    };
 
    if(bt_gatt_is_subscribed(conn, attr, BT_GATT_CCC_NOTIFY))
    {
        // Send the notification
        if(bt_gatt_notify_cb(conn, &params))
        {
            printk("Error, unable to send notification\n");
        }
    }
    else
    {
        printk("Warning, notification not enabled for temperature characteristic\n");
    }
}
 
void sensor_hub_update_pressure(struct bt_conn *conn, const double *data, uint16_t len)
{
    const struct bt_gatt_attr *attr = &sensor_env.attrs[5];
 
    struct bt_gatt_notify_params params =
    {
        .uuid   = BT_UUID_SENSOR_ENV_PRESSURE,
        .attr   = attr,
        .data   = data,
        .len    = len,
        .func   = NULL
    };
 
    if(bt_gatt_is_subscribed(conn, attr, BT_GATT_CCC_NOTIFY))
    {
        // Send the notification
        if(bt_gatt_notify_cb(conn, &params))
        {
            printk("Error, unable to send notification\n");
        }
    }
    else
    {
        printk("Warning, notification not enabled for pressure characteristic\n");
    }
}
 
void sensor_hub_update_humidity(struct bt_conn *conn, const double *data, uint16_t len)
{
    const struct bt_gatt_attr *attr = &sensor_env.attrs[8];
 
    struct bt_gatt_notify_params params =
    {
        .uuid   = BT_UUID_SENSOR_ENV_HUMIDITY,
        .attr   = attr,
        .data   = data,
        .len    = len,
        .func   = NULL
    };
 
    if(bt_gatt_is_subscribed(conn, attr, BT_GATT_CCC_NOTIFY))
    {
        // Send the notification
        if(bt_gatt_notify_cb(conn, &params))
        {
            printk("Error, unable to send notification\n");
        }
    }
    else
    {
        printk("Warning, notification not enabled for humidity characteristic\n");
    }
}

void sensor_hub_update_xAcceleration(struct bt_conn *conn, const double *data, uint16_t len)
{
    const struct bt_gatt_attr *attr = &sensor_pos.attrs[2];
 
    struct bt_gatt_notify_params params =
    {
        .uuid   = BT_UUID_SENSOR_POS_XACC,
        .attr   = attr,
        .data   = data,
        .len    = len,
        .func   = NULL
    };
 
    if(bt_gatt_is_subscribed(conn, attr, BT_GATT_CCC_NOTIFY))
    {
        // Send the notification
        if(bt_gatt_notify_cb(conn, &params))
        {
            printk("Error, unable to send notification\n");
        }
    }
    else
    {
        printk("Warning, notification not enabled for x acceleration characteristic\n");
    }
}

void sensor_hub_update_yAcceleration(struct bt_conn *conn, const double *data, uint16_t len)
{
    const struct bt_gatt_attr *attr = &sensor_pos.attrs[5];
 
    struct bt_gatt_notify_params params =
    {
        .uuid   = BT_UUID_SENSOR_POS_YACC,
        .attr   = attr,
        .data   = data,
        .len    = len,
        .func   = NULL
    };
 
    if(bt_gatt_is_subscribed(conn, attr, BT_GATT_CCC_NOTIFY))
    {
        // Send the notification
        if(bt_gatt_notify_cb(conn, &params))
        {
            printk("Error, unable to send notification\n");
        }
    }
    else
    {
        printk("Warning, notification not enabled for x acceleration characteristic\n");
    }
}

void sensor_hub_update_zAcceleration(struct bt_conn *conn, const double *data, uint16_t len)
{
    const struct bt_gatt_attr *attr = &sensor_pos.attrs[8];
 
    struct bt_gatt_notify_params params =
    {
        .uuid   = BT_UUID_SENSOR_POS_ZACC,
        .attr   = attr,
        .data   = data,
        .len    = len,
        .func   = NULL
    };
 
    if(bt_gatt_is_subscribed(conn, attr, BT_GATT_CCC_NOTIFY))
    {
        // Send the notification
        if(bt_gatt_notify_cb(conn, &params))
        {
            printk("Error, unable to send notification\n");
        }
    }
    else
    {
        printk("Warning, notification not enabled for x acceleration characteristic\n");
    }
}