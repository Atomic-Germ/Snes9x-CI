#!/usr/bin/env python3
import subprocess
import json
import sys
import os

cli = sys.argv[1] if len(sys.argv) > 1 else 'build/snes9x-cli'
rom = sys.argv[2] if len(sys.argv) > 2 else 'build/tests/data/dummy.sfc'

if not os.path.exists(cli):
    print(f'CLI binary not found: {cli}', file=sys.stderr)
    sys.exit(2)

proc = subprocess.run([cli, '--load', rom, '--run', '1', '--json-output'], capture_output=True, text=True)
print(proc.stdout)
if proc.returncode != 0:
    print('CLI failed', file=sys.stderr)
    print('STDERR:\n' + proc.stderr, file=sys.stderr)
    sys.exit(proc.returncode)

try:
    data = json.loads(proc.stdout)
    if not data.get('success'):
        print('Smoke test reported failure', file=sys.stderr)
        sys.exit(10)
except json.JSONDecodeError:
    print('Invalid JSON from CLI', file=sys.stderr)
    sys.exit(11)

print('Smoke test passed')
sys.exit(0)
