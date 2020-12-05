#!/usr/bin/env python3

import os
import sys
import tempfile
import argparse
import subprocess
import multiprocessing

clazy_checks=[
    "connect-by-name",
    "connect-not-normalized",
    "container-anti-pattern",
    "empty-qstringliteral",
    "fully-qualified-moc-types",
    "lambda-in-connect",
    "lambda-unique-connection",
    "lowercase-qml-type-name",
    "mutable-container-key",
    "overloaded-signal",
    "qcolor-from-literal",
    "qdatetime-utc",
    "qenums",
    "qfileinfo-exists",
    "qgetenv",
    "qmap-with-pointer-key",
    "qstring-arg",
    "qstring-comparison-to-implicit-char",
    "qstring-insensitive-allocation",
    "qstring-ref",
    "qt-macros",
    "strict-iterators",
    "temporary-iterator",
    "unused-non-trivial-variable",
    "writing-to-temporary",
    "wrong-qevent-cast",
    "wrong-qglobalstatic",
    "auto-unexpected-qstringbuilder",
    "child-event-qobject-cast",
    "connect-3arg-lambda",
    "const-signal-or-slot",
    "detaching-temporary",
    "foreach",
    "inefficient-qlist-soft",
    "install-event-filter",
    "overridden-signal",
    "post-event",
    "qdeleteall",
    "qhash-namespace",
    "qlatin1string-non-ascii",
    "qproperty-without-notify",
    "qstring-left",
    "range-loop",
    "returning-data-from-temporary",
    "rule-of-two-soft",
    "skipped-base-method",
    "virtual-signal",
    # "incorrect-emit",        # false positives
    # "connect-non-signal",    # many false positives
    # "non-pod-global-static"  # open issue.
]

parser = argparse.ArgumentParser(description="Apply the style guide.")
parser.add_argument("--format-exec", default="clang-format",
                    help="The clang-format executable.")
parser.add_argument("--tidy-exec", default="clang-tidy",
                    help="The clang-tidy executable.")
parser.add_argument("--clazy-exec", default="clazy",
                    help="The clazy executable.")
parser.add_argument("--files", required=True, nargs='+',
                    help="The files to check. The list may be filtered unless"
                         " --include-external and --include-generated are"
                         " specified. Files not ending in .cpp or .h are"
                         " ignored.")
parser.add_argument("--fix", action='store_true', default=False,
                    help="Apply the proposals.")
parser.add_argument("--compile-commands",
                    help="Path to the compile_commands.json file.")
parser.add_argument("--include-external", action='store_true', default=False,
                    help="Do not filter external files from file list.")
parser.add_argument("--jobs", "-j", type=int, default=2,
                    help="Number of parallel jobs.")
parser.add_argument("--mode", required=True,
                    choices=["tidy", "clazy", "format"],
                    help="The checker(s) to use")
parser.add_argument("--include-generated", action='store_true', default=False,
                    help="Do not filter generated files from file list.")
parser.add_argument("--clang-tidy-exec", default="clang-tidy",
                    help="The clang-tidy executable.")
parser.add_argument("--verbose", "-v", action='store_true', default=False)

args = parser.parse_args()

if args.mode in ['tidy', 'clazy'] and args.compile_commands is None:
    print("--compile_commands is required if mode is clazy or tidy.")
    parser.print_usage()
    sys.exit(2)


clazy_checks = ','.join(clazy_checks)

def check_clazy(fn):
    command = [args.clazy_exec, "--standalone", "--ignore-included-files",
               "-p", args.compile_commands,
               f"-checks={clazy_checks}", fn]
    if args.fix:
        command += ["--fix"]
    process = subprocess.run(command, capture_output=True)
    if process.returncode == 0:
        return True
    else:
        print(process.stderr.decode('utf-8'))
        return False

def check_tidy(fn):
    command = [args.tidy_exec, "-warnings-as-errors=*", "-p",
               args.compile_commands, fn]
    if args.fix:
        command += ["--fix"]
    process = subprocess.run(command, capture_output=True)
    if process.returncode == 0:
        return True
    else:
        print(process.stdout.decode('utf-8'))
        return False

def check_format(fn):
    command = [args.format_exec, fn]
    if args.fix:
        command += ['-i']
        process = subprocess.run(command, capture_output=True)
        if process.returncode == 0:
            return True
        else:
            print(process.stdout.decode('utf-8'))
            return False
    else:
        process = subprocess.run(command, capture_output=True)
        if process.returncode == 0:
            formatted_file = tempfile.NamedTemporaryFile()
            formatted_file.write(process.stdout)
            formatted_file.flush()

            diff_command = ['diff',
                            '--old-line-format=%5dn < %L',
                            '--new-line-format=%5dn > %L',
                            '--unchanged-line-format=',
                            fn,
                            formatted_file.name ]
            diff = subprocess.run(diff_command, capture_output=True)
            if diff.returncode == 0:
                return True
            elif diff.returncode == 1:
                print(diff.stdout.decode('utf-8'))
                return False;
            else:
                print(f"diff failed with code {diff.returncode}")
                return False
        else:
            print(f"Formatter failed with code {process.returncode}")
            return False


fns = [os.path.abspath(fn) for fn in args.files]
fns = [fn for fn in fns if fn.endswith('.cpp') or fn.endswith('.h')]
if not args.include_external:
    for external_pattern in ['/src/external/', '/test/external']:
        fns = [fn for fn in fns if external_pattern not in fn]
if not args.include_generated:
    for generated_pattern in ['/src/registers.cpp', '/src/translations.h']:
        fns = [fn for fn in fns if not fn.endswith(generated_pattern)]

def checker(fn):
    if args.verbose:
        print(fn)
    return globals()[f"check_{args.mode}"](fn)

with multiprocessing.Pool(args.jobs) as pool:
    result = pool.map(checker, fns)

if not args.fix:
    total = len(result)
    bad = total - sum(result)

    print(f"There were {bad} bad files out of {total} files:")
    for status, fn in zip(result, fns):
        if not status:
            print(fn)
    sys.exit(0 if bad == 0 else 1)
