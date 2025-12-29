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
    uint64_t originalBidRefNumber;
    uint64_t bidRefNumber;
    uint64_t originalAskRefNumber;
    uint64_t askRefNumber;
    uint32_t bidPrice;
    uint32_t bidSize;
    uint32_t askPrice;
    uint32_t askSize;
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
    
    uint32_t tmp32;
    memcpy(&tmp32, msg + 41, sizeof(tmp32));
    k.bidPrice = ntohl(tmp32);
    
    memcpy(&tmp32, msg + 45, sizeof(tmp32));
    k.bidSize = ntohl(tmp32);
    
    memcpy(&tmp32, msg + 49, sizeof(tmp32));
    k.askPrice = ntohl(tmp32);
    
    memcpy(&tmp32, msg + 53, sizeof(tmp32));
    k.askSize = ntohl(tmp32);
    
    return k;
}

int main() {
    unsigned char message[] = {
        0x4B, 0x00, 0x00, 0x1E, 0xD5, 0x62, 0x3E, 0x27, 0x8C, 
        0x00, 0x00, 0x00, 0x00, 0xB3, 0x28, 0xA5, 0x24, 0x00, 
        0x00, 0x00, 0x00, 0xB3, 0x29, 0xD9, 0xA4, 0x00, 0x00, 
        0x00, 0x00, 0xB3, 0x28, 0xA5, 0x28, 0x00, 0x00, 0x00, 
        0x00, 0xB3, 0x29, 0xD9, 0xA8, 0x00, 0x7E, 0xB1, 0x98, 
        0x00, 0x00, 0x00, 0x05, 0x00, 0x81, 0x61, 0x18, 0x00, 
        0x00, 0x00, 0x05
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
