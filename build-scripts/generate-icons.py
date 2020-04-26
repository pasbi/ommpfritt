#!/usr/bin/env python3

import os
import json
import sys
import argparse
import subprocess

def generate_qrc(items):
    lines = []
    lines.append('<RCC>')
    lines.append('    <qresource prefix="/icons">')
    ind = ' '*8
    for item in items:
        lines.append(ind + f'<file>{item}.png</file>')
    lines.append('    </qresource>')
    lines.append('</RCC>')
    lines.append('')
    return "\n".join(lines)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--specs',
            help="The spec input files",
            nargs="+",
            required=True)
    parser.add_argument('--qrc',
            help="The qrc output file",
            required=True)
    parser.add_argument('--command',
            help="The ommpfritt-cli executable.",
            required=True)
    parser.add_argument('--scenefile',
            help="The scene file that defines the icons.",
            required=True)
    parser.add_argument('--output',
            help="The output directory.",
            required=True)
    args = parser.parse_args()
    
    args.output = os.path.abspath(args.output)
    args.qrc = os.path.abspath(args.qrc)

    if os.path.dirname(args.qrc) != args.output:
        print("qrc file must be in the output directory, but:")
        print(f"qrc filename: {args.qrc}")
        print(f"output directory: {args.output}")
        sys.exit(1)

    items = []
    for fn in args.specs:
        with open(fn, 'r') as f:
            spec = json.load(f)
        category = spec["category"]
        for item in spec["items"]:
            items.append((category, item))

    with open(args.qrc, 'w') as f:
        f.write(generate_qrc([item for _, item in items]))
    
    for category, item in items:
        command = [
            args.command,
            "render",
            "-f", args.scenefile,
            "-V", "view",
            "-w", "128",
            "-p", f"_root_/{category}/{item}$",
            "-o", f"{args.output}/{item}.png",
            "-y"
        ]
        cp = subprocess.run(command, capture_output=True)
        print(f"Render icon for {category}/{item} ...", end="")
        if cp.returncode != 0:
            print(f" failed with code {cp.returncode}.")
            print("Command was:")
            print(cp.args)
            print("Message:")
            print(cp.stderr)
            exit(1)
        else:
            print(" done.")

