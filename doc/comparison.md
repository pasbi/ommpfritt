# Comparison of *omm* with other graphical applications

This document explains the shortcomings of other applications that motiviated the creation of this project
and the merits of those that influenced its development.

## Traditional 2D Vector Applications

Applications like [Inkscape](https://inkscape.org/) and Illustrator offer huge flexibility and come with a plethora of tools.
However, it is often hard to maintain a well-structured, semantically sensible document.
For example, the Objects-Dialog is at least not intuitively to use and buggy.
That might not even be a problem for the every-day artist, as many use-cases do not require a nice structured document.
Typical examples are those chaotic but beautiful comic drawings, where the artist depicts even specular light with 2d-shapes.
Inkscape (and the conceptually similar Illustrator) are perfect tools for such use cases and omm does not intend to be an alternative there.

omm excels when many similar objects have to be drawn in well-defined pattern.

## 3D Graphics

The user interface of omm is strongly influenced by [Cinema 4D](), that is, the proximity to 3D-graphic tools is somehow obvious.
It should be clear, that omm is inherently 2D (+ time).
The concept of transparency (the user intuitively knows what's happening under the hood) and structuredness can be transfered
from the 3D- into the 2D-world and is very useful for at least some use cases and workflows.

## CAD

omm is designed for artists, not for engineers.
It is planned to incorporate concepts from CAD into omm, e.g., constraint-based design.
However, this has low priority and will probably not happen very soon (contributions welcome).

## NodeBox

TODO: [NodeBox](https://www.nodebox.net/code/index.php/Home)

## Programming Languages

You can draw with almost any programming language.
Often, one uses not the bare language itself but some kind of framework, such as Qt (`QPainter`),
`matplotlib`, `OpenCV` or some other graphics or plotting library. TODO
