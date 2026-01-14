# NASDAQ ITCH To Trade Options(ITTO) Parser

A low-level NASDAQ ITTO message parser written in C/C++ that ingests a TCP byte stream, reconstructs framed messages, and writes each decoded message as newline-delimited JSON. This project focuses on binary correctness, stream-safe parsing, and fast output suitable for downstream analytics or order book research.
<br>
## What this program does

- Opens a TCP server and listens on a user-specified port
- Accepts a single client connection
- Receives a continuous stream of bytes and reconstructs messages using the ITCH framing:
  - [ITTO Documentation](https://www.nasdaqtrader.com/content/productsservices/trading/optionsmarket/itto_spec40.pdf)
- Decodes supported ITCH message types into structured JSON objects
- Writes one JSON object per line to an output file (**.jsonl**)
<br>
## Supported message types

This parser currently supports the following ITCH message types:

| Type | Meaning (common ITCH names) |
|------|------------------------------|
| `A`  | Add Order (No MPID Attribution) |
| `F`  | Add Order (With MPID Attribution) |
| `E`  | Order Executed |
| `C`  | Order Executed (With Price) |
| `X`  | Order Cancel |
| `D`  | Order Delete |
| `U`  | Order Replace |
| `P`  | Trade (Non-Cross) |
| `Q`  | Cross Trade |
| `I`  | NOII (Net Order Imbalance Indicator) |
| `S`  | System Event |
| `Y`  | Reg SHO Restriction |

If a message type is unknown or the payload length does not match the expected length for that type, it is counted as unknown and skipped. I am currently working on getting all message types implemented.
<br>

## Output format (JSON/JSONL)

Each decoded message is written as a single JSON object, followed by `\n`.

Example (formatted here for readability; actual output is one line per message):

```json
{
    "type": "A",
    "stock_locate": 1143,
    "tracking_number": 0,
    "timestamp": 34200000109536,
    "order_ref": 9798077,
    "buy_sell": "S",
    "shares": 1,
    "stock": "CACC",
    "price": 410.21
}
```
<br>
<br>

# Build and Run

### Build:
```
make
```
### Run Full Program:
```
make run
```
### Run Sample (for testing purposes):
```
make sample 
```
<br>

# Contact Me:
Heres a link to my resume LinkedIn and my email addresses:

[**LinkedIn**](https://www.linkedin.com/in/jacksonhoran/)

[**Resume**](https://drive.google.com/file/d/1EZzWBPda_TKml6mh_Cz_y4dXWE3T1b4_/view?usp=sharing)

**Personal Email:** JacksonHoran1@gmail.com

**School Email:** Jhoran1@luc.edu
