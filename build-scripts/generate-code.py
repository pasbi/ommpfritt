#!/usr/bin/env python3

import datetime
import json
import sys


def ensure_list(list_or_string):
    if isinstance(list_or_string, str):
        return [ list_or_string ]
    else:
        return list_or_string

if __name__ == "__main__":
    with open(sys.argv[1], 'r') as f:
        spec = json.load(f)

    timestamp = datetime.datetime.now().isoformat()

    with open(sys.argv[2], 'w') as f:
        f.write("///////////////////////////////////////////\n")
        f.write("// this file was auto generated.         //\n")
        f.write(f"// {timestamp}            //\n")
        f.write("// any changes will become overwritten.  //\n")
        f.write("///////////////////////////////////////////\n")
        f.write("\n")
        for item in spec["items"]:
            for inc in ensure_list(spec["include"]):
                f.write(inc.format(lc_item=item.lower()) + "\n")

        f.write("namespace\n")
        f.write("{\n")

        # the register_*s function is used for sure, but most code highlighters
        # don't get it.
        # Add the [[maybe_unused]] attribute to silence false warnings.
        f.write(f"[[maybe_unused]] void register_{spec['category']}()\n")
        f.write("{\n")
        f.write("  using namespace omm;\n")
        for item in spec["items"]:
            d = { "clazz": spec["clazz"], "item": item }
            for line in ensure_list(spec["register"]):
                line = "  " + line + "\n"
                f.write(line.format(clazz=spec["clazz"], item=item))
        f.write("}\n")
        f.write("}  // namespace\n")
    