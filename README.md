# TinyXPD, dependency-free and header-only C++11 XGen XPD cache  I/O library.

TinyXPD is a simple library to read/write Gen XPD cache file.

## Requirement

* C++11 compiler

## Supported version

* [x] XPD3(~Maya 2018)

## How to use

`XPDHeader` contains parsed XPD header.
`XPDHeader` contains the list of data offset from the beginning of a XPD data.

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

T.B.W.
```

## Generating XPD file from Maya

You can use `xgSplineToXpd` sample plug-in(located in `/usr/autodesk/maya/plug-ins/xgen/plug-ins/` or write your own XPD writer plugin.

## Supported types

* [x] Curve(grooming splines)

## License

MIT license.
