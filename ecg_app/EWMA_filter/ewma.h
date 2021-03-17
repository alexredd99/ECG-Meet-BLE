/*
    Exponential Weighted Moving Average:
        Modified port of Arduino EWMA library, https://github.com/jonnieZG/EWMA
*/

#include <stdint.h>

#define TRUE 1
#define FALSE 0

// EWMA struct defintion
typedef struct Ewma {
    uint32_t alpha;
    uint32_t alpha_scale;
    uint32_t output_scaled;
} Ewma_t;

// Initalize EWMA struct, alpha = alpha / alpha_scale
void ewma_init(Ewma_t *ewma, uint32_t alpha, uint32_t alpha_scale);

// Filter a value with a given EWMA struct, returns the filtered value
uint32_t ewma_filter(Ewma_t *ewma, uint32_t input);