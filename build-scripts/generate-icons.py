#!/usr/bin/env python3

import os
import json
import sys
import argparse
import subprocess

def generate_qrc(items):
    lines = []
    lines.append('<RCC>')
    lines.append('    <qresource prefix="/icons">')
    ind = ' '*8
    for resolution, item in items:
        lines.append(ind + f'<file>{item}_{resolution}.png</file>')
    lines.append('    </qresource>')
    lines.append('</RCC>')
    lines.append('')
    return "\n".join(lines)

def lst_decoder(fn):
    with open(fn, 'r') as f:
        spec = json.load(f)
    category = spec["category"]
    for item in spec["items"]:
        yield category, item

def cfg_decoder(fn):
    with open(fn, 'r') as f:
        for line in f.readlines():
            line = line.strip()
            if line.startswith("[") and line.endswith("]"):
                # category = line[1:-1]
                pass
            elif line.startswith("#"):
                pass
            elif len(line.strip()) == 0:
                pass
            else:
                item = line.split(":")[0]
                yield "actions", item

def get_omm_status_code(omm_command, description):
    command = [
        omm_command,
        "status",
        "-c", description
    ]
    print("run command: ", command)
    cp = subprocess.run(command, capture_output=True)
    if cp.returncode == 0:
        try:
            return int(cp.stdout)
        except ValueError:
            print(f"Failed to parse status code '{cp.stdout}' as int.")
            sys.exit(1);
    else:
        print(f"Failed to retrieve return code for '{description}'.")
        print(f"command: {command} exited with code '{cp.returncode}'.")
        print("STDOUT: \n", cp.stdout)
        print("STDERR: \n", cp.stderr)
        sys.exit(2);


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--specs',
            help="The spec input files",
            nargs="+",
            required=True)
    parser.add_argument('--qrc',
            help="The qrc output file",
            required=True)
    parser.add_argument('--command',
            help="The ommpfritt-cli executable.",
            required=True)
    parser.add_argument('--scenefile',
            help="The scene file that defines the icons.",
            required=True)
    parser.add_argument('--output',
            help="The output directory.",
            required=True)
    parser.add_argument('--dont-regenerate', action='store_const', const=True,
            help="If this option is set, the command will do nothing if there"
                 " is one or more png files in the output directory.")
    parser.add_argument('--canned_icons',
            help="Filenames of additional icons that are not in the icons.omm spec file",
            nargs="+",
            required=False,
            default=[])
    args = parser.parse_args()

    args.output = os.path.abspath(args.output)
    args.qrc = os.path.abspath(args.qrc)

    if os.path.dirname(args.qrc) != args.output:
        print("qrc file must be in the output directory, but:")
        print(f"qrc filename: {args.qrc}")
        print(f"output directory: {args.output}")
        sys.exit(1)

    if args.dont_regenerate:
        if any(f.endswith(".png") for f in os.listdir(args.output)):
            print("Early exit because there are already png files in output"
                  f" directory {args.output} and option '--dont-regenerate' is"
                  " set.")
            sys.exit(0)

    items = []
    decoders = {
        ".lst": lst_decoder,
        ".cfg": cfg_decoder
    }
    for fn in args.specs:
        for ext, decoder in decoders.items():
            if fn.endswith(ext):
                for item in decoder(fn):
                    items.append(item)
                break

    object_name_not_found_code = get_omm_status_code(args.command,
                                                     "object name not found")

    def required_resolutions(item):
        if item == "omm":
            return {16, 22, 32, 48, 64, 128, 2048}
        else:
            return {128}
    
    processed_icons = set()
    for category, item in items:
        for resolution in required_resolutions(item):
            command = [
                args.command,
                "render",
                "-f", args.scenefile,
                "-V", "view",
                "-w", f"{resolution}",
                "-p", f"_root_/{category}/{item}$",
                "-o", f"{args.output}/{item}_{resolution}.png",
                "-y",
                "--no-opengl",
                "--unique"
            ]
            cp = subprocess.run(command, capture_output=True)
            print(f"Render icon for {category}/{item} @{resolution}...", end="")
            print(" ".join(command))
            if cp.returncode == object_name_not_found_code:
                print(f" skip (undefined in {args.scenefile}).")
            elif cp.returncode != 0:
                print(f" failed with code {cp.returncode}.")
                print("Command was:")
                print(cp.args)
                print("Message:")
                print(cp.stderr)
                exit(1)
            else:
                processed_icons.add((resolution, item))
                print(" done.")

    for item in args.canned_icons:
        filename = item
        item = os.path.splitext(os.path.basename(filename))[0]
        for resolution in required_resolutions(item):
            cp = subprocess.run([
                'convert',
                filename,
                "-resize", f'{resolution}x{resolution}!',
                f'{args.output}/{item}_{resolution}.png'
            ])
            if cp.returncode == 0:
                processed_icons.add((resolution, item))
            else:
                print(f"Error: Failed to convert image {filename} (code {cp.returncode}).")

    with open(args.qrc, 'w') as f:
        f.write(generate_qrc(processed_icons))


