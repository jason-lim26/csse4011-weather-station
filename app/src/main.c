/**
 * @file main.c
 * @brief TagoIO IoT HTTP Client demo application for weather station.
 *
 * This application collects data from a weather station, formats it as a JSON payload,
 * and sends it to TagoIO via an HTTP client. It logs HTTP responses and sensor data.
 */
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/printk.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/http/client.h>
#include <zephyr/random/random.h>
#include <stdio.h>

LOG_MODULE_REGISTER(main);

#include "wifi.h"
#include "sockets.h"
#include "simple_adc.h"
#include "weather_station.h"

#define GPIO_0   DT_NODELABEL(gpio0)
#define GPIO_PIN 27

/* Get device instances */
const struct device *gpio_dev = DEVICE_DT_GET(GPIO_0);
const struct device *adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc0));

/* Global weather station instance */
WeatherStation ws;

/**
 * @brief Cycle Executive.
 *
 * Initializes the ADC and weather station, connects to Wi-Fi, and enters the main loop,
 * periodically collecting sensor data and transmitting it to TagoIO.
 *
 * @return Always returns 0.
 */
int main(void)
{
    printk("Starting program\n");

    /* Initialize weather station */
    weather_station_init(&ws, adc_dev, gpio_dev, GPIO_PIN);
    LOG_INF("Weather station initialised\n");
	wifi_connect();

    /* Main loop */
    while (1) {
        float wind_speed = weather_station_get_wind_speed(&ws);
        float wind_direction = weather_station_get_wind_direction(&ws);
        printk("Wind Speed: %f, Wind Direction: %f\n", (double)wind_speed, (double)wind_direction);
        if (http_get_dynamic(wind_speed, wind_direction) < 0) {
            LOG_INF("Error sending GET request.");
        }

        k_msleep(1000);
    }
    return 0;
}
