#include "utils.h"
#include "helper/helper.h"
#include "shim_vulkan.h"

uint32_t ShimVkTraceResources::cmdCount(VkCommandBuffer cb)
{
  return static_cast<uint32_t>(cbCommandInfo[cb].vec.size());
}
uint32_t ShimVkTraceResources::queryCount(VkCommandBuffer cb)
{
  uint32_t count = 0;
  for (auto info : cbCommandInfo[cb].vec)
    count += info.timestamps;
  return count;
}

uint32_t ShimVkTraceResources::queryOffset(VkCommandBuffer cb)
{
  return cbQueryRange[cb][0];
}

uint32_t ShimVkTraceResources::resetQueries(VkCommandBuffer cb)
{
  return cbQueryRange[cb][2] = 0;
}

uint32_t ShimVkTraceResources::queryInc(VkCommandBuffer cb)
{
  return cbQueryRange[cb][2]++;
}

// queueSubmit sets queue field in a cmdbuf's TimestampContext, and adds to be executed command
// buffers to cbSubmitOrder[queue].
void ShimVkTraceResources::queueSubmit(VkQueue queue, uint32_t cbCount, VkCommandBuffer *cbList)
{
  ExecCmdBufVec &exec = cbSubmitOrder[queue];
  for(uint32_t cbi = 0; cbi < cbCount; cbi++)
  {
    VkCommandBuffer currentCB = cbList[cbi];
    cbContext[currentCB].queue = queue;
    uint32_t idx = static_cast<uint32_t>(exec.size());
    exec.push_back(ExecuteCommandBuffer(currentCB, idx));
    for(auto execCB : cbExecCmdBufs[currentCB])
    {
      cbContext[execCB.cb].queue = queue;
    }
  }
}

