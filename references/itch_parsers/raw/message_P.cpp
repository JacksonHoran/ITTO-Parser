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
    char buySellIndicator;
    uint32_t optionId;
    uint32_t crossNumber;
    uint32_t matchNumber;
    uint32_t price;
    uint32_t volume;
} Message;

Message parseMessage(const unsigned char *msg) {
    Message p;
    p.messageType = msg[0];
    p.trackingNumber = (msg[1] << 8) | msg[2];
    
    uint64_t tmp64 = *(uint64_t*)(msg + 3);
    tmp64 = __builtin_bswap64(tmp64);
    p.timestamp = tmp64 >> 16;
    
    p.buySellIndicator = msg[9];
    
    uint32_t tmp32;
    memcpy(&tmp32, msg + 10, sizeof(tmp32));
    p.optionId = ntohl(tmp32);
    
    memcpy(&tmp32, msg + 14, sizeof(tmp32));
    p.crossNumber = ntohl(tmp32);
    
    memcpy(&tmp32, msg + 18, sizeof(tmp32));
    p.matchNumber = ntohl(tmp32);
    
    memcpy(&tmp32, msg + 22, sizeof(tmp32));
    p.price = ntohl(tmp32);
    
    memcpy(&tmp32, msg + 26, sizeof(tmp32));
    p.volume = ntohl(tmp32);
    
    return p;
}

int main() {
    unsigned char message[] = {
        0x50, 0x00, 0x00, 0x1F, 0x1A, 0xD9, 0x82, 0xB4, 0xD4, 
        0x42, 0x00, 0x03, 0xD5, 0x59, 0x00, 0x0F, 0x42, 0x40, 
        0x00, 0x4C, 0x4B, 0x48, 0x00, 0x00, 0x44, 0xC0, 0x00, 
        0x00, 0x00, 0x05
    };

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const int iterations = 10000000;
    volatile uint64_t dummy = 0;

    uint64_t start = mach_absolute_time();
    Message p;
    for (int i = 0; i < iterations; i++) {
        p = parseMessage(message);
        dummy += p.trackingNumber;
    }
    uint64_t end = mach_absolute_time();

    double elapsed_ns = (double)(end - start) * (double)timebase.numer / (double)timebase.denom;
    double elapsed_per_iter = elapsed_ns / iterations;

    printf("Total elapsed time: %.4f ns\n", elapsed_ns);
    printf("Average per iteration: %.4f ns\n", elapsed_per_iter);
    
    printf("\n--- Last Parsed Message ---\n");
    printf("Message Type: %c\n", p.messageType);
    printf("Tracking Number: %d\n", p.trackingNumber);
    printf("Timestamp: %llu\n", p.timestamp);
    printf("Buy/Sell Indicator: %c\n", p.buySellIndicator);
    printf("Option ID: %u\n", p.optionId);
    printf("Cross Number: %u\n", p.crossNumber);
    printf("Match Number: %u\n", p.matchNumber);
    printf("Price: %u\n", p.price);
    printf("Volume: %u\n", p.volume);

    if (dummy == 0) printf("");
    return 0;
}
