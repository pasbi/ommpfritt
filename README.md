# ommpfritt

*ommpfritt* or short *omm* is a free and open source vector modelling application.
It brings the workflow of (3D) modelling tools to the 2D vector graphics world.
The design of the application encourages and supports the user to produce maintainable documents,
using concepts like procedural modelling and non-destructive editing.

The concept lies in between traditional vector graphic editors (e.g., _inkscape_), CAD applications
and (3D) modelling tools.

Primary use case is creating and editing (animated) info graphics, diagrams, icons and semi-technical drawings.

![screenshot](https://raw.githubusercontent.com/pasbi/ommpfritt/master/screenshot.png)

## Features

### General
- load and save scenes to/from human readable **json** format
- all modifications to a scene can be undone (**unlimited undo/redo-stack**)
- **simple concept** with few very concrete item classes (_Object_, _Tag_, _Style_, _Tool_)
- wide **drag'n'drop** support: move, copy or link Objects, Tags and Styles naturally
- **non-destructive** editing: properties of Objects, Tags, Styles and Tools can be accessed and modified at any time.
- **programmable**: every property can be accessed and modified using an internal python interpreter
- build complex **object hierarchies** and object groups
- **flexible GUI** using dockable widgets
- properties can be added at runtime
- **multi-selection** support for properties: compatible intersection of properties of all selected items (objects, tags, styles, tools) is shown and can be modified simultaneously

### Styling
- build complex reusable **style cascades**

### Objects
- path object: any number of points, closed/open
  - **bézier** mode: modify the tangents of any point individually
  - **smooth** mode: tangents are set automatically
  - **linear** mode: no tangents
  - subdivide/remove points
- **procedural** path: control the points and tangents using python
- **ellipse**
- **empty**: no geometry, but useful as group parent
- **image**: load jpg, png, etc. from file.
- **instance**: all changes of the reference are updated in the instance-object, too
- **cloner**: many instances in a regular arrangement
   - grid
   - linear
   - along any path
   - radial
   - by python script
   
### Tags
- **path tag**: constrain position of any object to any path
- **style tag**: add a style to the object
- **script tag**: general purpose scripting without limits

### Tools
- object selection: select in viewport and rotate, move or scale* along common center of all selected objects
- point-level: select in viewport and rotate, move or scale* along common center of all selected points. Works fine even if many path-objects are selected.
- brush-select: select points as if you were painting

## Contributing

Although you can already use the app, it's still a long way until v1.0, so we need your help!
Currently we're looking for

- developers (have a look at the issues section; better get in touch with us before you start coding)
- users (aka. testers, file an issue, tell us your ideas)

## Building

_omm_ uses _cmake_.
Install the depencendies and building should work out of the box.
We only test on arch-linux using gcc and clang, but we don't see a reason why it shouldn't build on other linuxes/win/mac, too.

Dependencies:
- [Qt > 5.10](https://github.com/qt)
- [armadillo](http://arma.sourceforge.net/)
- [pybind11](https://github.com/pybind/pybind11)
- [glog](https://github.com/google/glog)
- [python3](https://github.com/python)
