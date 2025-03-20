#ifndef __SIMPLE_ADC_H__
#define __SIMPLE_ADC_H__

#include <stdbool.h>
#include <stdint.h>


/**
 * @brief Initializes the ADC channel.
 *
 * This function configures the ADC channel specified in the device tree overlay.
 * It first checks whether the ADC controller device is ready, and then sets up the ADC channel.
 *
 * @return 0 on success, or a negative error code if initialization fails.
 */
int adc_channel_initialise();

/**
 * @brief Reads the ADC channel and converts the raw value to millivolts.
 *
 * This function performs an ADC read operation on the specified channel.
 * It stores the raw ADC value, interprets it (as a signed value if using differential mode),
 * and converts it into millivolts. The resulting value is stored in the location provided.
 *
 * @param[out] out_val_mv Pointer to an integer where the resulting millivolt value will be stored.
 *
 * @return true if the read and conversion are successful, false otherwise.
 */
bool adc_channel_read_millivolt(int32_t *out_val_mv);

#endif
