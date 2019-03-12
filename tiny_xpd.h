#ifndef TINY_XPD_H_
#define TINY_XPD_H_

/*
The MIT License (MIT)

Copyright (c) 2019 Syoyo Fujita.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <map>
#include <string>
#include <vector>

namespace tiny_xpd {

// ---------------------------------------------
// Based on Xpd header file.
struct Xpd {
  enum PrimType { Point = 0, Spline, Card, Sphere, Archive, CustomPT = 99 };
  enum CoordSpace { World = 0, Object, Local, Micro, CustomCS = 99 };
};

// ---------------------------------------------

// Based on XPD3 file format
// https://knowledge.autodesk.com/support/maya/learn-explore/caas/CloudHelp/cloudhelp/2016/ENU/Maya/files/GUID-43899CB9-CE0F-476E-9E94-591AE2F1F807-htm.html
struct XPD {
  // See XpdFile.h in XGen SDK for details.

  unsigned char fileVersion;
  Xpd::PrimType primType;
  unsigned char primVersion;
  float time;
  uint32_t numCVs;
  Xpd::CoordSpace coordSpace;
  uint32_t numFaces;

  uint32_t numBlocks;
  std::vector<std::string> block;
  std::vector<uint32_t> primSize;

  std::vector<std::string> key;
  std::map<std::string, int> keyToId;

  std::vector<int> faceid;
  std::vector<uint32_t> numPrims;
  std::vector<uint64_t> blockPosition;

  XPD()
      : fileVersion(0),
        primType(Xpd::PrimType::Point),  // TODO(syoyo): Set invalid value
        primVersion(0),
        time(0.0f),
        numCVs(0),
        coordSpace(Xpd::CoordSpace::World),  // TODO(syoyo): Set invalid value
        numFaces(0),
        numBlocks(0) {}
};

///
/// Parse XPD file from a file.
///
/// @param[in] filename XPD filename.
/// @param[out] xpd Parsed XPD data.
/// @param[out] err Error string. Filled when failed to parse XPD file.
///
/// Return false when failed to parse XPD file and report an error string to
/// `err`
///
bool ParseXPDFromFile(const std::string &filename, XPD *xpd, std::string *err);

}  // namespace tiny_xpd

#if defined(TINY_XPD_IMPLEMENTATION)

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream> // dbg

namespace tiny_xpd {

///
/// Simple stream reader
///
class StreamReader {
  static inline void swap2(unsigned short *val) {
    unsigned short tmp = *val;
    uint8_t *dst = reinterpret_cast<uint8_t *>(val);
    uint8_t *src = reinterpret_cast<uint8_t *>(&tmp);

    dst[0] = src[1];
    dst[1] = src[0];
  }

  static inline void swap4(unsigned int *val) {
    unsigned int tmp = *val;
    uint8_t *dst = reinterpret_cast<uint8_t *>(val);
    uint8_t *src = reinterpret_cast<uint8_t *>(&tmp);

    dst[0] = src[3];
    dst[1] = src[2];
    dst[2] = src[1];
    dst[3] = src[0];
  }

  static inline void swap4(int *val) {
    int tmp = *val;
    uint8_t *dst = reinterpret_cast<uint8_t *>(val);
    uint8_t *src = reinterpret_cast<uint8_t *>(&tmp);

    dst[0] = src[3];
    dst[1] = src[2];
    dst[2] = src[1];
    dst[3] = src[0];
  }

  static inline void swap8(uint64_t *val) {
    uint64_t tmp = (*val);
    uint8_t *dst = reinterpret_cast<uint8_t *>(val);
    uint8_t *src = reinterpret_cast<uint8_t *>(&tmp);

    dst[0] = src[7];
    dst[1] = src[6];
    dst[2] = src[5];
    dst[3] = src[4];
    dst[4] = src[3];
    dst[5] = src[2];
    dst[6] = src[1];
    dst[7] = src[0];
  }

  static inline void swap8(int64_t *val) {
    int64_t tmp = (*val);
    uint8_t *dst = reinterpret_cast<uint8_t *>(val);
    uint8_t *src = reinterpret_cast<uint8_t *>(&tmp);

    dst[0] = src[7];
    dst[1] = src[6];
    dst[2] = src[5];
    dst[3] = src[4];
    dst[4] = src[3];
    dst[5] = src[2];
    dst[6] = src[1];
    dst[7] = src[0];
  }

 public:
  explicit StreamReader(const uint8_t *binary, const size_t length,
                        const bool swap_endian)
      : binary_(binary), length_(length), swap_endian_(swap_endian), idx_(0) {
    (void)pad_;
  }

  bool seek_set(const uint64_t offset) {
    if (offset > length_) {
      return false;
    }

    idx_ = offset;
    return true;
  }

  bool seek_from_currect(const int64_t offset) {
    if ((int64_t(idx_) + offset) < 0) {
      return false;
    }

    if (size_t((int64_t(idx_) + offset)) > length_) {
      return false;
    }

    idx_ = size_t(int64_t(idx_) + offset);
    return true;
  }

  size_t read(const size_t n, const uint64_t dst_len, uint8_t *dst) {
    size_t len = n;
    if ((idx_ + len) > length_) {
      len = length_ - idx_;
    }

    if (len > 0) {
      if (dst_len < len) {
        // dst does not have enough space. return 0 for a while.
        return 0;
      }

      memcpy(dst, &binary_[idx_], len);
      idx_ += len;
      return len;

    } else {
      return 0;
    }
  }

  bool read1(uint8_t *ret) {
    if ((idx_ + 1) > length_) {
      return false;
    }

    const uint8_t val = binary_[idx_];

    (*ret) = val;
    idx_ += 1;

    return true;
  }

  bool read_bool(bool *ret) {
    if ((idx_ + 1) > length_) {
      return false;
    }

    const char val = static_cast<const char>(binary_[idx_]);

    (*ret) = bool(val);
    idx_ += 1;

    return true;
  }

  bool read1(char *ret) {
    if ((idx_ + 1) > length_) {
      return false;
    }

    const char val = static_cast<const char>(binary_[idx_]);

    (*ret) = val;
    idx_ += 1;

    return true;
  }

  bool read2(unsigned short *ret) {
    if ((idx_ + 2) > length_) {
      return false;
    }

    unsigned short val =
        *(reinterpret_cast<const unsigned short *>(&binary_[idx_]));

    if (swap_endian_) {
      swap2(&val);
    }

    (*ret) = val;
    idx_ += 2;

    return true;
  }

  bool read4(unsigned int *ret) {
    if ((idx_ + 4) > length_) {
      return false;
    }

    unsigned int val =
        *(reinterpret_cast<const unsigned int *>(&binary_[idx_]));

    if (swap_endian_) {
      swap4(&val);
    }

    (*ret) = val;
    idx_ += 4;

    return true;
  }

  bool read4(int *ret) {
    if ((idx_ + 4) > length_) {
      return false;
    }

    int val = *(reinterpret_cast<const int *>(&binary_[idx_]));

    if (swap_endian_) {
      swap4(&val);
    }

    (*ret) = val;
    idx_ += 4;

    return true;
  }

  bool read8(uint64_t *ret) {
    if ((idx_ + 8) > length_) {
      return false;
    }

    uint64_t val = *(reinterpret_cast<const uint64_t *>(&binary_[idx_]));

    if (swap_endian_) {
      swap8(&val);
    }

    (*ret) = val;
    idx_ += 8;

    return true;
  }

  bool read8(int64_t *ret) {
    if ((idx_ + 8) > length_) {
      return false;
    }

    int64_t val = *(reinterpret_cast<const int64_t *>(&binary_[idx_]));

    if (swap_endian_) {
      swap8(&val);
    }

    (*ret) = val;
    idx_ += 8;

    return true;
  }

  bool read_float(float *ret) {
    if (!ret) {
      return false;
    }

    float value;
    if (!read4(reinterpret_cast<int *>(&value))) {
      return false;
    }

    (*ret) = value;

    return true;
  }

  bool read_double(double *ret) {
    if (!ret) {
      return false;
    }

    double value;
    if (!read8(reinterpret_cast<uint64_t *>(&value))) {
      return false;
    }

    (*ret) = value;

    return true;
  }

  bool read_string(std::string *ret) {
    if (!ret) {
      return false;
    }

    std::string value;

    // read untile '\0' or end of stream.
    for (;;) {
      char c;
      if (!read1(&c)) {
        return false;
      }

      value.push_back(c);

      if (c == '\0') {
        break;
      }
    }

    (*ret) = value;

    return true;
  }

#if 0
  bool read_value(Value *inout) {
    if (!inout) {
      return false;
    }

    if (inout->Type() == VALUE_TYPE_FLOAT) {
      float value;
      if (!read_float(&value)) {
        return false;
      }

      (*inout) = Value(value);
    } else if (inout->Type() == VALUE_TYPE_INT) {
      int value;
      if (!read4(&value)) {
        return false;
      }

      (*inout) = Value(value);
    } else {
      TINYVDBIO_ASSERT(0);
      return false;
    }

    return true;
  }
#endif

  size_t tell() const { return idx_; }

  const uint8_t *data() const { return binary_; }

  bool swap_endian() const { return swap_endian_; }

  size_t size() const { return length_; }

 private:
  const uint8_t *binary_;
  const size_t length_;
  bool swap_endian_;
  char pad_[7];
  uint64_t idx_;
};

static bool ParseXPDHeader(StreamReader *sr, XPD *xpd, std::string *err) {
  // Header: XPD3
  uint8_t magic[4];
  if (!sr->read(4, 4, magic)) {
    if (err) {
      (*err) += "Failed to read magic number.";
    }
    return false;
  }

  if ((magic[0] == 'X') && (magic[1] == 'P') && (magic[2] == 'D') &&
      (magic[3] == '3')) {
    // ok
  } else {
    if (err) {
      (*err) += "Magic number is not a 'XPD3'.";
    }
    return false;
  }

  // fileVersion(char)
  if (!sr->read1(&xpd->fileVersion)) {
    if (err) {
      (*err) += "Failed to read `fileVersion'.";
    }
    return false;
  }

  // primType
  if (!sr->read4(reinterpret_cast<uint32_t *>(&xpd->primType))) {
    if (err) {
      (*err) += "Failed to read `primType'.";
    }
    return false;
  }

  // primVersion
  if (!sr->read1(&xpd->primVersion)) {
    if (err) {
      (*err) += "Failed to read `primVersion'";
    }
    return false;
  }

  // time
  if (!sr->read_float(&xpd->time)) {
    if (err) {
      (*err) += "Failed to read `time'.";
    }
    return false;
  }

  // numCVs
  if (!sr->read4(&xpd->numCVs)) {
    if (err) {
      (*err) += "Failed to read `numCVs'.";
    }
    return false;
  }

  // coordSpace
  if (!sr->read4(reinterpret_cast<uint32_t *>(&xpd->coordSpace))) {
    if (err) {
      (*err) += "Failed to read `coordSpace'.";
    }
    return false;
  }

  // numBlocks
  if (!sr->read4(&xpd->numBlocks)) {
    if (err) {
      (*err) += "Failed to read `numBlocks'.";
    }
    return false;
  }

  // blockSize.
  // Number of characters for all block names combined(including the end of strinc character for each block)
  {
    uint32_t blockSize(0);
    if (!sr->read4(&blockSize)) {
      if (err) {
        (*err) += "Failed to parse `blockSize`.";
      }
      return false;
    }

    std::cout << "blockSize " << blockSize << "\n";

    std::vector<char> blockNames(blockSize);

    if (!sr->read(blockSize, blockSize, reinterpret_cast<uint8_t *>(blockNames.data()))) {

      if (err) {
        (*err) += "Failed to read `blockNames'.";
      }
      return false;

    }

    // split names
    {
      size_t last_idx = 0;
      for (size_t i = 0; i < blockSize; i++) {
        if (blockNames[i] == '\0') {
          std::string name(&blockNames[last_idx], &blockNames[i]);
          xpd->block.push_back(name);
          std::cout << "name = " << name  << std::endl;
          last_idx = i + 1;
        }
      }
    }

    // primSize
    for (size_t i = 0; i < xpd->block.size(); i++) {

      uint32_t primSize;
      if (!sr->read4(&primSize)) {

        if (err) {
          (*err) += "Failed to read `primSize'.";
        }
        return false;

      }

      if (primSize < 1) {
        // ???
        if (err) {
          (*err) += "primSize value is zero.";
        }
        return false;

      }

      xpd->primSize.push_back(primSize);

      std::cout << "primSize[" << i << "] = " << primSize << std::endl;

    }


  }

  // numKeys
  // Number of characters for all key names combined(including the end of strinc character for each key)
  {
    uint32_t keySize(0);
    if (!sr->read4(&keySize)) {
      if (err) {
        (*err) += "Failed to parse `numKey`.";
      }
      return false;
    }

    std::cout << "keySize " << keySize << "\n";

    if (keySize == 0) {
      // ???
      if (err) {
        (*err) += "keySize is zero.";
      }
      return false;

    }

    std::vector<char> keyNames(keySize);

    if (!sr->read(keySize, keySize, reinterpret_cast<uint8_t *>(keyNames.data()))) {

      if (err) {
        (*err) += "Failed to read `keyNames'.";
      }
      return false;

    }

    // split names
    {
      size_t last_idx = 0;
      for (size_t i = 0; i < keySize; i++) {
        if (keyNames[i] == '\0') {
          std::string name(&keyNames[last_idx], &keyNames[i]);
          xpd->key.push_back(name);
          std::cout << "name = " << name  << std::endl;
          last_idx = i + 1;
        }
      }
    }
  }

  return true;
}

bool ParseXPDFromFile(const std::string &filename, XPD *xpd, std::string *err) {
  std::ifstream ifs(filename, std::ios::in | std::ios::binary);
  if (!ifs) {
    if (err) {
      (*err) = "Failed to open a file";
    }
    return false;
  }

  // TODO(syoyo): Use mmap
  ifs.seekg(0, ifs.end);
  size_t sz = static_cast<size_t>(ifs.tellg());
  if (int64_t(sz) < 0) {
    // Looks reading directory, not a file.
    if (err) {
      (*err) += "Looks like filename is a directory.";
    }
    return false;
  }

  if (sz < 16) {
    // ???
    if (err) {
      (*err) +=
          "File size too short. Looks like this file is not a XPD format.";
    }
    return false;
  }

  std::vector<uint8_t> data;
  data.resize(sz);

  ifs.seekg(0, ifs.beg);
  ifs.read(reinterpret_cast<char *>(&data.at(0)),
           static_cast<std::streamsize>(sz));

  // TODO(syoyo): Consider endianness
  StreamReader sr(data.data(), data.size(), /* swap endian */ false);

  if (!ParseXPDHeader(&sr, xpd, err)) {
    return false;
  }

  return true;
}

}  // namespace tiny_xpd

#endif  // TINY_XPD_IMPLEMENTATION

#endif  // TINY_XPD_H_
