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
#include <inttypes.h>
#include "helper/helper.h"
#include "shim_vulkan.h"

uint32_t ShimVkTraceResources::timestampCollectCommandCount(VkCommandBuffer cb)
{
  return static_cast<uint32_t>(cbCommandInfo[cb].vec.size());
}
uint32_t ShimVkTraceResources::timestampQueryCount(VkCommandBuffer cb)
{
  uint32_t count = 0;
  for(auto info : cbCommandInfo[cb].vec)
    count += info.timestamps;
  return count;
}

VkQueryPool ShimVkTraceResources::timestampQueryPool(VkCommandBuffer cb)
{
  return cbTimestampContext[cb].pool;
}

VkQueryPool ShimVkTraceResources::pipelinestatsQueryPool(VkCommandBuffer cb)
{
  return cbPipelineStatsContext[cb].pool;
}

uint32_t ShimVkTraceResources::timestampQueryOffset(VkCommandBuffer cb)
{
  return cbTimestampQueryRange[cb][0];
}

uint32_t ShimVkTraceResources::timestampQueryInc(VkCommandBuffer cb)
{
  return cbTimestampQueryRange[cb][2]++;
}

uint32_t ShimVkTraceResources::resetTimestampQueries(VkCommandBuffer cb)
{
  return cbTimestampQueryRange[cb][2] = 0;
}

uint32_t ShimVkTraceResources::timestampReportCommandCount(VkCommandBuffer cb)
{
  // If we have captured timestamps for more than just 2 vkCmd* commands
  // then exclude vk[Begin|End]CommandBuffer from reporting.
  // If we only have 2 commands, then report time for vkEndCommandBuffer
  // which is equal to vkBeginCommandBuffer.
  size_t commands = cbCommandInfo[cb].vec.size() > 2 ? cbCommandInfo[cb].vec.size() - 1
                                                     : cbCommandInfo[cb].vec.size();
  return (uint32_t)commands;
}

uint32_t ShimVkTraceResources::pipelinestatsQueryCount(VkCommandBuffer cb)
{
  return cbCommandInfo[cb].pipelineQueryCount;
}

uint32_t ShimVkTraceResources::pipelinestatsQueryUnitsCount(VkCommandBuffer cb)
{
  return cbCommandInfo[cb].pipelineQueryCount * cbPipelineStatsContext[cb].stats;
}

uint32_t ShimVkTraceResources::pipelinestatsQueryOffset(VkCommandBuffer cb)
{
  return cbPipelineStatQueryRange[cb][2];
}

uint32_t ShimVkTraceResources::pipelinestatsQueryInc(VkCommandBuffer cb)
{
  uint32_t old = cbPipelineStatQueryRange[cb][2];
  cbPipelineStatQueryRange[cb][2]++;
  return old;
}

uint32_t ShimVkTraceResources::resetPipelinestatsQueries(VkCommandBuffer cb)
{
  return cbPipelineStatQueryRange[cb][2] = 0;
}

// queueSubmit sets queue field in a cmdbuf's TimestampContext, and adds to be executed command
// buffers to cbSubmitOrder[queue].
void ShimVkTraceResources::queueSubmit(VkQueue queue, uint32_t cbCount, VkCommandBuffer *cbList)
{
  ExecCmdBufVec &exec = cbSubmitOrder[queue];
  for(uint32_t cbi = 0; cbi < cbCount; cbi++)
  {
    VkCommandBuffer currentCB = cbList[cbi];
    cbTimestampContext[currentCB].queue = queue;
    cbPipelineStatsContext[currentCB].isGraphics = isGraphicsQueue(queue);
    uint32_t idx = static_cast<uint32_t>(exec.size());
    exec.push_back(ExecuteCommandBuffer(currentCB, idx));
    for(auto execCB : cbExecCmdBufs[currentCB])
    {
      cbTimestampContext[execCB.cb].queue = queue;
      cbPipelineStatsContext[execCB.cb].isGraphics = cbPipelineStatsContext[currentCB].isGraphics;
    }
  }
}

void ShimVkTraceResources::executeCommands(VkCommandBuffer cb, VkCommandBuffer exec,
                                           uint32_t remaining)
{
  aux.secondaryCB(exec);
  cbExecCmdBufs[cb].push_back(ExecuteCommandBuffer(exec, remaining));
}

