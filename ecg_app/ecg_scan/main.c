// BLE Scanning app
//
// Receives BLE advertisements

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "simple_ble.h"
#include "nrf_delay.h"

#include "nrf52840dk.h"

#include "payload.h"  // Include payload value definitions

#define PAYLOAD_SIZE  27   // ECG payload size (bytes)

// BLE configuration
// This is mostly irrelevant since we are scanning only
static simple_ble_config_t ble_config = {
        // BLE address is c0:98:e5:4e:00:02
        .platform_id       = 0x4E,
        .device_id         = 0x0002,    
        .adv_name          = "ECG_SCAN",                            // Irrelevant for scanning
        .adv_interval      = MSEC_TO_UNITS(1000, UNIT_0_625_MS),    // *
        .min_conn_interval = MSEC_TO_UNITS(500, UNIT_1_25_MS),      // *
        .max_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS),     // *
};

// Simple BLE struct
simple_ble_app_t* simple_ble_app;

// Count of last ECG payload
static uint8_t last_ecg_count = 0;

// Callback handler for advertisement reception
void ble_evt_adv_report(ble_evt_t const* p_ble_evt) {

  // extract the fields we care about
  ble_gap_evt_adv_report_t const* adv_report = &(p_ble_evt->evt.gap_evt.params.adv_report);
  uint8_t const* ble_addr = adv_report->peer_addr.addr; // 6 bytes of the address
  uint8_t* adv_buf = adv_report->data.p_data;           // array of advertisement payload data
  uint16_t adv_len = adv_report->data.len;              // length of advertisement payload data

  // If device has ID 0xAABB (ECG sender)
  if(ble_addr[0] == 0xBB && ble_addr[1] == 0xAA){

    uint8_t curr_count = adv_buf[adv_len - PAYLOAD_SIZE]; // Get current payload count from buffer

    // Same ECG payload as last
    if(curr_count == last_ecg_count){

      // Do nothing
      return;

    // Missed a few payloads, send appropriate # of 0s for each missing payload
    } else if(curr_count > last_ecg_count + 1){

      for(uint16_t i = 0; i < (((PAYLOAD_SIZE * 2) - 3) * (curr_count - last_ecg_count + 1)); i++) printf("250\n"); // -3 values for overhead (count, initial value)

    }

    // Continue to print (next payload case falls through)
    ECG_print_payload(adv_buf, 31, PAYLOAD_SIZE); // Decode and print ECG values from payload

    last_ecg_count = curr_count;                  // Update last ECG payload count to current count

    return;
  }
}

int main(void) {

  // Setup BLE
  simple_ble_app = simple_ble_init(&ble_config);
  advertising_stop();

  // Start scanning
  scanning_start();

  // Go into low power mode
  while(1) {
    power_manage();
  }

  return 0;
}