#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <mach/mach_time.h>
#include <arpa/inet.h>

typedef struct {
    char messageType;
    uint16_t trackingNumber;
    uint64_t timestamp;
    uint32_t optionId;
    char tradingState;
} Message;

Message parseMessage(const unsigned char *msg) {
    Message h;
    h.messageType = msg[0];
    h.trackingNumber = (msg[1] << 8) | msg[2];
    
    uint64_t tmp64 = *(uint64_t*)(msg + 3);
    tmp64 = __builtin_bswap64(tmp64);
    h.timestamp = tmp64 >> 16;
    
    uint32_t tmp32;
    memcpy(&tmp32, msg + 9, sizeof(tmp32));
    h.optionId = ntohl(tmp32);
    
    h.tradingState = msg[13];
    return h;
}

int main() {
    unsigned char message[] = {
        0x48, 0x00, 0x00, 0x07, 0xD7, 0x96, 0x1B, 0xDC, 0x7C, 
        0x00, 0x05, 0x3B, 0xA3, 0x54
    };

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const int iterations = 10000000;
    volatile uint64_t dummy = 0;

    uint64_t start = mach_absolute_time();
    Message h;
    for (int i = 0; i < iterations; i++) {
        h = parseMessage(message);
        dummy += h.trackingNumber;
    }
    uint64_t end = mach_absolute_time();

    double elapsed_ns = (double)(end - start) * (double)timebase.numer / (double)timebase.denom;
    double elapsed_per_iter = elapsed_ns / iterations;

    printf("Total elapsed time: %.4f ns\n", elapsed_ns);
    printf("Average per iteration: %.4f ns\n", elapsed_per_iter);
    
    printf("\n--- Last Parsed Message ---\n");
    printf("Message Type: %c\n", h.messageType);
    printf("Tracking Number: %d\n", h.trackingNumber);
    printf("Timestamp: %llu\n", h.timestamp);
    printf("Option ID: %u\n", h.optionId);
    printf("Trading State: %c\n", h.tradingState);

    if (dummy == 0) printf("");
    return 0;
}
