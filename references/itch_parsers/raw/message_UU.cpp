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
    uint64_t originalRefNumber;
    uint64_t newRefNumber;
    uint32_t price;
    uint32_t volume;
} Message;

Message parseMessage(const unsigned char *msg) {
    Message u;
    u.messageType = msg[0];
    u.trackingNumber = (msg[1] << 8) | msg[2];
    
    uint64_t tmp = *(uint64_t*)(msg + 3);
    tmp = __builtin_bswap64(tmp);
    u.timestamp = tmp >> 16;
    
    uint64_t tmp64;
    memcpy(&tmp64, msg + 9, sizeof(tmp64));
    u.originalRefNumber = __builtin_bswap64(tmp64);
    
    memcpy(&tmp64, msg + 17, sizeof(tmp64));
    u.newRefNumber = __builtin_bswap64(tmp64);
    
    uint32_t tmp32;
    memcpy(&tmp32, msg + 25, sizeof(tmp32));
    u.price = ntohl(tmp32);
    
    memcpy(&tmp32, msg + 29, sizeof(tmp32));
    u.volume = ntohl(tmp32);
    
    return u;
}

int main() {
    unsigned char message[] = {
        0x55, 0x00, 0x00, 0x1E, 0xD5, 0x06, 0x50, 0xB1, 0xF6, 
        0x00, 0x00, 0x00, 0x00, 0xB3, 0x28, 0xB0, 0x14, 0x00, 
        0x00, 0x00, 0x00, 0xB3, 0x28, 0xD0, 0xD0, 0x00, 0x64, 
        0xDE, 0x44, 0x00, 0x00, 0x00, 0x04
    };

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const int iterations = 10000000;
    volatile uint64_t dummy = 0;

    uint64_t start = mach_absolute_time();
    Message u;
    for (int i = 0; i < iterations; i++) {
        u = parseMessage(message);
        dummy += u.trackingNumber;
    }
    uint64_t end = mach_absolute_time();

    double elapsed_ns = (double)(end - start) * (double)timebase.numer / (double)timebase.denom;
    double elapsed_per_iter = elapsed_ns / iterations;

    printf("Total elapsed time: %.4f ns\n", elapsed_ns);
    printf("Average per iteration: %.4f ns\n", elapsed_per_iter);
    
    printf("\n--- Last Parsed Message ---\n");
    printf("Message Type: %c\n", u.messageType);
    printf("Tracking Number: %d\n", u.trackingNumber);
    printf("Timestamp: %llu\n", u.timestamp);
    printf("Original Ref Number: %llu\n", u.originalRefNumber);
    printf("New Ref Number: %llu\n", u.newRefNumber);
    printf("Price: %u\n", u.price);
    printf("Volume: %u\n", u.volume);

    if (dummy == 0) printf("");
    return 0;
}
