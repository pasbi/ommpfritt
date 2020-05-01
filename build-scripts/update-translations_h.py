#!/usr/bin/env python3

import os
import sys
import glob
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

def format_line(disambiguation, text):
    def escape_cpp(text):
        return text.replace("\n", "\\n").replace("\\", "\\\\").replace("\"", "\\\"");
    disambiguation = escape_cpp(disambiguation)
    text = escape_cpp(text)
    return f'QT_TRANSLATE_NOOP("{disambiguation}", "{text}");'

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

    lines = [ format_line(disambiguation, text) for disambiguation, text in items ]
    lines = sorted(lines)

    print(f"Writing translations into '{args.output}'...")
    with open(args.output, 'w') as f:
        f.write(generated_file_header.header())
        for line in lines:
            f.write(line + "\n")
        f.write("\n")
        print("done.")

