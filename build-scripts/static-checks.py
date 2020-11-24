#!/usr/bin/env python3

import sys
import multiprocessing
import subprocess
import argparse

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

clang_tidy_checks = [
    "-*,bugprone-*",
    "clang-*",
    "cppcoreguidelines-avoid-*",
    "cppcoreguidelines-c-copy-assignment-signature",
    "cppcoreguidelines-explicit-virtual-functions",
    "cppcoreguidelines-init-variables",
    "cppcoreguidelines-interfaces-global-init",
    "cppcoreguidelines-macro-usage",
    "cppcoreguidelines-narrowing-conversions",
    "cppcoreguidelines-no-malloc",
    "cppcoreguidelines-non-private-member-variables-in-classes",
    "cppcoreguidelines-owning-memory",
    "cppcoreguidelines-pro-type-*",
    "cppcoreguidelines-slicing",
    "cppcoreguidelines-special-member-functions",
    "misc-definitions-in-headers",
    "misc-misplaced-const",
    "misc-new-delete-overloads",
    "misc-non-*",
    "misc-redundant-expression",
    "misc-static-assert",
    "misc-throw-by-value-catch-by-reference",
    "misc-unconventional-assign-operator",
    "misc-uniqueptr-reset-release",
    "misc-unused-*",
    "modernize-avoid-*",
    "modernize-concat-nested-namespaces",
    "modernize-deprecated-*",
    "modernize-loop-convert",
    "modernize-make-*",
    "modernize-raw-string-literal",
    "modernize-redundant-void-arg",
    "modernize-replace-*",
    "modernize-return-braced-init-list",
    "modernize-shrink-to-fit",
    "modernize-unary-static-assert",
    "modernize-use-auto",
    "modernize-use-bool-literals",
    "modernize-use-default-member-init",
    "modernize-use-emplace",
    "modernize-use-equals-*",
    "modernize-use-nodiscard",
    "modernize-use-noexcept",
    "modernize-use-nullptr",
    "modernize-use-override",
    "modernize-use-transparent-functors",
    "modernize-use-uncaught-exceptions",
    "modernize-use-using",
    "openmp-*",
    "performance-*",
    "portability-*",
    "readability-avoid-const-params-in-decls",
    "readability-braces-around-statements",
    "readability-const-return-type",
    "readability-container-size-empty",
    "readability-convert-member-functions-to-static",
    "readability-delete-null-pointer",
    "readability-deleted-default",
    "readability-function-size",
    "readability-identifier-naming",
    "readability-implicit-bool-conversion",
    "readability-inconsistent-declaration-parameter-name",
    "readability-isolate-declaration",
    "readability-magic-numbers",
    "readability-make-member-function-const",
    "readability-misleading-indentation",
    "readability-misplaced-array-index",
    "readability-non-const-parameter",
    "readability-qualified-auto",
    "readability-redundant-*",
    "readability-simplify-*",
    "readability-static-*",
    "readability-string-compare",
    "readability-uniqueptr-delete-release",
    "readability-uppercase-literal-suffix",
    "readability-use-anyofallof",
    # "cppcoreguidelines-pro-bounds-array-to-pointer",
    # "misc-no-recursion",
    # "modernize-use-trailing-return-type",
    # "modernize-pass-by-value",
    # "readability-else-after-return",
    # "readability-named-parameter",
]

parser = argparse.ArgumentParser("Perform Static Checks")
parser.add_argument("--compile-commands", required=True,
                    help="The compile_commands.json file.")
parser.add_argument("--files", required=True, nargs='+',
                    help="The files to check")
parser.add_argument("--mode", required=True, choices=["clang-tidy", "clazy"],
                    help="The checker to use")
args = parser.parse_args()

if len(args.files) == 0:
    print("No files given, nothing to do.")
    sys.exit(0)

clazy_checks = ','.join(clazy_checks)
clang_tidy_checks = ','.join(clang_tidy_checks)

command_args = {
  "clazy":      ["clazy", "--standalone", "--ignore-included-files",
                 "-p", args.compile_commands,
                 f"-checks={clazy_checks}"],
  "clang-tidy": ["clang-tidy", "-warnings-as-errors=*",
                 "-p", args.compile_commands,
                 f"-checks={clang_tidy_checks}"]
}[args.mode]

def perform_clazy_check(fn):
    print(f"Checking {fn} ...")
    if subprocess.run(command_args + [fn]).returncode == 0:
        print(f"{args.mode} ok: {fn}")
        return True
    else:
        print(f"{args.mode} not ok: {fn}")
        return False

pool = multiprocessing.Pool(2)
return_codes = pool.map(perform_clazy_check, args.files)

if all(return_codes):
    print(f"{args.mode}: All checks passed.")
    sys.exit(0)
else:
    failed = sum(return_codes)
    total = len(return_codes)
    print(f"{args.mode}: {failed}/{total} checks failed.")
    sys.exit(1)
