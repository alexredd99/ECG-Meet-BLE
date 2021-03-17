#include <stdint.h>
#include "ewma.h"

// Init EWMA filter struct
void ewma_init(Ewma_t *ewma, uint32_t alpha, uint32_t alpha_scale){
    ewma->alpha = alpha;
    ewma->alpha_scale = alpha_scale;
    ewma->output_scaled = 2000;        // ~Average ECG value
    return;
}

// Filter a raw ECG value
uint32_t ewma_filter(Ewma_t *ewma, uint32_t input){
    ewma->output_scaled = ewma->alpha * input + (ewma->alpha_scale - ewma->alpha) * ewma->output_scaled/ewma->alpha_scale;
    return (ewma->output_scaled + ewma->alpha_scale / 2) / ewma->alpha_scale;
}