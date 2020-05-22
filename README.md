# ommpfritt

[![Build Status](https://travis-ci.org/pasbi/ommpfritt.svg?branch=master)](https://travis-ci.org/pasbi/ommpfritt)
[![Gitter](https://badges.gitter.im/ommpfritt/community.svg)](https://gitter.im/ommpfritt/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/pasbi/ommpfritt.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/pasbi/ommpfritt/alerts/)
[![Language grade: Python](https://img.shields.io/lgtm/grade/python/g/pasbi/ommpfritt.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/pasbi/ommpfritt/context:python)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/pasbi/ommpfritt.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/pasbi/ommpfritt/context:cpp)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/7c6bfee90e434bae8824a21de8e036fb)](https://www.codacy.com/manual/pasbi/ommpfritt?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=pasbi/ommpfritt&amp;utm_campaign=Badge_Grade)
[![AppImageBade](https://img.shields.io/badge/AppImage-download-blue)](https://github.com/pasbi/ommpfritt/releases/tag/continuous)

*ommpfritt* or short *omm* is a free and open source vector modelling and
motion graphics application.
It brings the workflow of (3D) modelling tools to the 2D vector graphics world.
The design of the application encourages and supports the user to produce
maintainable documents,
using concepts like procedural modelling and non-destructive editing.

Conceptually *omm* is a mix of traditional vector graphic editors
(e.g., *inkscape*), CAD applications and (3D) modelling tools.
Primary use case is creating, editing and rendering all sorts of animated and
still graphics, diagrams, icons and drawings.

![screenshot](sample-scenes/basic.png)


## How to Get It
*omm* is [free](https://en.wikipedia.org/wiki/Free_software).
You can get it's source from [github](https://github.com/pasbi/ommpfritt) and
build it.
You may find [build instructions](doc/build.md) helpful.


### Linux
You can download the latest
[AppImage](https://github.com/pasbi/ommpfritt/releases/tag/continuous),
make the [downloaded file executable and just run it](https://docs.appimage.org/introduction/quickstart.html#ref-quickstart).


### Windows
The last [Windows release](https://github.com/pasbi/ommpfritt/releases/tag/v0.1.2)
is quite outdated.
I can make a more recent release on demand.
Don't hesitate to ask me.


### Mac
*omm* has been tested on various Linux Distributions and Windows.
It has never been tested on Mac. It works in theory.


## How to use
*omm* was designed to be very easy to use.
So if you can handle a mouse and a keyboard, it should be no problem to get
started without studying the manual.

Maybe you find the [feature list](doc/features.md) helpful.

There is a (quite outdated)
[screen cast on youtube](https://www.youtube.com/watch?v=6X5Lo7kq5eM)
that shows some of the most important features and how they can be used.


## Contributing
Although you can already use the app, it's still a long way until v1.0.
So we need your help!

See [how to contribute](doc/contribute.md).
Don't hesitate to get in contact with us!


## Related Software

### Traditional 2D Vector Applications
Applications like [Inkscape](https://inkscape.org/) and Illustrator offer huge flexibility and come with a plethora of tools.
However, it is often hard to maintain a well-structured, semantically sensible document.
For example, the Objects-Dialog is at least not intuitively to use and buggy.
That might not even be a problem for the every-day artist, as many use-cases do not require a nice structured document.
Typical examples are those chaotic but beautiful comic drawings, where the artist depicts even specular light with 2d-shapes.
Inkscape (and the conceptually similar Illustrator) are perfect tools for such use cases and omm does not intend to be an alternative there.

omm excels when many similar objects have to be drawn in well-defined pattern.


### 3D Graphics
The user interface of omm is strongly influenced by [Cinema 4D](), that is, the proximity to 3D-graphic tools is somehow obvious.
It should be clear, that omm is inherently 2D (+ time).
The concept of transparency (the user intuitively knows what's happening under the hood) and structuredness can be transfered
from the 3D- into the 2D-world and is very useful for at least some use cases and workflows.


### CAD
omm is designed for artists, not for engineers.
It is planned to incorporate concepts from CAD into omm, e.g., constraint-based design.
However, this has low priority and will probably not happen very soon (contributions welcome).

### NodeBox
TODO: [NodeBox](https://www.nodebox.net/code/index.php/Home)

### Programming Languages
You can draw with almost any programming language.
Often, one uses not the bare language itself but some kind of framework, such as Qt (`QPainter`),
`matplotlib`, `OpenCV` or some other graphics or plotting library. TODO

