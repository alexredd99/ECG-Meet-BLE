#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_error.h"
#include "nrf_delay.h"
#include "nrfx_saadc.h"
#include "simple_ble.h"
#include "ble_advdata.h"

#include "nrf52840dk.h"

#include "ewma.h"         // Include Exponential Weighted Moving Average functions
#include "payload.h"      // Include payload value definitions

#define INPUT_CHANNEL    0                     // ADC channels
#define ANALOG_PIN       NRF_SAADC_INPUT_AIN7  // Analog pin we're reading from (P0.31)
#define ADVERT_INTERVAL  21                    // Advertisement interval (ms)
#define PAYLOAD_SIZE     27                    // Payload size (bytes)

// Callback for SAADC events
void saadc_callback (nrfx_saadc_evt_t const * p_event) {
  // Don't care about adc callbacks
}

// Sample a particular analog channel in blocking mode
nrf_saadc_value_t sample_value (uint8_t channel) {
  nrf_saadc_value_t val;
  ret_code_t error_code = nrfx_saadc_sample_convert(channel, &val);
  APP_ERROR_CHECK(error_code);
  return val;
}

// Intervals for advertising and connections
static simple_ble_config_t ble_config = {
        // c0:98:e5:4e:xx:xx
        .platform_id       = 0x4E,    // used as 4th octect in device BLE address
        .device_id         = 0xAABB,  // must be unique on each device you program!
        .adv_name          = "",   // used in advertisements if there is room 
        .adv_interval      = MSEC_TO_UNITS(ADVERT_INTERVAL, UNIT_0_625_MS),
        .min_conn_interval = MSEC_TO_UNITS(500, UNIT_1_25_MS),
        .max_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS),
};

// Create EWMA struct
static Ewma_t filter;

// Create ECG buffer struct
static ECG_buffer_t ecg_buf;

// Create simple_ble struct
static simple_ble_app_t* simple_ble_app;

// Create manufacturer data payload array (+4 for manufacturer data overhead)
static uint8_t data_payload[PAYLOAD_SIZE + 4 ] = {0};

int main (void) {

  ret_code_t error_code = NRF_SUCCESS;

  // Setup BLE
  simple_ble_app = simple_ble_init(&ble_config);

  // Initialize analog to digital converter
  nrfx_saadc_config_t saadc_config = NRFX_SAADC_DEFAULT_CONFIG;
  saadc_config.resolution = NRF_SAADC_RESOLUTION_12BIT;
  error_code = nrfx_saadc_init(&saadc_config, saadc_callback);
  APP_ERROR_CHECK(error_code);

  // Initialize analog inputs
  nrf_saadc_channel_config_t channel_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(0);
  channel_config.gain = NRF_SAADC_GAIN1_6; // Input gain of 1/6 Volts/Volt, multiply incoming signal by (1/6)
  channel_config.reference = NRF_SAADC_REFERENCE_INTERNAL; // 0.6 Volt reference, input after gain can be 0 to 0.6 Volts

  // Specify input pin and initialize that ADC channel
  channel_config.pin_p = ANALOG_PIN;
  error_code = nrfx_saadc_channel_init(INPUT_CHANNEL, &channel_config);
  APP_ERROR_CHECK(error_code);
  
  // Initialize EWMA filter struct
  ewma_init(&filter, 9, 100);   // Create filter, alpha = .09
  uint32_t filtered_value = 0;  // Current value outputted from filter

  // Initalize manufacturer data in BLE payload data array
  data_payload[0] = 0x1E;   //  Length 30(MAX 31 bytes)
  data_payload[1] = 0xFF;   //  Manufacturer specific data
  data_payload[2] = 0xFF;   //  ID (BLE internal testing) 
  data_payload[3] = 0xFF;   //  ID (BLE internal testing)

  // Initialize ECG buffer struct
  ECG_buffer_init(&ecg_buf, PAYLOAD_SIZE);

  // Initialize analog value and calibrate filter
  nrf_saadc_value_t analog_val = 0;

  for(int j = 0; j < 100; j++){
    analog_val = sample_value(INPUT_CHANNEL);
    ewma_filter(&filter, (uint32_t) analog_val);
    nrf_delay_ms(1);
  }

  // LOOP  
  for(int i = 0; i < 2500; i++) { // 10 seconds
  
    // Sample analog input
    analog_val = sample_value(INPUT_CHANNEL);

    // Filter analog input
    filtered_value = ewma_filter(&filter, (uint32_t) analog_val) / 6; // Scale to 1/6

    // Insert new value into ecg_buf and check to change advertisement payload
    if(ECG_buffer_insert_check(&ecg_buf, filtered_value, data_payload + 4)){  // Offset 4 bytes from header
      simple_ble_adv_raw(data_payload, 31);
      //simple_ble_set_adv(&advdata, NULL);
    }

    // Delay 4ms before getting next raw ECG value (sampling at 250-Hz)
    nrf_delay_ms(4);
  }

  advertising_stop();
  return 0;
}