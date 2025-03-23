/*
 * Copyright (c) 2020 Gerson Fernando Budke <nandojve@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

 #include <zephyr/logging/log.h>
 #include <zephyr/net/net_ip.h>
 #include <zephyr/net/socket.h>
 #include <zephyr/net/socketutils.h>
 #include <zephyr/net/dns_resolve.h>
 #include <zephyr/net/tls_credentials.h>
 #include <zephyr/net/http/client.h>
 
 #if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
 #include <zephyr/net/tls_credentials.h>
 #include "ca_certificate.h"
 #endif

 #include "sockets.h"
 
#define HTTP_HOST "csse4011-iot.uqcloud.net"
#define HTTP_PATH "/"
#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
#define HTTP_PORT "443"
#else
#define HTTP_PORT "80"
#endif
 
int http_get_dynamic(float wind_speed, float wind_direction)
{
    int ret;
    struct addrinfo hints = {0}, *res = NULL;
    int sock;

    /* Build the dynamic URL */
    char dynamic_path[100];
    ret = snprintk(dynamic_path, sizeof(dynamic_path),
                   "/add.php?stationid=4011&speed=%.2f&direction=%.2f",
                   (double)wind_speed, (double)wind_direction);
    if (ret <= 0 || ret >= sizeof(dynamic_path)) {
        printk("Error: Could not build dynamic URL.\n");
        return -1;
    }

    printk("Sending GET request to https://%s%s\n", HTTP_HOST, dynamic_path);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    ret = getaddrinfo(HTTP_HOST, HTTP_PORT, &hints, &res);
    if (ret != 0) {
        printk("Error: getaddrinfo() failed (%d)\n", ret);
        return -1;
    }

#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
    sock = socket(res->ai_family, res->ai_socktype, IPPROTO_TLS_1_2);
#else
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
#endif
    if (sock < 0) {
        printk("Error: socket() failed (%d)\n", sock);
        freeaddrinfo(res);
        return -1;
    }

#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
    {
        sec_tag_t sec_tag_opt[] = { CA_CERTIFICATE_TAG };
        ret = setsockopt(sock, SOL_TLS, TLS_SEC_TAG_LIST, sec_tag_opt, sizeof(sec_tag_opt));
        if (ret < 0) {
            printk("Error: setsockopt TLS_SEC_TAG_LIST: %d\n", ret);
            close(sock);
            freeaddrinfo(res);
            return ret;
        }
        ret = setsockopt(sock, SOL_TLS, TLS_HOSTNAME, HTTP_HOST, strlen(HTTP_HOST));
        if (ret < 0) {
            printk("Error: setsockopt TLS_HOSTNAME: %d\n", ret);
            close(sock);
            freeaddrinfo(res);
            return ret;
        }
    }
#endif

    ret = connect(sock, res->ai_addr, res->ai_addrlen);
    if (ret < 0) {
        printk("Error: connect() failed (%d)\n", ret);
        close(sock);
        freeaddrinfo(res);
        return ret;
    }
    freeaddrinfo(res);

    /* Build the HTTP GET request with the dynamic URL */
    char req_buf[512];
    int req_len = snprintk(req_buf, sizeof(req_buf),
                           "GET %s HTTP/1.1\r\n"
                           "Host: %s\r\n"
                           "Connection: close\r\n"
                           "\r\n",
                           dynamic_path, HTTP_HOST);
    if (req_len <= 0 || req_len >= sizeof(req_buf)) {
        printk("Error: Request buffer too small or snprintk error\n");
        close(sock);
        return -1;
    }

    ret = send(sock, req_buf, req_len, 0);
    if (ret < 0) {
        printk("Error: send() failed (%d)\n", ret);
        close(sock);
        return ret;
    }

    /* Receive and print the server response */
    // adds latency, by-pass this for now.
    // char response[512];
    // while (1) {
    //     ret = recv(sock, response, sizeof(response) - 1, 0);
    //     if (ret < 0) {
    //         printk("Error: recv() failed (%d)\n", ret);
    //         break;
    //     }
    //     if (ret == 0) {
    //         /* Server closed connection */
    //         break;
    //     }
    //     response[ret] = '\0';
    //     printk("%s", response);
    // }
    close(sock);
    printk("\nSocket closed.\n");
    return 0;
}
