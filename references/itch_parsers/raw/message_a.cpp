#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>      // for ntohl()
#include <mach/mach_time.h> // for mach_absolute_time()

typedef struct {
    char messageType;
    uint16_t trackingNumber;
    uint64_t timestamp;
    uint64_t orderRefNumber;
    char marketSide;
    uint32_t optionId;
    uint16_t price;
    uint16_t volume;
} AddOrderShortMessage;

AddOrderShortMessage parseAddOrderShort(const unsigned char *msg) {
    AddOrderShortMessage a;

    a.messageType = msg[0];
    a.trackingNumber = (msg[1] << 8) | msg[2];
    a.timestamp = ((uint64_t)msg[3] << 40) |
                  ((uint64_t)msg[4] << 32) |
                  ((uint64_t)msg[5] << 24) |
                  ((uint64_t)msg[6] << 16) |
                  ((uint64_t)msg[7] << 8)  |
                  ((uint64_t)msg[8]);

    uint64_t tmp64;
    memcpy(&tmp64, msg + 9, sizeof(tmp64));
    a.orderRefNumber = __builtin_bswap64(tmp64);

    a.marketSide = msg[17];

    uint32_t tmp32;
    memcpy(&tmp32, msg + 18, sizeof(tmp32));
    a.optionId = ntohl(tmp32);

    a.price = (msg[22] << 8) | msg[23];
    a.volume = (msg[24] << 8) | msg[25];

    return a;
}

int main(void) {
    unsigned char message[] = {
        0x61, 0x00, 0x00, 0x13, 0xF8, 0xF6, 0x49, 0x74, 0x92,
        0x00, 0x00, 0x00, 0x00, 0xB2, 0xD0, 0x5E, 0x08,
        0x53, 0x00, 0x02, 0x13, 0x45, 0x00, 0x05, 0x00, 0x08
    };

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const int iterations = 10000000;
    volatile uint64_t dummy = 0;

    uint64_t start = mach_absolute_time();
    AddOrderShortMessage s;
    for (int i = 0; i < iterations; i++) {
        s = parseAddOrderShort(message);
        dummy += s.trackingNumber;
    }
    uint64_t end = mach_absolute_time();

    double elapsed_ns = (double)(end - start) * (double)timebase.numer / (double)timebase.denom;
    double elapsed_per_iter = elapsed_ns / iterations;

    printf("Total elapsed time: %.4f ns\n", elapsed_ns);
    printf("Average per iteration: %.4f ns\n", elapsed_per_iter);

    printf("\n--- Last Parsed Message ---\n");
    printf("Message Type: %c\n", s.messageType);
    printf("Tracking Number: %u\n", s.trackingNumber);
    printf("Timestamp: %llu\n", (unsigned long long)s.timestamp);
    printf("Order Ref Number: %llu\n", (unsigned long long)s.orderRefNumber);
    printf("Market Side: %c\n", s.marketSide);
    printf("Option ID: %u\n", s.optionId);
    printf("Price: %u\n", s.price);
    printf("Volume: %u\n", s.volume);

    if (dummy == 0) printf(""); // prevent compiler optimization
    return 0;
}

