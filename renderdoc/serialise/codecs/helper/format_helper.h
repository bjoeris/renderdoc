#pragma once

#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdint.h>
#if defined(_WIN32)
#include <Windows.h>
#endif

#include <algorithm>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>


#include "vulkan/vulkan.h"

extern std::unordered_map<VkFormat, std::pair<VkImageAspectFlags, uint32_t>> kImageAspectsAndByteSize;

double             SizeOfFormat(VkFormat fmt, VkImageAspectFlagBits aspect);
uint32_t           ChannelsInFormat(VkFormat fmt);
bool               isHDRFormat(VkFormat fmt);
bool               IsFPFormat(VkFormat fmt);
bool               IsSignedFormat(VkFormat fmt);
bool               IsDepthFormat(VkFormat fmt);
uint32_t           BitsPerChannelInFormat(VkFormat fmt, VkImageAspectFlagBits aspect = VK_IMAGE_ASPECT_COLOR_BIT);
VkImageAspectFlags FullAspectFromFormat(VkFormat fmt);
VkImageAspectFlags AspectFromFormat(VkFormat fmt);
uint32_t           FixCompressedSizes(VkFormat fmt, VkExtent3D &dim, uint32_t &offset);
std::string        FormatToString(VkFormat f);
uint32_t           MinDimensionSize(VkFormat format);