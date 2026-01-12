#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define RECV_CHUNK 8192
#define INITIAL_BUFFER_CAP (1024 * 1024)

static uint16_t read_u16_be(const unsigned char *p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}

static uint32_t read_u32_be(const unsigned char *p) {
    uint32_t tmp;
    memcpy(&tmp, p, sizeof(tmp));
    return ntohl(tmp);
}

static uint64_t read_u64_be(const unsigned char *p) {
    uint64_t tmp;
    memcpy(&tmp, p, sizeof(tmp));
    return __builtin_bswap64(tmp);
}

static uint64_t read_u48_be(const unsigned char *p) {
    return ((uint64_t)p[0] << 40) |
           ((uint64_t)p[1] << 32) |
           ((uint64_t)p[2] << 24) |
           ((uint64_t)p[3] << 16) |
           ((uint64_t)p[4] << 8) |
           ((uint64_t)p[5]);
}

static void json_write_string(FILE *out, const char *data, size_t len, int trim_spaces) {
    size_t end = len;
    if (trim_spaces) {
        while (end > 0 && data[end - 1] == ' ') {
            end--;
        }
    }

    for (size_t i = 0; i < end; i++) {
        unsigned char c = (unsigned char)data[i];
        if (c == '\\' || c == '"') {
            fputc('\\', out);
            fputc(c, out);
        } else if (c == '\n') {
            fputs("\\n", out);
        } else if (c == '\r') {
            fputs("\\r", out);
        } else if (c == '\t') {
            fputs("\\t", out);
        } else if (c < 0x20 || c > 0x7E) {
            fprintf(out, "\\u%04x", c);
        } else {
            fputc(c, out);
        }
    }
}

static void json_write_price(FILE *out, uint32_t price) {
    uint64_t int_part = price / 10000;
    uint32_t frac = price % 10000;
    if (frac == 0) {
        fprintf(out, "%llu", (unsigned long long)int_part);
        return;
    }
    int width = 4;
    while (frac % 10 == 0) { frac /= 10; width--; }
    /* print fractional part with leading zeros trimmed on the right */
    fprintf(out, "%llu.%0*u", (unsigned long long)int_part, width, (unsigned)frac);
}

static size_t message_length(unsigned char type) {
    switch (type) {
        case 'A': return 36;
        case 'C': return 36;
        case 'D': return 19;
        case 'E': return 31;
        case 'F': return 40;
        case 'I': return 50;
        case 'P': return 44;
        case 'Q': return 40;
        case 'S': return 12;
        case 'U': return 35;
        case 'X': return 23;
        case 'Y': return 20;
        default: return 0;
    }
}

static void write_message_A(FILE *out, const unsigned char *msg) {
    uint16_t stock_locate = read_u16_be(msg + 1);
    uint16_t tracking = read_u16_be(msg + 3);
    uint64_t timestamp = read_u48_be(msg + 5);
    uint64_t order_ref = read_u64_be(msg + 11);
    const char *buy_sell = (const char *)(msg + 19);
    uint32_t shares = read_u32_be(msg + 20);
    const char *stock = (const char *)(msg + 24);
    uint32_t price = read_u32_be(msg + 32);

    fprintf(out,
            "{\"type\":\"A\",\"stock_locate\":%u,\"tracking_number\":%u"
            ",\"timestamp\":%" PRIu64 ",\"order_ref\":%" PRIu64
            ",\"buy_sell\":\"",
            stock_locate, tracking, timestamp, order_ref);
    json_write_string(out, buy_sell, 1, 0);
    fputs("\",\"shares\":", out);
    fprintf(out, "%" PRIu32 ",\"stock\":\"", shares);
    json_write_string(out, stock, 8, 1);
    fputs("\",\"price\":", out);
    json_write_price(out, price);
    fputs("}\n", out);
} 

