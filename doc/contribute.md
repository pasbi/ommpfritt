# Contributing
Although you can already use the app, it's still a long way until v1.0.
So we need your help!
Don't hesitate to get in contact, there are no obligations.

## There's always an issue waiting to be fixed:
If you're new, you might want reach for a [low-hanging fruit](https://github.com/pasbi/ommpfritt/labels/good%20first%20issue)
first.
You're of course welcome to solve one of the [brain twisters](https://github.com/pasbi/ommpfritt/issues), too.

## Translation
Currently *omm* is translated into
-   English (complete)
-   German (complete)
-   Spanish (incomplete)

[New languages are welcome!](doc/translate.md)

## Documentation
There is no manual yet, the screen cast must be updated and improved.

## Testing
There are many bugs yet to be discovered.

## The Build Process in Detail
This is only relevant for developers.

There are some non-standard things happening under the hood during a build.
Usually, all these steps will happen automatically, controlled by cmake.
It is, however, sometimes required to understand what happens for debugging
or pleasure.

Note that the steps are not necessarily executed in the order in which they
are listed below.
cmake does not define a particular oder, it only ensures that the steps are run
in a sensible sequence.

1.   In order to reduce code duplication, a list of available objects, tags,
managers, etc. is maintained in [lists/](../lists).
These files simply list available types with some meta information.
In a first step, these lists are translated into cpp-code using the script
[`generate-code.py`](../build-scripts/generate-code.py).
This generated cpp-code is later compiled into the binaries and responsible
for registering all the items and make the application aware of them.

2.   All the commands and preference keys are gathered from the
`*.cfg`-files and printed into the `translations.h`-file using the
[`update-translations_h`](../build-scripts/update-translations_h.py).
This step is required to make the translation system aware of the item, 
preferences and action names.
Thanks to this step, these strings will be translated properly.

3.  a `translations.qrc`-file is generated based on the languages listed in
[CMakeLists.txt](../CMakeLists.txt).

4.  `lupdate` updates the `*.ts`-files using the source files.
Usually, this is a no-op unless you have changed the source code.

5.  `lrelase` generates `*.qm`-files out of the `*.ts`-files

6.  `rcc` compiles the `*.qm`-files together with the manually created
resources (listed in [`resources.qrc`](../resources.qrc)) into a `*.cpp`-file.

7.  `libommpfritt` is built using both the generated files and those from vcs.

8.  `ommpfritt-cli` is built using `libommpfritt`

9.  The [`generate-icons.py`](../build-scripts/generate-icons.py)-script
renders an icon for each item in each [list](../lists).
It uses `ommpfritt-cli` and [the scene file](../icons/icons.omm).
If the scene file does not define an object for some item, a fully-transparent
icon is rendered.
The same script also creates a `icons.qrc` file.

10. `rcc` compiles all the files from step (6.) plus the `icons.qrc` file and
all icons into a cpp-file

11. `ommpfritt` is compiled using the generated files and those from vcs.

Consequently, `libommpfritt` has no resources and `ommpfritt-cli` lacks the
icons, which is not a big deal for a command line application.


## What if ...

### ... I want to translate *omm* into a new language?
Let's say you want to translate *omm* into French (fr).

1.  Create the file `omm_fr.ts`. This is where your translations goes into.
This step is a bit tricky, as there is little documentation of how this works.
A good workaround is to copy an existing `*.ts`-file, change the language-key
and delete most of its content.
Don't hesitate to ask for help.

2.  Add `fr` to the list of languages in [`CMakeLists.txt`](../CMakeLists.txt).

3.  Build the project.
This should add content to `ts/omm_fr.ts`.

4. See next section.

### ... I want to improve existing translation?

1.  Use the `linguist`: `linguist ts/omm_*.ts`.
2.  Build the project to see your results.

Note: Because of boring reasons, cmake does not update the binaries if the 
`*.ts`-files have changed (the `*.qm`-files are updated, though).
That is, if you only change the `*.ts`-files and then build the application,
you will not see your changes.

There are three workarounds:
-   Delete the build directory and build again. Simple but time-consuming.
-   Delete only the `build/qm`-directory.
-   Touch any of the source files.

### ... I want to add a new item?

If you want to add a new e.g., `Tag`, you create the `*.cpp` and `*.h` files,
add them to `src/tags/CMakeLists.txt` and `list/tags.lst`.
Remember to add a `TYPE` member.

Most other registered classes (`Manager`s, `Object`s, `Tool`s, `Node`s work
similarly.
You can add an icon for your new object into `../icons/icons.omm`.

If the new class is a `Manager`, `Object`, `Tool`, `Node` or `Tag`,
you must add a (possibly empty) key binding to
[`default_keybindings.cfg`](../keybindings/default_keybindings.cfg).

Properties, however, work slightly different.
Each property of type `X` requires the definition of three classes:

-   XProperty
-   XPropertyWidget
-   XPropertyConfigWidget

However, only the `XProperty` needs to be added to the
[`properties.lst`](../lists/properties.lst).
Unlike the other registered classes, the `TYPE`-member is a function.

### ... I want to add a new command?

Add it to [`default_keybindings.cfg`](../keybindings/default_keybindings.cfg).
If you want your command to show up in some menu, you can edit the
[`MainWindow::main_menu_entries()`](../src/mainwindow/mainwindow.cpp)
-method.

