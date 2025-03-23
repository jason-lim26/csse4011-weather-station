/*
 * Copyright (c) 2020 Gerson Fernando Budke <nandojve@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 
/**
 * @brief Sends an HTTP GET request with dynamic URL parameters.
 *
 * Constructs a URL using the given wind speed and wind direction, resolves the server address,
 * establishes a connection (with TLS if enabled), sends the HTTP GET request, and then closes the connection.
 *
 * @param wind_speed    The measured wind speed.
 * @param wind_direction The measured wind direction.
 *
 * @return int Returns 0 on success or a negative error code on failure.
 */
int http_get_dynamic(float wind_speed, float wind_direction);