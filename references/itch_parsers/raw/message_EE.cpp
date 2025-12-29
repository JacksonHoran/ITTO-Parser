
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
    uint32_t crossNumber;
    uint32_t matchNumber;
    char printable;
    uint32_t price;
    uint32_t volume;
} Message;

Message parseMessage(const unsigned char *msg) {
    Message c;
    c.messageType = msg[0];
    c.trackingNumber = (msg[1] << 8) | msg[2];
    
    uint64_t tmp64 = *(uint64_t*)(msg + 3);
    tmp64 = __builtin_bswap64(tmp64);
    c.timestamp = tmp64 >> 16;
    
    memcpy(&tmp64, msg + 9, sizeof(tmp64));
    c.referenceNumber = __builtin_bswap64(tmp64);
    
    uint32_t tmp32;
    memcpy(&tmp32, msg + 17, sizeof(tmp32));
    c.crossNumber = ntohl(tmp32);
    
    memcpy(&tmp32, msg + 21, sizeof(tmp32));
    c.matchNumber = ntohl(tmp32);
    
    c.printable = msg[25];
    
    memcpy(&tmp32, msg + 26, sizeof(tmp32));
    c.price = ntohl(tmp32);
    
    memcpy(&tmp32, msg + 30, sizeof(tmp32));
    c.volume = ntohl(tmp32);
    
    return c;
}

int main() {
    unsigned char message[] = {
        0x43, 0x00, 0x01, 0x1F, 0x1A, 0xD9, 0x82, 0xB4, 0xD4, 
        0x00, 0x00, 0x00, 0x00, 0xB2, 0xD1, 0x89, 0x14, 0x00, 
        0x0F, 0x42, 0x40, 0x00, 0x4C, 0x4B, 0x48, 0x4E, 0x00, 
        0x00, 0x44, 0xC0, 0x00, 0x00, 0x00, 0x01
    };

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const int iterations = 10000000;
    volatile uint64_t dummy = 0;

    uint64_t start = mach_absolute_time();
    Message c;
    for (int i = 0; i < iterations; i++) {
        c = parseMessage(message);
        dummy += c.trackingNumber;
    }
    uint64_t end = mach_absolute_time();

    double elapsed_ns = (double)(end - start) * (double)timebase.numer / (double)timebase.denom;
    double elapsed_per_iter = elapsed_ns / iterations;

    printf("Total elapsed time: %.4f ns\n", elapsed_ns);
    printf("Average per iteration: %.4f ns\n", elapsed_per_iter);
    
    printf("\n--- Last Parsed Message ---\n");
    printf("Message Type: %c\n", c.messageType);
    printf("Tracking Number: %d\n", c.trackingNumber);
    printf("Timestamp: %llu\n", c.timestamp);
    printf("Reference Number: %llu\n", c.referenceNumber);
    printf("Cross Number: %u\n", c.crossNumber);
    printf("Match Number: %u\n", c.matchNumber);
    printf("Printable: %c\n", c.printable);
    printf("Price: %u\n", c.price);
    printf("Volume: %u\n", c.volume);

    if (dummy == 0) printf("");
    return 0;
}


