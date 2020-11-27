#!/usr/bin/env python3

import sys
import multiprocessing
import subprocess
import argparse
import os

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

parser = argparse.ArgumentParser("Perform Static Checks")
parser.add_argument("--compile-commands", required=True,
                    help="The compile_commands.json file.")
parser.add_argument("--files", required=True, nargs='+',
                    help="The files to check")
parser.add_argument("--modes", required=True, choices=["clang-tidy", "clazy"],
                    nargs='+', help="The checker(s) to use")
parser.add_argument("-j", type=int, default=2,
                    help="Number of parallel jobs.")
args = parser.parse_args()

clazy_checks = ','.join(clazy_checks)

command_args = {
  "clazy":      ["clazy", "--standalone", "--ignore-included-files",
                 "-p", args.compile_commands,
                 f"-checks={clazy_checks}"],
  "clang-tidy": ["clang-tidy", "-warnings-as-errors=*",
                 "-p", args.compile_commands]
}


def file_filter(fn):
    fn = os.path.normpath(fn).replace("\\", "/")
    if "/external/" in fn:
        return False
    else:
        return fn.endswith(".h") or fn.endswith(".cpp")

files = [fn for fn in args.files if file_filter(fn)]

if len(files) == 0:
    print("No files given, nothing to do.")
    sys.exit(0)

def perform_checks_on_single_file(mode, fn):
    print(f"Checking {fn} ...")
    command = command_args[mode] + [fn]
    if subprocess.run(command).returncode == 0:
        print(f"{mode} ok: {fn}")
        return True
    else:
        print(f"{mode} not ok: {fn}")
        return False

def perform_checks(mode):
    pool = multiprocessing.Pool(args.j)
    zipped_args = zip([mode] * len(files), files)
    return_codes = pool.starmap(perform_checks_on_single_file, zipped_args)
    if all(return_codes):
        print(f"All {mode} checks passed.")
        return True
    else:
        total = len(return_codes)
        failed = total - sum(return_codes)
        print(f"{failed}/{total} {mode} checks failed.")
        return False

results = {mode: perform_checks(mode) for mode in args.modes}
for mode, result in results.items():
    print(mode + ": " + "ok" if result else "fail")
if all(results.values()):
    print("All checkers passed.")
    sys.exit(0)
else:
    total = len(results)
    failed = total - sum(results.values())
    print(f"{failed}/{total} checkers failed.")
    sys.exit(1)
