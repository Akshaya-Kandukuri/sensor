#include <stdint.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/drivers/sensor.h>

int initIO();
int initBluetooth();
void on_connected(struct bt_conn *conn, uint8_t err);
void on_disconnected(struct bt_conn *conn, uint8_t reason);
int sample_and_update_all_sensor_values(const struct device *bme688Dev);