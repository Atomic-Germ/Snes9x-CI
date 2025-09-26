#!/usr/bin/env python3
import sys
import json
from pathlib import Path

if len(sys.argv) < 2:
    print('Usage: check_output_file.py <file> [key] [expected]')
    sys.exit(2)

p = Path(sys.argv[1])
if not p.exists():
    print('File not found:', p)
    sys.exit(3)

text = p.read_text()
# Try parse last JSON line
lines = [l for l in text.splitlines() if l.strip()]
if not lines:
    print('No content in file')
    sys.exit(4)
last = lines[-1]
try:
    obj = json.loads(last)
except Exception as e:
    print('Failed to parse JSON:', e)
    sys.exit(5)

if len(sys.argv) >= 4:
    key = sys.argv[2]
    expected = sys.argv[3]
    actual = str(obj.get(key))
    if actual != expected:
        print(f'Assertion failed: key {key} expected {expected} actual {actual}')
        sys.exit(6)

print('OK')
sys.exit(0)
