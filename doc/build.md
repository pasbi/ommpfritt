# Building ommpfritt

## Overview

*omm* uses the [cmake](cmake.org) build system generator.
The sources contain

-    implementation (`*.cpp`), header (`*.h`) and interface files (`*.ui`).
-    configuration files (`*.cfg`)
-    translation files (`*.ts`)
-    resource files (`*.qrc`)
-    list files (`*.lst`)
-    cmake files (`CMakeLists.txt`)

Some implementation, header and resource files will be generated during the
build process.
That is an automated process and should not require any manual intervention.
The process is described in [contribute.md](contribute.md) and in
[`CMakeLists.txt`](../CMakeLists.txt).

## Build environment

*omm* is known to successfully compile and run on Arch Linux, Ubuntu Linux and 
Windows/MSYS2.
Neither building nor running on Mac has ever been tested, but it should work in theory!
We recommend using **g++8.x** or later. g++7.x or earlier will not work.
Building with a recent **clang** is possible (tested with 8.0.0).
A very recent Visual Studio might work, C++17 support is required.
**cmake 3.14** or later is required.

## Build from scratch

Once you've installed the dependencies, it's very straight forward to build omm
using the common cmake-make build idiom.
Detailed informations about the dependencies, how to get them and how to run
the build commands on various platforms can be obtained from the [.travis.yml](.travis.yml) file.

## Details

The build process of omm is quite complex under the hood.
If you make a clean build from scratch, you should not notice it, as all required files
will be generated in the correct order automatically.

However, there may be reasons that make you require to understand the build process, e.g. if
you want to update the translation without re-building everything.

### Targets

-   `ommpfritt`: the main target, this builds the executable that shows the GUI.
-   `ommpfritt-cli`: a command line version of `ommpfritt`, with currently very limited interaction possibilities.
-   `libommpfritt`: a library that contains the vast majority of the code, including the graphical user interface. Both `ommpfritt` and `ommpfritt-cli` use this library and only little individual code.
-   `icons_png` uses `ommpfritt-cli` to build the icons that are used in `ommpfritt`
-   `translations_qm` provides the translation files
-   `resources_cli` generates a resource file that contains configuration files and translations. Is used by `ommpfritt-cli`.
-   `resources` generates a resource file that contains all resources from `resources_cli` plus the icons. Is used by `ommpfritt`.

As you see, the complexity is introduces because `omm` renders (most) icons it uses during the build by itself.
That is, first a command line application `ommpfritt-cli` is built, which renders the icons and only then
the actual application `ommpfritt` is built.

### Updating Translation

Omm uses Qt's translator system.
In a nutshell, Qt employs a pre-processor that scans the code for marker macros (`tr`), extracts them and builds
a `ts`-file for each configured language.
The `ts`-files are then compiled into `qm`-files, which are loaded into the application.
The `ts`-files also contain all the full translation information and are checked into the repository.

If you want to update or add a translation, edit the `ts`-file of the respective language with the `linguist` tool provided by Qt.
Then build the application (no re-build required).

The `ts` files will *not* get updated automatically if you have changed, added or removed a macro in the source code (`*.cpp`-, `*.h`- or `*.ui`-file).
The reason for this lies in a compromise between speed and correctness:
In an ideally correct world, the `ts_target` would depend on all source files.
That is, if any source file changes (i.e., a `tr`-macro changed potentially), the `ts`-files are re-generated.
However, this is slow because if the `ts`-files are re-generated, the `qm`-files will be re-generated, too.
That, renders the `resources_cli` and the `resources` target out of date and they must be re-generated as well.
That procedure is slow (couple of seconds on a modern PC), and has to be done every time the source code changes (which is quite frequently).

If you want the correct behaviour, simply add the `TS_SOURCES` list as a dependency in the `add_custom_command`-call that has the `lupdate`-executable as command, in the file `cmake/generate_translations.cmake`.

### Updating Icons

For icons, there is a similar problem as for translations:
Generating the icons depends on the source code.
Unlike the translation problem, this dependency is indirect.
I.e., in order to generate the icons, the target `ommpritt-cli` must be there and that target depends on the source code.
That means, that we cannot simply trade correctness and speed, as correctness is required here.
If we would remove that dependency, the build system would not know that `ommpfritt-cli` is required to generate the icons and could possibly try to render the icons *before* `ommpfritt-cli` has been built.
That would, of course not work and the build would fail.

As a workaround, the very time consuming rendering of all icons is skipped by passing the `dont-regenerate`-flag to the render script in `cmake/generate_icons.cmake`.
If this flag is present and the script finds a single `PNG`-file in the icons directory, it exits early with SUCCESS-return code.
That is, for CMake it looks like the script did it's job very quickly and everyone is happy.
Note that the individual icon-files (PNG) are not known to CMake, i.e., they are not declared as `OUTPUT` or `BYPRODUCTS` in any command.

This behavior is fine until one actually wants to re-generate the icons.
To force re-generation of icons, it's recommended to delete the `icons`-directory in your build-directory.
The icons will also be re-generated if, i.a., the scene file (`icons/icons.omm`) is touched.

