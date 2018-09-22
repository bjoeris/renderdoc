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
#pragma once

#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#if defined(_WIN32)
#include <Windows.h>
#endif

#include <algorithm>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "vulkan/vulkan.h"

extern std::map<VkFormat, std::pair<VkImageAspectFlags, uint32_t>> kImageAspectsAndByteSize;

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
