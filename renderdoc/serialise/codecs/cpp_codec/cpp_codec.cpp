/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2018-2019 Google LLC
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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "common/common.h"
#include "core/core.h"
#include "serialise/rdcfile.h"
#include "cpp_codec_writer.h"
#include "vk_cpp_codec_writer.h"

namespace cpp_codec
{
CodeWriter *CreateCodeWriter(rdcstr path, RDCDriver driver)
{
  switch(driver)
  {
    case RDCDriver::Vulkan: return new vk_cpp_codec::VkCodeWriter(path);
    default:
    {
      rdcstr driverName = ToStr(driver);
      RDCERR("No CPP writer found for driver %s", driverName.c_str());
      return NULL;
    }
  }
};
}

ReplayStatus exportCPPZ(const char *filename, const RDCFile &rdc, const SDFile &structData,
                        RENDERDOC_ProgressCallback progress)
{
  std::string s_filename(filename);
  size_t found = s_filename.find_last_of(".");
  RDCASSERT(found != std::string::npos);

  cpp_codec::CodeWriter *code =
      cpp_codec::CreateCodeWriter(s_filename.substr(0, found), rdc.GetDriver());

  // StructuredChunkList chunks;
  // StructuredBufferList buffers;
  // for(uint32_t i = 0; i < structData.chunks.size(); i++)
  //{
  //  chunks.push_back(structData.chunks[i]);
  //}
  // for(uint32_t i = 0; i < structData.buffers.size(); i++)
  //{
  //  buffers.push_back(structData.buffers[i]);
  //}

  // code.PrintReadBuffers(buffers);

  ReplayStatus status = code->Structured2Code(rdc, structData.version, structData.chunks,
                                              structData.buffers, progress);
  code->Close();
  delete code;

  return status;
}

static ConversionRegistration CPPConversionRegistration(
    &exportCPPZ, {
                     "cpp", "CPP capture project",
                     R"(Stores the structured data in an cpp project, with large buffer data
 stored in indexed blobs in binary files. It cannot be reimported.)",
                     false,
                 });
