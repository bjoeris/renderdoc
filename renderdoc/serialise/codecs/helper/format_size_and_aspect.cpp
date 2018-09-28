/******************************************************************************
* The MIT License (MIT)
*
* Copyright (c) 2018 Google LLC
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
******************************************************************************/
#include "format_helper.h"
#include "helper.h"

#define FORMAT_ASPECT_BYTE_SIZE(f, a, bs)                                                 \
  {                                                                                       \
    VK_FORMAT_##f, std::pair<VkImageAspectFlags, uint32_t>(VK_IMAGE_ASPECT_##a##_BIT, bs) \
  }

std::map<VkFormat, std::pair<VkImageAspectFlags, uint32_t>> kImageAspectsAndByteSize = {
    FORMAT_ASPECT_BYTE_SIZE(A8B8G8R8_USCALED_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R5G5B5A1_UNORM_PACK16, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_UINT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16_SFLOAT, COLOR, 6),
    FORMAT_ASPECT_BYTE_SIZE(A2R10G10B10_SNORM_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R32_UINT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8_UINT, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(R64_UINT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R32G32B32_UINT, COLOR, 12),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8_SRGB, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_SINT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_SINT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16G16_USCALED, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_USCALED, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8G8_UNORM, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R64G64_UINT, COLOR, 16),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_SSCALED, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8_UNORM, COLOR, 1),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8_SNORM, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_UNORM, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16_USCALED, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16_SSCALED, COLOR, 6),
    FORMAT_ASPECT_BYTE_SIZE(R32G32B32A32_UINT, COLOR, 16),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16_UINT, COLOR, 6),
    FORMAT_ASPECT_BYTE_SIZE(R8_USCALED, COLOR, 1),
    FORMAT_ASPECT_BYTE_SIZE(R8_SSCALED, COLOR, 1),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16_USCALED, COLOR, 6),
    FORMAT_ASPECT_BYTE_SIZE(R4G4_UNORM_PACK8, COLOR, 1),
    FORMAT_ASPECT_BYTE_SIZE(A2R10G10B10_UINT_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R64_SINT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R8G8_SSCALED, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_SFLOAT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R64G64_SINT, COLOR, 16),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_UINT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_SSCALED_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(A2R10G10B10_UNORM_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R32G32_UINT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R4G4B4A4_UNORM_PACK16, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8_SRGB, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(A8B8G8R8_SNORM_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_UNORM, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_SSCALED, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R8_SINT, COLOR, 1),
    FORMAT_ASPECT_BYTE_SIZE(R32_SFLOAT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_SNORM, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_SNORM, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8_SINT, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(R8G8_SNORM, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_SINT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(A8B8G8R8_SINT_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8_SRGB, COLOR, 1),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8_USCALED, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_SRGB, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16_UNORM, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8A8_SRGB, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(A2R10G10B10_USCALED_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16G16_SINT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16G16_SNORM, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8_USCALED, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16_UNORM, COLOR, 6),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_UINT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R16G16_SSCALED, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16_SNORM, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(A2R10G10B10_SINT_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8_SSCALED, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(R8_UINT, COLOR, 1),
    FORMAT_ASPECT_BYTE_SIZE(R64G64_SFLOAT, COLOR, 16),
    FORMAT_ASPECT_BYTE_SIZE(R16_SINT, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R32G32B32A32_SINT, COLOR, 16),
    FORMAT_ASPECT_BYTE_SIZE(R5G6B5_UNORM_PACK16, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_USCALED, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R64G64B64_SINT, COLOR, 24),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_UNORM, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R32G32B32_SINT, COLOR, 12),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8_SNORM, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_UINT_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(B5G5R5A1_UNORM_PACK16, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8_UINT, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_SINT_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16_SFLOAT, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(B4G4R4A4_UNORM_PACK16, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R16G16_UNORM, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R64_SFLOAT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(B10G11R11_UFLOAT_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R64G64B64A64_SFLOAT, COLOR, 32),
    FORMAT_ASPECT_BYTE_SIZE(R8G8_USCALED, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R32G32B32_SFLOAT, COLOR, 12),
    FORMAT_ASPECT_BYTE_SIZE(R8G8_SINT, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8_SINT, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(R32G32B32A32_SFLOAT, COLOR, 16),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8_SSCALED, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_UNORM_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R32_SINT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R64G64B64A64_SINT, COLOR, 32),
    FORMAT_ASPECT_BYTE_SIZE(R16G16_SFLOAT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8_UNORM, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(A2R10G10B10_SSCALED_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8_SNORM, COLOR, 1),
    FORMAT_ASPECT_BYTE_SIZE(A8B8G8R8_UINT_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(A8B8G8R8_SSCALED_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R64G64B64_SFLOAT, COLOR, 24),
    FORMAT_ASPECT_BYTE_SIZE(R8G8_SRGB, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16_SNORM, COLOR, 6),
    FORMAT_ASPECT_BYTE_SIZE(B5G6R5_UNORM_PACK16, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R16_UINT, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R32G32_SINT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_SSCALED, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16_SINT, COLOR, 6),
    FORMAT_ASPECT_BYTE_SIZE(A8B8G8R8_UNORM_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R64G64B64_UINT, COLOR, 24),
    FORMAT_ASPECT_BYTE_SIZE(R16_SSCALED, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(A8B8G8R8_SRGB_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(B8G8R8A8_SNORM, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(A1R5G5B5_UNORM_PACK16, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_SNORM_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R16G16_UINT, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(R8G8_UINT, COLOR, 2),
    FORMAT_ASPECT_BYTE_SIZE(R32G32_SFLOAT, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(R64G64B64A64_UINT, COLOR, 32),
    FORMAT_ASPECT_BYTE_SIZE(R8G8B8_UNORM, COLOR, 3),
    FORMAT_ASPECT_BYTE_SIZE(R16G16B16A16_USCALED, COLOR, 8),
    FORMAT_ASPECT_BYTE_SIZE(A2B10G10R10_USCALED_PACK32, COLOR, 4),
    FORMAT_ASPECT_BYTE_SIZE(D32_SFLOAT, DEPTH, 4),
    FORMAT_ASPECT_BYTE_SIZE(D16_UNORM, DEPTH, 2),
    {VK_FORMAT_D16_UNORM_S8_UINT, std::pair<VkImageAspectFlags, uint32_t>(
                                      VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 3)},
    {VK_FORMAT_D24_UNORM_S8_UINT, std::pair<VkImageAspectFlags, uint32_t>(
                                      VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 4)},
    {VK_FORMAT_D32_SFLOAT_S8_UINT, std::pair<VkImageAspectFlags, uint32_t>(
                                       VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 5)},
};

std::string FormatToString(VkFormat f) {
#define RETURN_VK_FORMAT_STRING(x) \
  case VK_FORMAT_##x: return std::string(var_to_string(x));

  switch (f) {
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

uint32_t FixCompressedSizes(VkFormat fmt, VkExtent3D &dim, uint32_t &offset) {
  switch (fmt) {
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC4_SNORM_BLOCK:
    case VK_FORMAT_BC4_UNORM_BLOCK:
    case VK_FORMAT_BC5_SNORM_BLOCK:
    case VK_FORMAT_BC5_UNORM_BLOCK:
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
    case VK_FORMAT_BC7_UNORM_BLOCK:
    dim.width = (uint32_t) AlignedSize(dim.width, 4);
    dim.height = (uint32_t) AlignedSize(dim.height, 4);
    dim.depth = (uint32_t) AlignedSize(dim.depth, 1);
    offset = (uint32_t) AlignedSize(offset, 4);
    return 1;

    default: break;
  }

  offset = (uint32_t) AlignedSize(offset, 4);
  return 0;
}

VkImageAspectFlags FullAspectFromFormat(VkFormat fmt) {
  if (fmt == VK_FORMAT_D16_UNORM || fmt == VK_FORMAT_D32_SFLOAT)
    return VK_IMAGE_ASPECT_DEPTH_BIT;
  else if (fmt == VK_FORMAT_D16_UNORM_S8_UINT || fmt == VK_FORMAT_D24_UNORM_S8_UINT ||
    fmt == VK_FORMAT_D32_SFLOAT_S8_UINT)
    return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
  else    //  TODO(akharlamov): this is actually incorrect but OK for now
    return VK_IMAGE_ASPECT_COLOR_BIT;
}

VkImageAspectFlags AspectFromFormat(VkFormat fmt) {
  if (fmt == VK_FORMAT_D16_UNORM || fmt == VK_FORMAT_D32_SFLOAT || fmt == VK_FORMAT_D16_UNORM_S8_UINT ||
    fmt == VK_FORMAT_D24_UNORM_S8_UINT || fmt == VK_FORMAT_D32_SFLOAT_S8_UINT)
    return VK_IMAGE_ASPECT_DEPTH_BIT;
  else
    return VK_IMAGE_ASPECT_COLOR_BIT;
}
double SizeOfFormat(VkFormat fmt, VkImageAspectFlagBits aspect) {
  if (aspect == VK_IMAGE_ASPECT_STENCIL_BIT) {
    switch (fmt) {
      case VK_FORMAT_D32_SFLOAT_S8_UINT:
      case VK_FORMAT_D24_UNORM_S8_UINT:
      case VK_FORMAT_D16_UNORM_S8_UINT: return 1.0;
      default: assert(0);
    }
  } else if (aspect == VK_IMAGE_ASPECT_COLOR_BIT || aspect == VK_IMAGE_ASPECT_DEPTH_BIT) {
    switch (fmt) {
      case VK_FORMAT_R4G4_UNORM_PACK8:
      case VK_FORMAT_R8_UNORM:
      case VK_FORMAT_R8_SNORM:
      case VK_FORMAT_R8_USCALED:
      case VK_FORMAT_R8_SSCALED:
      case VK_FORMAT_R8_UINT:
      case VK_FORMAT_R8_SINT:
      case VK_FORMAT_R8_SRGB: return 1.0;

      case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
      case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
      case VK_FORMAT_R5G6B5_UNORM_PACK16:
      case VK_FORMAT_B5G6R5_UNORM_PACK16:
      case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
      case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
      case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
      case VK_FORMAT_R8G8_UNORM:
      case VK_FORMAT_R8G8_SNORM:
      case VK_FORMAT_R8G8_USCALED:
      case VK_FORMAT_R8G8_SSCALED:
      case VK_FORMAT_R8G8_UINT:
      case VK_FORMAT_R8G8_SINT:
      case VK_FORMAT_R8G8_SRGB:
      case VK_FORMAT_R16_UNORM:
      case VK_FORMAT_R16_SNORM:
      case VK_FORMAT_R16_USCALED:
      case VK_FORMAT_R16_SSCALED:
      case VK_FORMAT_R16_UINT:
      case VK_FORMAT_R16_SINT:
      case VK_FORMAT_R16_SFLOAT:
      case VK_FORMAT_D16_UNORM:
      case VK_FORMAT_D16_UNORM_S8_UINT: return 2.0;

      case VK_FORMAT_R8G8B8_UNORM:
      case VK_FORMAT_R8G8B8_SNORM:
      case VK_FORMAT_R8G8B8_USCALED:
      case VK_FORMAT_R8G8B8_SSCALED:
      case VK_FORMAT_R8G8B8_UINT:
      case VK_FORMAT_R8G8B8_SINT:
      case VK_FORMAT_R8G8B8_SRGB:
      case VK_FORMAT_B8G8R8_UNORM:
      case VK_FORMAT_B8G8R8_SNORM:
      case VK_FORMAT_B8G8R8_USCALED:
      case VK_FORMAT_B8G8R8_SSCALED:
      case VK_FORMAT_B8G8R8_UINT:
      case VK_FORMAT_B8G8R8_SINT:
      case VK_FORMAT_B8G8R8_SRGB:
      case VK_FORMAT_D24_UNORM_S8_UINT: return 3.0;

      case VK_FORMAT_R8G8B8A8_UNORM:
      case VK_FORMAT_R8G8B8A8_SNORM:
      case VK_FORMAT_R8G8B8A8_USCALED:
      case VK_FORMAT_R8G8B8A8_SSCALED:
      case VK_FORMAT_R8G8B8A8_UINT:
      case VK_FORMAT_R8G8B8A8_SINT:
      case VK_FORMAT_R8G8B8A8_SRGB:
      case VK_FORMAT_B8G8R8A8_UNORM:
      case VK_FORMAT_B8G8R8A8_SNORM:
      case VK_FORMAT_B8G8R8A8_USCALED:
      case VK_FORMAT_B8G8R8A8_SSCALED:
      case VK_FORMAT_B8G8R8A8_UINT:
      case VK_FORMAT_B8G8R8A8_SINT:
      case VK_FORMAT_B8G8R8A8_SRGB:
      case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
      case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
      case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
      case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
      case VK_FORMAT_A8B8G8R8_UINT_PACK32:
      case VK_FORMAT_A8B8G8R8_SINT_PACK32:
      case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
      case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
      case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
      case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
      case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
      case VK_FORMAT_A2R10G10B10_UINT_PACK32:
      case VK_FORMAT_A2R10G10B10_SINT_PACK32:
      case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
      case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
      case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
      case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
      case VK_FORMAT_A2B10G10R10_UINT_PACK32:
      case VK_FORMAT_A2B10G10R10_SINT_PACK32:
      case VK_FORMAT_R16G16_UNORM:
      case VK_FORMAT_R16G16_SNORM:
      case VK_FORMAT_R16G16_USCALED:
      case VK_FORMAT_R16G16_SSCALED:
      case VK_FORMAT_R16G16_UINT:
      case VK_FORMAT_R16G16_SINT:
      case VK_FORMAT_R16G16_SFLOAT:
      case VK_FORMAT_R32_UINT:
      case VK_FORMAT_R32_SINT:
      case VK_FORMAT_R32_SFLOAT:
      case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
      case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
      case VK_FORMAT_D32_SFLOAT:
      case VK_FORMAT_D32_SFLOAT_S8_UINT: return 4.0;

      case VK_FORMAT_R16G16B16_UNORM:
      case VK_FORMAT_R16G16B16_SNORM:
      case VK_FORMAT_R16G16B16_USCALED:
      case VK_FORMAT_R16G16B16_SSCALED:
      case VK_FORMAT_R16G16B16_UINT:
      case VK_FORMAT_R16G16B16_SINT:
      case VK_FORMAT_R16G16B16_SFLOAT:
      return 6.0f;

      case VK_FORMAT_R16G16B16A16_UNORM:
      case VK_FORMAT_R16G16B16A16_SNORM:
      case VK_FORMAT_R16G16B16A16_USCALED:
      case VK_FORMAT_R16G16B16A16_SSCALED:
      case VK_FORMAT_R16G16B16A16_UINT:
      case VK_FORMAT_R16G16B16A16_SINT:
      case VK_FORMAT_R16G16B16A16_SFLOAT:
      case VK_FORMAT_R32G32_UINT:
      case VK_FORMAT_R32G32_SINT:
      case VK_FORMAT_R32G32_SFLOAT:
      case VK_FORMAT_R64_UINT:
      case VK_FORMAT_R64_SINT:
      case VK_FORMAT_R64_SFLOAT: return 8.0;

      case VK_FORMAT_R32G32B32_UINT:
      case VK_FORMAT_R32G32B32_SINT:
      case VK_FORMAT_R32G32B32_SFLOAT:

      return 12.0;

      case VK_FORMAT_R32G32B32A32_UINT:
      case VK_FORMAT_R32G32B32A32_SINT:
      case VK_FORMAT_R32G32B32A32_SFLOAT:
      case VK_FORMAT_R64G64_UINT:
      case VK_FORMAT_R64G64_SINT:
      case VK_FORMAT_R64G64_SFLOAT: return 16.0;

      case VK_FORMAT_R64G64B64_UINT:
      case VK_FORMAT_R64G64B64_SINT:
      case VK_FORMAT_R64G64B64_SFLOAT:
      return 24.0;

      case VK_FORMAT_R64G64B64A64_UINT:
      case VK_FORMAT_R64G64B64A64_SINT:
      case VK_FORMAT_R64G64B64A64_SFLOAT:
      return 32.0;

      case VK_FORMAT_BC2_SRGB_BLOCK:
      case VK_FORMAT_BC2_UNORM_BLOCK:
      case VK_FORMAT_BC3_SRGB_BLOCK:
      case VK_FORMAT_BC3_UNORM_BLOCK:
      case VK_FORMAT_BC5_SNORM_BLOCK:
      case VK_FORMAT_BC5_UNORM_BLOCK:
      case VK_FORMAT_BC6H_SFLOAT_BLOCK:
      case VK_FORMAT_BC6H_UFLOAT_BLOCK:
      case VK_FORMAT_BC7_SRGB_BLOCK:
      case VK_FORMAT_BC7_UNORM_BLOCK: return 1.0;

      case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
      case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
      case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
      case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
      case VK_FORMAT_BC4_SNORM_BLOCK:
      case VK_FORMAT_BC4_UNORM_BLOCK: return 0.5;

      default: assert(0);
    }
  }
  return 0.0;
}

uint32_t ChannelsInFormat(VkFormat fmt) {
    switch (fmt) {

      case VK_FORMAT_R8_UNORM:
      case VK_FORMAT_R8_SNORM:
      case VK_FORMAT_R8_USCALED:
      case VK_FORMAT_R8_SSCALED:
      case VK_FORMAT_R8_UINT:
      case VK_FORMAT_R8_SINT:
      case VK_FORMAT_R8_SRGB:
      case VK_FORMAT_R16_UNORM:
      case VK_FORMAT_R16_SNORM:
      case VK_FORMAT_R16_USCALED:
      case VK_FORMAT_R16_SSCALED:
      case VK_FORMAT_R16_UINT:
      case VK_FORMAT_R16_SINT:
      case VK_FORMAT_R16_SFLOAT:
      case VK_FORMAT_D16_UNORM:
      case VK_FORMAT_D16_UNORM_S8_UINT:
      case VK_FORMAT_D24_UNORM_S8_UINT:
      case VK_FORMAT_R32_UINT:
      case VK_FORMAT_R32_SINT:
      case VK_FORMAT_R32_SFLOAT:
      case VK_FORMAT_D32_SFLOAT:
      case VK_FORMAT_D32_SFLOAT_S8_UINT:
      case VK_FORMAT_R64_UINT:
      case VK_FORMAT_R64_SINT:
      case VK_FORMAT_R64_SFLOAT:
      return 1;

      case VK_FORMAT_R4G4_UNORM_PACK8:
      case VK_FORMAT_R8G8_UNORM:
      case VK_FORMAT_R8G8_SNORM:
      case VK_FORMAT_R8G8_USCALED:
      case VK_FORMAT_R8G8_SSCALED:
      case VK_FORMAT_R8G8_UINT:
      case VK_FORMAT_R8G8_SINT:
      case VK_FORMAT_R8G8_SRGB:
      case VK_FORMAT_R16G16_UNORM:
      case VK_FORMAT_R16G16_SNORM:
      case VK_FORMAT_R16G16_USCALED:
      case VK_FORMAT_R16G16_SSCALED:
      case VK_FORMAT_R16G16_UINT:
      case VK_FORMAT_R16G16_SINT:
      case VK_FORMAT_R16G16_SFLOAT:
      case VK_FORMAT_R32G32_UINT:
      case VK_FORMAT_R32G32_SINT:
      case VK_FORMAT_R32G32_SFLOAT:
      case VK_FORMAT_R64G64_UINT:
      case VK_FORMAT_R64G64_SINT:
      case VK_FORMAT_R64G64_SFLOAT:
      return 2;

      case VK_FORMAT_R5G6B5_UNORM_PACK16:
      case VK_FORMAT_B5G6R5_UNORM_PACK16:
      case VK_FORMAT_R8G8B8_UNORM:
      case VK_FORMAT_R8G8B8_SNORM:
      case VK_FORMAT_R8G8B8_USCALED:
      case VK_FORMAT_R8G8B8_SSCALED:
      case VK_FORMAT_R8G8B8_UINT:
      case VK_FORMAT_R8G8B8_SINT:
      case VK_FORMAT_R8G8B8_SRGB:
      case VK_FORMAT_B8G8R8_UNORM:
      case VK_FORMAT_B8G8R8_SNORM:
      case VK_FORMAT_B8G8R8_USCALED:
      case VK_FORMAT_B8G8R8_SSCALED:
      case VK_FORMAT_B8G8R8_UINT:
      case VK_FORMAT_B8G8R8_SINT:
      case VK_FORMAT_B8G8R8_SRGB:
      case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
      case VK_FORMAT_R16G16B16_UNORM:
      case VK_FORMAT_R16G16B16_SNORM:
      case VK_FORMAT_R16G16B16_USCALED:
      case VK_FORMAT_R16G16B16_SSCALED:
      case VK_FORMAT_R16G16B16_UINT:
      case VK_FORMAT_R16G16B16_SINT:
      case VK_FORMAT_R16G16B16_SFLOAT:
      case VK_FORMAT_R32G32B32_UINT:
      case VK_FORMAT_R32G32B32_SINT:
      case VK_FORMAT_R32G32B32_SFLOAT:
      case VK_FORMAT_R64G64B64_UINT:
      case VK_FORMAT_R64G64B64_SINT:
      case VK_FORMAT_R64G64B64_SFLOAT:
      return 3;

      case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
      case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
      case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
      case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
      case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
      case VK_FORMAT_R8G8B8A8_UNORM:
      case VK_FORMAT_R8G8B8A8_SNORM:
      case VK_FORMAT_R8G8B8A8_USCALED:
      case VK_FORMAT_R8G8B8A8_SSCALED:
      case VK_FORMAT_R8G8B8A8_UINT:
      case VK_FORMAT_R8G8B8A8_SINT:
      case VK_FORMAT_R8G8B8A8_SRGB:
      case VK_FORMAT_B8G8R8A8_UNORM:
      case VK_FORMAT_B8G8R8A8_SNORM:
      case VK_FORMAT_B8G8R8A8_USCALED:
      case VK_FORMAT_B8G8R8A8_SSCALED:
      case VK_FORMAT_B8G8R8A8_UINT:
      case VK_FORMAT_B8G8R8A8_SINT:
      case VK_FORMAT_B8G8R8A8_SRGB:
      case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
      case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
      case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
      case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
      case VK_FORMAT_A8B8G8R8_UINT_PACK32:
      case VK_FORMAT_A8B8G8R8_SINT_PACK32:
      case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
      case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
      case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
      case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
      case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
      case VK_FORMAT_A2R10G10B10_UINT_PACK32:
      case VK_FORMAT_A2R10G10B10_SINT_PACK32:
      case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
      case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
      case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
      case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
      case VK_FORMAT_A2B10G10R10_UINT_PACK32:
      case VK_FORMAT_A2B10G10R10_SINT_PACK32:
      case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
      case VK_FORMAT_R16G16B16A16_UNORM:
      case VK_FORMAT_R16G16B16A16_SNORM:
      case VK_FORMAT_R16G16B16A16_USCALED:
      case VK_FORMAT_R16G16B16A16_SSCALED:
      case VK_FORMAT_R16G16B16A16_UINT:
      case VK_FORMAT_R16G16B16A16_SINT:
      case VK_FORMAT_R16G16B16A16_SFLOAT:
      case VK_FORMAT_R32G32B32A32_UINT:
      case VK_FORMAT_R32G32B32A32_SINT:
      case VK_FORMAT_R32G32B32A32_SFLOAT:
      case VK_FORMAT_R64G64B64A64_UINT:
      case VK_FORMAT_R64G64B64A64_SINT:
      case VK_FORMAT_R64G64B64A64_SFLOAT:
      return 4;

      default: assert(0);
    }
  return 0;
}

bool isHDRFormat(VkFormat fmt) {
  switch (fmt) {
    case VK_FORMAT_R16_UINT:
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16_SFLOAT:
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_R32_UINT:
    case VK_FORMAT_R32_SINT:
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
    case VK_FORMAT_R64_UINT:
    case VK_FORMAT_R64_SINT:
    case VK_FORMAT_R64_SFLOAT:
    case VK_FORMAT_R16G16_UINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16_SFLOAT:
    case VK_FORMAT_R32G32_UINT:
    case VK_FORMAT_R32G32_SINT:
    case VK_FORMAT_R32G32_SFLOAT:
    case VK_FORMAT_R64G64_UINT:
    case VK_FORMAT_R64G64_SINT:
    case VK_FORMAT_R64G64_SFLOAT:
    case VK_FORMAT_R16G16B16_UINT:
    case VK_FORMAT_R16G16B16_SINT:
    case VK_FORMAT_R16G16B16_SFLOAT:
    case VK_FORMAT_R32G32B32_UINT:
    case VK_FORMAT_R32G32B32_SINT:
    case VK_FORMAT_R32G32B32_SFLOAT:
    case VK_FORMAT_R64G64B64_UINT:
    case VK_FORMAT_R64G64B64_SINT:
    case VK_FORMAT_R64G64B64_SFLOAT:
    case VK_FORMAT_R16G16B16A16_UINT:
    case VK_FORMAT_R16G16B16A16_SINT:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
    case VK_FORMAT_R32G32B32A32_UINT:
    case VK_FORMAT_R32G32B32A32_SINT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
    case VK_FORMAT_R64G64B64A64_UINT:
    case VK_FORMAT_R64G64B64A64_SINT:
    case VK_FORMAT_R64G64B64A64_SFLOAT:
    return true;

    default: return false;
  }
  return false;
}

uint32_t BitsPerChannelInFormat(VkFormat fmt, VkImageAspectFlagBits aspect) {
  return static_cast<uint32_t>(SizeOfFormat(fmt, aspect) * 8 / ChannelsInFormat(fmt));
}

bool IsFPFormat(VkFormat fmt) {
  switch (fmt) {
    case VK_FORMAT_R16_SFLOAT:
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
    case VK_FORMAT_R64_SFLOAT:
    case VK_FORMAT_R16G16_SFLOAT:
    case VK_FORMAT_R32G32_SFLOAT:
    case VK_FORMAT_R64G64_SFLOAT:
    case VK_FORMAT_R16G16B16_SFLOAT:
    case VK_FORMAT_R32G32B32_SFLOAT:
    case VK_FORMAT_R64G64B64_SFLOAT:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
    case VK_FORMAT_R64G64B64A64_SFLOAT:
    return true;
    default: return false;
  }
  return false;
}

bool IsSignedFormat(VkFormat fmt) {
  switch (fmt) {
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
    case VK_FORMAT_B8G8R8_SNORM:
    case VK_FORMAT_B8G8R8_SSCALED:
    case VK_FORMAT_B8G8R8_SINT:
    case VK_FORMAT_B8G8R8A8_SNORM:
    case VK_FORMAT_B8G8R8A8_SSCALED:
    case VK_FORMAT_B8G8R8A8_SINT:
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16_SFLOAT:
    case VK_FORMAT_R32_SINT:
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
    case VK_FORMAT_R64_SINT:
    case VK_FORMAT_R64_SFLOAT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16_SFLOAT:
    case VK_FORMAT_R32G32_SINT:
    case VK_FORMAT_R32G32_SFLOAT:
    case VK_FORMAT_R64G64_SINT:
    case VK_FORMAT_R64G64_SFLOAT:
    case VK_FORMAT_R16G16B16_SINT:
    case VK_FORMAT_R16G16B16_SFLOAT:
    case VK_FORMAT_R32G32B32_SINT:
    case VK_FORMAT_R32G32B32_SFLOAT:
    case VK_FORMAT_R64G64B64_SINT:
    case VK_FORMAT_R64G64B64_SFLOAT:
    case VK_FORMAT_R16G16B16A16_SINT:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
    case VK_FORMAT_R32G32B32A32_SINT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
    case VK_FORMAT_R64G64B64A64_SINT:
    case VK_FORMAT_R64G64B64A64_SFLOAT:
    return true;
    default: return false;
  }
  return false;
}

uint32_t MinDimensionSize(VkFormat format) {
  switch (format) {
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC4_SNORM_BLOCK:
    case VK_FORMAT_BC4_UNORM_BLOCK:
    case VK_FORMAT_BC5_SNORM_BLOCK:
    case VK_FORMAT_BC5_UNORM_BLOCK:
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
    case VK_FORMAT_BC7_UNORM_BLOCK:
    return 4;

    default: return 1;
  }
}

bool IsDepthFormat(VkFormat fmt) {
  return
    (fmt == VK_FORMAT_D16_UNORM ||
     fmt == VK_FORMAT_D16_UNORM_S8_UINT ||
     fmt == VK_FORMAT_D24_UNORM_S8_UINT ||
     fmt == VK_FORMAT_D32_SFLOAT ||
     fmt == VK_FORMAT_D32_SFLOAT_S8_UINT);
}