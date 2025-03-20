#include <weather_station.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------
 * Macros and Calibration Constants
 *----------------------------------------------------------------------------
 * Define the number of wind vane positions and the calibration ADC values.
 * Adjust these values to suit your hardware and calibration.
 */
// Dummy ADC calibration values – replace with real values.
#define SFE_WMK_ADC_ANGLE_0_0   0
#define SFE_WMK_ADC_ANGLE_22_5  100
#define SFE_WMK_ADC_ANGLE_45_0  200
#define SFE_WMK_ADC_ANGLE_67_5  300
#define SFE_WMK_ADC_ANGLE_90_0  400
#define SFE_WMK_ADC_ANGLE_112_5 500
#define SFE_WMK_ADC_ANGLE_135_0 600
#define SFE_WMK_ADC_ANGLE_157_5 700
#define SFE_WMK_ADC_ANGLE_180_0 800
#define SFE_WMK_ADC_ANGLE_202_5 900
#define SFE_WMK_ADC_ANGLE_225_0 1000
#define SFE_WMK_ADC_ANGLE_247_5 1100
#define SFE_WMK_ADC_ANGLE_270_0 1200
#define SFE_WMK_ADC_ANGLE_292_5 1300
#define SFE_WMK_ADC_ANGLE_315_0 1400
#define SFE_WMK_ADC_ANGLE_337_5 1500

#define SFE_WMK_ADC_RESOLUTION          10  // 10-bit ADC resolution
#define SFE_WIND_VANE_DEGREES_PER_INDEX 22.5f

/*----------------------------------------------------------------------------
 * Global Instance Pointer for Interrupt Context
 *----------------------------------------------------------------------------
 * For simplicity (and because typical ISRs cannot have a context pointer),
 * we use a global pointer to the one active kit instance.
 */
static SFEWeatherMeterKit *kit_instance = NULL;

/*----------------------------------------------------------------------------
 * Forward Declarations for Callbacks
 *----------------------------------------------------------------------------
 */
static void wind_speed_callback(const struct device *dev,
                                struct gpio_callback *cb,
                                uint32_t pins);

/*----------------------------------------------------------------------------
 * ADC Channel Configuration Helper
 *----------------------------------------------------------------------------
 * Configures the ADC channel used by the wind direction sensor.
 */
static int configure_adc_channel(SFEWeatherMeterKit *kit)
{
    struct adc_channel_cfg channel_cfg = {
        .gain             = ADC_GAIN_1,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = kit->wind_dir_adc_channel,
        .differential     = 0,
#if defined(CONFIG_ADC_CONFIGURABLE_INPUTS)
        .input_positive   = kit->wind_dir_adc_channel,
#endif
    };
    return adc_channel_setup(kit->adc_dev, &channel_cfg);
}

/*----------------------------------------------------------------------------
 * Initialization Function
 *----------------------------------------------------------------------------
 * Initializes the kit structure, sets calibration values, configures the ADC channel,
 * and stores the GPIO and ADC devices and pin numbers.
 *
 * Parameters:
 *  - kit: Pointer to an SFEWeatherMeterKit structure.
 *  - adc_dev: Pointer to the ADC device (e.g. obtained via device_get_binding()).
 *  - wind_dir_adc_channel: The ADC channel number for the wind direction sensor.
 *  - gpio_dev: Pointer to the GPIO device used for the wind speed sensor.
 *  - wind_speed_pin: The GPIO pin number for the wind speed sensor.
 */
