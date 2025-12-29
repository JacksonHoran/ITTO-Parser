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
    char securitySymbol[6];
    uint8_t expirationYear;
    uint8_t expirationMonth;
    uint8_t expirationDate;
    uint32_t strikePrice;
    char optionType;
    uint8_t source;
    char underlyingSymbol[13];
    char closingType;
    char tradable;
    char mpv;
} Message;

Message parseMessage(const unsigned char *msg) {
    Message r;
    r.messageType = msg[0];
    r.trackingNumber = (msg[1] << 8) | msg[2];
    
    uint64_t tmp = *(uint64_t*)(msg + 3);
    tmp = __builtin_bswap64(tmp);
    r.timestamp = tmp >> 16;
    
    uint32_t tmp32;
    memcpy(&tmp32, msg + 9, sizeof(tmp32));
    r.optionId = ntohl(tmp32);
    
    memcpy(r.securitySymbol, msg + 13, 6);
    r.expirationYear = msg[19];
    r.expirationMonth = msg[20];
    r.expirationDate = msg[21];
    
    memcpy(&tmp32, msg + 22, sizeof(tmp32));
    r.strikePrice = ntohl(tmp32);
    
    r.optionType = msg[26];
    r.source = msg[27];
    memcpy(r.underlyingSymbol, msg + 28, 13);
    r.closingType = msg[41];
    r.tradable = msg[42];
    r.mpv = msg[43];
    
    return r;
}

int main() {
    unsigned char message[] = {
        0x52, 0x00, 0x00, 0x07, 0xD7, 0x96, 0x11, 0x5F, 0x18, 
        0x00, 0x05, 0x3B, 0xA3, 0x45, 0x50, 0x41, 0x4D, 0x20, 
        0x20, 0x17, 0x06, 0x10, 0x00, 0x21, 0x91, 0xC0, 0x43, 
        0x01, 0x45, 0x50, 0x41, 0x4D, 0x20, 0x20, 0x20, 0x20, 
        0x20, 0x20, 0x20, 0x20, 0x20, 0x4E, 0x59, 0x53
    };

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    const int iterations = 10000000;
    volatile uint64_t dummy = 0;

    uint64_t start = mach_absolute_time();
    Message r;
    for (int i = 0; i < iterations; i++) {
        r = parseMessage(message);
        dummy += r.trackingNumber;
    }
    uint64_t end = mach_absolute_time();

    double elapsed_ns = (double)(end - start) * (double)timebase.numer / (double)timebase.denom;
    double elapsed_per_iter = elapsed_ns / iterations;

    printf("Total elapsed time: %.4f ns\n", elapsed_ns);
    printf("Average per iteration: %.4f ns\n", elapsed_per_iter);
    
    printf("\n--- Last Parsed Message ---\n");
    printf("Message Type: %c\n", r.messageType);
    printf("Tracking Number: %d\n", r.trackingNumber);
    printf("Timestamp: %llu\n", r.timestamp);
    printf("Option ID: %u\n", r.optionId);
    printf("Security Symbol: %.6s\n", r.securitySymbol);
    printf("Expiration: %02d/%02d/%02d\n", r.expirationMonth, r.expirationDate, r.expirationYear);
    printf("Strike Price: %u\n", r.strikePrice);
    printf("Option Type: %c\n", r.optionType);
    printf("Source: %u\n", r.source);
    printf("Underlying Symbol: %.13s\n", r.underlyingSymbol);
    printf("Closing Type: %c\n", r.closingType);
    printf("Tradable: %c\n", r.tradable);
    printf("MPV: %c\n", r.mpv);

    if (dummy == 0) printf("");
    return 0;
}
