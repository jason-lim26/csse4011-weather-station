# CSSE4011 Weather Station IoT Demo
<div style="text-align:center;">
  <img src="misc/weather_meter.jpg" alt="Weather Meter">
</div>

This project is an IoT demo designed for the M5Stack Core2. It focuses on measuring wind speed and wind direction using a weather station sensor, then transmitting the data to a server via HTTP.

## Features

- **Wind Speed Measurement:**  
  Uses a GPIO interrupt to count wind sensor pulses for calculating wind speed.
  
- **Wind Direction Measurement:**  
  Reads ADC values from the wind direction sensor and converts them into degrees.

- **IoT Connectivity:**  
  Sends sensor data to a remote server using simple HTTP GET request.

## Requirements

- **Hardware:**  
  - M5Stack Core2  
  - Weather Meter Kit (Sparkfun)

- **Software:**  
  - Zephyr RTOS  
  - West command-line tool
  - Proper toolchain configured for the M5Stack Core2

## Setup and Build Instructions

1. **Navigate to the Application Directory:**

   ```sh
   cd app/
   ```

2. **Build the Application:**

   Perform a pristine build to ensure all files are recompiled:

   ```sh
   west build --pristine
   ```

3. **Flash the Application:**

   Once the build is complete, flash the binary to your M5Stack Core2 device:

   ```sh
   west flash
   ```

4. **Monitor the Device Logs:**

   To view real-time logs and monitor sensor readings:

   ```sh
   west espressif monitor
   ```

## Overview

### Flowchart
<div style="text-align:center;">
  <img src="misc/csse4011-weather-station-flowchart.png" alt="Flowchart">
</div>

### Message Protocol
<div style="text-align:center;">
  <img src="misc/csse4011-weather-station-message-protocol.png" alt="Message Protocol">
</div>

### Schematic
<div style="text-align:center;">
  <img src="misc/csse4011-weather-station-schematic.png" alt="Schematic">
</div>


## License

This project is licensed under the Apache-2.0 license.

## Contact

For issues or further inquiries, please refer to the project repository or contact the project maintainer.