void SFEWeatherMeterKit_init(SFEWeatherMeterKit *kit,
                             const struct device *adc_dev,
                             int wind_dir_adc_channel,
                             const struct device *gpio_dev,
                             int wind_speed_pin)
{
    /* Save device pointers and pin numbers */
    kit->adc_dev = adc_dev;
    kit->wind_dir_adc_channel = wind_dir_adc_channel;
    kit->gpio_dev = gpio_dev;
    kit->wind_speed_pin = wind_speed_pin;

    /* Configure the ADC channel */
    if (configure_adc_channel(kit) < 0) {
        printk("Failed to configure ADC channel\n");
    }

    /* Set calibration ADC values for the wind vane */
    kit->calibrationParams.vaneADCValues[WMK_ANGLE_0_0]   = SFE_WMK_ADC_ANGLE_0_0;
    kit->calibrationParams.vaneADCValues[WMK_ANGLE_22_5]  = SFE_WMK_ADC_ANGLE_22_5;
    kit->calibrationParams.vaneADCValues[WMK_ANGLE_45_0]  = SFE_WMK_ADC_ANGLE_45_0;
    kit->calibrationParams.vaneADCValues[WMK_ANGLE_67_5]  = SFE_WMK_ADC_ANGLE_67_5;
    kit->calibrationParams.vaneADCValues[WMK_ANGLE_90_0]  = SFE_WMK_ADC_ANGLE_90_0;
    kit->calibrationParams.vaneADCValues[WMK_ANGLE_112_5] = SFE_WMK_ADC_ANGLE_112_5;
    kit->calibrationParams.vaneADCValues[WMK_ANGLE_135_0] = SFE_WMK_ADC_ANGLE_135_0;
    kit->calibrationParams.vaneADCValues[WMK_ANGLE_157_5] = SFE_WMK_ADC_ANGLE_157_5;
    kit->calibrationParams.vaneADCValues[WMK_ANGLE_180_0] = SFE_WMK_ADC_ANGLE_180_0;
    kit->calibrationParams.vaneADCValues[WMK_ANGLE_202_5] = SFE_WMK_ADC_ANGLE_202_5;
    kit->calibrationParams.vaneADCValues[WMK_ANGLE_225_0] = SFE_WMK_ADC_ANGLE_225_0;
    kit->calibrationParams.vaneADCValues[WMK_ANGLE_247_5] = SFE_WMK_ADC_ANGLE_247_5;
    kit->calibrationParams.vaneADCValues[WMK_ANGLE_270_0] = SFE_WMK_ADC_ANGLE_270_0;
    kit->calibrationParams.vaneADCValues[WMK_ANGLE_292_5] = SFE_WMK_ADC_ANGLE_292_5;
    kit->calibrationParams.vaneADCValues[WMK_ANGLE_315_0] = SFE_WMK_ADC_ANGLE_315_0;
    kit->calibrationParams.vaneADCValues[WMK_ANGLE_337_5] = SFE_WMK_ADC_ANGLE_337_5;

    /* Set other calibration parameters */
    kit->calibrationParams.kphPerCountPerSec = 2.4f;
    kit->calibrationParams.windSpeedMeasurementPeriodMillis = 1000;

    /* Reset counters and timers using Zephyr’s uptime (milliseconds) */
    kit->windCountsPrevious = 0;
    kit->windCounts = 0;
    kit->lastWindSpeedMillis = k_uptime_get_32();

    /* Save this instance for use in the interrupt callback */
    kit_instance = kit;
}

/*----------------------------------------------------------------------------
 * Begin Function
 *----------------------------------------------------------------------------
 * Configures the GPIO pin for the wind speed sensor and attaches an interrupt
 * callback using Zephyr’s GPIO API.
 */
int SFEWeatherMeterKit_begin(SFEWeatherMeterKit *kit)
{
    int ret;

    /* Configure wind speed pin as input with pull-up */
    ret = gpio_pin_configure(kit->gpio_dev, kit->wind_speed_pin, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Error configuring wind speed pin\n");
        return ret;
    }

    /* Initialize and add the wind speed callback */
    gpio_init_callback(&kit->wind_speed_cb, wind_speed_callback, BIT(kit->wind_speed_pin));
    ret = gpio_add_callback(kit->gpio_dev, &kit->wind_speed_cb);
    if (ret < 0) {
        printk("Error adding wind speed callback\n");
        return ret;
    }

    /* Configure interrupt for wind speed sensor on both edges */
    ret = gpio_pin_interrupt_configure(kit->gpio_dev, kit->wind_speed_pin, GPIO_INT_EDGE_BOTH);
    if (ret < 0) {
        printk("Error configuring wind speed interrupt\n");
        return ret;
    }

    return 0;
}

/*----------------------------------------------------------------------------
 * Calibration Parameter Accessor and ADC Resolution Adjustment
 *----------------------------------------------------------------------------
 */
SFEWeatherMeterKitCalibrationParams SFEWeatherMeterKit_getCalibrationParams(SFEWeatherMeterKit *kit)
{
    return kit->calibrationParams;
}

void SFEWeatherMeterKit_setCalibrationParams(SFEWeatherMeterKit *kit,
                                             SFEWeatherMeterKitCalibrationParams params)
{
    memcpy(&kit->calibrationParams, &params, sizeof(SFEWeatherMeterKitCalibrationParams));
}

void SFEWeatherMeterKit_setADCResolutionBits(SFEWeatherMeterKit *kit, uint8_t resolutionBits)
{
    for (uint8_t i = 0; i < WMK_NUM_ANGLES; i++) {
        int8_t bitShift = SFE_WMK_ADC_RESOLUTION - resolutionBits;
        if (bitShift > 0) {
            kit->calibrationParams.vaneADCValues[i] >>= bitShift;
        } else if (bitShift < 0) {
            kit->calibrationParams.vaneADCValues[i] <<= (-bitShift);
        }
    }
}

