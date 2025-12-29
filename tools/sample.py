import json
from itertools import islice

SRC = 'out/parsed.jsonl'
DST = 'out/parsed_sample.json'

items = []
with open(SRC, 'r', encoding='utf-8') as f:
    for line in islice(f, 50):
        line = line.strip()
        if not line:
            continue
        items.append(json.loads(line))

with open(DST, 'w', encoding='utf-8') as f:
    json.dump(items, f, indent=2)
    f.write('\n')

print(f'Wrote {len(items)} records to {DST}')