static void write_message_F(FILE *out, const unsigned char *msg) {
    uint16_t stock_locate = read_u16_be(msg + 1);
    uint16_t tracking = read_u16_be(msg + 3);
    uint64_t timestamp = read_u48_be(msg + 5);
    uint64_t order_ref = read_u64_be(msg + 11);
    const char *buy_sell = (const char *)(msg + 19);
    uint32_t shares = read_u32_be(msg + 20);
    const char *stock = (const char *)(msg + 24);
    uint32_t price = read_u32_be(msg + 32);
    const char *attribution = (const char *)(msg + 36);

    fprintf(out,
            "{\"type\":\"F\",\"stock_locate\":%u,\"tracking_number\":%u"
            ",\"timestamp\":%" PRIu64 ",\"order_ref\":%" PRIu64
            ",\"buy_sell\":\"",
            stock_locate, tracking, timestamp, order_ref);
    json_write_string(out, buy_sell, 1, 0);
    fputs("\",\"shares\":", out);
    fprintf(out, "%" PRIu32 ",\"stock\":\"", shares);
    json_write_string(out, stock, 8, 1);
    fputs("\",\"price\":", out);
    json_write_price(out, price);
    fputs(",\"attribution\":\"", out);
    json_write_string(out, attribution, 4, 1);
    fputs("\"}\n", out);
} 

static void write_message_E(FILE *out, const unsigned char *msg) {
    uint16_t stock_locate = read_u16_be(msg + 1);
    uint16_t tracking = read_u16_be(msg + 3);
    uint64_t timestamp = read_u48_be(msg + 5);
    uint64_t order_ref = read_u64_be(msg + 11);
    uint32_t executed_shares = read_u32_be(msg + 19);
    uint64_t match_number = read_u64_be(msg + 23);

    fprintf(out,
            "{\"type\":\"E\",\"stock_locate\":%u,\"tracking_number\":%u"
            ",\"timestamp\":%" PRIu64 ",\"order_ref\":%" PRIu64
            ",\"executed_shares\":%" PRIu32 ",\"match_number\":%" PRIu64 "}\n",
            stock_locate, tracking, timestamp, order_ref, executed_shares, match_number);
}

static void write_message_C(FILE *out, const unsigned char *msg) {
    uint16_t stock_locate = read_u16_be(msg + 1);
    uint16_t tracking = read_u16_be(msg + 3);
    uint64_t timestamp = read_u48_be(msg + 5);
    uint64_t order_ref = read_u64_be(msg + 11);
    uint32_t executed_shares = read_u32_be(msg + 19);
    uint64_t match_number = read_u64_be(msg + 23);
    const char *printable = (const char *)(msg + 31);
    uint32_t price = read_u32_be(msg + 32);

    fprintf(out,
            "{\"type\":\"C\",\"stock_locate\":%u,\"tracking_number\":%u"
            ",\"timestamp\":%" PRIu64 ",\"order_ref\":%" PRIu64
            ",\"executed_shares\":%" PRIu32 ",\"match_number\":%" PRIu64
            ",\"printable\":\"",
            stock_locate, tracking, timestamp, order_ref, executed_shares, match_number);
    json_write_string(out, printable, 1, 0);
    fputs("\",\"price\":", out);
    json_write_price(out, price);
    fputs("}\n", out);
} 

static void write_message_X(FILE *out, const unsigned char *msg) {
    uint16_t stock_locate = read_u16_be(msg + 1);
    uint16_t tracking = read_u16_be(msg + 3);
    uint64_t timestamp = read_u48_be(msg + 5);
    uint64_t order_ref = read_u64_be(msg + 11);
    uint32_t canceled_shares = read_u32_be(msg + 19);

    fprintf(out,
            "{\"type\":\"X\",\"stock_locate\":%u,\"tracking_number\":%u"
            ",\"timestamp\":%" PRIu64 ",\"order_ref\":%" PRIu64
            ",\"canceled_shares\":%" PRIu32 "}\n",
            stock_locate, tracking, timestamp, order_ref, canceled_shares);
}

