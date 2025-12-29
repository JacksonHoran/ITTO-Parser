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
    uint32_t cancelledContracts;
} Message;

Message parseMessage(const unsigned char *msg) {
    Message x;
    x.messageType = msg[0];
    x.trackingNumber = (msg[1] << 8) | msg[2];
    
    uint64_t tmp = *(uint64_t*)(msg + 3);
    tmp = __builtin_bswap64(tmp);
    x.timestamp = tmp >> 16;
    
    uint64_t tmp64;
    memcpy(&tmp64, msg + 9, sizeof(tmp64));
    x.orderRefNumber = __builtin_bswap64(tmp64);
    
    uint32_t tmp32;
    memcpy(&tmp32, msg + 17, sizeof(tmp32));
    x.cancelledContracts = ntohl(tmp32);
    
    return x;
}

int main() {
    unsigned char message[] = {
        0x58, 0x00, 0x01, 0x1F, 0x1C, 0x04, 0x0B, 0x45, 0x1C, 
        0x00, 0x00, 0x00, 0x00, 0xB3, 0x7B, 0x95, 0xDC, 0x00, 
        0x00, 0x00, 0x03
    };

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const int iterations = 10000000;
    volatile uint64_t dummy = 0;

    uint64_t start = mach_absolute_time();
    Message x;
    for (int i = 0; i < iterations; i++) {
        x = parseMessage(message);
        dummy += x.trackingNumber;
    }
    uint64_t end = mach_absolute_time();

    double elapsed_ns = (double)(end - start) * (double)timebase.numer / (double)timebase.denom;
    double elapsed_per_iter = elapsed_ns / iterations;

    printf("Total elapsed time: %.4f ns\n", elapsed_ns);
    printf("Average per iteration: %.4f ns\n", elapsed_per_iter);
    
    printf("\n--- Last Parsed Message ---\n");
    printf("Message Type: %c\n", x.messageType);
    printf("Tracking Number: %d\n", x.trackingNumber);
    printf("Timestamp: %llu\n", x.timestamp);
    printf("Order Ref Number: %llu\n", x.orderRefNumber);
    printf("Cancelled Contracts: %u\n", x.cancelledContracts);

    if (dummy == 0) printf("");
    return 0;
}
