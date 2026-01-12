ITCH Parser Repo

Structure
- src/: main parser
- bin/: built binaries
- tools/: TCP/UDP utilities
- data/: input files (e.g., ITCH)
- out/: parsed outputs
- references/: message parser references

Build
```
make
```
Run (parse ITCH via local TCP)
```
make run
```
Regenerate a small readable sample
```
make sample
```

Notes
- The ITCH file here uses a 2-byte big-endian length prefix per message.