static void write_message_D(FILE *out, const unsigned char *msg) {
    uint16_t stock_locate = read_u16_be(msg + 1);
    uint16_t tracking = read_u16_be(msg + 3);
    uint64_t timestamp = read_u48_be(msg + 5);
    uint64_t order_ref = read_u64_be(msg + 11);

    fprintf(out,
            "{\"type\":\"D\",\"stock_locate\":%u,\"tracking_number\":%u"
            ",\"timestamp\":%" PRIu64 ",\"order_ref\":%" PRIu64 "}\n",
            stock_locate, tracking, timestamp, order_ref);
}

static void write_message_U(FILE *out, const unsigned char *msg) {
    uint16_t stock_locate = read_u16_be(msg + 1);
    uint16_t tracking = read_u16_be(msg + 3);
    uint64_t timestamp = read_u48_be(msg + 5);
    uint64_t original_ref = read_u64_be(msg + 11);
    uint64_t new_ref = read_u64_be(msg + 19);
    uint32_t shares = read_u32_be(msg + 27);
    uint32_t price = read_u32_be(msg + 31);

    fprintf(out,
            "{\"type\":\"U\",\"stock_locate\":%u,\"tracking_number\":%u"
            ",\"timestamp\":%" PRIu64 ",\"original_ref\":%" PRIu64
            ",\"new_ref\":%" PRIu64 ",\"shares\":%" PRIu32,
            stock_locate, tracking, timestamp, original_ref, new_ref, shares);
    fputs(",\"price\":", out);
    json_write_price(out, price);
    fputs("}\n", out);
} 

static void write_message_P(FILE *out, const unsigned char *msg) {
    uint16_t stock_locate = read_u16_be(msg + 1);
    uint16_t tracking = read_u16_be(msg + 3);
    uint64_t timestamp = read_u48_be(msg + 5);
    uint64_t order_ref = read_u64_be(msg + 11);
    const char *buy_sell = (const char *)(msg + 19);
    uint32_t shares = read_u32_be(msg + 20);
    const char *stock = (const char *)(msg + 24);
    uint32_t price = read_u32_be(msg + 32);
    uint64_t match_number = read_u64_be(msg + 36);

    fprintf(out,
            "{\"type\":\"P\",\"stock_locate\":%u,\"tracking_number\":%u"
            ",\"timestamp\":%" PRIu64 ",\"order_ref\":%" PRIu64
            ",\"buy_sell\":\"",
            stock_locate, tracking, timestamp, order_ref);
    json_write_string(out, buy_sell, 1, 0);
    fputs("\",\"shares\":", out);
    fprintf(out, "%" PRIu32 ",\"stock\":\"", shares);
    json_write_string(out, stock, 8, 1);
    fputs("\",\"price\":", out);
    json_write_price(out, price);
    fprintf(out, ",\"match_number\":%" PRIu64 "}\n", match_number);
} 

static void write_message_Q(FILE *out, const unsigned char *msg) {
    uint16_t stock_locate = read_u16_be(msg + 1);
    uint16_t tracking = read_u16_be(msg + 3);
    uint64_t timestamp = read_u48_be(msg + 5);
    uint64_t shares = read_u64_be(msg + 11);
    const char *stock = (const char *)(msg + 19);
    uint32_t cross_price = read_u32_be(msg + 27);
    uint64_t match_number = read_u64_be(msg + 31);
    const char *cross_type = (const char *)(msg + 39);

    fprintf(out,
            "{\"type\":\"Q\",\"stock_locate\":%u,\"tracking_number\":%u"
            ",\"timestamp\":%" PRIu64 ",\"shares\":%" PRIu64 ",\"stock\":\"",
            stock_locate, tracking, timestamp, shares);
    json_write_string(out, stock, 8, 1);
    fputs("\",\"cross_price\":", out);
    json_write_price(out, cross_price);
    fprintf(out, ",\"match_number\":%" PRIu64 ",\"cross_type\":\"", match_number);
    json_write_string(out, cross_type, 1, 0);
    fputs("\"}\n", out);
}

