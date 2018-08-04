#include <fstream>
#include <functional>
#include <limits>
#include <map>

#include "helper/helper.h"
#include "shim_vulkan.h"
#include "utils.h"

uint32_t sw_width = 1920;
uint32_t sw_height = 1080;
std::map<VkImage, VkDeviceMemory> stagingImages;
std::map<VkBuffer, VkDeviceMemory> stagingBuffers;

#define FORMAT_ASPECT_BYTE_SIZE(f, a, bs)                                                 \
  {                                                                                       \
    VK_FORMAT_##f, std::pair<VkImageAspectFlags, uint32_t>(VK_IMAGE_ASPECT_##a##_BIT, bs) \
  }

// TODO(haiyu): This is not a complete list.
std::map<VkFormat, std::pair<VkImageAspectFlags, uint32_t>> imageFormatInfoMap = {
  FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_UNORM, COLOR, 4),
  FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_USCALED, COLOR, 4),
  FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_UINT, COLOR, 4),
  FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_SNORM, COLOR, 4),
  FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_SSCALED, COLOR, 4),
  FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_SINT, COLOR, 4),
  FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_UNORM, COLOR, 4),
  FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_USCALED, COLOR, 4),
  FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_UINT, COLOR, 4),
  FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_SNORM, COLOR, 4),
  FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_SSCALED, COLOR, 4),
  FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_SINT, COLOR, 4),
  FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_UNORM, COLOR, 8),
  FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_SNORM, COLOR, 8),
  FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_USCALED, COLOR, 8),
  FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_SSCALED, COLOR, 8),
  FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_UINT, COLOR, 8),
  FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_SINT, COLOR, 8),
  FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_SFLOAT, COLOR, 8),
  FORMAT_ASPECT_BYTE_SIZE(D32_SFLOAT_S8_UINT, DEPTH, 4),
  FORMAT_ASPECT_BYTE_SIZE(D24_UNORM_S8_UINT, DEPTH, 4),
  FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_UNORM_PACK32, COLOR, 4),
  FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_SNORM_PACK32, COLOR, 4),
  FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_USCALED_PACK32, COLOR, 4),
  FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_SSCALED_PACK32, COLOR, 4),
  FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_UINT_PACK32, COLOR, 4),
  FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_SINT_PACK32, COLOR, 4),
};

