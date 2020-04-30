# ommpfritt

[![Build Status](https://travis-ci.org/pasbi/ommpfritt.svg?branch=master)](https://travis-ci.org/pasbi/ommpfritt)
[![Gitter](https://badges.gitter.im/ommpfritt/community.svg)](https://gitter.im/ommpfritt/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/pasbi/ommpfritt.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/pasbi/ommpfritt/alerts/)
[![Language grade: Python](https://img.shields.io/lgtm/grade/python/g/pasbi/ommpfritt.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/pasbi/ommpfritt/context:python)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/pasbi/ommpfritt.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/pasbi/ommpfritt/context:cpp)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/7c6bfee90e434bae8824a21de8e036fb)](https://www.codacy.com/manual/pasbi/ommpfritt?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=pasbi/ommpfritt&amp;utm_campaign=Badge_Grade)

*ommpfritt* or short *omm* is a free and open source vector modelling application.
It brings the workflow of (3D) modelling tools to the 2D vector graphics world.
The design of the application encourages and supports the user to produce maintainable documents,
using concepts like procedural modelling and non-destructive editing.

The concept lies in between traditional vector graphic editors (e.g., _inkscape_), CAD applications
and (3D) modelling tools.

Primary use case is creating and editing (animated) info graphics, diagrams, icons and semi-technical drawings.

![screenshot](https://raw.githubusercontent.com/pasbi/ommpfritt/master/screenshot.png)

## Download

### Linux ([AppImage](https://appimage.org/))

-   Download the [`.AppImage` file](https://github.com/pasbi/ommpfritt/releases/tag/continuous)
-   Open a terminal in the directory where the file was downloaded and run `chmod a+x ommpfritt*.AppImage`
-   Double click the `.AppImage` file in your file explorer (e.g. nautilus)

### Windows

-   Download the [`.zip`-file](https://github.com/pasbi/ommpfritt/releases/tag/v0.1.2)
-   Extract the contents, go into the folder and double-click `ommpfritt.exe`

### macOS

On macOS you currently need to create your own build, see [build.md](build.md)

## How to use

There is a [screencast on youtube](https://www.youtube.com/watch?v=6X5Lo7kq5eM) that shows some of the most important features and how they can be used

## Features

### General
-   load and save scenes to/from human readable **json** format
-   all modifications to a scene can be undone (**unlimited undo/redo-stack**)
-   **simple concept** with few very concrete item classes (*Object*, *Tag*, *Style*, *Tool*)
-   wide **drag'n'drop** support: move, copy or link Objects, Tags and Styles naturally
-   **non-destructive**, **non-linear** editing: properties of Objects, Tags, Styles and Tools can be accessed and modified at any time.
-   **programmable**: every property can be accessed and modified using an internal python interpreter
-   build complex **object hierarchies** and object groups
-   **flexible GUI** using dockable widgets
-   **dark** and **light** skins
-   highly customizable interface colors
-   properties can be added at runtime
-   **multi-selection** support for properties: compatible intersection of properties of all selected items (objects, tags, styles, tools) is shown and can be modified simultaneously
-   **flexible, customizable key binding sequences** (aka short-cuts)
-   **multi-language** (currently English, Spanish and German only)
-   **rasterize** to `png` and `jpg`
-   **export** to SVG

### Styling
-   build complex reusable **style cascades**
-   dynamic **Named Colors**

### Objects
-   path object: any number of points, closed/open
    - **bézier** mode: modify the tangents of any point individually
    - **smooth** mode: tangents are set automatically
    - **linear** mode: no tangents
    - subdivide/remove points
-   **procedural** path: control the points and tangents using python
-   **ellipse**, **rectangle**, **line**
-   **empty**: no geometry, but useful as group parent
-   **image**: load JPG, PNG, SVG, PDF as object
-   **instance**: all changes of the reference are updated in the instance-object, too
-   **cloner**: many instances in a regular arrangement
    - grid
    - linear
    - along any path
    - radial
    - by python script
-   **mirror**
-   **view**: define a view onto the scene for export
    - set the export region comfortably within the editor
    - adjust to viewport or vice versa
    - switch between any number of self-defined views
    - supports translation, scaling, rotation and shear

### Tags
-   **path tag**: constrain position of any object to any path
-   **style tag**: add a style to the object
-   **script tag**: general purpose scripting without limits

### Tools
-   object selection: select in viewport and rotate, move or scale* along common center of all selected objects
-   point-level: select in viewport and rotate, move or scale* along common center of all selected points. Works fine even if many path-objects are selected.
-   optinally transform axes only. Re-set the origin of the object without affecting its geometry.
-   brush-select: select points as if you were painting
-   Bounding Box-Handle

## Animation
-   Keyframe Animation
-   Responsive Timeline
-   Dope Sheet
-   Seamless integration into undo/redo and multi-property- and multi-object-selection

## Contributing

Although you can already use the app, it's still a long way until v1.0, so we need your help!
Currently we're looking for

-   developers (have a look at the issues section; better get in touch with us before you start coding)
-   users (aka. testers, file an issue, tell us your ideas)

## Building

see [build.md](build.md)
