#define TINY_XPD_IMPLEMENTATION
#include "tiny_xpd.h"


#include <cstdlib>
#include <cstdio>
#include <string>
#include <iostream>

using namespace tiny_xpd;

static std::string PrintPrimType(Xpd::PrimType prim) {
  if (prim == Xpd::PrimType::Point) {
    return "Point";
  } else if (prim == Xpd::PrimType::Spline) {
    return "Spline";
  } else if (prim == Xpd::PrimType::Card) {
    return "Card";
  } else if (prim == Xpd::PrimType::Sphere) {
    return "Sphere";
  } else if (prim == Xpd::PrimType::Archive) {
    return "Archive";
  } else if (prim == Xpd::PrimType::CustomPT) {
    return "CustomPT";
  }

  return "UNKNOWN PrimType. value = " + std::to_string(int(prim));
}

static std::string PrintCoordSpace(Xpd::CoordSpace space) {
  if (space == Xpd::CoordSpace::World) {
    return "World";
  } else if (space == Xpd::CoordSpace::Object) {
    return "Object";
  } else if (space == Xpd::CoordSpace::Local) {
    return "Local";
  } else if (space == Xpd::CoordSpace::Micro) {
    return "Micro";
  } else if (space == Xpd::CoordSpace::CustomCS) {
    return "CustomCS";
  }

  return "UNKNOWN CoordSpace. value = " + std::to_string(int(space));
}

static void PrintXPD(const tiny_xpd::XPD &xpd) {
  std::cout << "fileVersion : " << int(xpd.fileVersion) << "\n";
  std::cout << "primType : " << PrintPrimType(xpd.primType) << "\n";
  std::cout << "primVersion : " << int(xpd.primVersion) << "\n";
  std::cout << "time : " << xpd.time << "\n";
  std::cout << "numCVs : " << xpd.numCVs << "\n";
  std::cout << "coordSpace : " << PrintCoordSpace(xpd.coordSpace) << "\n";
  std::cout << "numBlocks : " << xpd.numBlocks << "\n";
}


int main(int argc, char **argv)
{
  if (argc < 2) {
    std::cerr << "Requires input.xpd\n";
    return EXIT_FAILURE;
  }

  std::string xpd_filename = argv[1];
  std::string err;
  tiny_xpd::XPD xpd;

  if (!tiny_xpd::ParseXPDFromFile(xpd_filename, &xpd, &err)) {
    if (!err.empty()) {
      std::cerr << "Parse error message: " << err << "\n";
    }

    std::cerr << "Failed to parse XPD file : " << xpd_filename << "\n";
    return EXIT_FAILURE;
  }

  PrintXPD(xpd);

  return EXIT_SUCCESS;

}

