# ommpfritt

*ommpfritt* or short *omm* is a free and open source vector modelling application.
It brings the workflow of (3D) modelling tools to the 2D vector graphics world.
The design of the application encourages and supports the user to produce maintainable documents,
using concepts like procedural modelling and non-destructive editing.

The concept lies in between traditional vector graphic editors (e.g., _inkscape_), CAD applications
and (3D) modelling tools.

Primary use case is creating and editing (animated) info graphics, diagrams, icons and semi-technical drawings.

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
- [Qt > 5.0](https://github.com/qt)
- [armadillo](http://arma.sourceforge.net/)
- [pybind11](https://github.com/pybind/pybind11)
- [glog](https://github.com/google/glog)
- [python3](https://github.com/python)
