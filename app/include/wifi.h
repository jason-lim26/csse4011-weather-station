/*
 * Copyright (c) 2020 Gerson Fernando Budke <nandojve@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#if defined(CONFIG_WIFI)
/**
 * @brief Connect to a WiFi network.
 *
 * Initiates a connection to a WiFi network using the configuration parameters (SSID and PSK)
 * defined in the build configuration.
 */
void wifi_connect(void);

#else
#define wifi_connect()
#endif