#define TINY_XPD_IMPLEMENTATION
#include "tiny_xpd.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <random>

using namespace tiny_xpd;

static std::vector<uint8_t> GenerateSplinePrimData(const uint32_t num_faces, const uint32_t num_blocks, const uint32_t num_cvs, const uint32_t num_prims, XPDHeaderInput *header)
{

  std::random_device seed_gen;
  std::default_random_engine engine(seed_gen());

  std::uniform_real_distribution<float> dist(0.0f, 1.0f);

  std::vector<float> prim_data;

  header->primType = tiny_xpd::Xpd::PrimType::Spline;
  header->primVersion = 3;

  header->coordSpace = tiny_xpd::Xpd::CoordSpace::Object;

  header->time = 1.0f;

  header->numFaces = num_faces;
  header->numBlocks = num_blocks;
  header->numCVs = num_cvs;
  header->numPrims.resize(num_faces);

  header->block.resize(1);
  header->block[0] = "BakedGroom"; // FIXME(syoyo): define your own block name

  header->faceid.resize(num_faces, 0);
  header->primSize.resize(num_prims);


  for (size_t i = 0; i < header->faceid.size(); i++) {
    header->faceid[i] = int32_t(i);
  }

  for (size_t i = 0; i < header->primSize.size(); i++) {
    // size = 10 + cvs * 3(xyz)
    // 10 = (id, u, v, length, width, taper, taper start, width vector xyz)
    header->primSize[i] = 10 + num_cvs * 3;
  }

  // TODO(syoyo): compute primsize
  for (size_t f = 0; f < header->numFaces; f++) {
    header->numPrims[f] = num_prims;
  }

  header->blockOffset.resize(num_faces * num_blocks, 0);


  size_t offset = 0;

  for (size_t f = 0; f < header->numFaces; f++) {

    size_t id = 0;

    for (size_t b = 0; b < header->numBlocks; b++) {
      header->blockOffset[f * header->numBlocks + b] = offset;

      for (size_t i = 0; i < header->numPrims[f]; i++) {

        prim_data.push_back(float(id)); // id
        id++;
        prim_data.push_back(dist(engine)); // u
        prim_data.push_back(dist(engine)); // v

        // Assume underlying plane has extent [-5, 5]^2 and defined in xz axis.
        float root_xz[2] = {10.0f * dist(engine) - 5.0f, 10.0f * dist(engine) - 5.0f};

        // Same data layout as in `xgSplineDataToXpd` sample.
        for (size_t c = 0; c < header->numCVs; c++) {
          // Growing up in Y-axis and random jitter
          prim_data.push_back(root_xz[0] + 0.1f * dist(engine));
          prim_data.push_back(c * 1.1f);
          prim_data.push_back(root_xz[1] + 0.1f * dist(engine));
        }

        prim_data.push_back(1.0f); // length
        prim_data.push_back(0.1f + dist(engine)); // widtgh
        prim_data.push_back(1.0f); // taper
        prim_data.push_back(0.0f);

        prim_data.push_back(1.0f); // width vector x
        prim_data.push_back(0.0f); // width vector y
        prim_data.push_back(0.0f); // width vector z

        offset += header->primSize[i] * sizeof(float); // = 100 for numCVs=5
      }
    }
  }

  std::vector<uint8_t> data;
  data.resize(prim_data.size() * sizeof(float));
  memcpy(data.data(), prim_data.data(), data.size());

  return data;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Requires output.xpd\n";
    return EXIT_FAILURE;
  }

  std::string xpd_filename = argv[1];
  std::string err;
  tiny_xpd::XPDHeaderInput header_input;

  uint32_t num_faces = 3;

  if (argc > 2) {
   num_faces = uint32_t(std::stoi(argv[2]));
  }

  uint32_t num_blocks = 1;
  uint32_t num_cvs = 5;
  uint32_t num_prims = 1; // TODO(syoyo): Varying num_prims per face

  std::vector<uint8_t> prim_data = GenerateSplinePrimData(num_faces, num_blocks, num_cvs, num_prims, &header_input);

  header_input.fileVersion = 0;

  std::vector<uint8_t> xpd;

  if (!SerializeToXPD(header_input, prim_data, &xpd, &err)) {
    if (!err.empty()) {
      std::cerr << err << "\n";
    }
    std::cerr << "Failed to serialize data to XPD.\n";
    return EXIT_FAILURE;
  }

  // save it to file.
  {
    std::ofstream ofs(xpd_filename, std::ios::binary);
    if (!ofs) {
      std::cerr << "Failed to open a file for write: " << xpd_filename << "\n";
      return EXIT_FAILURE;
    }

    ofs.write(reinterpret_cast<const char *>(xpd.data()), std::streamsize(xpd.size()));
  }

  std::cout << "Wrote XPD file : " << xpd_filename << std::endl;

  return EXIT_SUCCESS;
}
