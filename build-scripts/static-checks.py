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
    "cppcoreguidelines-owning-memory",
    "cppcoreguidelines-pro-type-const-cast",
    "cppcoreguidelines-pro-type-cstyle-cast",
    "cppcoreguidelines-pro-type-member-init",
    "cppcoreguidelines-pro-type-reinterpret-cast",
    "cppcoreguidelines-pro-type-static-cast-downcast",
    "cppcoreguidelines-pro-type-union-access",
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
    "readability-redundant-control-flow",
    "readability-redundant-declaration",
    "readability-redundant-function-ptr-dereference",
    "readability-redundant-member-init",
    "readability-redundant-preprocessor",
    "readability-redundant-smartptr-get",
    "readability-redundant-string-*"
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
    "readability-simplify-*",
    "readability-static-*",
    "readability-string-compare",
    "readability-uniqueptr-delete-release",
    "readability-uppercase-literal-suffix",
    "readability-use-anyofallof",

    # "cppcoreguidelines-non-private-member-variables-in-classes",
    # "readability-redundant-access-specifiers", # Flags `public Q_SLOTS` after `public`
    # "cppcoreguidelines-pro-bounds-array-to-pointer-decay",  # https://github.com/isocpp/CppCoreGuidelines/issues/1589
    # "cppcoreguidelines-pro-type-vararg" # flaggs QFATAL
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
parser.add_argument("--modes", required=True, choices=["clang-tidy", "clazy"],
                    nargs='+', help="The checker(s) to use")
args = parser.parse_args()

clazy_checks = ','.join(clazy_checks)
clang_tidy_checks = ','.join(clang_tidy_checks)

command_args = {
  "clazy":      ["clazy", "--standalone", "--ignore-included-files",
                 "-p", args.compile_commands,
                 f"-checks={clazy_checks}"],
  "clang-tidy": ["clang-tidy", "-warnings-as-errors=*",
                 "-p", args.compile_commands,
                 f"-checks={clang_tidy_checks}"]
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
    pool = multiprocessing.Pool(2)
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
