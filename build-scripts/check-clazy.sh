#!/usr/bin/env bash

cd "$(dirname "$0")/.."

clazy_checks= \
level0,\
auto-unexpected-qstringbuilder,\
child-event-qobject-cast,\
connect-3arg-lambda,\
const-signal-or-slot,\
detaching-temporary,\
foreach,\
inefficient-qlist-soft,\
install-event-filter,\
overridden-signal,\
post-event,\
qdeleteall,\
qhash-namespace,\
qlatin1string-non-ascii,\
qproperty-without-notify,\
qstring-left,\
range-loop,\
returning-data-from-temporary,\
rule-of-two-soft,\
skipped-base-method,\
virtual-signal\

# Enable all checks from level0 and all checks from level1 except:
# "incorrect-emit"            # yields false positives
# "non-pod-global-static"     # having many non-pod-global-static objects is an open issue.

echo "clazy checks: $clazy_checks"

find . -name "*.cpp"  \
  | xargs clazy --standalone -p build/compile_commands.json -checks="$clazy_checks"

exit $?
