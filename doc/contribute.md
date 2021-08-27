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
-   Spanish ([incomplete](#-i-want-to-improve-existing-translation))
-   [New languages are welcome!](#-i-want-to-translate-omm-into-a-new-language)

## Documentation
There is no manual yet, the screen cast must be updated and improved.

## Testing
There are many bugs yet to be discovered.

## What if ...

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

-   `XProperty`
-   `XPropertyWidget`
-   `XPropertyConfigWidget`

However, only the `XProperty` needs to be added to the
[`properties.lst`](../lists/properties.lst).
Unlike the other registered classes, the `TYPE`-member is a function.

### ... I want to add a new command?

Add it to [`default_keybindings.cfg`](../keybindings/default_keybindings.cfg).
If you want your command to show up in some menu, you can edit the
[`MainWindow::main_menu_entries()`](../src/mainwindow/mainwindow.cpp)
-method.

### ... I want to translate the application into a new language?

Add the file to the list of translation files in `cmake/generate_resources.cmake`.
Follow the instructions to update the languaes (i.e., build the `update-ts`-target).

### ... I want to update translations in an existing language?

Build the `update-ts`-target.
See [`build.md`](build.md) for details.
