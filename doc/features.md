# Features

*omm* is
-   general purpose: create any kind of graphics. No limits!
-   interactive: you receive immediate feedback
-   [wysiwyg](https://en.wikipedia.org/wiki/WYSIWYG)

## User Interface
-   load and save scenes to/from human readable **json** format
-   all modifications to a scene can be undone (**unlimited undo/redo-stack**)
-   **simple concept** with few very concrete item classes (*Object*, *Tag*, *Style*, *Tool*)
-   wide **drag'n'drop** support: move, copy or link Objects, Tags and Styles naturally
-   **non-destructive**, **non-linear** editing: properties of Objects, Tags, Styles and Tools can be accessed and modified at any time.
-   build complex, reusable **object hierarchies** and object groups
-   **flexible GUI** using dockable widgets
-   customizable **dark** and **light** skins
-   **multi-selection** support for properties: compatible intersection of properties of all selected items (objects, tags, styles, tools) is shown and can be modified simultaneously
-   **flexible, customizable key binding sequences** (aka short-cuts)
-   **multi-language** (currently English, Spanish and German only)
-   **rasterize** to `png` and `jpg`
-   **export** to SVG

## Styling
-   build complex reusable **style cascades**
-   dynamic **Named Colors**
-   interactive OpenGL-Shaders

[!styling](sample-scenes/glshader.png)

## Objects
-   path object: any number of points, closed/open
    -   **bézier** mode: modify the tangents of any point individually
    -   **smooth** mode: tangents are set automatically
    -   **linear** mode: no tangents
    -   subdivide/remove points
-   **procedural** path: control the points and tangents using python
-   **ellipse**, **rectangle**, **line**
-   **empty**: no geometry, but useful as group parent
-   **image**: load JPG, PNG, SVG, PDF as object
-   **instance**: all changes of the reference are updated in the instance-object, too
-   **cloner**: many instances in a regular arrangement
    -   grid
    -   linear
    -   along any path
    -   radial
    -   by python script
-   **mirror**
-   **view**: define a view onto the scene for export
    -   set the export region comfortably within the editor
    -   adjust to viewport or vice versa
    -   switch between any number of self-defined views
    -   supports translation, scaling, rotation and shear

## Tags
-   **path tag**: constrain position of any object to any path
-   **style tag**: define the color of an object
-   **script tag**: general purpose scripting without limits

## Tools
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

[!animation](sample-scenes/animation.png)

## Programmable
-   **programmable**: every property can be controlled via embedded python scripting

[python](sample-scenes/python.png)

-   visual node-based scripting alternative

[nodes](sample-scenes/nodes.png)

-   user properties can be added at runtime to any object

## Optional Command Line Interface
-   Batch rendering from command line
-   Get basic scene information from command line

