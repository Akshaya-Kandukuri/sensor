# Nordic Thingy 53 API Documentation
The idea of the documentation is to list example and information about programming the Nordic Thingy 53 or the nrf5340 DK.

## Debugging
It is not possible to directly use a debugger for the thingy because the code is running on the chip and no debug information is provided. Hence, logging has to be used for debugging. The easiest way is to use the USB-C plug to retrieve the log information. For this, please configure the build as follows:

1. Add ```CONFIG_LOG=y``` and ```
CONFIG_LOG_BACKEND_UART=y``` to the prj.conf. This enables logging in general and uses the USB for logging
2. Use ```LOG_INF```, ```LOG_DBG```, ```LOG_WRN``` or ```LOG_ERR``` to log information.

Connect the Thingy with a USB-C cable to the computer. Check the Windows device manager for a new communication Port (COM). Connect to this port via PuTTy with a baud rate of 115200. Then you should see the logs in the PuTTy terminal.

## Hardware
This short explanation is based on the Nordic documentation at [https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/lessons/lesson-2-reading-buttons-and-controlling-leds/](https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/lessons/lesson-2-reading-buttons-and-controlling-leds/). 

### Prepare the hardware access
Hardware properties are defined in the device trees. The device trees are stored in `install-path/zephyr/boards/nordic/5340dk` (for the development kit) and `install-path/zephyr/boards/nordic/thingy53` (for the thingy 53). An excerpt of the device tree (`thingy53_nrf5340_common.dtsi`) of the thingy 53 looks like below. It includes awailable buttons and leds. There are 2 buttons `button_1` and `button_2`. Furthermore, the thingy has a RGB Led. The different colors may be accessed by `led_1`,`led_2` and `led_3` for red, green and blue respectively.
```c
buttons {
    compatible = "gpio-keys";
    button0: button_0 {
        gpios = <&gpio1 14 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>;
        label = "Push button 1";
        zephyr,code = <INPUT_KEY_0>;
    };
    button1: button_1 {
        gpios = <&gpio1 13 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>;
        label = "Push button 2";
        zephyr,code = <INPUT_KEY_1>;
    };
};

leds {
    compatible = "gpio-leds";
    red_led: led_1 {
        gpios = <&gpio1 8 GPIO_ACTIVE_HIGH>;
        label = "RGB red LED";
    };
    green_led: led_2 {
        gpios = <&gpio1 6 GPIO_ACTIVE_HIGH>;
        label = "RGB green LED";
    };
    blue_led: led_3 {
        gpios = <&gpio1 7 GPIO_ACTIVE_HIGH>;
        label = "RGB blue LED";
    };
};
```
The hardware access has to prepared before it is used. This contains defining nodes and getting the access informtion. A node is initialized wih
```
#define BT0_NODE DT_ALIAS(sw0)
```
`sw0` is the alias of button 1. Aliases can be found in the device trees, e.g., `thingy53_nrf5340_common.dtsi`:
```
aliases {
    led0 = &led0;
    led1 = &led1;
    led2 = &led2;
    led3 = &led3;
    pwm-led0 = &pwm_led0;
    sw0 = &button0;
    sw1 = &button1;
    sw2 = &button2;
    sw3 = &button3;
    bootloader-led0 = &led0;
    mcuboot-button0 = &button0;
    mcuboot-led0 = &led0;
    watchdog0 = &wdt0;
    spi-flash0 = &mx25r64;
};
```

Aftr defining the node, the access information has to be retrieved via
```
static const struct gpio_dt_spec button0 = 
    GPIO_DT_SPEC_GET (BT0_NODE, gpios);
```

### Initializing the hardware
Check if the Hardware is ready with the code below.
```
if (!gpio_is_ready_dt(&button0)) {
		return 0;
	}
```

Configure the GPIO with
```
ret = gpio_pin_configure_dt(&button0, GPIO_INPUT);
	if (ret < 0) {
		return 0;
	}
```

### Access the hardware
A GPIO can be set with
```
ret = gpio_pin_set_dt(&led1, 1);
```

A GPIO can be toggled with
```
ret = gpio_pin_toggle_dt(&led0);
```

A GPIO can be read with
```
ret = gpio_pin_get_dt(&button0);
```

