#!/usr/bin/env python3

import re
import os
import argparse
import generated_file_header

def collect(fn):
    with open(fn) as f:
        group = ""
        for line in f.read().splitlines():
            line = line.strip()
            if line.startswith("[") and line.endswith("]"):
                group = line[1:-1]
                yield group, group
            elif len(line) > 0 and not line.startswith('#'):
                item = line.split(":")[0]
                yield group, item

def generate_prefixes(text):
    tokens = re.split(r"\b\.\b", text)
    for i in range(0, len(tokens)):
        prefix = ".".join(tokens[:i+1])
        yield prefix

def format_line(disambiguation, text):
    def escape_cpp(text):
        return text.replace("\n", "\\n").replace("\\", "\\\\").replace("\"", "\\\"");

    disambiguation = escape_cpp(disambiguation)
    for prefix in generate_prefixes(text):
        prefix = escape_cpp(prefix)
        yield f'QT_TRANSLATE_NOOP("{disambiguation}", "{prefix}");'

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True, help="The output file. (should end on '.h')")
    parser.add_argument("--input", required=True, help="The input files.", nargs='+')
    args = parser.parse_args()
    items = set()
    for fn in args.input:
        subdir = os.path.split(os.path.dirname(os.path.abspath(fn)))[1]
        for group, item in collect(fn):
            disambiguation = subdir + "/" + group
            items.add((disambiguation, item))

    lines = set()
    for disambiguation, text in items:
        for line in format_line(disambiguation, text):
            lines.add(line)

    lines = sorted(lines)

    print(f"Writing translations into '{args.output}'...")
    with open(args.output, 'w') as f:
        f.write(generated_file_header.header())
        for line in lines:
            f.write(line + "\n")
        print("done.")