static void write_message_I(FILE *out, const unsigned char *msg) {
    uint16_t stock_locate = read_u16_be(msg + 1);
    uint16_t tracking = read_u16_be(msg + 3);
    uint64_t timestamp = read_u48_be(msg + 5);
    uint64_t paired_shares = read_u64_be(msg + 11);
    uint64_t imbalance_shares = read_u64_be(msg + 19);
    const char *imbalance_direction = (const char *)(msg + 27);
    const char *stock = (const char *)(msg + 28);
    uint32_t far_price = read_u32_be(msg + 36);
    uint32_t near_price = read_u32_be(msg + 40);
    uint32_t current_ref_price = read_u32_be(msg + 44);
    const char *cross_type = (const char *)(msg + 48);
    const char *price_variation_indicator = (const char *)(msg + 49);

    fprintf(out,
            "{\"type\":\"I\",\"stock_locate\":%u,\"tracking_number\":%u"
            ",\"timestamp\":%" PRIu64 ",\"paired_shares\":%" PRIu64
            ",\"imbalance_shares\":%" PRIu64 ",\"imbalance_direction\":\"",
            stock_locate, tracking, timestamp, paired_shares, imbalance_shares);
    json_write_string(out, imbalance_direction, 1, 0);
    fputs("\",\"stock\":\"", out);
    json_write_string(out, stock, 8, 1);
    fputs("\",\"far_price\":", out);
    json_write_price(out, far_price);
    fputs(",\"near_price\":", out);
    json_write_price(out, near_price);
    fputs(",\"current_ref_price\":", out);
    json_write_price(out, current_ref_price);
    fputs(",\"cross_type\":\"", out);
    json_write_string(out, cross_type, 1, 0);
    fputs("\",\"price_variation_indicator\":\"", out);
    json_write_string(out, price_variation_indicator, 1, 0);
    fputs("\"}\n", out);
}

static void write_message_S(FILE *out, const unsigned char *msg) {
    uint16_t stock_locate = read_u16_be(msg + 1);
    uint16_t tracking = read_u16_be(msg + 3);
    uint64_t timestamp = read_u48_be(msg + 5);
    const char *event_code = (const char *)(msg + 11);

    fprintf(out,
            "{\"type\":\"S\",\"stock_locate\":%u,\"tracking_number\":%u"
            ",\"timestamp\":%" PRIu64 ",\"event_code\":\"",
            stock_locate, tracking, timestamp);
    json_write_string(out, event_code, 1, 0);
    fputs("\"}\n", out);
}

static void write_message_Y(FILE *out, const unsigned char *msg) {
    uint16_t stock_locate = read_u16_be(msg + 1);
    uint16_t tracking = read_u16_be(msg + 3);
    uint64_t timestamp = read_u48_be(msg + 5);
    const char *stock = (const char *)(msg + 11);
    const char *reg_sho_action = (const char *)(msg + 19);

    fprintf(out,
            "{\"type\":\"Y\",\"stock_locate\":%u,\"tracking_number\":%u"
            ",\"timestamp\":%" PRIu64 ",\"stock\":\"",
            stock_locate, tracking, timestamp);
    json_write_string(out, stock, 8, 1);
    fputs("\",\"reg_sho_action\":\"", out);
    json_write_string(out, reg_sho_action, 1, 0);
    fputs("\"}\n", out);
}

