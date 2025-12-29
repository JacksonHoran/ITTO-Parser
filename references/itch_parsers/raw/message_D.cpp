#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <mach/mach_time.h>

typedef struct {
    char messageType;
    uint16_t trackingNumber;
    uint64_t timestamp;
    uint64_t referenceNumber;
} Message;

Message parseMessage(const unsigned char *msg) {
    Message d;
    d.messageType = msg[0];
    d.trackingNumber = (msg[1] << 8) | msg[2];
    
    uint64_t tmp = *(uint64_t*)(msg + 3);
    tmp = __builtin_bswap64(tmp);
    d.timestamp = tmp >> 16;
    
    uint64_t tmp64;
    memcpy(&tmp64, msg + 9, sizeof(tmp64));
    d.referenceNumber = __builtin_bswap64(tmp64);
    
    return d;
}

int main() {
    unsigned char message[] = {
        0x44, 0x00, 0x00, 0x18, 0xEB, 0xCA, 0xB3, 0x7B, 0x80, 
        0x00, 0x00, 0x00, 0x00, 0xB2, 0xD0, 0x6C, 0xE8
    };

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const int iterations = 10000000;
    volatile uint64_t dummy = 0;

    uint64_t start = mach_absolute_time();
    Message d;
    for (int i = 0; i < iterations; i++) {
        d = parseMessage(message);
        dummy += d.trackingNumber;
    }
    uint64_t end = mach_absolute_time();

    double elapsed_ns = (double)(end - start) * (double)timebase.numer / (double)timebase.denom;
    double elapsed_per_iter = elapsed_ns / iterations;

    printf("Total elapsed time: %.4f ns\n", elapsed_ns);
    printf("Average per iteration: %.4f ns\n", elapsed_per_iter);
    
    printf("\n--- Last Parsed Message ---\n");
    printf("Message Type: %c\n", d.messageType);
    printf("Tracking Number: %d\n", d.trackingNumber);
    printf("Timestamp: %llu\n", d.timestamp);
    printf("Reference Number: %llu\n", d.referenceNumber);

    if (dummy == 0) printf("");
    return 0;
}
