#define TINY_XPD_IMPLEMENTATION
#include "tiny_xpd.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

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

static void GetPrimData(const tiny_xpd::XPDHeader &xpd, const std::vector<uint8_t> &xpd_data, size_t face_idx, size_t block_idx,  std::vector<float> *prims)
{

  prims->clear();

  uint32_t num_prims = xpd.numPrims[face_idx];
  if (num_prims == 0) {
    return;
  }

  // Flatten primitive values
  for (size_t p = 0; p < num_prims; p++) {

    // Pritive value is always float.
    size_t num_bytes = sizeof(float) * xpd.primSize[p];
    const size_t src_offset = xpd.blockPosition[face_idx * xpd.numBlocks + block_idx];
    std::vector<float> buffer;
    buffer.resize(xpd.primSize[p]);
    memcpy(buffer.data(), xpd_data.data() + src_offset, num_bytes);

    prims->insert(prims->end(), buffer.begin(), buffer.end());

  }



}

static void PrintXPD(const tiny_xpd::XPDHeader &xpd,
                     const std::vector<uint8_t> &xpd_data) {
  (void)xpd_data;

  std::cout << "fileVersion : " << int(xpd.fileVersion) << "\n";
  std::cout << "primType : " << PrintPrimType(xpd.primType) << "\n";
  std::cout << "primVersion : " << int(xpd.primVersion) << "\n";
  std::cout << "time : " << xpd.time << "\n";
  std::cout << "numCVs : " << xpd.numCVs << "\n";
  std::cout << "coordSpace : " << PrintCoordSpace(xpd.coordSpace) << "\n";
  std::cout << "numBlocks : " << xpd.numBlocks << "\n";
  std::cout << "numFaces : " << xpd.numFaces << "\n";

  // print blockPositon
  for (size_t i = 0; i < xpd.blockPosition.size(); i++) {
    std::cout << "blockPosition[" << i << "] = " << xpd.blockPosition[i]
              << "\n";
  }

  // foreach face.
  for (size_t f = 0; f < xpd.numFaces; f++) {
    // foreach block
    for (size_t b = 0; b < xpd.numBlocks; b++) {
      std::cout << "face[" << f << "] block[" << b
                << "].numPrims = " << xpd.numPrims[f] << "\n";

      std::vector<float> prims;
      GetPrimData(xpd, xpd_data, f, b, &prims);
      std::cout << "{";
      for (size_t p = 0; p < prims.size(); p++) {
        std::cout << prims[p];
        if (p != (prims.size() - 1)) {
          std::cout << ", ";
        }
      }
      std::cout << "}\n";
    }
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Requires input.xpd\n";
    return EXIT_FAILURE;
  }

  std::string xpd_filename = argv[1];
  std::string err;
  tiny_xpd::XPDHeader xpd_header;
  std::vector<uint8_t> xpd_data;

  if (!tiny_xpd::ParseXPDFromFile(xpd_filename, &xpd_header, &xpd_data, &err)) {
    if (!err.empty()) {
      std::cerr << "Parse error message: " << err << "\n";
    }

    std::cerr << "Failed to parse XPD file : " << xpd_filename << "\n";
    return EXIT_FAILURE;
  }

  if (xpd_header.primType != tiny_xpd::Xpd::PrimType::Spline) {
    std::cerr << "Currently we only support Spline primitive.\n";
    return EXIT_FAILURE;
  }

  PrintXPD(xpd_header, xpd_data);

  return EXIT_SUCCESS;
}
