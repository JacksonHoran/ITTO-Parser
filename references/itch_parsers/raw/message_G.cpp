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
    uint64_t referenceNumber;
    char changeReason;
    uint32_t price;
    uint32_t volume;
} Message;

Message parseMessage(const unsigned char *msg) {
    Message g;
    g.messageType = msg[0];
    g.trackingNumber = (msg[1] << 8) | msg[2];
    
    uint64_t tmp = *(uint64_t*)(msg + 3);
    tmp = __builtin_bswap64(tmp);
    g.timestamp = tmp >> 16;
    
    uint64_t tmp64;
    memcpy(&tmp64, msg + 9, sizeof(tmp64));
    g.referenceNumber = __builtin_bswap64(tmp64);
    
    g.changeReason = msg[17];
    
    uint32_t tmp32;
    memcpy(&tmp32, msg + 18, sizeof(tmp32));
    g.price = ntohl(tmp32);
    
    memcpy(&tmp32, msg + 22, sizeof(tmp32));
    g.volume = ntohl(tmp32);
    
    return g;
}

int main() {
    unsigned char message[] = {
        0x47, 0x00, 0x00, 0x1E, 0xD5, 0x62, 0x15, 0x33, 0xF8, 
        0x00, 0x00, 0x00, 0x00, 0xB3, 0x28, 0x80, 0x98, 0x55, 
        0x00, 0x0B, 0xEA, 0xC8, 0x00, 0x00, 0x00, 0x01
    };

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const int iterations = 10000000;
    volatile uint64_t dummy = 0;

    uint64_t start = mach_absolute_time();
    Message g;
    for (int i = 0; i < iterations; i++) {
        g = parseMessage(message);
        dummy += g.trackingNumber;
    }
    uint64_t end = mach_absolute_time();

    double elapsed_ns = (double)(end - start) * (double)timebase.numer / (double)timebase.denom;
    double elapsed_per_iter = elapsed_ns / iterations;

    printf("Total elapsed time: %.4f ns\n", elapsed_ns);
    printf("Average per iteration: %.4f ns\n", elapsed_per_iter);
    
    printf("\n--- Last Parsed Message ---\n");
    printf("Message Type: %c\n", g.messageType);
    printf("Tracking Number: %d\n", g.trackingNumber);
    printf("Timestamp: %llu\n", g.timestamp);
    printf("Reference Number: %llu\n", g.referenceNumber);
    printf("Change Reason: %c\n", g.changeReason);
    printf("Price: %u\n", g.price);
    printf("Volume: %u\n", g.volume);

    if (dummy == 0) printf("");
    return 0;
}
