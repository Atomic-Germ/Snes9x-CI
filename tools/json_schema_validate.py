#!/usr/bin/env python3
"""
Lightweight JSON validator for snes9x-cli output.
Validates either a single JSON object (file contains one JSON document) or a JSONL/NDJSON file (one JSON object per line).
This validator does not require external dependencies and performs basic type checks matching docs/json_schema/output_schema.json.
"""
import sys
import json
from pathlib import Path
from datetime import datetime


def parse_iso8601(s):
    # Accept forms like 2025-09-26T12:34:56Z or with milliseconds 2025-09-26T12:34:56.123Z
    try:
        if s.endswith('Z'):
            # strip Z and parse
            base = s[:-1]
            try:
                return datetime.strptime(base, '%Y-%m-%dT%H:%M:%S.%f')
            except ValueError:
                return datetime.strptime(base, '%Y-%m-%dT%H:%M:%S')
        else:
            return datetime.fromisoformat(s)
    except Exception:
        raise


def validate_obj(obj):
    if not isinstance(obj, dict):
        print('Object is not a JSON object')
        return False
    if 'success' not in obj or not isinstance(obj['success'], bool):
        print('Missing or invalid success field')
        return False
    if 'message' in obj and not isinstance(obj['message'], str):
        print('message must be a string')
        return False
    if 'frames' in obj:
        if not isinstance(obj['frames'], int) or obj['frames'] < 0:
            print('frames must be an integer >= 0')
            return False
    if 'memory' in obj:
        if not isinstance(obj['memory'], list):
            print('memory must be an array')
            return False
        for v in obj['memory']:
            if not isinstance(v, int) or v < 0 or v > 255:
                print('memory values must be integers 0-255')
                return False
    if 'timestamp' in obj:
        if not isinstance(obj['timestamp'], str):
            print('timestamp must be a string')
            return False
        try:
            parse_iso8601(obj['timestamp'])
        except Exception:
            print('timestamp must be valid ISO8601 date-time')
            return False
    return True


def main():
    if len(sys.argv) < 2:
        print('Usage: json_schema_validate.py <file>')
        sys.exit(2)
    p = Path(sys.argv[1])
    if not p.exists():
        print('File not found:', p)
        sys.exit(3)
    text = p.read_text()
    # Try parse as single JSON doc
    try:
        obj = json.loads(text)
        ok = validate_obj(obj)
        if not ok:
            sys.exit(4)
        print('OK')
        sys.exit(0)
    except Exception:
        # Fall back to JSONL
        lines = [l for l in text.splitlines() if l.strip()]
        if not lines:
            print('No JSON lines found')
            sys.exit(5)
        for i, line in enumerate(lines):
            try:
                obj = json.loads(line)
            except Exception as e:
                print(f'Failed to parse JSON on line {i+1}: {e}')
                sys.exit(6)
            if not validate_obj(obj):
                print(f'Validation failed on line {i+1}')
                sys.exit(7)
        print('OK')
        sys.exit(0)

if __name__ == '__main__':
    main()
