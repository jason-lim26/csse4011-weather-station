/*
 * Copyright (c) 2020 Gerson Fernando Budke <nandojve@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(wifi); 
#include <zephyr/net/wifi_mgmt.h>

static int connected;
static struct net_mgmt_event_callback wifi_shell_mgmt_cb;
 
/**
 * @brief Handle the result of a WiFi connection attempt.
 *
 * Processes the connection status received from the WiFi management event callback.
 *
 * @param cb Pointer to the net management event callback containing WiFi status information.
 */
static void handle_wifi_connect_result(struct net_mgmt_event_callback *cb)
{
    const struct wifi_status *status = (const struct wifi_status *)
                    cb->info;

    if (status->status) {
        LOG_ERR("Connection request failed (%d)", status->status);
    } else {
        LOG_INF("WIFI Connected");
        connected = 1;
    }
}
 
/**
 * @brief WiFi management event handler.
 *
 * Handles WiFi management events.
 *
 * @param cb Pointer to the net management event callback.
 * @param mgmt_event The management event identifier.
 * @param iface Pointer to the network interface associated with the event.
 */
static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                    uint32_t mgmt_event, struct net_if *iface)
{
    switch (mgmt_event) {
    case NET_EVENT_WIFI_CONNECT_RESULT:
        handle_wifi_connect_result(cb);
        break;
    default:
        break;
    }
}
 
/**
 * @brief Initiate a WiFi connection.
 *
 * Registers the WiFi management callback, configures connection parameters, 
 * and attempts to connect up to 10 times, waiting until a connection is established.
 */
void wifi_connect(void)
{
    int nr_tries = 10;
    int ret = 0;

    net_mgmt_init_event_callback(&wifi_shell_mgmt_cb,
                    wifi_mgmt_event_handler,
                    NET_EVENT_WIFI_CONNECT_RESULT);

    net_mgmt_add_event_callback(&wifi_shell_mgmt_cb);

    struct net_if *iface = net_if_get_default();
    static struct wifi_connect_req_params cnx_params = {
        .ssid = CONFIG_HTTP_WIFI_SSID,
        .ssid_length = 0,
        .psk = CONFIG_HTTP_WIFI_PSK,
        .psk_length = 0,
        .channel = 0,
        .security = WIFI_SECURITY_TYPE_PSK,
    };

    cnx_params.ssid_length = strlen(CONFIG_HTTP_WIFI_SSID);
    cnx_params.psk_length = strlen(CONFIG_HTTP_WIFI_PSK);

    connected = 0;

    LOG_INF("WIFI try connecting to %s...", CONFIG_HTTP_WIFI_SSID);

    /* Let's wait few seconds to allow wifi device be on-line */
    while (nr_tries-- > 0) {
        ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &cnx_params,
                sizeof(struct wifi_connect_req_params));
        if (ret == 0) {
            break;
        }

        LOG_INF("Connect request failed %d. Waiting iface be up...", ret);
        k_msleep(500);
    }

    while (connected == 0) {
        k_msleep(100);
    }
}