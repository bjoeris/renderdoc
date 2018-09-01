#include "utils.h"

#include <array>
#include <limits>

#include "float.h"

float SFLOAT16ToSFLOAT32(uint32_t bits) {
  uint32_t M = bits & 0x3ff;
  uint32_t E = (bits >> 10) & 0x1f;
  uint32_t S = (bits >> 15) & 1;
  float sign = 1;
  if (S > 0)
    sign = -1;
  if (E == 0 && M == 0)
    return sign * float(0.0);
  else if (E == 0 && M != 0)
    return sign * float(M) / 1024.0 / 16384.0;
  else if (E == 31 && M == 0)
    return sign * std::numeric_limits<float>::infinity();
  else if (E == 31 && M != 0) {
#if defined(_WIN32)
    OutputDebugStringA("NaN \n");
#endif
    return 0;
  } else if (E <= 15)
    return sign * (1 + float(M) / 1024.0) / float(uint32_t(1 << (15 - E)));
  else
    return sign * (1 + float(M) / 1024.0) * float(uint32_t(1 << (E - 15)));
}

struct half {
  uint16_t value;

  half(float v) {}

  operator float() {
    return SFLOAT16ToSFLOAT32(value);
  }

  operator double() {
    return SFLOAT16ToSFLOAT32(value);
  }
};

#pragma pack(push)
#pragma pack(1)

struct depth24 {
  uint8_t value[3];

  depth24(float v) {}

  operator float() {
    uint32_t v = 0;
    v |= ((uint32_t) value[0]) << 16;
    v |= ((uint32_t) value[1]) << 8;
    v |= ((uint32_t) value[2]);
    return 1.0f * v / 0x00FFFFFF;
  }

  operator double() {
    uint32_t v = 0;
    v |= ((uint32_t) value[0]) << 16;
    v |= ((uint32_t) value[1]) << 8;
    v |= ((uint32_t) value[2]);
    return 1.0 * v / 0x00FFFFFF;
  }
};

struct rgb8 {
  uint8_t value[3];

  rgb8(uint32_t lum) {
    value[0] = lum;
    value[1] = lum;
    value[2] = lum;
  }
};
#pragma pack(pop)


uint8_t SFLOAT32ToUint8(float f) {
  if (f < 0.0f) {
    return 0;
  }
  if (f > 1.0f) {
    return 255;
  }
  return uint8_t(f * 255);
}

uint8_t SRGBToUint8(uint8_t x) {
  float f = x / 255.0f;
  if (f <= 0.0f)
    return 0;
  else if (f >= 1.0f)
    return 255;
  else if (f < 0.04045f)
    return SFLOAT32ToUint8(f / 12.92f);
  else
    return SFLOAT32ToUint8(std::pow((f + 0.055f) / 1.055f, 2.4f));

  if (f < 0.0f) {
    return 0;
  }
  if (f > 1.0f) {
    return 255;
  }
  return uint8_t(f * 255);
}

uint8_t UFLOAT11ToUint8(uint32_t x) {
  return SFLOAT32ToUint8(SFLOAT16ToSFLOAT32((x & 0x7FF) << 4));
}

uint8_t UFLOAT10ToUint8(uint32_t x) {
  return SFLOAT32ToUint8(SFLOAT16ToSFLOAT32((x & 0x7FF) << 5));
}

template <class T>
bool isNormal(T v) {
  return true;
}

template <> bool isNormal<float>(float v) {
  return std::isnormal(v) && v < FLT_MAX && v > -FLT_MAX;
}

template <> bool isNormal<double>(double v) {
  return std::isnormal(v) && v < FLT_MAX && v > -FLT_MAX;
}

