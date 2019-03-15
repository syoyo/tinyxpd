# TinyXPD, dependency-free and header-only C++11 XGen XPD cache  I/O library.

TinyXPD is a simple library to read/write Gen XPD cache file.

## Requirement

* C++11 compiler

## Supported platform

* [x] macOS
* [x] Linux
* [x] Windows(Visual Studio 2017 or later)
* [x] Android
* [x] iOS
* [ ] Big endian machine(e.g. POWER)

## Supported XPD version

* [x] XPD3(~Maya 2018)

## How to use

```
+-----------------------------+   |\
| XPD header                  |   |
+-----------------------------+   | XPD data
|                             |   |
| XPD prim data               |   |
|                             |   |
|                             |   |
+-----------------------------+   |/
```

`XPDHeader` contains header information and the list of data offset from the beginning of a XPD data.

Whole XPD data is provided by the app user(`ParseXPDHeaderFromMemory` API), or read from a file(`ParseXPDFromFile`).
`ParseXPDFromFile` API is handy but consumes memory.
If you want to open large XPD file(e.g. 1GB or more), consider using `ParseXPDHeaderFromMemory` API.

```
// Do this only in **one** .cc file.
#define TINY_XPD_IMPLEMENTATION
#include "tiny_xpd.h"

std::string filename = "sample.xpd";

XPDHeader xpd_header;
std::vector<uint8_t> xpd_data;
std::string err;

bool ret = ParseXPDFromFile(filename, &xpd_header, &xpd_data, &err);

// From memory(stream) version.
// Fill xpd_data by yourself in some way.
// xpd_data = ...
// bool ret = ParseXPDHeader(xpd_data.data(), xpd_data.size(), &xpd_header, &err);

if (!err.empty()) {
  std::cerr << err << std::endl;
}

if (!ret) {
  std::cerr << "Failed to parse XPD" << std::endl;
}

// See `xpd_reader_example.cc` for how to access primitive data.
```

## Generating XPD file from Maya

You can use `xgSplineDataToXpd` sample plug-in(located in `/usr/autodesk/maya/plug-ins/xgen/plug-ins/` or write your own XPD writer plugin.

## Note on importing XPD in legacy XGen

It looks legaxy XGen's `From XPD File` expects spline data layout is as defined in `xgSplineDataToXpd` sample code.

### Per CV width

Data layout used in `xgSplineDataToXpd` example does not support exporting per-CV width. It can only support constant width over CVs(strand), and taper. `width ramp` is not support.

If you need a spline curve with varying width, you may need to write your own XGen loader(generator) plugin, or `bake` taper parameter to `width ramp` value and export/import it in other data format(e.g. python + JSON).

## Supported types

* [x] Curve(XGen interactive grooming splines)
* [ ] Point(xuv)

## TODO

* [ ] Support xuv format.


## License

MIT license.
