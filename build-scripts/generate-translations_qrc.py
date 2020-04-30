#!/usr/bin/env python3

import os
import argparse

def generate_qrc(items):
    lines = []
    lines.append('<RCC version="1.0">')
    lines.append('    <qresource prefix="/qm">')
    ind = ' '*8
    for item in items:
        lines.append(ind + f'<file>{item}.qm</file>')
    lines.append('    </qresource>')
    lines.append('</RCC>')
    lines.append('')
    return "\n".join(lines)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--languages',
            help="The languages (e.g., 'de' or 'en')",
            nargs="+",
            required=True)
    parser.add_argument('--prefixes',
            help="The files to consider (e.g., 'qt' or 'qtbase')",
            nargs="+",
            required=True)
    parser.add_argument('--qrc',
            help="The qrc output file",
            required=True)
    args = parser.parse_args()
    
    args.qrc = os.path.abspath(args.qrc)

    items = []
    for prefix in args.prefixes:
        for language in args.languages:
            items.append(f"{prefix}_{language}")

    with open(args.qrc, 'w') as f:
        f.write(generate_qrc(items))