template <class T, typename LargeType, typename IntType>
void histogramEqualization(T * data, uint32_t w, uint32_t h, uint32_t channels,
  LargeType absMin, LargeType absMax) {
  std::vector<std::array<uint32_t, 256>> bins(channels);

  std::vector<LargeType> maxV(channels);
  std::vector<LargeType> minV(channels);
  for (uint32_t c = 0; c < channels; c++) {
    maxV[c] = absMin;
    minV[c] = absMax;
  }

  for (uint32_t j = 0; j < h; j++) {
    for (uint32_t i = 0; i < w; i++) {
      for (uint32_t c = 0; c < channels; c++) {
        LargeType v = (LargeType) data[(i + j * w) * channels + c];
        if (!(isNormal(v) || v == 0) || v > absMax || v < absMin)
          continue; // skip inf and nan values
        maxV[c] = maxV[c] > v ? maxV[c] : v;
        minV[c] = minV[c] < v ? minV[c] : v;
      }
    }
  }

  std::vector<LargeType> denom(channels);
  for (uint32_t c = 0; c < channels; c++) {
    denom[c] = maxV[c] - minV[c];
    if (abs((long double) denom[c]) < FLT_EPSILON)
      denom[c] = LargeType(1.0);
  }

  for (uint32_t j = 0; j < h; j++) {
    for (uint32_t i = 0; i < w; i++) {
      for (uint32_t c = 0; c < channels; c++) {
        LargeType v = data[(i + j * w) * channels + c];
        if (!(isNormal(v) || v == 0) || v > (absMax) || v < absMin) {
          continue;
        }
        uint32_t bin = 255 * ((v - minV[c]) / denom[c]);
        bins[c][bin]++;
      }
    }
  }

  std::vector<std::array<double, 256>> probability(channels);
  for (uint32_t c = 0; c < channels; c++) {
    probability[c][0] = bins[c][0];
    for (uint32_t i = 1; i < 256; i++) {
      probability[c][i] = probability[c][i - 1] + bins[c][i];
    }
    for (uint32_t i = 0; i < 256; i++) {
      probability[c][i] /= double(probability[c][255]);
    }
  }

  IntType *ptr = (IntType *) data;
  for (uint32_t j = 0; j < h; j++) {
    for (uint32_t i = 0; i < w; i++) {
      for (uint32_t c = 0; c < channels; c++) {
        LargeType v = data[(i + j * w) * channels + c];
        if (!(isNormal(v) || v == 0) || v > absMax || v < absMin) {
          if (v > absMax)
            v = maxV[c];
          if (v < absMin)
            v = minV[c];
        }
        uint32_t bin = 255 * (v - minV[c]) / denom[c];
        ptr[(i + j * w) * channels + c] = 255 * probability[c][bin];
      }
    }
  }
}

#define HISTOGRAM_EQ_SWITCH_FP(bits, channels, minV, maxV) switch (bits) {\
    case 16: histogramEqualization<half, float, uint16_t>((half *) p, w, h, channels, minV, maxV); break; \
    case 24: histogramEqualization<depth24, float, rgb8>((depth24 *) p, w, h, channels, minV, maxV); break; \
    case 32: histogramEqualization<float, float, uint32_t>((float *) p, w, h, channels, minV, maxV); break; \
    case 64: histogramEqualization<double, double, uint64_t>((double *) p, w, h, channels, minV, maxV); break; \
    default: assert(0);                                                           \
  }

#define HISTOGRAM_EQ_SWITCH_UNSIGNED(bits, channels) switch (bits) {\
    case 8: histogramEqualization<uint8_t, uint8_t, uint8_t>((uint8_t *) p, w, h, channels, 0, UINT8_MAX); break; \
    case 16: histogramEqualization<uint16_t, uint16_t, uint16_t>((uint16_t *) p, w, h, channels, 0, UINT16_MAX); break; \
    case 32: histogramEqualization<uint32_t, uint32_t, uint32_t>((uint32_t *) p, w, h, channels, 0, UINT32_MAX); break; \
    case 64: histogramEqualization<uint64_t, uint64_t, uint64_t>((uint64_t *) p, w, h, channels, 0, UINT64_MAX); break; \
    default: assert(0);                                                           \
  }

