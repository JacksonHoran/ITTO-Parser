#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <mach/mach_time.h>

typedef struct {
    char messageType;
    uint16_t trackingNumber;
    uint64_t timestamp;
    uint64_t bidRefNumber;
    uint64_t askRefNumber;
} Message;

Message parseMessage(const unsigned char *msg) {
    Message y;
    y.messageType = msg[0];
    y.trackingNumber = (msg[1] << 8) | msg[2];
    
    uint64_t tmp = *(uint64_t*)(msg + 3);
    tmp = __builtin_bswap64(tmp);
    y.timestamp = tmp >> 16;
    
    uint64_t tmp64;
    memcpy(&tmp64, msg + 9, sizeof(tmp64));
    y.bidRefNumber = __builtin_bswap64(tmp64);
    
    memcpy(&tmp64, msg + 17, sizeof(tmp64));
    y.askRefNumber = __builtin_bswap64(tmp64);
    
    return y;
}

int main() {
    unsigned char message[] = {
        0x59, 0x00, 0x00, 0x1E, 0xD4, 0xF9, 0x30, 0x08, 0x03, 
        0x00, 0x00, 0x00, 0x00, 0xB3, 0x28, 0x55, 0x50, 0x00, 
        0x00, 0x00, 0x00, 0xB3, 0x28, 0x55, 0x54
    };

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const int iterations = 10000000;
    volatile uint64_t dummy = 0;

    uint64_t start = mach_absolute_time();
    Message y;
    for (int i = 0; i < iterations; i++) {
        y = parseMessage(message);
        dummy += y.trackingNumber;
    }
    uint64_t end = mach_absolute_time();

    double elapsed_ns = (double)(end - start) * (double)timebase.numer / (double)timebase.denom;
    double elapsed_per_iter = elapsed_ns / iterations;

    printf("Total elapsed time: %.4f ns\n", elapsed_ns);
    printf("Average per iteration: %.4f ns\n", elapsed_per_iter);
    
    printf("\n--- Last Parsed Message ---\n");
    printf("Message Type: %c\n", y.messageType);
    printf("Tracking Number: %d\n", y.trackingNumber);
    printf("Timestamp: %llu\n", y.timestamp);
    printf("Bid Ref Number: %llu\n", y.bidRefNumber);
    printf("Ask Ref Number: %llu\n", y.askRefNumber);

    if (dummy == 0) printf("");
    return 0;
}
