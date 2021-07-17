#!/usr/bin/env python3

import argparse
import clazychecks
import subprocess
import files
import json
import sys


parser = argparse.ArgumentParser(description='Perform Clazy Checks')
parser.add_argument('--clazy-executable', default='clazy')
parser.add_argument('--mode', choices=["changed", "all", "manual"])
parser.add_argument('--compile-commands', required=True,
                    help="Path of the compile_commands.json file in your build directory.")
parser.add_argument('files', nargs='*')

args = parser.parse_args()

if args.mode == "changed":
    files = files.changed_build_files()
    if len(args.files) > 0:
        print("Warning: Ignoring manually specified files.")
elif args.mode == "all":
    files = files.all_build_files()
    if len(args.files) > 0:
        print("Warning: Ignoring manually specified files.")
elif args.mode == "manual":
    files = args.files
else:
    assert False, f"Unexpected mode: {args.files}"

with open(args.compile_commands) as f:
    compile_commands = json.load(f)
    compilers = set(cmd['command'].split(' ', 1)[0] for cmd in compile_commands)
    if any('gcc' in compiler or 'g++' in compiler for compiler in compilers):
        print("Warning: The compile_commands.json must be build with a clang compiler.")
        sys.exit(1)

command = [
        args.clazy_executable,
        "--standalone",
        "--ignore-included-files",
        f"-checks={clazychecks.checks}",
        "-p", args.compile_commands,
        *files
]

proc = subprocess.run(command)
sys.exit(proc.returncode)
