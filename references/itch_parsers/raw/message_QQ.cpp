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
    uint32_t crossNumber;
    uint32_t matchNumber;
    char crossType;
    uint32_t price;
    uint32_t volume;
} Message;

Message parseMessage(const unsigned char *msg) {
    Message q;
    q.messageType = msg[0];
    q.trackingNumber = (msg[1] << 8) | msg[2];
    
    uint64_t tmp = *(uint64_t*)(msg + 3);
    tmp = __builtin_bswap64(tmp);
    q.timestamp = tmp >> 16;
    
    uint32_t tmp32;
    memcpy(&tmp32, msg + 9, sizeof(tmp32));
    q.optionId = ntohl(tmp32);
    
    memcpy(&tmp32, msg + 13, sizeof(tmp32));
    q.crossNumber = ntohl(tmp32);
    
    memcpy(&tmp32, msg + 17, sizeof(tmp32));
    q.matchNumber = ntohl(tmp32);
    
    q.crossType = msg[21];
    
    memcpy(&tmp32, msg + 22, sizeof(tmp32));
    q.price = ntohl(tmp32);
    
    memcpy(&tmp32, msg + 26, sizeof(tmp32));
    q.volume = ntohl(tmp32);
    
    return q;
}

int main() {
    unsigned char message[] = {
        0x51, 0x00, 0x05, 0x1F, 0x1A, 0xD9, 0x82, 0xB4, 0xD4, 
        0x00, 0x03, 0xD5, 0x59, 0x00, 0x0F, 0x42, 0x40, 0x00, 
        0x4C, 0x4B, 0x58, 0x4F, 0x00, 0x00, 0x44, 0xC0, 0x00, 
        0x00, 0x00, 0x02
    };

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const int iterations = 10000000;
    volatile uint64_t dummy = 0;

    uint64_t start = mach_absolute_time();
    Message q;
    for (int i = 0; i < iterations; i++) {
        q = parseMessage(message);
        dummy += q.trackingNumber;
    }
    uint64_t end = mach_absolute_time();

    double elapsed_ns = (double)(end - start) * (double)timebase.numer / (double)timebase.denom;
    double elapsed_per_iter = elapsed_ns / iterations;

    printf("Total elapsed time: %.4f ns\n", elapsed_ns);
    printf("Average per iteration: %.4f ns\n", elapsed_per_iter);
    
    printf("\n--- Last Parsed Message ---\n");
    printf("Message Type: %c\n", q.messageType);
    printf("Tracking Number: %d\n", q.trackingNumber);
    printf("Timestamp: %llu\n", q.timestamp);
    printf("Option ID: %u\n", q.optionId);
    printf("Cross Number: %u\n", q.crossNumber);
    printf("Match Number: %u\n", q.matchNumber);
    printf("Cross Type: %c\n", q.crossType);
    printf("Price: %u\n", q.price);
    printf("Volume: %u\n", q.volume);

    if (dummy == 0) printf("");
    return 0;
}