void ShimVkTraceResources::markExecCmdBufRelation(VkCommandBuffer cb, VkCommandBuffer exec, uint32_t remaining)
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
  TimestampContext &context = cbContext[cb];
  if(cbAccumTimestamps[cb].empty())
  {
    cbAccumTimestamps[cb].resize(cbCommandInfo[cb].vec.size(), {0.0, 0.0f});
    context.mask = getTimestampValidBits(queue);
    context.period = physDeviceProperties.limits.timestampPeriod;
    context.accum = 0;
  }

  int64_t signedTS = (data.back() & context.mask) - (data.front() & context.mask);
  signedTS = signedTS < 0 ? -signedTS : signedTS;
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
    if (tsCount == 2) {
      signedTS = (data[j + 1] & context.mask) - (data[j] & context.mask);
      signedTS = signedTS < 0 ? -signedTS : signedTS;
      double elapsedTimeNsec = double(signedTS * context.period);
      cbAccumTimestamps[cb][i][accum] += elapsedTimeNsec;
    } else if (tsCount == 1) {
      // one timestamp is not enough to produce delta timings
      cbAccumTimestamps[cb][i][accum] = 0.0f;
    } else if (tsCount == 0) {
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
  // that's been recorded in cbContext[cb], and all of it's
  // secondary command buffers were also submitted on the
  // same queue.
  VkQueue queue = cbContext[cb].queue;

  for(VkCommandBuffer cbi : todo)
  {
    uint32_t offset = queryOffset(cbi);
    uint32_t count = queryCount(cbi);
    std::vector<uint64_t> data(count);
    VkResult pollResult = vkGetQueryPoolResults(
        device, queryPool(cbi), offset, count, sizeof(uint64_t) * count, data.data(),
        sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    assert(pollResult == VK_SUCCESS);
    accumTimestamps(cbi, data, frameID, queue);
  }
}

VkQueryPool ShimVkTraceResources::queryPool(VkCommandBuffer cb)
{
  return cbContext[cb].pool;
}

VkQueue ShimVkTraceResources::queryQueue(VkCommandBuffer cb)
{
  return cbContext[cb].queue;
}

VkResult ShimVkTraceResources::createQueryPools()
{
  for(auto cbQuery : cbCommandInfo)
  {
    VkQueryPoolCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    ci.queryCount = queryCount(cbQuery.first);
    cbQueryRange[cbQuery.first][0] = 0;
    cbQueryRange[cbQuery.first][1] = ci.queryCount;
    VK_CHECK_RESULT(vkCreateQueryPool(device, &ci, NULL, &cbContext[cbQuery.first].pool));
  }
  return VK_SUCCESS;
}

float ShimVkTraceResources::commandTime(VkCommandBuffer cb,
  uint32_t cmd_index) {
  TupleVec accum = cbAccumTimestamps[cb];
  float ts = float((accum[cmd_index][1] + accum[cmd_index][0]) /
                   (1000000.0 * cbContext[cb].accum));
  return ts;
}

void ShimVkTraceResources::writeCSV(FILE * f, VkCommandBuffer cb, uint32_t cmd_index,
  uint32_t cb_index, float cb_time) {
  float ts = commandTime(cb, cmd_index);
  std::string primary = isSecondary(cb) ? "secondary" : "primary";
  fprintf(f, "%d, %s, %f, %s, %s, %d, %f\n",
    cb_index, primary.c_str(), cb_time,
    cbCommandInfo[cb].vec[cmd_index].name.c_str(),
    cbCommandInfo[cb].vec[cmd_index].info.c_str(),
    cmd_index, ts);
}

uint32_t ShimVkTraceResources::commandCount(VkCommandBuffer cb) {
  // If we have captured timestamps for more than just 2 vkCmd* commands
  // then exclude vk[Begin|End]CommandBuffer from reporting.
  // If we only have 2 commands, then report time for vkEndCommandBuffer
  // which is equal to vkBeginCommandBuffer.
  uint32_t commands = cbCommandInfo[cb].vec.size() > 2 ?
    cbCommandInfo[cb].vec.size() - 1 : cbCommandInfo[cb].vec.size();
  return commands;
}

void ShimVkTraceResources::writeAllCSV(const char *name)
{
  FILE *csv = OpenFile(name, "wt");

  fprintf(csv, "%s\n",
          "Command Buffer Index, Command Buffer Type, Command Buffer Total Time, "
          "Command Name, Command Info, Command Index, Elapsed Time (ms)");

  for(auto submit : cbSubmitOrder)
  {
    ExecCmdBufVec &order = submit.second;
    for(uint32_t i = 0; i < order.size(); i++)
    {
      VkCommandBuffer cb = order[i].cb;
      float cb_time = commandTime(cb, 0); // time for command buffer total execution.
      uint32_t commands = commandCount(cb);
      for(uint32_t j = 1; j < commands; j++) // skips vkBeginCommandBuffer
      {
        if (cbCommandInfo[cb].vec[j].name == "shim_vkCmdExecuteCommands") {
          // Replace this command with the list of commands in secondary cmd buffer.
          uint32_t &offset = cbCommandInfo[cb].executeCommands;
          uint32_t remaining = cbExecCmdBufs[cb][offset].remaining;
          for (uint32_t r = 0; r < remaining; r++) {
            VkCommandBuffer execCB = cbExecCmdBufs[cb][offset + r].cb;
            float exec_cb_time = commandTime(execCB, 0); // time for command buffer total execution.
            uint32_t exec_commands = commandCount(execCB);
            for (uint32_t execj = 1; execj < exec_commands; execj++) {
              writeCSV(csv, execCB, execj, i, exec_cb_time);
            }
          }
          offset += remaining;
        } else {
          writeCSV(csv, cb, j, i, cb_time);
        }
      }
    }
  }
  fclose(csv);
}

void ShimVkTraceResources::addCommandInfo(VkCommandBuffer cb, const CommandInfo & info) {
  aux.cbCommandInfo[cb].vec.push_back(info);
  isInline(cb, info.isInlinedSubpass);
}

bool ShimVkTraceResources::isInline(VkCommandBuffer cb) {
  return cbCommandInfo[cb].isInlinedSubpass;
}

void ShimVkTraceResources::isInline(VkCommandBuffer cb, bool current) {
  cbCommandInfo[cb].isInlinedSubpass = current;
}