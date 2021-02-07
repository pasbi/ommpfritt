#!/usr/bin/env python3

import os
import formatoptions
import re

class Checker:
    def __init__(self, filename):
        """Loads a file with given filename and constructs an extensible Checker class.

           Define your checker member methods, then override self.line_based_checks and call them.
           Each checker must call self.error with a sensibe message, if a formatting error was
           detected that must be fixed, or self.warning for problems that are optional to fix.
        """
        with open(filename, 'rb') as f:
            # open as binary, otherwise \r\n will be hidden.
            self.lines = f.read().decode('utf-8').split("\n")
        self.filename = filename
        self.warnings = 0
        self.errors = 0

    def line_based_checks(self, line, n):
        pass

    def check(self):
        for i, line in enumerate(self.lines):
            n = i + 1
            self.line_based_checks(line, n)

    def _print(self, message, line, tag):
        line = "" if line is None else f":{line}"
        print(f"{tag} {self.filename}{line}: {message}")

    def warn(self, message, line):
        self.warnings += 1
        self._print(message, line, "WARNING")

    def error(self, message, line):
        self.errors += 1
        self._print(message, line, "ERROR")

class CppChecker(Checker):
    def __init__(self, filename):
        super(CppChecker, self).__init__(filename)
        self.re_size_t = re.compile(r'\b(?<!std::)size_t\b')
        self.re_c_include = re.compile(r'^\s*#include <(?!(2geom|poppler|KF5)).*\.h>')
        self.re_avoid_qdebug_include = re.compile(r'^\s*#include .QDebug.')
        self.re_avoid_qdebug = re.compile(r'qdebug|qinfo|qwarning|qcritical|qfatal', re.IGNORECASE)

    def line_based_checks(self, line, n):
        self.check_line_length(line, n)
        self.check_trailing_whitespace(line, n)
        self.check_no_tabs(line, n)
        self.check_unix_linebreak(line, n)
        self.check_bad_patterns(line, n)
        super(CppChecker, self).line_based_checks(line, n)

    def check_line_length(self, line, n):
        if len(line) > formatoptions.maxcols:
            self.error(f"Line too long ({len(line)}).", n)
        elif len(line) > formatoptions.maxcols_soft:
            self.warn(f"Line too long ({len(line)}) if there's no good reason.", n)

    def check_trailing_whitespace(self, line, n):
        if line.rstrip() != line:
            self.error("Line contains trailing whitespace.", n)

    def check_no_tabs(self, line, n):
        ws_length = len(line) - len(line.lstrip())
        ws = line[:ws_length]
        if '\t' in ws:
            self.error("Line is indented using tabs.", n)

    def check_unix_linebreak(self, line, n):
        if line.endswith("\r"):
            self.error("Line ends with CRLF.", n)

    def check_bad_patterns(self, line, n):
        if self.re_size_t.search(line):
            self.error("Replace size_t with std::size_t.", n)
        if self.re_c_include.search(line):
            self.error("Don't use C includes (like <math.h>), use C++ includes (like <cmath>).", n)
        if not any(self.filename.endswith(l) for l in ("src/logging.cpp", "src/logging.h")):
            if self.re_avoid_qdebug.search(line):
                self.error("Use LINFO, LERROR, LWARNING, LFATAL, LDEBUG instead of Qt-logging macros "
                           "qDebug, qInfo, qWarning, qCritical, qFatal.", n)
            if self.re_avoid_qdebug_include.search(line):
                self.error("Include logging.h instead of QDebug.", n)

class HeaderChecker(CppChecker):
    def __init__(self, filename):
        super(HeaderChecker, self).__init__(filename)

class ImplementationChecker(CppChecker):
    def __init__(self, filename):
        super(ImplementationChecker, self).__init__(filename)

def check_single_file(filename):
    if filename.endswith(".cpp"):
        checker = ImplementationChecker(filename)
    elif filename.endswith(".h"):
        checker = HeaderChecker(filename)
    elif os.path.basename(filename) == "CMakeLists.txt":
        return True  # No style check yet.
    elif filename.endswith(".ui"):
        return True  # No style check yet.
    else:
        print("Unexpected file: ", filename)
        return False

    checker.check()
    return checker.errors == 0

def check(filenames):
    results = list(check_single_file(fn) for fn in filenames)
    uncompliant_files = [f for r, f in zip(results, filenames) if not r]
    for fn in uncompliant_files:
        print(fn)

    return len(uncompliant_files) == 0
