/*
 * dudect test for gr-linux-crypto encryption operation
 * 
 * Tests whether encryption timing varies with input data patterns
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

#define DUDECT_IMPLEMENTATION
#include "src/dudect.h"

#define BLOCK_SIZE 16  // AES block size
#define KEY_SIZE 32    // AES-256

static uint8_t test_key[KEY_SIZE] = {0};
static EVP_CIPHER_CTX *ctx_encrypt = NULL;
static int initialized = 0;

/**
 * Initialize OpenSSL context (called once)
 */
static void init_crypto(void) {
    if (initialized) return;
    
    // Initialize with random key
    randombytes(test_key, KEY_SIZE);
    
    ctx_encrypt = EVP_CIPHER_CTX_new();
    if (!ctx_encrypt) {
        fprintf(stderr, "Failed to create EVP context\n");
        exit(1);
    }
    
    // Set up AES-256-GCM encryption
    if (EVP_EncryptInit_ex(ctx_encrypt, EVP_aes_256_gcm(), NULL, test_key, NULL) != 1) {
        fprintf(stderr, "Failed to initialize encryption\n");
        exit(1);
    }
    
    initialized = 1;
}

/**
 * Test function: encryption operation
 * This simulates what happens in gr-linux-crypto when encrypting data
 */
uint8_t do_one_computation(uint8_t *data) {
    uint8_t plaintext[BLOCK_SIZE];
    uint8_t ciphertext[BLOCK_SIZE];
    uint8_t iv[12] = {0};  // GCM uses 12-byte IV
    uint8_t tag[16] = {0};
    int outlen;
    uint8_t ret = 0;
    
    if (!initialized) {
        init_crypto();
    }
    
    // Copy input data to plaintext
    memcpy(plaintext, data, BLOCK_SIZE);
    
    // Perform encryption (simulating gr-linux-crypto encryption)
    EVP_EncryptInit_ex(ctx_encrypt, NULL, NULL, NULL, iv);
    EVP_EncryptUpdate(ctx_encrypt, ciphertext, &outlen, plaintext, BLOCK_SIZE);
    EVP_EncryptFinal_ex(ctx_encrypt, ciphertext + outlen, &outlen);
    EVP_CIPHER_CTX_ctrl(ctx_encrypt, EVP_CTRL_GCM_GET_TAG, 16, tag);
    
    // XOR result to prevent optimization
    for (int i = 0; i < BLOCK_SIZE; i++) {
        ret ^= ciphertext[i];
    }
    ret ^= tag[0];
    
    return ret;
}

/**
 * Prepare test inputs
 * Class 0: All zeros (might trigger special handling)
 * Class 1: Random data (normal case)
 */
void prepare_inputs(dudect_config_t *c, uint8_t *input_data, uint8_t *classes) {
    randombytes(input_data, c->number_measurements * c->chunk_size);
    
    for (size_t i = 0; i < c->number_measurements; i++) {
        classes[i] = randombit();
        uint8_t *data = input_data + (size_t)i * c->chunk_size;
        
        if (classes[i] == 0) {
            // Class 0: All zeros (special pattern)
            memset(data, 0x00, c->chunk_size);
        } else {
            // Class 1: Random data (already randomized)
        }
    }
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    printf("dudect: Testing gr-linux-crypto encryption timing\n");
    printf("This tests whether encryption timing varies with input patterns\n\n");
    
    dudect_config_t config = {
        .chunk_size = BLOCK_SIZE,
        .number_measurements = 500000,
    };
    
    dudect_ctx_t ctx;
    dudect_init(&ctx, &config);
    
    printf("Running encryption timing test...\n");
    printf("(This may take several minutes. Press Ctrl+C to stop early)\n\n");
    
    dudect_state_t state = DUDECT_NO_LEAKAGE_EVIDENCE_YET;
    int iterations = 0;
    
    while (state == DUDECT_NO_LEAKAGE_EVIDENCE_YET && iterations < 100) {
        state = dudect_main(&ctx);
        iterations++;
        
        if (iterations % 10 == 0) {
            printf("Iteration %d: still testing...\n", iterations);
        }
    }
    
    dudect_free(&ctx);
    
    if (ctx_encrypt) {
        EVP_CIPHER_CTX_free(ctx_encrypt);
    }
    
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