#define HISTOGRAM_EQ_SWITCH_SIGNED(bits, channels) switch (bits) {\
    case 16: histogramEqualization<int16_t, int16_t, int16_t>((int16_t *) p, w, h, channels, INT16_MIN, INT16_MAX); break; \
    case 32: histogramEqualization<int32_t, int32_t, int32_t>((int32_t *) p, w, h, channels, INT32_MIN, INT32_MAX); break; \
    case 64: histogramEqualization<int64_t, int64_t, int64_t>((int64_t *) p, w, h, channels, INT64_MIN, INT64_MAX); break; \
    default: assert(0);                                                           \
  }

bool fillPPM(void *output, void *input, uint32_t w, uint32_t h, VkFormat format, bool isStencil) {
  unsigned char *out = (unsigned char *) output;
  uint8_t *p = (uint8_t *) input;
  uint32_t size_in_bytes = 0;
  uint32_t channels = ChannelsInFormat(format);
  uint32_t bits = BitsPerChannelInFormat(format);
  bool isFP = IsFPFormat(format);
  bool isHDR = isHDRFormat(format);
  bool isSigned = IsSignedFormat(format);
  bool isDepth = IsDepthFormat(format);
  if (!isStencil) {
    size_in_bytes = SizeOfFormat(format, VK_IMAGE_ASPECT_COLOR_BIT);
    if (isHDR) {
      if (isFP) {
        if (isDepth) {
          HISTOGRAM_EQ_SWITCH_FP(bits, channels, 0.0, 1.0);
        } else {
          HISTOGRAM_EQ_SWITCH_FP(bits, channels, -65505.0, 65505.0);
        }
      } else {
        if (isSigned) {
          HISTOGRAM_EQ_SWITCH_SIGNED(bits, channels);
        } else {
          HISTOGRAM_EQ_SWITCH_UNSIGNED(bits, channels);
        }
      }
    }
  } else {
    size_in_bytes = SizeOfFormat(format, VK_IMAGE_ASPECT_DEPTH_BIT);
    p += size_in_bytes * w * h; // skip depth bytes
    HISTOGRAM_EQ_SWITCH_UNSIGNED(8, 1);
    size_in_bytes = SizeOfFormat(format, VK_IMAGE_ASPECT_STENCIL_BIT);
  }

#undef HISTOGRAM_EQ_SWITCH_FP
#undef HISTOGRAM_EQ_SWITCH_SIGNED
#undef HISTOGRAM_EQ_SWITCH_UNSIGNED

  for (uint32_t r = 0; r < h; ++r) {
    for (uint32_t c = 0; c < w; ++c) {
      switch (format) {
        case VK_FORMAT_R4G4_UNORM_PACK8:
        {
          uint8_t v = *p;
          *out++ = ((v >> 4) & 0x0f) * 16;       // R
          *out++ = (v & 0x0f) * 16;              // G
          *out++ = 0;                            // B
        }
        break;

        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        {
          uint16_t v = *(uint16_t *) p;
          *out++ = ((v >> 12) & 0x000f) * 16;    // R
          *out++ = ((v >> 8) & 0x000f) * 16;     // G
          *out++ = ((v >> 4) & 0x000f) * 16;     // B
        }
        break;

        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        {
          uint16_t v = *(uint16_t *) p;
          *out++ = ((v >> 4) & 0x000f) * 16;     // R
          *out++ = ((v >> 8) & 0x000f) * 16;     // G
          *out++ = ((v >> 12) & 0x000f) * 16;    // B
        }
        break;

        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        {
          uint16_t v = *(uint16_t *) p;
          *out++ = ((v >> 11) & 0x001f) * 8;     // R
          *out++ = ((v >> 5) & 0x003f) * 4;      // G
          *out++ = (v & 0x001f) * 8;             // B
        }
        break;

        case VK_FORMAT_B5G6R5_UNORM_PACK16:
        {
          uint16_t v = *(uint16_t *) p;
          *out++ = (v & 0x001f) * 8;             // R
          *out++ = ((v >> 5) & 0x003f) * 4;      // G
          *out++ = ((v >> 11) & 0x001f) * 8;     // B
        }
        break;

        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        {
          uint16_t v = *(uint16_t *) p;
          *out++ = ((v >> 11) & 0x001f) * 8;     // R
          *out++ = ((v >> 6) & 0x001f) * 8;      // G
          *out++ = ((v >> 1) & 0x001f) * 8;      // B
        }
        break;

        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        {
          uint16_t v = *(uint16_t *) p;
          *out++ = ((v >> 1) & 0x001f) * 8;      // R
          *out++ = ((v >> 6) & 0x001f) * 8;      // G
          *out++ = ((v >> 11) & 0x001f) * 8;     // B
        }
        break;

        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        {
          uint16_t v = *(uint16_t *) p;
          *out++ = ((v >> 10) & 0x001f) * 8;     // R
          *out++ = ((v >> 5) & 0x001f) * 8;      // G
          *out++ = (v & 0x001f) * 8;             // B
        }
        break;

        // All 8 bit unsigned formats can be copied without modifications.
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8_USCALED:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_USCALED:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_USCALED:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_USCALED:
        case VK_FORMAT_R8G8B8A8_UINT:
        // All 8 bit signed formats can be copied but we need to flip the sign bit.
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_SSCALED:
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_SSCALED:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_SSCALED:
        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_R8G8B8A8_SINT:
        {
          for (uint32_t c = 0; c < std::min<uint32_t>(channels, 3); c++)
            *out++ = isSigned ? p[c] ^ 0x80 : p[c];
          for (uint32_t c = channels; c < 3; c++) // alpha is dropped
            *out++ = 0;
        }
        break;

        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_R8G8B8A8_SRGB:
        {
          for (uint32_t c = 0; c < std::min<uint32_t>(channels, 3); c++)
            *out++ = SRGBToUint8(*(p + c));
          for (uint32_t c = channels; c < 3; c++) // alpha is dropped
            *out++ = 0;
        }
        break;

        case VK_FORMAT_B8G8R8_UNORM:
        case VK_FORMAT_B8G8R8_USCALED:
        case VK_FORMAT_B8G8R8_UINT:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_USCALED:
        case VK_FORMAT_B8G8R8A8_UINT:
        case VK_FORMAT_B8G8R8_SNORM:
        case VK_FORMAT_B8G8R8_SSCALED:
        case VK_FORMAT_B8G8R8_SINT:
        case VK_FORMAT_B8G8R8A8_SNORM:
        case VK_FORMAT_B8G8R8A8_SSCALED:
        case VK_FORMAT_B8G8R8A8_SINT:
        {
          *out++ = isSigned ? p[2] ^ 0x80 : p[2];    // R
          *out++ = isSigned ? p[1] ^ 0x80 : p[1];    // G
          *out++ = isSigned ? p[0] ^ 0x80 : p[0];    // B
        }
        break;

        case VK_FORMAT_B8G8R8_SRGB:
        case VK_FORMAT_B8G8R8A8_SRGB:
        {
          *out++ = SRGBToUint8(p[2]);    // R
          *out++ = SRGBToUint8(p[1]);    // G
          *out++ = SRGBToUint8(p[0]);    // B
        }
        break;

        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = v & 0xff;            // R
          *out++ = (v >> 8) & 0xff;     // G
          *out++ = (v >> 16) & 0xff;    // B
        }
        break;

        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (v & 0xff) ^ 0x80;            // R
          *out++ = ((v >> 8) & 0xff) ^ 0x80;     // G
          *out++ = ((v >> 16) & 0xff) ^ 0x80;    // B
        }
        break;

        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = v & 0xff;            // R
          *out++ = (v >> 8) & 0xff;     // G
          *out++ = (v >> 16) & 0xff;    // B
        }
        break;

        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (v & 0xff) ^ 0x80;            // R
          *out++ = ((v >> 8) & 0xff) ^ 0x80;     // G
          *out++ = ((v >> 16) & 0xff) ^ 0x80;    // B
        }
        break;

        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = v & 0xff;            // R
          *out++ = (v >> 8) & 0xff;     // G
          *out++ = (v >> 16) & 0xff;    // B
        }
        break;

        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (v & 0xff) ^ 0x80;            // R
          *out++ = ((v >> 8) & 0xff) ^ 0x80;     // G
          *out++ = ((v >> 16) & 0xff) ^ 0x80;    // B
        }
        break;

        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = SRGBToUint8(v & 0xff);            // R
          *out++ = SRGBToUint8((v >> 8) & 0xff);     // G
          *out++ = SRGBToUint8((v >> 16) & 0xff);    // B
        }
        break;

        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = ((v >> 20) & 0x3ff) / 4;    // R
          *out++ = ((v >> 10) & 0x3ff) / 4;    // G
          *out++ = (v & 0x3ff) / 4;            // B
        }
        break;

        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (((v >> 20) & 0x3ff) / 4) ^ 0x80;    // R
          *out++ = (((v >> 10) & 0x3ff) / 4) ^ 0x80;    // G
          *out++ = ((v & 0x3ff) / 4) ^ 0x80;            // B
        }
        break;

        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = ((v >> 20) & 0x3ff) / 4;    // R
          *out++ = ((v >> 10) & 0x3ff) / 4;    // G
          *out++ = (v & 0x3ff) / 4;            // B
        }
        break;

        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (((v >> 20) & 0x3ff) / 4) ^ 0x80;    // R
          *out++ = (((v >> 10) & 0x3ff) / 4) ^ 0x80;    // G
          *out++ = ((v & 0x3ff) / 4) ^ 0x80;            // B
        }
        break;

        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = ((v >> 20) & 0x3ff) / 4;    // R
          *out++ = ((v >> 10) & 0x3ff) / 4;    // G
          *out++ = (v & 0x3ff) / 4;            // B
        }
        break;

        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (((v >> 20) & 0x3ff) / 4) ^ 0x80;    // R
          *out++ = (((v >> 10) & 0x3ff) / 4) ^ 0x80;    // G
          *out++ = ((v & 0x3ff) / 4) ^ 0x80;            // B
        }
        break;

        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (v & 0x3ff) / 4;            // R
          *out++ = ((v >> 10) & 0x3ff) / 4;    // G
          *out++ = ((v >> 20) & 0x3ff) / 4;    // B
        }
        break;

        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = ((v & 0x3ff) / 4) ^ 0x80;            // R
          *out++ = (((v >> 10) & 0x3ff) / 4) ^ 0x80;    // G
          *out++ = (((v >> 20) & 0x3ff) / 4) ^ 0x80;    // B
        }
        break;

        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (v & 0x3ff) / 4;            // R
          *out++ = ((v >> 10) & 0x3ff) / 4;    // G
          *out++ = ((v >> 20) & 0x3ff) / 4;    // B
        }
        break;

        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = ((v & 0x3ff) / 4) ^ 0x80;            // R
          *out++ = (((v >> 10) & 0x3ff) / 4) ^ 0x80;    // G
          *out++ = (((v >> 20) & 0x3ff) / 4) ^ 0x80;    // B
        }
        break;

        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = (v & 0x3ff) / 4;            // R
          *out++ = ((v >> 10) & 0x3ff) / 4;    // G
          *out++ = ((v >> 20) & 0x3ff) / 4;    // B
        }
        break;

        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = ((v & 0x3ff) / 4) ^ 0x80;            // R
          *out++ = (((v >> 10) & 0x3ff) / 4) ^ 0x80;    // G
          *out++ = (((v >> 20) & 0x3ff) / 4) ^ 0x80;    // B
        }
        break;

        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16_USCALED:
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_R16G16_USCALED:
        case VK_FORMAT_R16G16B16_UNORM:
        case VK_FORMAT_R16G16B16_USCALED:
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16B16A16_USCALED:
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_SSCALED:
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_SSCALED:
        case VK_FORMAT_R16G16B16_SNORM:
        case VK_FORMAT_R16G16B16_SSCALED:
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_SSCALED:
        {
          for (uint32_t c = 0; c < std::min<uint32_t>(channels, 3); c++)
            *out++ = isSigned ? (((uint16_t*) p)[c] / 256) ^ 0x80 : ((uint16_t*) p)[c] / 256;
          for (uint32_t c = channels; c < 3; c++)
            *out++ = 0;
        }
        break;

        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16_SINT:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R16G16B16_UINT:
        case VK_FORMAT_R16G16B16_SINT:
        case VK_FORMAT_R16G16B16_SFLOAT:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        {
          for (uint32_t c = 0; c < std::min<uint32_t>(channels, 3); c++)
            *out++ = ((uint16_t*) p)[c];
          for (uint32_t c = channels; c < 3; c++)
            *out++ = 0;
        }
        break;

        case VK_FORMAT_R32_UINT:
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_R32G32_UINT:
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R32G32B32_UINT:
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_SFLOAT:
        case VK_FORMAT_R32G32B32A32_UINT:
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        {
          for (uint32_t c = 0; c < std::min<uint32_t>(channels, 3); c++)
            *out++ = ((uint32_t*) p)[c];
          for (uint32_t c = channels; c < 3; c++)
            *out++ = 0;
        }
        break;

        case VK_FORMAT_R64_UINT:
        case VK_FORMAT_R64_SINT:
        case VK_FORMAT_R64_SFLOAT:
        case VK_FORMAT_R64G64_UINT:
        case VK_FORMAT_R64G64_SINT:
        case VK_FORMAT_R64G64_SFLOAT:
        case VK_FORMAT_R64G64B64_UINT:
        case VK_FORMAT_R64G64B64_SINT:
        case VK_FORMAT_R64G64B64_SFLOAT:
        case VK_FORMAT_R64G64B64A64_UINT:
        case VK_FORMAT_R64G64B64A64_SINT:
        case VK_FORMAT_R64G64B64A64_SFLOAT:
        {
          for (uint32_t c = 0; c < std::min<uint32_t>(channels, 3); c++)
            *out++ = ((uint64_t*) p)[c];
          for (uint32_t c = channels; c < 3; c++)
            *out++ = 0;
        }
        break;

        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        {
          uint32_t v = *(uint32_t *) p;
          *out++ = UFLOAT11ToUint8(v & 0x7ff);            // R
          *out++ = UFLOAT11ToUint8((v >> 11) & 0x7ff);    // G
          *out++ = UFLOAT10ToUint8((v >> 22) & 0x3ff);    // B
        }
        break;

        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        {
          if (isStencil) {
            *out++ = p[0];    // R
            *out++ = p[0];    // G
            *out++ = p[0];    // B
          } else {
            *out++ = ((uint16_t *) p)[0];    // R
            *out++ = ((uint16_t *) p)[0];    // G
            *out++ = ((uint16_t *) p)[0];    // B
          }
        }
        break;

        case VK_FORMAT_D24_UNORM_S8_UINT:
        {
          if (isStencil) {
            *out++ = p[0];    // R
            *out++ = p[0];    // G
            *out++ = p[0];    // B
          } else {
            *out++ = p[0];
            *out++ = p[1];
            *out++ = p[2];
          }
        }
        break;

        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
        {
          if (isStencil) {
            *out++ = p[0];    // R
            *out++ = p[0];    // G
            *out++ = p[0];    // B
          } else {
            *out++ = ((uint32_t *) p)[0];    // R
            *out++ = ((uint32_t *) p)[0];    // G
            *out++ = ((uint32_t *) p)[0];    // B
          }
        }
        break;

        default: return false;
      }
      p += size_in_bytes;
    }
  }

  return true;
}