std::string VkFormatToString(VkFormat f)
{
#define RETURN_VK_FORMAT_STRING(x) \
  case VK_FORMAT_##x: return std::string(var_to_string(x));

  switch(f)
  {
    RETURN_VK_FORMAT_STRING(UNDEFINED);
    RETURN_VK_FORMAT_STRING(R4G4_UNORM_PACK8);
    RETURN_VK_FORMAT_STRING(R4G4B4A4_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(B4G4R4A4_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(R5G6B5_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(B5G6R5_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(R5G5B5A1_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(B5G5R5A1_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(A1R5G5B5_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(R8_UNORM);
    RETURN_VK_FORMAT_STRING(R8_SNORM);
    RETURN_VK_FORMAT_STRING(R8_USCALED);
    RETURN_VK_FORMAT_STRING(R8_SSCALED);
    RETURN_VK_FORMAT_STRING(R8_UINT);
    RETURN_VK_FORMAT_STRING(R8_SINT);
    RETURN_VK_FORMAT_STRING(R8_SRGB);
    RETURN_VK_FORMAT_STRING(R8G8_UNORM);
    RETURN_VK_FORMAT_STRING(R8G8_SNORM);
    RETURN_VK_FORMAT_STRING(R8G8_USCALED);
    RETURN_VK_FORMAT_STRING(R8G8_SSCALED);
    RETURN_VK_FORMAT_STRING(R8G8_UINT);
    RETURN_VK_FORMAT_STRING(R8G8_SINT);
    RETURN_VK_FORMAT_STRING(R8G8_SRGB);
    RETURN_VK_FORMAT_STRING(R8G8B8_UNORM);
    RETURN_VK_FORMAT_STRING(R8G8B8_SNORM);
    RETURN_VK_FORMAT_STRING(R8G8B8_USCALED);
    RETURN_VK_FORMAT_STRING(R8G8B8_SSCALED);
    RETURN_VK_FORMAT_STRING(R8G8B8_UINT);
    RETURN_VK_FORMAT_STRING(R8G8B8_SINT);
    RETURN_VK_FORMAT_STRING(R8G8B8_SRGB);
    RETURN_VK_FORMAT_STRING(B8G8R8_UNORM);
    RETURN_VK_FORMAT_STRING(B8G8R8_SNORM);
    RETURN_VK_FORMAT_STRING(B8G8R8_USCALED);
    RETURN_VK_FORMAT_STRING(B8G8R8_SSCALED);
    RETURN_VK_FORMAT_STRING(B8G8R8_UINT);
    RETURN_VK_FORMAT_STRING(B8G8R8_SINT);
    RETURN_VK_FORMAT_STRING(B8G8R8_SRGB);
    RETURN_VK_FORMAT_STRING(R8G8B8A8_UNORM);
    RETURN_VK_FORMAT_STRING(R8G8B8A8_SNORM);
    RETURN_VK_FORMAT_STRING(R8G8B8A8_USCALED);
    RETURN_VK_FORMAT_STRING(R8G8B8A8_SSCALED);
    RETURN_VK_FORMAT_STRING(R8G8B8A8_UINT);
    RETURN_VK_FORMAT_STRING(R8G8B8A8_SINT);
    RETURN_VK_FORMAT_STRING(R8G8B8A8_SRGB);
    RETURN_VK_FORMAT_STRING(B8G8R8A8_UNORM);
    RETURN_VK_FORMAT_STRING(B8G8R8A8_SNORM);
    RETURN_VK_FORMAT_STRING(B8G8R8A8_USCALED);
    RETURN_VK_FORMAT_STRING(B8G8R8A8_SSCALED);
    RETURN_VK_FORMAT_STRING(B8G8R8A8_UINT);
    RETURN_VK_FORMAT_STRING(B8G8R8A8_SINT);
    RETURN_VK_FORMAT_STRING(B8G8R8A8_SRGB);
    RETURN_VK_FORMAT_STRING(A8B8G8R8_UNORM_PACK32);
    RETURN_VK_FORMAT_STRING(A8B8G8R8_SNORM_PACK32);
    RETURN_VK_FORMAT_STRING(A8B8G8R8_USCALED_PACK32);
    RETURN_VK_FORMAT_STRING(A8B8G8R8_SSCALED_PACK32);
    RETURN_VK_FORMAT_STRING(A8B8G8R8_UINT_PACK32);
    RETURN_VK_FORMAT_STRING(A8B8G8R8_SINT_PACK32);
    RETURN_VK_FORMAT_STRING(A8B8G8R8_SRGB_PACK32);
    RETURN_VK_FORMAT_STRING(A2R10G10B10_UNORM_PACK32);
    RETURN_VK_FORMAT_STRING(A2R10G10B10_SNORM_PACK32);
    RETURN_VK_FORMAT_STRING(A2R10G10B10_USCALED_PACK32);
    RETURN_VK_FORMAT_STRING(A2R10G10B10_SSCALED_PACK32);
    RETURN_VK_FORMAT_STRING(A2R10G10B10_UINT_PACK32);
    RETURN_VK_FORMAT_STRING(A2R10G10B10_SINT_PACK32);
    RETURN_VK_FORMAT_STRING(A2B10G10R10_UNORM_PACK32);
    RETURN_VK_FORMAT_STRING(A2B10G10R10_SNORM_PACK32);
    RETURN_VK_FORMAT_STRING(A2B10G10R10_USCALED_PACK32);
    RETURN_VK_FORMAT_STRING(A2B10G10R10_SSCALED_PACK32);
    RETURN_VK_FORMAT_STRING(A2B10G10R10_UINT_PACK32);
    RETURN_VK_FORMAT_STRING(A2B10G10R10_SINT_PACK32);
    RETURN_VK_FORMAT_STRING(R16_UNORM);
    RETURN_VK_FORMAT_STRING(R16_SNORM);
    RETURN_VK_FORMAT_STRING(R16_USCALED);
    RETURN_VK_FORMAT_STRING(R16_SSCALED);
    RETURN_VK_FORMAT_STRING(R16_UINT);
    RETURN_VK_FORMAT_STRING(R16_SINT);
    RETURN_VK_FORMAT_STRING(R16_SFLOAT);
    RETURN_VK_FORMAT_STRING(R16G16_UNORM);
    RETURN_VK_FORMAT_STRING(R16G16_SNORM);
    RETURN_VK_FORMAT_STRING(R16G16_USCALED);
    RETURN_VK_FORMAT_STRING(R16G16_SSCALED);
    RETURN_VK_FORMAT_STRING(R16G16_UINT);
    RETURN_VK_FORMAT_STRING(R16G16_SINT);
    RETURN_VK_FORMAT_STRING(R16G16_SFLOAT);
    RETURN_VK_FORMAT_STRING(R16G16B16_UNORM);
    RETURN_VK_FORMAT_STRING(R16G16B16_SNORM);
    RETURN_VK_FORMAT_STRING(R16G16B16_USCALED);
    RETURN_VK_FORMAT_STRING(R16G16B16_SSCALED);
    RETURN_VK_FORMAT_STRING(R16G16B16_UINT);
    RETURN_VK_FORMAT_STRING(R16G16B16_SINT);
    RETURN_VK_FORMAT_STRING(R16G16B16_SFLOAT);
    RETURN_VK_FORMAT_STRING(R16G16B16A16_UNORM);
    RETURN_VK_FORMAT_STRING(R16G16B16A16_SNORM);
    RETURN_VK_FORMAT_STRING(R16G16B16A16_USCALED);
    RETURN_VK_FORMAT_STRING(R16G16B16A16_SSCALED);
    RETURN_VK_FORMAT_STRING(R16G16B16A16_UINT);
    RETURN_VK_FORMAT_STRING(R16G16B16A16_SINT);
    RETURN_VK_FORMAT_STRING(R16G16B16A16_SFLOAT);
    RETURN_VK_FORMAT_STRING(R32_UINT);
    RETURN_VK_FORMAT_STRING(R32_SINT);
    RETURN_VK_FORMAT_STRING(R32_SFLOAT);
    RETURN_VK_FORMAT_STRING(R32G32_UINT);
    RETURN_VK_FORMAT_STRING(R32G32_SINT);
    RETURN_VK_FORMAT_STRING(R32G32_SFLOAT);
    RETURN_VK_FORMAT_STRING(R32G32B32_UINT);
    RETURN_VK_FORMAT_STRING(R32G32B32_SINT);
    RETURN_VK_FORMAT_STRING(R32G32B32_SFLOAT);
    RETURN_VK_FORMAT_STRING(R32G32B32A32_UINT);
    RETURN_VK_FORMAT_STRING(R32G32B32A32_SINT);
    RETURN_VK_FORMAT_STRING(R32G32B32A32_SFLOAT);
    RETURN_VK_FORMAT_STRING(R64_UINT);
    RETURN_VK_FORMAT_STRING(R64_SINT);
    RETURN_VK_FORMAT_STRING(R64_SFLOAT);
    RETURN_VK_FORMAT_STRING(R64G64_UINT);
    RETURN_VK_FORMAT_STRING(R64G64_SINT);
    RETURN_VK_FORMAT_STRING(R64G64_SFLOAT);
    RETURN_VK_FORMAT_STRING(R64G64B64_UINT);
    RETURN_VK_FORMAT_STRING(R64G64B64_SINT);
    RETURN_VK_FORMAT_STRING(R64G64B64_SFLOAT);
    RETURN_VK_FORMAT_STRING(R64G64B64A64_UINT);
    RETURN_VK_FORMAT_STRING(R64G64B64A64_SINT);
    RETURN_VK_FORMAT_STRING(R64G64B64A64_SFLOAT);
    RETURN_VK_FORMAT_STRING(B10G11R11_UFLOAT_PACK32);
    RETURN_VK_FORMAT_STRING(E5B9G9R9_UFLOAT_PACK32);
    RETURN_VK_FORMAT_STRING(D16_UNORM);
    RETURN_VK_FORMAT_STRING(X8_D24_UNORM_PACK32);
    RETURN_VK_FORMAT_STRING(D32_SFLOAT);
    RETURN_VK_FORMAT_STRING(S8_UINT);
    RETURN_VK_FORMAT_STRING(D16_UNORM_S8_UINT);
    RETURN_VK_FORMAT_STRING(D24_UNORM_S8_UINT);
    RETURN_VK_FORMAT_STRING(D32_SFLOAT_S8_UINT);
    RETURN_VK_FORMAT_STRING(BC1_RGB_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC1_RGB_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(BC1_RGBA_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC1_RGBA_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(BC2_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC2_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(BC3_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC3_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(BC4_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC4_SNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC5_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC5_SNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC6H_UFLOAT_BLOCK);
    RETURN_VK_FORMAT_STRING(BC6H_SFLOAT_BLOCK);
    RETURN_VK_FORMAT_STRING(BC7_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(BC7_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ETC2_R8G8B8_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ETC2_R8G8B8_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ETC2_R8G8B8A1_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ETC2_R8G8B8A1_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ETC2_R8G8B8A8_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ETC2_R8G8B8A8_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(EAC_R11_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(EAC_R11_SNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(EAC_R11G11_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(EAC_R11G11_SNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_4x4_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_4x4_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_5x4_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_5x4_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_5x5_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_5x5_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_6x5_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_6x5_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_6x6_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_6x6_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_8x5_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_8x5_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_8x6_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_8x6_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_8x8_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_8x8_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_10x5_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_10x5_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_10x6_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_10x6_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_10x8_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_10x8_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_10x10_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_10x10_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_12x10_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_12x10_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_12x12_UNORM_BLOCK);
    RETURN_VK_FORMAT_STRING(ASTC_12x12_SRGB_BLOCK);
    RETURN_VK_FORMAT_STRING(G8B8G8R8_422_UNORM);
    RETURN_VK_FORMAT_STRING(B8G8R8G8_422_UNORM);
    RETURN_VK_FORMAT_STRING(G8_B8_R8_3PLANE_420_UNORM);
    RETURN_VK_FORMAT_STRING(G8_B8R8_2PLANE_420_UNORM);
    RETURN_VK_FORMAT_STRING(G8_B8_R8_3PLANE_422_UNORM);
    RETURN_VK_FORMAT_STRING(G8_B8R8_2PLANE_422_UNORM);
    RETURN_VK_FORMAT_STRING(G8_B8_R8_3PLANE_444_UNORM);
    RETURN_VK_FORMAT_STRING(R10X6_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(R10X6G10X6_UNORM_2PACK16);
    RETURN_VK_FORMAT_STRING(R10X6G10X6B10X6A10X6_UNORM_4PACK16);
    RETURN_VK_FORMAT_STRING(G10X6B10X6G10X6R10X6_422_UNORM_4PACK16);
    RETURN_VK_FORMAT_STRING(B10X6G10X6R10X6G10X6_422_UNORM_4PACK16);
    RETURN_VK_FORMAT_STRING(G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(R12X4_UNORM_PACK16);
    RETURN_VK_FORMAT_STRING(R12X4G12X4_UNORM_2PACK16);
    RETURN_VK_FORMAT_STRING(R12X4G12X4B12X4A12X4_UNORM_4PACK16);
    RETURN_VK_FORMAT_STRING(G12X4B12X4G12X4R12X4_422_UNORM_4PACK16);
    RETURN_VK_FORMAT_STRING(B12X4G12X4R12X4G12X4_422_UNORM_4PACK16);
    RETURN_VK_FORMAT_STRING(G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16);
    RETURN_VK_FORMAT_STRING(G16B16G16R16_422_UNORM);
    RETURN_VK_FORMAT_STRING(B16G16R16G16_422_UNORM);
    RETURN_VK_FORMAT_STRING(G16_B16_R16_3PLANE_420_UNORM);
    RETURN_VK_FORMAT_STRING(G16_B16R16_2PLANE_420_UNORM);
    RETURN_VK_FORMAT_STRING(G16_B16_R16_3PLANE_422_UNORM);
    RETURN_VK_FORMAT_STRING(G16_B16R16_2PLANE_422_UNORM);
    RETURN_VK_FORMAT_STRING(G16_B16_R16_3PLANE_444_UNORM);
    RETURN_VK_FORMAT_STRING(PVRTC1_2BPP_UNORM_BLOCK_IMG);
    RETURN_VK_FORMAT_STRING(PVRTC1_4BPP_UNORM_BLOCK_IMG);
    RETURN_VK_FORMAT_STRING(PVRTC2_2BPP_UNORM_BLOCK_IMG);
    RETURN_VK_FORMAT_STRING(PVRTC2_4BPP_UNORM_BLOCK_IMG);
    RETURN_VK_FORMAT_STRING(PVRTC1_2BPP_SRGB_BLOCK_IMG);
    RETURN_VK_FORMAT_STRING(PVRTC1_4BPP_SRGB_BLOCK_IMG);
    RETURN_VK_FORMAT_STRING(PVRTC2_2BPP_SRGB_BLOCK_IMG);
    RETURN_VK_FORMAT_STRING(PVRTC2_4BPP_SRGB_BLOCK_IMG);

    default: assert(0);
  }

  return "ERROR";
}

uint8_t float32ToUint8(float f)
{
  if(f < 0.0f)
  {
    return 0.0f;
  }
  if(f > 1.0f)
  {
    return 255;
  }
  return uint8_t(f * 255);
}

float float16Tofloat32(uint32_t bits)
{
  uint32_t M = bits & 0x3FF;
  uint32_t E = (bits >> 10) & 0x1F;
  uint32_t S = (bits >> 15) & 1;
  int sign = 1;
  if(S > 0)
    sign = -1;
  if(E == 0 && M == 0)
    return sign * float(0.0);
  else if(E == 0 && M != 0)
    return sign * float(M) / 1024.0 / 16384.0;
  else if(E == 31 && M == 0)
    return sign * std::numeric_limits<float>::infinity();
  else if(E == 31 && M != 0)
  {
#if defined(_WIN32)
    OutputDebugStringA("NaN \n");
#endif
    return 0;
  }
  else if(E <= 15)
    return sign * (1 + float(M) / 1024.0) / float(uint32_t(1 << (15 - E)));
  else
    return sign * (1 + float(M) / 1024.0) * float(uint32_t(1 << (E - 15)));
}

template <typename T>
std::pair<T, T> mapData(std::vector<std::vector<T>> *data)
{
  T maxV = 0;
  T minV = FLT_MAX;
  for(auto row : *data)
  {
    for(auto col : row)
    {
      maxV = maxV < col ? col : maxV;
      minV = minV < col ? minV : col;
    }
  }
  if(maxV == 0)
    return std::pair<T,T>(minV, maxV);

  T denom = maxV - minV;
  if (std::abs(denom) < FLT_EPSILON)
    denom = T(1);

  for(auto &row : *data)
  {
    for(auto &col : row)
    {
      col = 255 * (col - minV) / denom;
    }
  }
  return std::pair<T,T>(minV, maxV);
}

void bufferToPpm(VkBuffer buffer, std::string filename, uint32_t width, uint32_t height,
                 VkFormat format)
{
  char *data;
  vkMapMemory(aux.device, stagingBuffers[buffer], 0, VK_WHOLE_SIZE, 0, (void **)&data);
  if(imageFormatInfoMap.find(format) == imageFormatInfoMap.end())
    return;

  std::ofstream file(filename, std::ios::out | std::ios::binary);
  std::string rawFilename = filename.substr(0, filename.find_last_of(".")) + "_" +
                            std::to_string(width) + "x" + std::to_string(height);
  std::ofstream rawFile(rawFilename, std::ios::out | std::ios::binary);
  rawFile.write(data, width * height * imageFormatInfoMap[format].second);
  rawFile.close();
  // ppm header
  file << "P6\n" << width << "\n" << height << "\n" << 255 << "\n";
  // ppm binary pixel data
  switch(format)
  {
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_USCALED:
    case VK_FORMAT_B8G8R8A8_UINT:
      [&]() {
        for(uint32_t y = 0; y < height; y++)
        {
          char *row = (char *)data;
          for(uint32_t x = 0; x < width; x++)
          {
            file.write(row + 2, 1);
            file.write(row + 1, 1);
            file.write(row, 1);
            row = row + imageFormatInfoMap[format].second;
          }
          data += width * imageFormatInfoMap[format].second;
        }
      }();
      break;
    case VK_FORMAT_B8G8R8A8_SNORM:
    case VK_FORMAT_B8G8R8A8_SSCALED:
    case VK_FORMAT_B8G8R8A8_SINT:
      [&]() {
        for(uint32_t y = 0; y < height; y++)
        {
          char *row = (char *)data;
          for(uint32_t x = 0; x < width; x++)
          {
            file << (*((uint8_t *)row + 2) ^ 0x80);
            file << (*((uint8_t *)row + 1) ^ 0x80);
            file << (*(uint8_t *)row ^ 0x80);
            row = row + imageFormatInfoMap[format].second;
          }
          data += width * imageFormatInfoMap[format].second;
        }
      }();
      break;
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_USCALED:
    case VK_FORMAT_R8G8B8A8_UINT:
      [&]() {
        for(uint32_t y = 0; y < height; y++)
        {
          char *row = (char *)data;
          for(uint32_t x = 0; x < width; x++)
          {
            file.write(row, 3);
            row = row + imageFormatInfoMap[format].second;
          }
          data += width * imageFormatInfoMap[format].second;
        }
      }();
      break;
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_R8G8B8A8_SSCALED:
    case VK_FORMAT_R8G8B8A8_SINT:
      [&]() {
        for(uint32_t y = 0; y < height; y++)
        {
          char *row = (char *)data;
          for(uint32_t x = 0; x < width; x++)
          {
            file << (*(uint8_t *)row ^ 0x80);
            file << (*((uint8_t *)row + 1) ^ 0x80);
            file << (*((uint8_t *)row + 2) ^ 0x80);
            row = row + imageFormatInfoMap[format].second;
          }
          data += width * imageFormatInfoMap[format].second;
        }
      }();
      break;
    case VK_FORMAT_R16G16B16A16_UNORM:
    case VK_FORMAT_R16G16B16A16_USCALED:
    case VK_FORMAT_R16G16B16A16_UINT:
      [&]() {
        for(uint32_t y = 0; y < height; y++)
        {
          char *row = (char *)data;
          for(uint32_t x = 0; x < width; x++)
          {
            file << (*(uint16_t *)row >> 8);
            file << (*((uint16_t *)row + 1) >> 8);
            file << (*((uint16_t *)row + 2) >> 8);
            row = row + imageFormatInfoMap[format].second;
          }
          data += width * imageFormatInfoMap[format].second;
        }
      }();
    case VK_FORMAT_R16G16B16A16_SNORM:
    case VK_FORMAT_R16G16B16A16_SSCALED:
    case VK_FORMAT_R16G16B16A16_SINT:
      [&]() {
        for(uint32_t y = 0; y < height; y++)
        {
          char *row = (char *)data;
          for(uint32_t x = 0; x < width; x++)
          {
            file << ((*(uint16_t *)row ^ 0x8000) >> 8);
            file << ((*((uint16_t *)row + 1) ^ 0x8000) >> 8);
            file << ((*((uint16_t *)row + 2) ^ 0x8000) >> 8);
            row = row + imageFormatInfoMap[format].second;
          }
          data += width * imageFormatInfoMap[format].second;
        }
      }();
      break;
    case VK_FORMAT_R16G16B16A16_SFLOAT:
      [&]() {
        for(uint32_t y = 0; y < height; y++)
        {
          char *row = (char *)data;
          for(uint32_t x = 0; x < width; x++)
          {
            file << float32ToUint8(float16Tofloat32(uint32_t(*(uint16_t *)row)));
            file << float32ToUint8(float16Tofloat32(uint32_t(*((uint16_t *)row + 1))));
            file << float32ToUint8(float16Tofloat32(uint32_t(*((uint16_t *)row + 2))));
            row = row + imageFormatInfoMap[format].second;
          }
          data += width * imageFormatInfoMap[format].second;
        }
      }();
      break;
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
      [&]() {
        std::vector<std::vector<float>> data_8((int)height, std::vector<float>(width, 0));
        for(uint32_t y = 0; y < height; y++)
        {
          char *row = (char *)data;
          for(uint32_t x = 0; x < width; x++)
          {
            data_8[y][x] = *(float *) row;
            row += imageFormatInfoMap[format].second;
          }
          data += width * imageFormatInfoMap[format].second;
        }
        std::pair<float, float> minmax = mapData(&data_8);
#if defined(_WIN32)
        std::string minmaxstr =
            "minmax value = " + std::to_string(minmax.first) + " " + std::to_string(minmax.second);
        OutputDebugStringA(minmaxstr.c_str());
        OutputDebugStringA("\n");
#endif
        for(auto row : data_8)
        {
          for(auto col : row)
          {
            uint8_t color8 = uint8_t(col);
            file << color8 << color8 << color8;
          }
        }
      }();
      break;
    case VK_FORMAT_D24_UNORM_S8_UINT:
      [&]() {
        for(uint32_t y = 0; y < height; y++)
        {
          char *row = (char *)data;
          for(uint32_t x = 0; x < width; x++)
          {
            file << *(uint8_t *)row;
            file << *((uint8_t *)row + 1);
            file << *((uint8_t *)row + 2);
            row = row + imageFormatInfoMap[format].second;
          }
          data += width * imageFormatInfoMap[format].second;
        }
      }();
      break;
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:
      [&]() {
        for(uint32_t y = 0; y < height; y++)
        {
          char *row = (char *)data;
          for(uint32_t x = 0; x < width; x++)
          {
            uint32_t v = *(uint32_t *)row;
            file << uint8_t((v & 0x3ff) >> 2);
            file << uint8_t((v >> 10 & 0x3ff) >> 2);
            file << uint8_t((v >> 20 & 0x3ff) >> 2);
            row = row + imageFormatInfoMap[format].second;
          }
          data += width * imageFormatInfoMap[format].second;
        }
      }();
      break;
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_SINT_PACK32:
      [&]() {
        for(uint32_t y = 0; y < height; y++)
        {
          char *row = (char *)data;
          for(uint32_t x = 0; x < width; x++)
          {
            uint32_t v = *(uint32_t *)row;
            file << (uint8_t((v & 0x3ff) ^ 0x200) >> 2);
            file << (uint8_t((v >> 10 & 0x3ff) ^ 0x200) >> 2);
            file << (uint8_t((v >> 20 & 0x3ff) ^ 0x200) >> 2);
            row = row + imageFormatInfoMap[format].second;
          }
          data += width * imageFormatInfoMap[format].second;
        }
      }();
      break;
    default: break;
  }
  file.close();
}

void imgToBuffer(VkImage image, VkBuffer buffer, uint32_t width, uint32_t height, VkFormat format)
{
  VkBufferImageCopy region = {};
  region.imageSubresource = {imageFormatInfoMap[format].first, 0, 0, 1};
  region.imageExtent = {width, height, 1};
  vkCmdCopyImageToBuffer(aux.command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1,
                         &region);
}

void cleanup(RenderPassInfo *pRpInfo)
{
  for(const auto &entry : stagingImages)
  {
    // vkUnmapMemory(aux.device, entry.second);
    vkFreeMemory(aux.device, entry.second, NULL);
    vkDestroyImage(aux.device, entry.first, NULL);
  }
  for(const auto &entry : stagingBuffers)
  {
    vkUnmapMemory(aux.device, entry.second);
    vkFreeMemory(aux.device, entry.second, NULL);
    vkDestroyBuffer(aux.device, entry.first, NULL);
  }
  stagingBuffers.clear();
  stagingImages.clear();
  if(pRpInfo != NULL)
  {
    pRpInfo->attachments.clear();
    pRpInfo->images.clear();
    pRpInfo->imageCIs.clear();
  }
}

VkImage getStagingImage(VkImageCreateInfo ci)
{
  VkImage dstImage;
  vkCreateImage(aux.device, &ci, nullptr, &dstImage);

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(aux.device, dstImage, &memRequirements);

  VkMemoryAllocateInfo memAllocInfo{};
  memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAllocInfo.allocationSize = memRequirements.size;
  // Memory must be host visible to copy from
  memAllocInfo.memoryTypeIndex =
      MemoryTypeIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memRequirements.memoryTypeBits,
                      physicalDeviceMemoryProperties);
  assert(memAllocInfo.memoryTypeIndex != -1);
  VkDeviceMemory dstImageMemory;
  vkAllocateMemory(aux.device, &memAllocInfo, nullptr, &dstImageMemory);

  vkBindImageMemory(aux.device, dstImage, dstImageMemory, 0);

  stagingImages[dstImage] = dstImageMemory;

  return dstImage;
}

VkBuffer getStagingBuffer(uint32_t width, uint32_t height, uint32_t bytes)
{
  VkBuffer dstBuffer;
  VkBufferCreateInfo bufCI = {};
  bufCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufCI.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  bufCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  bufCI.size = VkDeviceSize(width * height * bytes);
  VkResult result = vkCreateBuffer(aux.device, &bufCI, NULL, &dstBuffer);
  assert(result == VK_SUCCESS);

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(aux.device, dstBuffer, &memRequirements);

  VkMemoryAllocateInfo memAllocInfo{};
  memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAllocInfo.allocationSize = memRequirements.size;
  memAllocInfo.memoryTypeIndex =
      MemoryTypeIndex(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      memRequirements.memoryTypeBits, physicalDeviceMemoryProperties);
  assert(memAllocInfo.memoryTypeIndex != -1);
  VkDeviceMemory dstMemory;
  vkAllocateMemory(aux.device, &memAllocInfo, NULL, &dstMemory);
  vkBindBufferMemory(aux.device, dstBuffer, dstMemory, 0);
  stagingBuffers[dstBuffer] = dstMemory;
  return dstBuffer;
}

void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height, VkFormat format)
{
  VkImageCopy imageCopyRegion{};
  imageCopyRegion.srcSubresource.aspectMask = imageFormatInfoMap[format].first;
  imageCopyRegion.srcSubresource.layerCount = 1;
  imageCopyRegion.dstSubresource.aspectMask = imageFormatInfoMap[format].first;
  imageCopyRegion.dstSubresource.layerCount = 1;
  imageCopyRegion.extent.width = width;
  imageCopyRegion.extent.height = height;
  imageCopyRegion.extent.depth = 1;
  vkCmdCopyImage(aux.command_buffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage,
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
}

void copyFramebufferAttachments(RenderPassInfo *rpInfo)
{
  if(presentIndex > 0)
    return;
  for(int i = 0; i < rpInfo->attachments.size(); i++)
  {
    VkImageCreateInfo ci = rpInfo->imageCIs[i];

    if(imageFormatInfoMap.find(ci.format) == imageFormatInfoMap.end())
    {
#if defined(_WIN32)
      std::string f = VkFormatToString(ci.format);
      OutputDebugStringA(f.c_str());
      OutputDebugStringA("\n");
#endif
      continue;
    }
    VkImage srcImage = rpInfo->images[i];
    VkImageCreateInfo imageCreateCI{};
    imageCreateCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateCI.imageType = VK_IMAGE_TYPE_2D;
    imageCreateCI.format = ci.format;
    imageCreateCI.extent.width = ci.extent.width;
    imageCreateCI.extent.height = ci.extent.height;
    assert(ci.extent.depth == 1);
    imageCreateCI.extent.depth = 1;
    imageCreateCI.arrayLayers = 1;
    imageCreateCI.mipLevels = 1;
    imageCreateCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateCI.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImage stagingImage = getStagingImage(imageCreateCI);
    VkBuffer stagingBuffer =
        getStagingBuffer(ci.extent.width, ci.extent.height, imageFormatInfoMap[ci.format].second);

    VkCommandBufferBeginInfo cmdbufBI = VkCommandBufferBeginInfo{};
    cmdbufBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(aux.command_buffer, &cmdbufBI);

    // Transition staging image to IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.
    ImageLayoutTransition(aux, stagingImage, imageCreateCI, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_UNDEFINED);
    // Transition source image to IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL.
    ImageLayoutTransition(aux, srcImage,
                          VkImageSubresourceRange{GetFullAspectFromFormat(ci.format), 0, 1, 0, 1},
                          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, rpInfo->finalLayouts[i]);

    copyImage(srcImage, stagingImage, ci.extent.width, ci.extent.height, ci.format);

    // Transition staging image from IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL to
    // IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL.
    ImageLayoutTransition(aux, stagingImage, imageCreateCI, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    // Transition source image back to the original layout.
    ImageLayoutTransition(aux, srcImage,
                          VkImageSubresourceRange{GetFullAspectFromFormat(ci.format), 0, 1, 0, 1},
                          rpInfo->finalLayouts[i], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    imgToBuffer(stagingImage, stagingBuffer, ci.extent.width, ci.extent.height, ci.format);

    vkEndCommandBuffer(aux.command_buffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &aux.command_buffer;
    vkQueueSubmit(aux.queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(aux.queue);

    std::string filename = std::to_string(renderPassCount++) + "_" +
                           std::to_string((uint64_t)rpInfo->renderPass) + "_attachment_" +
                           std::to_string(i) + "_" + VkFormatToString(ci.format) + ".ppm";

    bufferToPpm(stagingBuffer, filename, ci.extent.width, ci.extent.height, ci.format);
  }
  cleanup(rpInfo);
}

void screenshot(VkImage srcImage, const char *filename)
{
  VkImageCreateInfo imageCreateCI{};
  imageCreateCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateCI.imageType = VK_IMAGE_TYPE_2D;
  imageCreateCI.format = swapchainImageFormat;
  imageCreateCI.extent.width = sw_width;
  imageCreateCI.extent.height = sw_height;
  imageCreateCI.extent.depth = 1;
  imageCreateCI.arrayLayers = 1;
  imageCreateCI.mipLevels = 1;
  imageCreateCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageCreateCI.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCreateCI.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCreateCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  VkImage dstImage = getStagingImage(imageCreateCI);
  VkBuffer stagingBuffer =
      getStagingBuffer(sw_width, sw_height, imageFormatInfoMap[swapchainImageFormat].second);

  VkCommandBufferBeginInfo cmdbufBI = VkCommandBufferBeginInfo{};
  cmdbufBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(aux.command_buffer, &cmdbufBI);
  ImageLayoutTransition(aux, dstImage, imageCreateCI, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_UNDEFINED);
  ImageLayoutTransition(aux, srcImage, VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  copyImage(srcImage, dstImage, sw_width, sw_height, swapchainImageFormat);

  // Transition staging image from IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL to
  // IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL.
  ImageLayoutTransition(aux, dstImage, imageCreateCI, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  // Transition source image back to the original layout.
  ImageLayoutTransition(aux, srcImage, VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

  imgToBuffer(dstImage, stagingBuffer, sw_width, sw_height, swapchainImageFormat);

  vkEndCommandBuffer(aux.command_buffer);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &aux.command_buffer;
  vkQueueSubmit(aux.queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(aux.queue);

  bufferToPpm(stagingBuffer, filename, sw_width, sw_height, swapchainImageFormat);
  cleanup(NULL);
}