static int parse_and_write(FILE *out, const unsigned char *msg, size_t len) {
    (void)len;
    switch (msg[0]) {
        case 'A': write_message_A(out, msg); return 1;
        case 'C': write_message_C(out, msg); return 1;
        case 'D': write_message_D(out, msg); return 1;
        case 'E': write_message_E(out, msg); return 1;
        case 'F': write_message_F(out, msg); return 1;
        case 'I': write_message_I(out, msg); return 1;
        case 'P': write_message_P(out, msg); return 1;
        case 'Q': write_message_Q(out, msg); return 1;
        case 'S': write_message_S(out, msg); return 1;
        case 'U': write_message_U(out, msg); return 1;
        case 'X': write_message_X(out, msg); return 1;
        case 'Y': write_message_Y(out, msg); return 1;
        default: return 0;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <listen_port> <output_jsonl>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int listen_port = atoi(argv[1]);
    const char *output_filename = argv[2];

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(listen_port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return EXIT_FAILURE;
    }

    if (listen(server_socket, 1) < 0) {
        perror("Listen failed");
        close(server_socket);
        return EXIT_FAILURE;
    }

    printf("Listening on port %d...\n", listen_port);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
    if (client_socket < 0) {
        perror("Accept failed");
        close(server_socket);
        return EXIT_FAILURE;
    }

    FILE *outfile = fopen(output_filename, "wb");
    if (!outfile) {
        perror("Failed to open output file");
        close(client_socket);
        close(server_socket);
        return EXIT_FAILURE;
    }
    setvbuf(outfile, NULL, _IOLBF, 0);

    unsigned char *buffer = malloc(INITIAL_BUFFER_CAP);
    if (!buffer) {
        perror("Buffer allocation failed");
        fclose(outfile);
        close(client_socket);
        close(server_socket);
        return EXIT_FAILURE;
    }
    size_t buffer_len = 0;
    size_t buffer_cap = INITIAL_BUFFER_CAP;

    unsigned char recv_buf[RECV_CHUNK];
    ssize_t bytes_received;
    size_t parsed_messages = 0;
    size_t unknown_messages = 0;

    while ((bytes_received = recv(client_socket, recv_buf, sizeof(recv_buf), 0)) > 0) {
        if (buffer_len + (size_t)bytes_received > buffer_cap) {
            size_t new_cap = buffer_cap;
            while (new_cap < buffer_len + (size_t)bytes_received) {
                new_cap *= 2;
            }
            unsigned char *new_buf = realloc(buffer, new_cap);
            if (!new_buf) {
                perror("Buffer realloc failed");
                break;
            }
            buffer = new_buf;
            buffer_cap = new_cap;
        }

        memcpy(buffer + buffer_len, recv_buf, (size_t)bytes_received);
        buffer_len += (size_t)bytes_received;

        size_t offset = 0;
        while (buffer_len - offset >= 2) {
            uint16_t payload_len = read_u16_be(buffer + offset);
            if (payload_len == 0) {
                offset += 2;
                continue;
            }
            if (buffer_len - offset < (size_t)payload_len + 2) {
                break;
            }

            const unsigned char *payload = buffer + offset + 2;
            size_t expected_len = message_length(payload[0]);
            if (expected_len != 0 && expected_len != payload_len) {
                unknown_messages++;
            }

            if (parse_and_write(outfile, payload, payload_len)) {
                parsed_messages++;
            } else {
                unknown_messages++;
            }
            offset += (size_t)payload_len + 2;
        }

        if (offset > 0) {
            memmove(buffer, buffer + offset, buffer_len - offset);
            buffer_len -= offset;
        }
    }

    if (bytes_received < 0) {
        fprintf(stderr, "Receive error: %s\n", strerror(errno));
    }

    if (buffer_len > 0) {
        fprintf(stderr, "Warning: %zu trailing bytes left unparsed.\n", buffer_len);
    }

    printf("Parsed %zu messages, skipped %zu unknown bytes.\n",
           parsed_messages, unknown_messages);

    free(buffer);
    fclose(outfile);
    close(client_socket);
    close(server_socket);
    return EXIT_SUCCESS;
}