bool ShimVkTraceResources::isPresent(VkCommandBuffer cb)
{
  return cbCommandInfo.find(cb) != cbCommandInfo.end();
}

double ShimVkTraceResources::accumTimestamps(VkCommandBuffer cb, const std::vector<uint64_t> &data,
                                             uint64_t frameID, VkQueue queue)
{
  TimestampContext &context = cbTimestampContext[cb];
  if(cbAccumTimestamps[cb].empty())
  {
    cbAccumTimestamps[cb].resize(cbCommandInfo[cb].vec.size(), {0.0, 0.0f});
    context.mask = getTimestampValidBits(queue);
    context.period = physDeviceProperties.limits.timestampPeriod;
    context.accum = 0;
  }

  int64_t signedTS = (data.back() & context.mask) - (data.front() & context.mask);
  if(signedTS < 0)
  {
    fprintf(stderr, "timestamp for command buffer is negative %" PRId64 "\n", signedTS);
    signedTS = std::abs(signedTS);
  }
  double elapsedTimeNsec = double(signedTS * context.period);
  uint32_t accum = frameID % 2;
  cbAccumTimestamps[cb].front()[accum] +=
      elapsedTimeNsec;    // this is the total time for the command buffer submission.
  cbAccumTimestamps[cb].back()[accum] += elapsedTimeNsec;

  CommandInfoVec &cbVec = cbCommandInfo[cb].vec;
  uint32_t j = cbVec[0].timestamps;
  for(uint32_t i = 1; i < (cbVec.size() - 1); i++)
  {
    uint32_t tsCount = cbVec[i].timestamps;
    if(tsCount == 2)
    {
      signedTS = (data[j + 1] & context.mask) - (data[j] & context.mask);
      if(signedTS < 0)
      {
        fprintf(stderr, "timestamp for command %u is negative %" PRId64 "\n", i, signedTS);
        signedTS = std::abs(signedTS);
      }
      double elapsedTimeNsec = double(signedTS * context.period);
      cbAccumTimestamps[cb][i][accum] += elapsedTimeNsec;
    }
    else if(tsCount == 1)
    {
      // one timestamp is not enough to produce delta timings
      cbAccumTimestamps[cb][i][accum] = 0.0f;
    }
    else if(tsCount == 0)
    {
      // this is ExecuteCommands, it's handled in a special way
      cbAccumTimestamps[cb][i][accum] = 0.0f;
    }
    j += tsCount;
  }

  context.accum++;

  return elapsedTimeNsec;
}

