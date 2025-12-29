#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <mach/mach_time.h>
#include <arpa/inet.h> // for ntoh

typedef struct {
    char messageType;           // 1 byte alpha # you can determine this from documentation
    int trackingNumber;         // 2-byte unsigned int # for 2, just link the bytes manually
    uint64_t timestamp;         // 6-byte unsigned int # trick
    int auctionID;              // 4-byte unsigned int # cpu has a tool to link
    char auctionType;           // 1-byte alpha # cpu has a tool to link
    int pairedContracts;        // 4-byte unsigned int
    char imbalanceDirection;    // 1-byte alpha
    int optionId;               // 4 byte unsigned int # you can determine this from documentation
    int imbalancePrice;         // 4-byte unsigned int # for 2, just link the bytes manually
    int imbalanceVolume;        // 4-byte unsigned int # trick
    char customerfirmIndicator; // 1-byte char 
    int reserved;               // 3-byte empty 

} Message;

Message parseMessage(const unsigned char *msg) {
	Message I;

	I.messageType = msg[0];
	I.trackingNumber = (msg[1] << 8) | msg[2];





















 
