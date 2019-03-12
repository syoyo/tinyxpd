# TinyXPD, dependency-free and header-only C++11 XGen XPD cache  I/O library.

TinyXPD is a simple library to read/write Gen XPD cache file.

## Requirement

* C++11 compiler

## Supported version

* [x] XPD3(~Maya 2018)

## How to use

```
// Do this only in **one** .cc file.
#define TINY_XPD_IMPLEMENTATION
#include "tiny_xpd.h"

T.B.W.
```

## Generating XPD file from Maya

You can use `xgSplineToXpd` sample plug-in(located in `/usr/autodesk/maya/plug-ins/xgen/plug-ins/` or write your own XPD writer plugin.

## Supported types

* [x] Curve(grooming splines)

## License

MIT license.
