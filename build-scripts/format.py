#!/usr/bin/env python3

import sys
import tempfile
import argparse
import subprocess


parser = argparse.ArgumentParser(description="Apply the style guide.")
parser.add_argument("--clang-format-exec", default="clang-format",
                    help="The clang-format executable.")
parser.add_argument("--files", required=True, nargs='+',
                    help="The files to check")
parser.add_argument("--apply", action='store_true', default=False,
                    help="Apply the proposals.")
args = parser.parse_args()

def format_file(fn):
    clang_format_command = [args.clang_format_exec, fn]
    process = subprocess.run(clang_format_command, capture_output=True)
    if process.returncode != 0:
        print(f"{command} exited with {process.returncode}")
    else:
        formatted_file = tempfile.NamedTemporaryFile()
        formatted_file.write(process.stdout)

        diff_command = ['diff',
                        '--old-line-format=%5dn < %L',
                        '--new-line-format=%5dn > %L',
                        '--unchanged-line-format=',
                        fn,
                        formatted_file.name ]
        process = subprocess.run(diff_command, capture_output=True)
        returncode = process.returncode
        if returncode == 0:
            pass  # files are same
        elif returncode == 1:
            print(f"\n{fn} is not correctly formatted:")
            print(process.stdout.decode("utf-8"))
            return fn
        elif returncode == 2:
            print("Error computing diff.")

if args.apply:
    for fn in args.files:
        command = [args.clang_format_exec, "-i", fn]
        returncode = subprocess.run(command).returncode
        if returncode != 0:
            print(f"{command} exited with {returncode}")
            sys.exit(1)
        else:
            sys.exit(0)
else:
    bad_fns = [format_file(fn) for fn in args.files]
    bad_fns = [fn for fn in bad_fns if fn is not None]
    if len(bad_fns) == 0:
        print("Nothing to do.")
        sys.exit(0)
    else:
        print(f"There are {len(bad_fns)} files:")
        for fn in bad_fns:
            print(fn)
        sys.exit(1)
