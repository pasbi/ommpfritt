#!/usr/bin/env python3

import os
import sys
import glob

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

def format(disambiguation, text):
    def escape_cpp(text):
        return text.replace("\n", "\\n").replace("\\", "\\\\").replace("\"", "\\\"");
    disambiguation = escape_cpp(disambiguation)
    text = escape_cpp(text)
    return f'QT_TRANSLATE_NOOP("{disambiguation}", "{text}");'

if __name__ == "__main__":
    items = set()
    base_path = os.path.dirname(sys.argv[0])
    for fn in glob.glob(base_path + '/**/*.cfg', recursive=True):
        subdir = os.path.split(os.path.dirname(os.path.abspath(fn)))[1]
        for group, item in collect(fn):
            disambiguation = subdir + "/" + group
            items.add((disambiguation, item))

    lines = [ format(disambiguation, text) for disambiguation, text in items ]
    lines = sorted(lines)

    with open(base_path + "/src/translations.h", 'w') as f:
        for line in lines:
            f.write(line + "\n")



