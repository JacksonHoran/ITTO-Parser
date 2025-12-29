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
    uint64_t bidRefNumber;
    uint64_t askRefNumber;
    uint32_t optionId;
    uint16_t bidPrice;
    uint16_t bidSize;
    uint16_t askPrice;
    uint16_t askSize;
} Message;

Message parseMessage(const unsigned char *msg) {
    Message j;
    j.messageType = msg[0];
    j.trackingNumber = (msg[1] << 8) | msg[2];
    
    uint64_t tmp = *(uint64_t*)(msg + 3);
    tmp = __builtin_bswap64(tmp);
    j.timestamp = tmp >> 16;
    
    uint64_t tmp64;
    memcpy(&tmp64, msg + 9, sizeof(tmp64));
    j.bidRefNumber = __builtin_bswap64(tmp64);
    
    memcpy(&tmp64, msg + 17, sizeof(tmp64));
    j.askRefNumber = __builtin_bswap64(tmp64);
    
    uint32_t tmp32;
    memcpy(&tmp32, msg + 25, sizeof(tmp32));
    j.optionId = ntohl(tmp32);
    
    j.bidPrice = (msg[29] << 8) | msg[30];
    j.bidSize = (msg[31] << 8) | msg[32];
    j.askPrice = (msg[33] << 8) | msg[34];
    j.askSize = (msg[35] << 8) | msg[36];
    
    return j;
}

int main() {
    unsigned char message[] = {
        0x6A, 0x00, 0x00, 0x1E, 0xD4, 0xF5, 0x7D, 0xBD, 0xA2, 
        0x00, 0x00, 0x00, 0x00, 0xB3, 0x28, 0x53, 0x68, 0x00, 
        0x00, 0x00, 0x00, 0xB3, 0x28, 0x53, 0x6C, 0x00, 0x01, 
        0xE3, 0xC1, 0x00, 0x78, 0x00, 0x01, 0x02, 0x6C, 0x00, 
        0x01
    };

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const int iterations = 10000000;
    volatile uint64_t dummy = 0;

    uint64_t start = mach_absolute_time();
    Message j;
    for (int i = 0; i < iterations; i++) {
        j = parseMessage(message);
        dummy += j.trackingNumber;
    }
    uint64_t end = mach_absolute_time();

    double elapsed_ns = (double)(end - start) * (double)timebase.numer / (double)timebase.denom;
    double elapsed_per_iter = elapsed_ns / iterations;

    printf("Total elapsed time: %.4f ns\n", elapsed_ns);
    printf("Average per iteration: %.4f ns\n", elapsed_per_iter);
    
    printf("\n--- Last Parsed Message ---\n");
    printf("Message Type: %c\n", j.messageType);
    printf("Tracking Number: %d\n", j.trackingNumber);
    printf("Timestamp: %llu\n", j.timestamp);
    printf("Bid Ref Number: %llu\n", j.bidRefNumber);
    printf("Ask Ref Number: %llu\n", j.askRefNumber);
    printf("Option ID: %u\n", j.optionId);
    printf("Bid Price: %u\n", j.bidPrice);
    printf("Bid Size: %u\n", j.bidSize);
    printf("Ask Price: %u\n", j.askPrice);
    printf("Ask Size: %u\n", j.askSize);

    if (dummy == 0) printf("");
    return 0;
}