void ShimVkTraceResources::accumulateAllTimestamps(VkCommandBuffer cb, uint64_t frameID)
{
  std::vector<VkCommandBuffer> todo(1, cb);

  // If command buffer had secondary buffers, add them to todo list of work.
  for(auto execCB : cbExecCmdBufs[cb])
    todo.push_back(execCB.cb);

  // The primary command buffer is submitted on the queue
  // that's been recorded in cbTimestampContext[cb], and all of it's
  // secondary command buffers were also submitted on the
  // same queue.
  VkQueue queue = cbTimestampContext[cb].queue;

  for(VkCommandBuffer cbi : todo)
  {
    uint32_t offset = timestampQueryOffset(cbi);
    uint32_t count = timestampQueryCount(cbi);
    std::vector<uint64_t> data(count);
    VkResult pollResult = vkGetQueryPoolResults(
        device, timestampQueryPool(cbi), offset, count, sizeof(uint64_t) * count, data.data(),
        sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    assert(pollResult == VK_SUCCESS);
    accumTimestamps(cbi, data, frameID, queue);
  }
}

void ShimVkTraceResources::fetchPipelineStats(uint64_t frameID)
{
  for(auto cb : cbCommandInfo)
  {
    std::vector<VkCommandBuffer> todo(1, cb.first);

    // If command buffer had secondary buffers, add them to todo list of work.
    for(auto execCB : cbExecCmdBufs[cb.first])
      todo.push_back(execCB.cb);

    for(VkCommandBuffer cbi : todo)
    {
      uint32_t offset = 0;
      uint32_t count = pipelinestatsQueryCount(cbi);
      VkQueryPool pool = pipelinestatsQueryPool(cbi);
      if(pool == VK_NULL_HANDLE)
        continue;

      std::vector<uint64_t> ps_data(pipelinestatsQueryUnitsCount(cbi));
      VkResult pollResult =
          vkGetQueryPoolResults(device, pool, offset, count, sizeof(uint64_t) * ps_data.size(),
                                ps_data.data(), sizeof(uint64_t) * cbPipelineStatsContext[cbi].stats,
                                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
      assert(pollResult == VK_SUCCESS);

      uint32_t s = sizeof(AllPipelineStats);
      uint32_t statsIndex = 0;
      for(auto &cbInfo : aux.cbCommandInfo[cbi].vec)
        if(cbInfo.hasStats)
        {
          if(cbPipelineStatsContext[cbi].stats == 1)
            cbInfo.stats.computeInvocations = ps_data[statsIndex++];
          else
            cbInfo.stats = (reinterpret_cast<AllPipelineStats *>(ps_data.data()))[statsIndex++];
        }
    }
  }
}

VkResult ShimVkTraceResources::createQueryPools()
{
  for(auto cbQuery : cbCommandInfo)
  {
    VkQueryPoolCreateInfo tsci = {};
    tsci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    tsci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    tsci.queryCount = timestampQueryCount(cbQuery.first);
    cbTimestampQueryRange[cbQuery.first][0] = 0;
    cbTimestampQueryRange[cbQuery.first][1] = tsci.queryCount;
    VK_CHECK_RESULT(vkCreateQueryPool(device, &tsci, NULL, &cbTimestampContext[cbQuery.first].pool));

    if(cbQuery.second.pipelineQueryCount == 0)
      continue;

    // Account for compute stats only first.
    cbPipelineStatsContext[cbQuery.first].stats = 1;

    VkQueryPoolCreateInfo psci = {};
    psci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    psci.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
    psci.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
    if(cbPipelineStatsContext[cbQuery.first].isGraphics)
    {
      psci.pipelineStatistics |=
          VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
          VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
          VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
          VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT |
          VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT |
          VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
          VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
          VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
          VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT |
          VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT;
      cbPipelineStatsContext[cbQuery.first].stats += 10;    // account for all graphics bits.
    }
    psci.queryCount = pipelinestatsQueryUnitsCount(cbQuery.first);
    cbPipelineStatQueryRange[cbQuery.first][0] = 0;
    cbPipelineStatQueryRange[cbQuery.first][1] = psci.queryCount;
    VK_CHECK_RESULT(
        vkCreateQueryPool(device, &psci, NULL, &cbPipelineStatsContext[cbQuery.first].pool));
  }
  return VK_SUCCESS;
}

float ShimVkTraceResources::commandTime(VkCommandBuffer cb, uint32_t cmd_index)
{
  TupleVec accum = cbAccumTimestamps[cb];
  float ts =
      float((accum[cmd_index][1] + accum[cmd_index][0]) / (1000000.0 * cbTimestampContext[cb].accum));
  return ts;
}

void ShimVkTraceResources::writeCSV(FILE *f, VkCommandBuffer cb, const char *cb_name,
                                    uint32_t cb_index, float cb_time, uint32_t cmd_index)
{
  float ts = commandTime(cb, cmd_index);
  std::string primary = isSecondary(cb) ? "secondary" : "primary";
  fprintf(f, "%d, %s, %s, %f, %s, %s, %d, %f", cb_index, cb_name, primary.c_str(), cb_time,
          cbCommandInfo[cb].vec[cmd_index].name.c_str(),
          cbCommandInfo[cb].vec[cmd_index].info.c_str(), cmd_index, ts);
  std::string str;
  if(cbCommandInfo[cb].vec[cmd_index].hasStats)
  {
    str = ", " + std::to_string(cbCommandInfo[cb].vec[cmd_index].stats.inputAssemblyVertices) +
          ", " + std::to_string(cbCommandInfo[cb].vec[cmd_index].stats.inputAssemblyPrimitive) +
          ", " + std::to_string(cbCommandInfo[cb].vec[cmd_index].stats.vertexInvocations) + ", " +
          std::to_string(cbCommandInfo[cb].vec[cmd_index].stats.geometryInvocations) + ", " +
          std::to_string(cbCommandInfo[cb].vec[cmd_index].stats.geometryPrimitives) + ", " +
          std::to_string(cbCommandInfo[cb].vec[cmd_index].stats.clippingInvocations) + ", " +
          std::to_string(cbCommandInfo[cb].vec[cmd_index].stats.clippingPrimitives) + ", " +
          std::to_string(cbCommandInfo[cb].vec[cmd_index].stats.fragmentInvocations) + ", " +
          std::to_string(cbCommandInfo[cb].vec[cmd_index].stats.controlInvocations) + ", " +
          std::to_string(cbCommandInfo[cb].vec[cmd_index].stats.evaluationInvocations) + ", " +
          std::to_string(cbCommandInfo[cb].vec[cmd_index].stats.computeInvocations);
  }
  fprintf(f, "%s\n", str.c_str());
}

void ShimVkTraceResources::writeAllCSV(const char *name)
{
  FILE *csv = OpenFile(name, "wt");

  fprintf(csv, "%s\n",
          "Command Buffer Index, Command Buffer Variable, Command Buffer Type, Command Buffer "
          "Total Time, "
          "Command Name, Command Info, Command Index, Elapsed Time (ms), "
          "IA Vertices, IA Primitives, VS Invocations, GS Invocations, GS Primitives, "
          "Clipping Invocations, Clipping Primitives, FS Invocations, TCS Invocations, "
          "TES Invocations, CS Invocations");

  for(auto submit : cbSubmitOrder)
  {
    ExecCmdBufVec &order = submit.second;
    for(uint32_t cb_i = 0; cb_i < order.size(); cb_i++)
    {
      VkCommandBuffer cb = order[cb_i].cb;
      const char *cb_name =
          GetResourceName(ResourceNames, VkHandle((uint64_t)cb, "VkCommandBuffer"));
      float cb_time = commandTime(cb, 0);    // time for command buffer total execution.
      uint32_t commands = timestampReportCommandCount(cb);
      for(uint32_t j = 1; j < commands; j++)    // skips vkBeginCommandBuffer
      {
        if(cbCommandInfo[cb].vec[j].name == "shim_vkCmdExecuteCommands")
        {
          // Replace this command with the list of commands in secondary cmd buffer.
          uint32_t &offset = cbCommandInfo[cb].executeCommands;
          uint32_t remaining = cbExecCmdBufs[cb][offset].remaining;
          for(uint32_t r = 0; r < remaining; r++)
          {
            VkCommandBuffer execCB = cbExecCmdBufs[cb][offset + r].cb;
            const char *execCB_name =
                GetResourceName(ResourceNames, VkHandle((uint64_t)execCB, "VkCommandBuffer"));
            float execCB_time =
                commandTime(execCB, 0);    // time for command buffer total execution.
            uint32_t exec_commands = timestampReportCommandCount(execCB);
            for(uint32_t execj = 1; execj < exec_commands; execj++)
            {
              writeCSV(csv, execCB, execCB_name, cb_i, execCB_time, execj);
            }
          }
          offset += remaining;
        }
        else
        {
          writeCSV(csv, cb, cb_name, cb_i, cb_time, j);
        }
      }
    }
  }
  fclose(csv);
}

void ShimVkTraceResources::addCommandInfo(VkCommandBuffer cb, const CommandInfo &info)
{
  aux.cbCommandInfo[cb].vec.push_back(info);
  if(info.name.find("vkCmdDraw") != std::string::npos ||
     info.name.find("vkCmdDispatch") != std::string::npos)
  {
    aux.cbCommandInfo[cb].vec.back().hasStats = true;
    aux.cbCommandInfo[cb].pipelineQueryCount++;
  }

  isInline(cb, info.isInlinedSubpass);
}

bool ShimVkTraceResources::isInline(VkCommandBuffer cb)
{
  return cbCommandInfo[cb].isInlinedSubpass;
}

void ShimVkTraceResources::isInline(VkCommandBuffer cb, bool current)
{
  cbCommandInfo[cb].isInlinedSubpass = current;
}

void ShimVkTraceResources::secondaryCB(VkCommandBuffer cb)
{
  cbSecondary[cb] = true;
}

bool ShimVkTraceResources::isSecondary(VkCommandBuffer cb)
{
  return cbSecondary.count(cb) > 0;
}