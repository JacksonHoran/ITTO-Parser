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
    char openState;
} Message;

Message parseMessage(const unsigned char *msg) {
    Message o;
    o.messageType = msg[0];
    o.trackingNumber = (msg[1] << 8) | msg[2];
    
    uint64_t tmp = *(uint64_t*)(msg + 3);
    tmp = __builtin_bswap64(tmp);
    o.timestamp = tmp >> 16;
    
    uint32_t tmp32;
    memcpy(&tmp32, msg + 9, sizeof(tmp32));
    o.optionId = ntohl(tmp32);
    
    o.openState = msg[13];
    
    return o;
}

int main() {
    unsigned char message[] = {
        0x4F, 0x00, 0x05, 0x1F, 0x1A, 0xD9, 0x82, 0xB4, 0xD4, 
        0x00, 0x03, 0xD5, 0x59, 0x59
    };

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const int iterations = 10000000;
    volatile uint64_t dummy = 0;

    uint64_t start = mach_absolute_time();
    Message o;
    for (int i = 0; i < iterations; i++) {
        o = parseMessage(message);
        dummy += o.trackingNumber;
    }
    uint64_t end = mach_absolute_time();

    double elapsed_ns = (double)(end - start) * (double)timebase.numer / (double)timebase.denom;
    double elapsed_per_iter = elapsed_ns / iterations;

    printf("Total elapsed time: %.4f ns\n", elapsed_ns);
    printf("Average per iteration: %.4f ns\n", elapsed_per_iter);
    
    printf("\n--- Last Parsed Message ---\n");
    printf("Message Type: %c\n", o.messageType);
    printf("Tracking Number: %d\n", o.trackingNumber);
    printf("Timestamp: %llu\n", o.timestamp);
    printf("Option ID: %u\n", o.optionId);
    printf("Open State: %c\n", o.openState);

    if (dummy == 0) printf("");
    return 0;
}
