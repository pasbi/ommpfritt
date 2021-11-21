#!/usr/bin/env python3

import os
from glob import glob
import subprocess
import argparse


def is_blacklisted(fn):
    bad_patterns = ['src/external', 'test/external', 'test/unit']
    return any(bad_pattern in fn for bad_pattern in bad_patterns)

exts = ['cpp', 'h']

def all_build_files():
    files = [fn for ext in exts for fn in glob(f'**/*.{ext}', recursive=True)]
    return [fn for fn in files if not is_blacklisted(fn)]

def changed_build_files():
    command = ["git", "diff", "--name-only", "--diff-filter=d", "main..HEAD"]
    proc = subprocess.run(command, capture_output=True)
    files = proc.stdout.decode('utf-8').split('\n')
    def is_considered(fn):
        return not is_blacklisted(fn) and any(fn.endswith(ext) for ext in exts)
    return [fn for fn in files if is_considered(fn)]

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--path", help="Set the working directory.")
    parser.add_argument("--mode", choices=["all", "changed"], required=True)

    args = parser.parse_args()

    if args.path is not None:
        os.chdir(args.path)

    method = {"all": all_build_files, "changed": changed_build_files}[args.mode]
    for f in method():
        print(f)
