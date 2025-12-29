#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <mach/mach_time.h>

typedef struct {
    char messageType;
    int trackingNumber;
    uint64_t timestamp;
    char eventCode;
} Message;

Message parseMessage(const unsigned char *msg) {
    Message s;
    s.messageType = msg[0];
    s.trackingNumber = (msg[1] << 8) | msg[2];
    uint64_t tmp = *(uint64_t*)(msg + 3);
    tmp = __builtin_bswap64(tmp);
    s.timestamp = tmp >> 16;
    s.eventCode = msg[9];
    return s;
}

int main() {
    unsigned char message[] = {
        0x53, 0x00, 0x00, 0x07, 0x3E, 0xE0, 0x35, 0xAE, 0x45, 0x4F
    };

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const int iterations = 10000000; // increase iterations
    volatile uint64_t dummy = 0;      // prevent optimization

    uint64_t start = mach_absolute_time();

    for (int i = 0; i < iterations; i++) {
        Message s = parseMessage(message);
        dummy += s.trackingNumber; // use the result
    }

    uint64_t end = mach_absolute_time();

    double elapsed_ns = (double)(end - start) * (double)timebase.numer / (double)timebase.denom;
    double elapsed_per_iter = elapsed_ns / iterations;

    printf("Total elapsed time: %.4f ns\n", elapsed_ns);
    printf("Average per iteration: %.4f ns\n", elapsed_per_iter);

    // avoid unused variable warning
    if (dummy == 0) printf("");  

    return 0;
}

