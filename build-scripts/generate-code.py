#!/usr/bin/env python3

import json
import sys
import generated_file_header


def ensure_list(list_or_string):
    if isinstance(list_or_string, str):
        return [list_or_string]
    else:
        return list_or_string

if __name__ == "__main__":
    with open(sys.argv[1], 'r') as f:
        spec = json.load(f)

    with open(sys.argv[2], 'w') as f:
        f.write(generated_file_header.header())
        f.write("\n")
        for item in spec["items"]:
            for inc in ensure_list(spec["include"]):
                f.write(inc.format(lc_item=item.lower()) + "\n")

        f.write("#include \"logging.h\"\n")
        f.write("namespace omm_generated\n")
        f.write("{\n")

        # the register_*s function is used for sure, but most code highlighters
        # don't get it.
        # Add the [[maybe_unused]] attribute to silence false warnings.
        category = spec['category']
        clazz = spec["clazz"]
        f.write(f"[[maybe_unused]] void register_{category}()\n")
        f.write("{\n")
        f.write("  using namespace omm;\n")
        f.write(f"  LDEBUG << \"registering {category}\";\n")
        for item in spec["items"]:
            d = { "clazz": clazz, "item": item }
            for line in ensure_list(spec["register"]):
                line = "  " + line + "\n"
                f.write(line.format(clazz=clazz, item=item))
        f.write(f"  LDEBUG << \"registered \" << {clazz}::keys().size() << "\
                f"\" {category}.\";\n")
        f.write("}\n")
        f.write("}  // namespace omm_generated\n")
        f.write("\n")
