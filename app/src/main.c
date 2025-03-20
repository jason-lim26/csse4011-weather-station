/**
 * @file main.c
 * @brief TagoIO IoT HTTP Client demo application for weather station.
 *
 * This application collects data from a weather station, formats it as a JSON payload,
 * and sends it to TagoIO via an HTTP client. It logs HTTP responses and sensor data.
 */
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(tagoio_http_post, CONFIG_TAGOIO_HTTP_POST_LOG_LEVEL);

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

/* Global TagoIO context instance */
static struct tagoio_context ctx;

/**
 * @brief HTTP response callback.
 *
 * This callback function is invoked when the HTTP client receives a response.
 * It logs whether partial or final data was received along with the HTTP status.
 *
 * @param rsp Pointer to the http_response structure containing the response data.
 * @param final_data Enum indicating if more data is expected or if this is the final data.
 * @param user_data Pointer to user-defined data (unused in this implementation).
 */
static void response_cb(struct http_response *rsp,
			enum http_final_call final_data,
			void *user_data)
{
	if (final_data == HTTP_DATA_MORE) {
		LOG_DBG("Partial data received (%zd bytes)", rsp->data_len);
	} else if (final_data == HTTP_DATA_FINAL) {
		LOG_DBG("All the data received (%zd bytes)", rsp->data_len);
	}

	LOG_DBG("Response status %s", rsp->http_status);
}

/**
 * @brief Collects wind sensor data.
 *
 * This function retrieves the wind speed and wind direction from the weather station,
 * formats them into a JSON payload, and logs the values.
 *
 * @return 0 on success.
 */
static int collect_data(void)
{
    float wind_speed = weather_station_get_wind_speed(&ws);
    float wind_direction = weather_station_get_wind_direction(&ws);

    (void)snprintf(ctx.payload, sizeof(ctx.payload),
         "[{\"variable\": \"Speed\", \"value\": %.2f, \"unit\": \"kmh\"},"
         "{\"variable\": \"Direction\", \"value\": %.2f, \"unit\": \"degree\"}]",
         (double)wind_speed, (double)wind_direction);

    LOG_INF("Wind Speed: %f, Wind Direction: %f\n", (double)wind_speed, (double)wind_direction);
    return 0;
}

/**
 * @brief Executes the next data collection and transmission cycle.
 *
 * This function retrieves sensor data, attempts to connect to TagoIO, and pushes
 * the data via an HTTP POST request. It logs errors if any step fails.
 */
static void next_turn(void)
{
	if (collect_data() < 0) {
		LOG_INF("Error collecting data.");
		return;
	}

	if (tagoio_connect(&ctx) < 0) {
		LOG_INF("No connection available.");
		return;
	}

	if (tagoio_http_push(&ctx, response_cb) < 0) {
		LOG_INF("Error pushing data.");
		return;
	}
}

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

    /* Initialize ADC and weather station */
    adc_channel_initialise();
    weather_station_init(&ws, adc_dev, gpio_dev, GPIO_PIN);
    LOG_INF("Weather station initialised\n");
    LOG_INF("TagoIO IoT - HTTP Client - Temperature demo");
	wifi_connect();

    /* Main loop */
    while (1) {
        next_turn();
		k_sleep(K_SECONDS(CONFIG_TAGOIO_HTTP_PUSH_INTERVAL));
    }
    return 0;
}