/*----------------------------------------------------------------------------
 * Wind Direction Measurement
 *----------------------------------------------------------------------------
 * Reads the wind direction sensor via the ADC, compares the reading against
 * calibration values, and returns the closest matching wind direction in degrees.
 */
float SFEWeatherMeterKit_getWindDirection(SFEWeatherMeterKit *kit)
{
    int16_t rawADC = 0;
    int16_t sample_buffer = 0;
    struct adc_sequence sequence = {
        .channels    = BIT(kit->wind_dir_adc_channel),
        .buffer      = &sample_buffer,
        .buffer_size = sizeof(sample_buffer),
        .resolution  = SFE_WMK_ADC_RESOLUTION,
    };

    if (adc_read(kit->adc_dev, &sequence) < 0) {
        printk("ADC read error\n");
        return -1.0f;
    }
    rawADC = sample_buffer;

    int16_t closestDifference = 32767;
    uint8_t closestIndex = 0;
    for (uint8_t i = 0; i < WMK_NUM_ANGLES; i++) {
        int16_t diff = kit->calibrationParams.vaneADCValues[i] - rawADC;
        diff = abs(diff);
        if (diff < closestDifference) {
            closestDifference = diff;
            closestIndex = i;
        }
    }
    float direction = closestIndex * SFE_WIND_VANE_DEGREES_PER_INDEX;
    return direction;
}

/*----------------------------------------------------------------------------
 * Wind Speed Measurement Update
 *----------------------------------------------------------------------------
 * Uses a fixed measurement window to update the wind speed counters.
 */
static void updateWindSpeed(SFEWeatherMeterKit *kit)
{
    uint32_t tNow = k_uptime_get_32();
    uint32_t dt = tNow - kit->lastWindSpeedMillis;

    if (dt < kit->calibrationParams.windSpeedMeasurementPeriodMillis) {
        /* Still within the current measurement window */
    } else {
        if (dt > (kit->calibrationParams.windSpeedMeasurementPeriodMillis * 2)) {
            /* No pulses for over two periods: reset counters */
            kit->windCountsPrevious = 0;
            kit->windCounts = 0;
            kit->lastWindSpeedMillis = tNow;
        } else {
            /* End of the measurement window: store the count and reset */
            kit->windCountsPrevious = kit->windCounts;
            kit->windCounts = 0;
            kit->lastWindSpeedMillis += kit->calibrationParams.windSpeedMeasurementPeriodMillis;
        }
    }
}

/*----------------------------------------------------------------------------
 * Get Wind Speed
 *----------------------------------------------------------------------------
 * Computes the wind speed in kph based on the counts recorded during the last
 * measurement window.
 */
float SFEWeatherMeterKit_getWindSpeed(SFEWeatherMeterKit *kit)
{
    updateWindSpeed(kit);
    float windSpeed = ((float) kit->windCountsPrevious / kit->calibrationParams.windSpeedMeasurementPeriodMillis)
                      * 1000 * kit->calibrationParams.kphPerCountPerSec / 2;
    return windSpeed;
}

/*----------------------------------------------------------------------------
 * Additional Accessors and Reset Functions
 *----------------------------------------------------------------------------
 */
uint32_t SFEWeatherMeterKit_getWindSpeedCounts(SFEWeatherMeterKit *kit)
{
    return kit->windCounts;
}

void SFEWeatherMeterKit_resetWindSpeedFilter(SFEWeatherMeterKit *kit)
{
    kit->windCountsPrevious = 0;
    kit->windCounts = 0;
    kit->lastWindSpeedMillis = k_uptime_get_32();
}

float weather_station_get_wind_speed(WeatherStation *ws)
{
    return SFEWeatherMeterKit_getWindSpeed(&ws->kit);
}

float weather_station_get_wind_direction(WeatherStation *ws)
{
    return SFEWeatherMeterKit_getWindDirection(&ws->kit);
}

/*----------------------------------------------------------------------------
 * GPIO Interrupt Callback for Wind Speed Sensor
 *----------------------------------------------------------------------------
 */
static void wind_speed_callback(const struct device *dev,
                                struct gpio_callback *cb,
                                uint32_t pins)
{
    if (kit_instance != NULL) {
        updateWindSpeed(kit_instance);
        kit_instance->windCounts++;
    }
}

/*----------------------------------------------------------------------------
 * Helper Functions for Initializations
 *----------------------------------------------------------------------------
 */
void weather_station_init(WeatherStation *ws,
    const struct device *adc_dev,
    const struct device *gpio_dev,
    uint32_t gpio_pin)
{
    SFEWeatherMeterKit_init(&ws->kit, adc_dev, 0, gpio_dev, gpio_pin);
    SFEWeatherMeterKit_setADCResolutionBits(&ws->kit, 10);
    SFEWeatherMeterKit_begin(&ws->kit);
}
