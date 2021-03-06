checks = [
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
    "returning-data-from-temporary",
    "rule-of-two-soft",
    "skipped-base-method",
    "virtual-signal",
    "range-loop",
    # "incorrect-emit",        # false positives
    # "connect-non-signal",    # many false positives
    # "non-pod-global-static"  # open issue.
]

checks = ','.join(checks)
