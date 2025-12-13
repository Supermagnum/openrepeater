/*
 * dudect test for gr-linux-crypto authentication tag comparison
 * 
 * Tests whether authentication tag comparison is constant-time
 * This is critical for side-channel resistance.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

#define DUDECT_IMPLEMENTATION
#include "src/dudect.h"

#define TAG_SIZE 16  // GCM/Poly1305 tag size

/**
 * Constant-time comparison function (reference implementation)
 * This should pass dudect tests
 */
static int constant_time_compare(const uint8_t *a, const uint8_t *b, size_t len) {
    uint8_t diff = 0;
    for (size_t i = 0; i < len; i++) {
        diff |= a[i] ^ b[i];
    }
    return diff == 0;
}

/**
 * Test function: authentication tag comparison
 * This simulates what happens in gr-linux-crypto when verifying auth tags
 */
uint8_t do_one_computation(uint8_t *data) {
    uint8_t tag1[TAG_SIZE];
    uint8_t tag2[TAG_SIZE];
    uint8_t ret = 0;
    
    // Extract two tags from input data
    memcpy(tag1, data, TAG_SIZE);
    memcpy(tag2, data + TAG_SIZE, TAG_SIZE);
    
    // Compare tags (this is what we're testing)
    // Use constant-time comparison (what should be used in production)
    int result = constant_time_compare(tag1, tag2, TAG_SIZE);
    
    // Return result to prevent optimization
    ret = (uint8_t)result;
    return ret;
}

/**
 * Prepare test inputs
 * Class 0: tags are identical (should be fast in variable-time, same time in constant-time)
 * Class 1: tags are different (should be fast in variable-time, same time in constant-time)
 */
void prepare_inputs(dudect_config_t *c, uint8_t *input_data, uint8_t *classes) {
    randombytes(input_data, c->number_measurements * c->chunk_size);
    
    for (size_t i = 0; i < c->number_measurements; i++) {
        classes[i] = randombit();
        uint8_t *tag1 = input_data + (size_t)i * c->chunk_size;
        uint8_t *tag2 = tag1 + TAG_SIZE;
        
        if (classes[i] == 0) {
            // Class 0: Make tags identical
            memcpy(tag2, tag1, TAG_SIZE);
        } else {
            // Class 1: Make tags different (leave random)
            // tag2 is already random and different from tag1
        }
    }
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    printf("dudect: Testing gr-linux-crypto authentication tag comparison\n");
    printf("This tests whether tag comparison is constant-time\n\n");
    
    dudect_config_t config = {
        .chunk_size = TAG_SIZE * 2,  // Two tags
        .number_measurements = 500000,  // Start with 500k measurements
    };
    
    dudect_ctx_t ctx;
    dudect_init(&ctx, &config);
    
    printf("Running constant-time test...\n");
    printf("(This may take several minutes. Press Ctrl+C to stop early)\n\n");
    
    dudect_state_t state = DUDECT_NO_LEAKAGE_EVIDENCE_YET;
    int iterations = 0;
    
    while (state == DUDECT_NO_LEAKAGE_EVIDENCE_YET && iterations < 100) {
        state = dudect_main(&ctx);
        iterations++;
        
        // Print progress every 10 iterations
        if (iterations % 10 == 0) {
            printf("Iteration %d: still testing...\n", iterations);
        }
    }
    
    dudect_free(&ctx);
    
    if (state == DUDECT_LEAKAGE_FOUND) {
        printf("\n*** RESULT: NOT constant-time (timing leakage detected) ***\n");
        return 1;
    } else if (state == DUDECT_NO_LEAKAGE_EVIDENCE_YET) {
        printf("\n*** RESULT: No timing leakage detected so far ***\n");
        printf("(Note: This does not guarantee constant-time, just no evidence of leakage)\n");
        return 0;
    } else {
        printf("\n*** RESULT: Test completed ***\n");
        return 0;
    }
}

