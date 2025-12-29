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
    uint64_t orderRefNumber;
    char marketSide;
    uint32_t optionId;
    uint32_t price;
    uint32_t volume;
} Message;

Message parseMessage(const unsigned char *msg) {
    Message a;
    a.messageType = msg[0];
    a.trackingNumber = (msg[1] << 8) | msg[2];
    
    uint64_t tmp = *(uint64_t*)(msg + 3);
    tmp = __builtin_bswap64(tmp);
    a.timestamp = tmp >> 16;
    
    uint64_t tmp64;
    memcpy(&tmp64, msg + 9, sizeof(tmp64));
    a.orderRefNumber = __builtin_bswap64(tmp64);
    
    a.marketSide = msg[17];
    
    uint32_t tmp32;
    memcpy(&tmp32, msg + 18, sizeof(tmp32));
    a.optionId = ntohl(tmp32);
    
    memcpy(&tmp32, msg + 22, sizeof(tmp32));
    a.price = ntohl(tmp32);
    
    memcpy(&tmp32, msg + 26, sizeof(tmp32));
    a.volume = ntohl(tmp32);
    
    return a;
}

int main() {
    unsigned char message[] = {
        0x41, 0x00, 0x00, 0x1B, 0xBB, 0xD2, 0x33, 0x22, 0xBD, 
        0x00, 0x00, 0x00, 0x00, 0xB2, 0xD1, 0x42, 0xF0, 0x53, 
        0x00, 0x00, 0x0D, 0x51, 0x00, 0x7A, 0x25, 0x88, 0x00, 
        0x00, 0x00, 0x01
    };

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const int iterations = 10000000;
    volatile uint64_t dummy = 0;

    uint64_t start = mach_absolute_time();
    Message a;
    for (int i = 0; i < iterations; i++) {
        a = parseMessage(message);
        dummy += a.trackingNumber;
    }
    uint64_t end = mach_absolute_time();

    double elapsed_ns = (double)(end - start) * (double)timebase.numer / (double)timebase.denom;
    double elapsed_per_iter = elapsed_ns / iterations;

    printf("Total elapsed time: %.4f ns\n", elapsed_ns);
    printf("Average per iteration: %.4f ns\n", elapsed_per_iter);
    
    printf("\n--- Last Parsed Message ---\n");
    printf("Message Type: %c\n", a.messageType);
    printf("Tracking Number: %d\n", a.trackingNumber);
    printf("Timestamp: %llu\n", a.timestamp);
    printf("Order Ref Number: %llu\n", a.orderRefNumber);
    printf("Market Side: %c\n", a.marketSide);
    printf("Option ID: %u\n", a.optionId);
    printf("Price: %u\n", a.price);
    printf("Volume: %u\n", a.volume);

    if (dummy == 0) printf("");
    return 0;
}
