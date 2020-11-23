#!/usr/bin/env bash

cd "$(dirname "$0")/.."

tidy_checks=\
-*,bugprone-*,\
clang-*,\
cppcoreguidelines-avoid-*,\
cppcoreguidelines-c-copy-assignment-signature,\
cppcoreguidelines-explicit-virtual-functions,\
cppcoreguidelines-init-variables,\
cppcoreguidelines-interfaces-global-init,\
cppcoreguidelines-macro-usage,\
cppcoreguidelines-narrowing-conversions,\
cppcoreguidelines-no-malloc,\
cppcoreguidelines-non-private-member-variables-in-classes,\
cppcoreguidelines-owning-memory,\
cppcoreguidelines-pro-type-*,\
cppcoreguidelines-slicing,\
cppcoreguidelines-special-member-functions
misc-definitions-in-headers,\
misc-misplaced-const,\
misc-new-delete-overloads,\
misc-non-*,\
misc-redundant-expression,\
misc-static-assert,\
misc-throw-by-value-catch-by-reference,\
misc-unconventional-assign-operator,\
misc-uniqueptr-reset-release,\
misc-unused-*
modernize-avoid-*,\
modernize-concat-nested-namespaces,\
modernize-deprecated-*,\
modernize-loop-convert,\
modernize-make-*,\
modernize-raw-string-literal,\
modernize-redundant-void-arg,\
modernize-replace-*,\
modernize-return-braced-init-list,\
modernize-shrink-to-fit,\
modernize-unary-static-assert,\
modernize-use-auto,\
modernize-use-bool-literals,\
modernize-use-default-member-init,\
modernize-use-emplace,\
modernize-use-equals-*,\
modernize-use-nodiscard,\
modernize-use-noexcept,\
modernize-use-nullptr,\
modernize-use-override,\
modernize-use-transparent-functors,\
modernize-use-uncaught-exceptions,\
modernize-use-using,\
openmp-*,\
performance-*,\
portability-*,\
readability-avoid-const-params-in-decls,\
readability-braces-around-statements,\
readability-const-return-type,\
readability-container-size-empty,\
readability-convert-member-functions-to-static,\
readability-delete-null-pointer,\
readability-deleted-default,\
readability-function-size,\
readability-identifier-naming,\
readability-implicit-bool-conversion,\
readability-inconsistent-declaration-parameter-name,\
readability-isolate-declaration,\
readability-magic-numbers,\
readability-make-member-function-const,\
readability-misleading-indentation,\
readability-misplaced-array-index,\
readability-non-const-parameter,\
readability-qualified-auto,\
readability-redundant-*,\
readability-simplify-*,\
readability-static-*,\
readability-string-compare,\
readability-uniqueptr-delete-release,\
readability-uppercase-literal-suffix,\
readability-use-anyofallof

# cppcoreguidelines-pro-bounds-array-to-pointer
# misc-no-recursion
# modernize-use-trailing-return-type
# modernize-pass-by-value
# readability-else-after-return
# readability-named-parameter

# find . -name "*.cpp" \
#   | xargs clang-tidy -p build/compile_commands.json -checks="$tidy_checks"

exit_code=0
files=$(find src -name "*.cpp")
files="src/serializers/jsonserializer.cpp"
for f in $files; do
  echo -e "\n\n@PROCESSING $f"
  if ! clang-tidy -p build/compile_commands.json -checks="$tidy_checks" -warnings-as-errors=* $f;
  then
    echo "clang-tidy failed on $f."
    exit_code=1
  else
    echo "clang-tidy succeeded on $f."
  fi
done

exit $exit_code
