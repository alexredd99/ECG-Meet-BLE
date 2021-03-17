/*
    Payload value struct and ECG buffer struct definitions
*/

#include <stdint.h>
#include <stdbool.h>
#include "simple_ble.h"

// Payload struct definition
typedef union Payload {
    struct {
        int8_t first    :4;    // First 4 bits
        int8_t second   :4;    // Second 4 bits
    } value_pair;
    int8_t byte_value;         // Whole byte (count and initial value)
} Payload_t;

// Struct definition of relevant buffers and counts
typedef struct ECG_buffer {
    uint16_t *raw_buffer;           // Array of raw (filtered) ECG values
    uint16_t raw_size;              // Size of raw_buffer array
    uint16_t raw_count;             // Track count of raw values in raw_buffer 
    Payload_t *payload_buffer;      // Array of Payload_t structs to be transmitted
    uint16_t payload_size;          // Size of payload_buffer array
    uint16_t last_value;            // Last value to be inputted into payload buffer, for encoding
    uint8_t payload_count;          // Current count of sent payloads (resets to 0 after 255)
    bool ready_send;                // Are we ready to send a new payload?
} ECG_buffer_t;

// HELPER: Reset raw and payload buffers to all zeroes, reset raw count
void ECG_reset(ECG_buffer_t *ecg_buf);

// Init ECG_buffer_t struct
void ECG_buffer_init(ECG_buffer_t* ecg_buf, uint16_t payload_size);

// HELPER: Insert new value into raw buffer, update raw count (MUST CALL buffer_ready_send AFTER)
void ECG_buffer_insert(ECG_buffer_t* ecg_buf, uint16_t new_val);

// HELPER: Check if we're ready to send a new advertising payload, update ready_send bool
void ECG_buffer_ready_send(ECG_buffer_t* ecg_buf);

// HELPER: Update payload count
void ECG_update_payload_count(ECG_buffer_t* ecg_buf);

// Insert new value into raw buffer
// Returns true if ready to transmit payload, gets written into adv_data
//    : adv_data should be a static array
bool ECG_buffer_insert_check(ECG_buffer_t* ecg_buf, uint16_t new_val, uint8_t* adv_data);

// Print actual ECG values from advertisement buffer (payload_size = ECG payload)
void ECG_print_payload(uint8_t* adv_data, uint8_t adv_size, uint8_t payload_size);