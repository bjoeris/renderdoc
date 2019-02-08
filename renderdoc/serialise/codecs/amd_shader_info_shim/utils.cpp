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
#include "utils.h"

#include <fstream>
#include <sstream>

std::string getStageStr(VkShaderStageFlagBits stage)
{
  switch(stage)
  {
    case(VK_SHADER_STAGE_VERTEX_BIT): return "VERTEX";
    case(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT): return "TESSELLATION_CONTROL";
    case(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT): return "TESSELLATION_EVALUATION";
    case(VK_SHADER_STAGE_GEOMETRY_BIT): return "GEOMETRY";
    case(VK_SHADER_STAGE_FRAGMENT_BIT): return "FRAGMENT";
    case(VK_SHADER_STAGE_COMPUTE_BIT): return "COMPUTE";
    case(VK_SHADER_STAGE_ALL_GRAPHICS): return "ALL_GRAPHICS";
    case(VK_SHADER_STAGE_ALL): return "ALL";
    default: return "UNKNOWN";
  }
}

void printShaderInfo(VkPipeline p, VkShaderStageFlagBits stage, const char *disassembly, size_t size)
{
  std::stringstream ss;
  if(!outputDir.empty())
    ss << outputDir;
  ss << GetResourceName(ResourceNames, VkHandle((uint64_t)p, "VkPipeline")) << "_VK_SHADER_STAGE_"
     << getStageStr(stage) << "_disassembly.txt";
  std::string filename;
  ss >> filename;
  std::ofstream file(filename);
  file.write(disassembly, size);
  file.close();
}

void printShaderInfo(VkPipeline p, VkShaderStageFlagBits stage, VkShaderStatisticsInfoAMD &statistics)
{
  std::stringstream ss;
  if(!outputDir.empty())
    ss << outputDir;
  ss << GetResourceName(ResourceNames, VkHandle((uint64_t)p, "VkPipeline")) << "_VK_SHADER_STAGE_"
     << getStageStr(stage) << "_statistics.txt";
  std::string filename;
  ss >> filename;
#if defined(__yeti__) || defined(__ggp__)
  filename = "/var/game/" + filename;
#endif
  std::ofstream file(filename);
  file << "vkGetShaderInfoAMD" << std::endl
       << "{" << std::endl
       << "  pipeline: "
       << "0x" << std::hex << reinterpret_cast<uint64_t>(p) << std::endl
       << "  shaderStage: "
       << "VK_SHADER_STAGE_" << getStageStr(stage) << std::endl
       << "  infoType: VK_SHADER_INFO_TYPE_STATISTICS_AMD" << std::endl
       << "  pInfo: " << std::endl
       << "  {" << std::endl
       << "    shaderStageMask: " << statistics.shaderStageMask << std::endl
       << "    resourceUsage: " << std::endl
       << std::dec << "    {" << std::endl
       << "      numUsedVgprs: " << statistics.resourceUsage.numUsedVgprs << std::endl
       << "      numUsedSgprs: " << statistics.resourceUsage.numUsedSgprs << std::endl
       << "      ldsSizePerLocalWorkGroup: " << statistics.resourceUsage.ldsSizePerLocalWorkGroup
       << std::endl
       << "      ldsUsageSizeInBytes: " << statistics.resourceUsage.ldsUsageSizeInBytes << std::endl
       << "      scratchMemUsageInBytes: " << statistics.resourceUsage.scratchMemUsageInBytes
       << std::endl
       << "    }" << std::endl
       << "    numPhysicalVgprs: " << statistics.numPhysicalVgprs << std::endl
       << "    numPhysicalSgprs: " << statistics.numPhysicalSgprs << std::endl
       << "    numAvailableVgprs: " << statistics.numAvailableVgprs << std::endl
       << "    numAvailableSgprs: " << statistics.numAvailableSgprs << std::endl
       << "    computeWorkGroupSize: "
       << "[" << statistics.computeWorkGroupSize[0] << "," << statistics.computeWorkGroupSize[1]
       << "," << statistics.computeWorkGroupSize[2] << "]" << std::endl
       << "  }" << std::endl
       << "}" << std::endl;
  file.close();
}
