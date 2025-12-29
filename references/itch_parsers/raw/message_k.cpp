#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <mach/mach_time.h>

typedef struct {
    char messageType;
    uint16_t trackingNumber;
    uint64_t timestamp;
    uint64_t originalBidRefNumber;
    uint64_t bidRefNumber;
    uint64_t originalAskRefNumber;
    uint64_t askRefNumber;
    uint16_t bidPrice;
    uint16_t bidSize;
    uint16_t askPrice;
    uint16_t askSize;
} Message;

Message parseMessage(const unsigned char *msg) {
    Message k;
    k.messageType = msg[0];
    k.trackingNumber = (msg[1] << 8) | msg[2];
    
    uint64_t tmp = *(uint64_t*)(msg + 3);
    tmp = __builtin_bswap64(tmp);
    k.timestamp = tmp >> 16;
    
    uint64_t tmp64;
    memcpy(&tmp64, msg + 9, sizeof(tmp64));
    k.originalBidRefNumber = __builtin_bswap64(tmp64);
    
    memcpy(&tmp64, msg + 17, sizeof(tmp64));
    k.bidRefNumber = __builtin_bswap64(tmp64);
    
    memcpy(&tmp64, msg + 25, sizeof(tmp64));
    k.originalAskRefNumber = __builtin_bswap64(tmp64);
    
    memcpy(&tmp64, msg + 33, sizeof(tmp64));
    k.askRefNumber = __builtin_bswap64(tmp64);
    
    k.bidPrice = (msg[41] << 8) | msg[42];
    k.bidSize = (msg[43] << 8) | msg[44];
    k.askPrice = (msg[45] << 8) | msg[46];
    k.askSize = (msg[47] << 8) | msg[48];
    
    return k;
}

int main() {
    unsigned char message[] = {
        0x6B, 0x00, 0x00, 0x1E, 0xD5, 0x00, 0xAC, 0x76, 0xEF, 
        0x00, 0x00, 0x00, 0x00, 0xB3, 0x28, 0x55, 0x0C, 0x00, 
        0x00, 0x00, 0x00, 0xB3, 0x28, 0x8E, 0xB0, 0x00, 0x00, 
        0x00, 0x00, 0xB3, 0x28, 0x55, 0x10, 0x00, 0x00, 0x00, 
        0x00, 0xB3, 0x28, 0x8E, 0xB4, 0x00, 0x00, 0x00, 0x00, 
        0x01, 0xF4, 0x00, 0x01
    };

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const int iterations = 10000000;
    volatile uint64_t dummy = 0;

    uint64_t start = mach_absolute_time();
    Message k;
    for (int i = 0; i < iterations; i++) {
        k = parseMessage(message);
        dummy += k.trackingNumber;
    }
    uint64_t end = mach_absolute_time();

    double elapsed_ns = (double)(end - start) * (double)timebase.numer / (double)timebase.denom;
    double elapsed_per_iter = elapsed_ns / iterations;

    printf("Total elapsed time: %.4f ns\n", elapsed_ns);
    printf("Average per iteration: %.4f ns\n", elapsed_per_iter);
    
    printf("\n--- Last Parsed Message ---\n");
    printf("Message Type: %c\n", k.messageType);
    printf("Tracking Number: %d\n", k.trackingNumber);
    printf("Timestamp: %llu\n", k.timestamp);
    printf("Original Bid Ref Number: %llu\n", k.originalBidRefNumber);
    printf("Bid Ref Number: %llu\n", k.bidRefNumber);
    printf("Original Ask Ref Number: %llu\n", k.originalAskRefNumber);
    printf("Ask Ref Number: %llu\n", k.askRefNumber);
    printf("Bid Price: %u\n", k.bidPrice);
    printf("Bid Size: %u\n", k.bidSize);
    printf("Ask Price: %u\n", k.askPrice);
    printf("Ask Size: %u\n", k.askSize);

    if (dummy == 0) printf("");
    return 0;
}
