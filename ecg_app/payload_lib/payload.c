#include <stdlib.h>
#include <stdio.h>

#include "payload.h"

// HELPER: Reset raw and payload buffers to all zeroes, reset raw count
void ECG_reset(ECG_buffer_t *ecg_buf){
    memset(ecg_buf->raw_buffer, 0, ecg_buf->raw_size * sizeof(uint16_t));           // Reset raw buffer
    memset(ecg_buf->payload_buffer, 0, ecg_buf->payload_size * sizeof(Payload_t));  // Reset payload buffer
    ecg_buf->raw_count = 0;
    return;
}

// Init ECG_buffer_t struct
void ECG_buffer_init(ECG_buffer_t* ecg_buf, uint16_t payload_size){
    ecg_buf->payload_size   = payload_size;
    ecg_buf->raw_size       = (payload_size * 2) - 4;  // -1 byte for count, -1 for initial value (2 bytes = 4 values)
    ecg_buf->payload_buffer = malloc(payload_size * sizeof(uint16_t));          // Create payload buffer array
    ecg_buf->raw_buffer     = malloc(ecg_buf->raw_size * sizeof(Payload_t));    // Create raw buffer array
    ECG_reset(ecg_buf);
    ecg_buf->payload_count  = 0;
    ecg_buf->last_value     = 0;
    ecg_buf->ready_send     = false;
    return;
}

// HELPER: Insert new value into raw buffer, update raw count (MUST CALL buffer_ready_send AFTER)
void ECG_buffer_insert(ECG_buffer_t* ecg_buf, uint16_t new_val){
    ecg_buf->raw_count++;
    ecg_buf->raw_buffer[ecg_buf->raw_count - 1] = new_val; // Index = count - 1

    return;
}

// HELPER: Check if we're ready to send a new advertising payload, update ready_send bool
void ECG_buffer_ready_send(ECG_buffer_t* ecg_buf){

    if(ecg_buf->ready_send == true) ecg_buf->ready_send = false;    // Reset ready_send bool

    if(ecg_buf->raw_count == ecg_buf->raw_size){                    // Have max raw values, ready to send

        ecg_buf->ready_send = true;
    }

    return;
}

// HELPER: Update payload count
void ECG_update_payload_count(ECG_buffer_t* ecg_buf){
    
    if(ecg_buf->payload_count == 0xFF){ // Reset payload count to 0 if at max 255
        ecg_buf->payload_count = 0;
        return;
    }

    ecg_buf->payload_count++;
    return;
}

// Insert new value into raw buffer
// Returns true if ready to transmit encoded payload, gets written into adv_data
//    : adv_data should be a static array
bool ECG_buffer_insert_check(ECG_buffer_t* ecg_buf, uint16_t new_val, uint8_t* adv_data){

    ECG_buffer_insert(ecg_buf, new_val);    // Insert new value into raw buffer, update raw count
    ECG_buffer_ready_send(ecg_buf);         // Check if ready to send, update read_send bool

    // Ready to send
    if(ecg_buf->ready_send){
        
        uint8_t p_idx;  // Payload buffer index
        uint8_t r_idx;  // Raw buffer index

        p_idx = 2;  // Start loop at 3rd byte in payload
        r_idx = 1;  //            at 2nd raw value

        ecg_buf->payload_buffer[0].byte_value = ecg_buf->payload_count;                 // Payload byte 1 = count
        if(ecg_buf->raw_buffer[0] < 173) ecg_buf->raw_buffer[0] = 173;                  // Bounds check (min)
        if(ecg_buf->raw_buffer[0] > 427) ecg_buf->raw_buffer[0] = 427;                  // Bounds check (max)
        ecg_buf->payload_buffer[1].byte_value = (int8_t)(ecg_buf->raw_buffer[0] - 300); // Payload byte 2 = first full ECG value (signed dif from 300)
        ecg_buf->last_value                   = ecg_buf->raw_buffer[0];                 // Init last value to first value in raw buffer

        for(; p_idx < ecg_buf->payload_size; p_idx++, r_idx += 2){  // Loop through payload and raw buffers

            ecg_buf->payload_buffer[p_idx].value_pair.first  = ecg_buf->raw_buffer[r_idx] - ecg_buf->last_value;
            ecg_buf->payload_buffer[p_idx].value_pair.second = ecg_buf->raw_buffer[r_idx + 1] - ecg_buf->raw_buffer[r_idx];

            ecg_buf->last_value = ecg_buf->raw_buffer[r_idx + 1];
        }

        memcpy(adv_data, ecg_buf->payload_buffer, ecg_buf->payload_size * sizeof(Payload_t));   // Copy payload buffer to advert data array
        ECG_reset(ecg_buf);                  // Reset raw and payload buffers for next transmission
        ECG_update_payload_count(ecg_buf);   // Update payload count for next transmission

        return true;
    }

    return false;
}

// Print actual ECG values from advertisement buffer (payload_size = ECG payload)
void ECG_print_payload(uint8_t* adv_data, uint8_t adv_size, uint8_t payload_size){
    
    uint8_t p_idx       = adv_size - payload_size + 1;                 // Payload buffer index (ECG payload)
    uint16_t prev_value = (uint16_t)(((int8_t)adv_data[p_idx]) + 300); // Init previous value to initial value (+ 300)
    Payload_t curr      = {0};                                         // Current payload value

    printf("%u\n", prev_value);                  // Print initial value
    p_idx++;                                     // Start loop at following value

    for(; p_idx < payload_size; p_idx++){        // Loop through advertisement buffer
        
        curr.byte_value = adv_data[p_idx];                   // Get current payload value
        printf("%u\n", prev_value + curr.value_pair.first);  // Print prev + change 1
        prev_value += curr.value_pair.first;                 // Update prev
        printf("%u\n", prev_value + curr.value_pair.second); // Print prev + change 2
        prev_value += curr.value_pair.second;                // Update prev     

    }

    return;
}