#!/usr/bin/env python3

import functools
import multiprocessing
import os
import sys
import formatchecker
import argparse
import subprocess
import clazychecks


RETURN_NO_ISSUES = 0
RETURN_DETECTED_ISSUES = 1
RETURN_INTERNAL_ERROR = 2


def find_files(mode):
    cwd = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), ".."))

    if mode == "none":
        return []
    if mode == "all":
        command = ['find', 'src', '-type', 'f']
    elif mode == "diff":
        command = ['git', 'diff', '--name-only', '--diff-filter=d', 'origin/master..HEAD']
    else:
        raise AssertionError(f"Unexpected mode {mode}.")

    proc = subprocess.run(command, capture_output=True, text=True, cwd=cwd)
    if proc.returncode != 0:
        print(f"Failed to find files ({proc.returncode})")
        print(proc.stderr)
        sys.exit(RETURN_INTERNAL_ERROR)

    def file_predicate(filename):
        if not filename.startswith("src/"):
            return False
        elif filename.startswith("src/external/"):
            return False
        elif filename in ("src/translations.h", "src/registers.cpp"):
            return False
        else:
            return True

    return [os.path.join(cwd, f) for f in proc.stdout.split("\n") if file_predicate(f)]

def check_clazy_single_file(filename, clazy_executable, build_dir, fix):
    command = [clazy_executable, "--standalone", "--ignore-included-files",
               "-p", os.path.join(build_dir, "compile_commands.json"),
               "-checks=" + clazychecks.checks, filename]
    if fix:
        command += ["--fix"]

    proc = subprocess.run(command)
    return proc.returncode == 0

def run_clazy(files, build_dir, clazy_executable, fix):
    worker = functools.partial(check_clazy_single_file,
                               clazy_executable=clazy_executable,
                               build_dir=build_dir,
                               fix=fix)
    files = [fn for fn in files if fn.endswith(".cpp") or fn.endswith(".h")]
    if len(files) == 0:
        print("Skip clazy: no relevant files.")
        return True
    else:
        with multiprocessing.Pool() as pool:
            results = pool.map(worker, files)
            return all(results)

def run_clang_tidy(files, build_dir, clang_tidy_binary, run_clang_executable, fix):
    command = [run_clang_executable,
               "-clang-tidy-binary", clang_tidy_binary,
               "-p", build_dir]
    if fix:
        command += ["-fix"]

    if not isinstance(files, list):
        raise AssertionError("files must be a list.")
    if len(files) == 0:
        print("Skip clang-tidy: no relevant files.")
        return True
    else:
        proc = subprocess.run(command + files)
        return proc.returncode == 0

def check_args_sanity(args):
    if not args.skip_clazy or not args.skip_clang_tidy:
        if args.build_dir is None:
            print("--build-dir must be present if clazy or clang are used.")
            sys.exit(RETURN_INTERNAL_ERROR)

    if args.mode == 'none' and args.files is None:
        print("--files must be present if --mode is none")
        sys.exit(RETURN_INTERNAL_ERROR)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", choices=['none', 'all', 'diff'], default="diff",
                        help="Test all files or only those that have been modified w.r.t. master branch.")
    parser.add_argument("--files", nargs='*', default=None, help="Additional files to check.")
    parser.add_argument("--build-dir", required=False,
                        help="The build directory, should contain a compile_commands.json file. "
                             "This argument is not required if clazy and clang are skipped.")
    parser.add_argument("--skip-clazy", action="store_true", help="Don't run the clazy checker.")
    parser.add_argument("--skip-clang-tidy", action="store_true", help="Don't run clang-tidy.")
    parser.add_argument("--skip-format", action="store_true", help="Don't check formatting.")

    parser.add_argument("--clang-tidy-binary", default="clang-tidy")
    parser.add_argument("--run-clang-tidy-executable", default="run-clang-tidy")
    parser.add_argument("--clazy-executable", default="clazy")
    parser.add_argument("--fix", action="store_true",
                        help="Try to fix the problem. By default, it will only be reported.")

    args = parser.parse_args()

    check_args_sanity(args)

    files = find_files(args.mode)
    if args.files is not None:
        files += [os.path.join(os.getcwd(), fn) for fn in args.files]
    files = list(set(files))

    if len(files) == 0:
        print("No relevant files.")
        sys.exit(RETURN_NO_ISSUES)

    problem_found = False

    if not args.skip_format:
        print(f"Check format of {len(files)} files ...")
        if not formatchecker.check(files):
            problem_found = True
        print("Format checks done.")

    if not args.skip_clazy:
        print(f"Apply clazy to {len(files)} files ...")
        if not run_clazy(files, args.build_dir, args.clazy_executable, args.fix):
            problem_found = True
        print("clazy done.")

    if not args.skip_clang_tidy:
        print(f"Apply clang-tidy to {len(files)} files ...")
        if not run_clang_tidy(files, args.build_dir, args.clang_tidy_binary,
                              args.run_clang_tidy_executable, args.fix):
            problem_found = True
        print("clang-tidy done.")

    if problem_found:
        print(f"At least one file (out of {len(files)}) is not compliant.")
        sys.exit(RETURN_DETECTED_ISSUES)
    else:
        print(f"All checked files ({len(files)}) are compliant.")
        sys.exit(RETURN_NO_ISSUES)

if __name__ == "__main__":
    main()
