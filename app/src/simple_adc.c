/*
 * Copyright (c) 2020 Libre Solar Technologies GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

/** 
 * ADC channel specification retrieved from the device tree.
 */
static const struct adc_dt_spec adc_channel0 = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

/**
 * @brief Initializes the ADC channel.
 *
 * This function configures the ADC channel specified in the device tree overlay.
 * It first checks whether the ADC controller device is ready, and then sets up the ADC channel.
 *
 * @return 0 on success, or a negative error code if initialization fails.
 */
int adc_channel_initialise()
{
	/* Configure channels individually prior to sampling. */
	if (!adc_is_ready_dt(&adc_channel0)) {
		printk("ADC controller device %s not ready\n", adc_channel0.dev->name);
		return -ENODEV;
	}

	int err = adc_channel_setup_dt(&adc_channel0);
	if (err < 0) {
		printk("Could not setup channel (%d)\n", err);
		return 0;
	}
	return err;
}

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
bool adc_channel_read_millivolt(int32_t *out_val_mv)
{

	uint16_t buf;
	struct adc_sequence sequence = {
		.buffer = &buf,
		/* buffer size in bytes, not number of samples */
		.buffer_size = sizeof(buf),
	};

	(void)adc_sequence_init_dt(&adc_channel0, &sequence);

	int err = adc_read_dt(&adc_channel0, &sequence);
	if (err < 0) {
		printk("Could not read (%d)\n", err);
		return false;
	}

	/*
	 * If using differential mode, the 16 bit value
	 * in the ADC sample buffer should be a signed 2's
	 * complement value.
	 */
	if (adc_channel0.channel_cfg.differential) {
		*out_val_mv = (int32_t)((int16_t)buf);
	} else {
		*out_val_mv = (int32_t)buf;
	}

	err = adc_raw_to_millivolts_dt(&adc_channel0, out_val_mv);
	/* conversion to mV may not be supported, skip if not */
	if (err < 0) {
		return false;
	}

	return true;
}
