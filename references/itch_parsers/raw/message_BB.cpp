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
    uint32_t crossNumber;
    uint32_t matchNumber;
} Message;

Message parseMessage(const unsigned char *msg) {
    Message b;
    b.messageType = msg[0];
    b.trackingNumber = (msg[1] << 8) | msg[2];
    
    uint64_t tmp = *(uint64_t*)(msg + 3);
    tmp = __builtin_bswap64(tmp);
    b.timestamp = tmp >> 16;
    
    uint32_t tmp32;
    memcpy(&tmp32, msg + 9, sizeof(tmp32));
    b.crossNumber = ntohl(tmp32);
    
    memcpy(&tmp32, msg + 13, sizeof(tmp32));
    b.matchNumber = ntohl(tmp32);
    
    return b;
}

int main() {
    unsigned char message[] = {
        0x42, 0x00, 0x00, 0x1F, 0x1A, 0xD9, 0x82, 0xB4, 0xD4, 
        0x00, 0x0F, 0x42, 0x40, 0x00, 0x4C, 0x4B, 0x48
    };

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const int iterations = 10000000;
    volatile uint64_t dummy = 0;

    uint64_t start = mach_absolute_time();
    Message b;
    for (int i = 0; i < iterations; i++) {
        b = parseMessage(message);
        dummy += b.trackingNumber;
    }
    uint64_t end = mach_absolute_time();

    double elapsed_ns = (double)(end - start) * (double)timebase.numer / (double)timebase.denom;
    double elapsed_per_iter = elapsed_ns / iterations;

    printf("Total elapsed time: %.4f ns\n", elapsed_ns);
    printf("Average per iteration: %.4f ns\n", elapsed_per_iter);
    
    printf("\n--- Last Parsed Message ---\n");
    printf("Message Type: %c\n", b.messageType);
    printf("Tracking Number: %d\n", b.trackingNumber);
    printf("Timestamp: %llu\n", b.timestamp);
    printf("Cross Number: %u\n", b.crossNumber);
    printf("Match Number: %u\n", b.matchNumber);

    if (dummy == 0) printf("");
    return 0;
}
