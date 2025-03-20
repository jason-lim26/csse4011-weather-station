#ifndef SFE_WEATHER_METER_KIT_H
#define SFE_WEATHER_METER_KIT_H

#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------
 * Macros and Constants
 *----------------------------------------------------------------------------
 * Define the number of wind vane positions, ADC resolution, and conversion
 * constants. Adjust these values as needed.
 */
#define WMK_NUM_ANGLES                 16

#define WMK_ANGLE_0_0                  0
#define WMK_ANGLE_22_5                 1
#define WMK_ANGLE_45_0                 2
#define WMK_ANGLE_67_5                 3
#define WMK_ANGLE_90_0                 4
#define WMK_ANGLE_112_5                5
#define WMK_ANGLE_135_0                6
#define WMK_ANGLE_157_5                7
#define WMK_ANGLE_180_0                8
#define WMK_ANGLE_202_5                9
#define WMK_ANGLE_225_0                10
#define WMK_ANGLE_247_5                11
#define WMK_ANGLE_270_0                12
#define WMK_ANGLE_292_5                13
#define WMK_ANGLE_315_0                14
#define WMK_ANGLE_337_5                15

#define SFE_WMK_ADC_RESOLUTION         10   // Example: 10-bit ADC resolution
#define SFE_WIND_VANE_DEGREES_PER_INDEX 22.5f

/*----------------------------------------------------------------------------
 * Data Structures
 *----------------------------------------------------------------------------
 */

/**
 * @brief Calibration parameters for wind measurements.
 *
 * Contains ADC calibration values for the wind vane and measurement
 * parameters for computing wind speed.
 */
typedef struct {
    uint16_t vaneADCValues[WMK_NUM_ANGLES];
    float    kphPerCountPerSec;
    uint32_t windSpeedMeasurementPeriodMillis;
} SFEWeatherMeterKitCalibrationParams;

/**
 * @brief Main structure for the Weather Meter Kit.
 *
 * Holds calibration parameters, measurement counters, timing information,
 * and device/pin configuration for wind speed and wind direction.
 */
typedef struct {
    SFEWeatherMeterKitCalibrationParams calibrationParams;
    uint32_t windCountsPrevious;
    uint32_t windCounts;
    uint32_t lastWindSpeedMillis;
    const struct device *adc_dev;          /**< ADC device for wind direction sensor */
    int                   wind_dir_adc_channel; /**< ADC channel for wind direction */
    const struct device *gpio_dev;         /**< GPIO device for wind speed sensor */
    int                   wind_speed_pin;   /**< GPIO pin for wind speed sensor */
    struct gpio_callback  wind_speed_cb;    /**< GPIO callback structure */
} SFEWeatherMeterKit;

/*----------------------------------------------------------------------------
 * Public Function Prototypes
 *----------------------------------------------------------------------------
 */

/**
 * @brief Initialize the Weather Meter Kit structure.
 *
 * Configures device pointers, ADC channel, calibration parameters, and resets
 * counters/timers.
 *
 * @param kit Pointer to a SFEWeatherMeterKit structure.
 * @param adc_dev Pointer to the ADC device (obtained via device_get_binding()).
 * @param wind_dir_adc_channel ADC channel number for the wind direction sensor.
 * @param gpio_dev Pointer to the GPIO device for the wind speed sensor.
 * @param wind_speed_pin GPIO pin number for the wind speed sensor.
 */
void SFEWeatherMeterKit_init(SFEWeatherMeterKit *kit,
                             const struct device *adc_dev,
                             int wind_dir_adc_channel,
                             const struct device *gpio_dev,
                             int wind_speed_pin);

/**
 * @brief Begin sensor operation.
 *
 * Configures the wind speed GPIO pin (with pull-up) and sets up the interrupt
 * callback.
 *
 * @param kit Pointer to a SFEWeatherMeterKit structure.
 * @return 0 on success, negative error code on failure.
 */
int SFEWeatherMeterKit_begin(SFEWeatherMeterKit *kit);

/**
 * @brief Get the current calibration parameters.
 *
 * @param kit Pointer to a SFEWeatherMeterKit structure.
 * @return A SFEWeatherMeterKitCalibrationParams structure.
 */
SFEWeatherMeterKitCalibrationParams SFEWeatherMeterKit_getCalibrationParams(SFEWeatherMeterKit *kit);

/**
 * @brief Set new calibration parameters.
 *
 * @param kit Pointer to a SFEWeatherMeterKit structure.
 * @param params New calibration parameters.
 */
void SFEWeatherMeterKit_setCalibrationParams(SFEWeatherMeterKit *kit,
                                             SFEWeatherMeterKitCalibrationParams params);

/**
 * @brief Adjust the ADC resolution of the calibration values.
 *
 * @param kit Pointer to a SFEWeatherMeterKit structure.
 * @param resolutionBits The desired ADC resolution (in bits).
 */
void SFEWeatherMeterKit_setADCResolutionBits(SFEWeatherMeterKit *kit, uint8_t resolutionBits);

/**
 * @brief Get the wind direction in degrees.
 *
 * Reads the ADC value from the wind vane, compares it to calibration values,
 * and returns the closest matching direction.
 *
 * @param kit Pointer to a SFEWeatherMeterKit structure.
 * @return Wind direction in degrees.
 */
float SFEWeatherMeterKit_getWindDirection(SFEWeatherMeterKit *kit);

/**
 * @brief Get the measured wind speed in kilometers per hour.
 *
 * Computes wind speed based on the counts recorded during the last measurement window.
 *
 * @param kit Pointer to a SFEWeatherMeterKit structure.
 * @return Wind speed in kph.
 */
float SFEWeatherMeterKit_getWindSpeed(SFEWeatherMeterKit *kit);

/**
 * @brief Get the number of wind speed counts.
 *
 * Returns the current count of wind speed pulses in the current measurement window.
 *
 * @param kit Pointer to a SFEWeatherMeterKit structure.
 * @return Wind speed counts.
 */
uint32_t SFEWeatherMeterKit_getWindSpeedCounts(SFEWeatherMeterKit *kit);

/**
 * @brief Reset the wind speed measurement filter.
 *
 * Resets wind speed counters and timer to start a new measurement window.
 *
 * @param kit Pointer to a SFEWeatherMeterKit structure.
 */
void SFEWeatherMeterKit_resetWindSpeedFilter(SFEWeatherMeterKit *kit);

/**
 * @brief Generic wrapper for the weather station.
 *
 * This structure encapsulates your SFEWeatherMeterKit instance.
 */
 typedef struct {
    SFEWeatherMeterKit kit;
} WeatherStation;

/**
 * @brief Initialize the weather station.
 *
 * @param ws Pointer to the WeatherStation instance.
 * @param adc_dev ADC device pointer.
 * @param gpio_dev GPIO device pointer.
 * @param gpio_pin The GPIO pin number used.
 */
void weather_station_init(WeatherStation *ws,
                          const struct device *adc_dev,
                          const struct device *gpio_dev,
                          uint32_t gpio_pin);

/**
 * @brief Get the current wind speed.
 *
 * @param ws Pointer to the WeatherStation instance.
 * @return Wind speed as a float.
 */
float weather_station_get_wind_speed(WeatherStation *ws);

/**
 * @brief Get the current wind direction.
 *
 * @param ws Pointer to the WeatherStation instance.
 * @return Wind direction as a float.
 */
float weather_station_get_wind_direction(WeatherStation *ws);

#ifdef __cplusplus
}
#endif

#endif /* SFE_WEATHER_METER_KIT_H */

