#include "utils.h"
#include "helper/helper.h"
#include "shim_vulkan.h"

uint32_t ShimVkTraceResources::cmdCount(VkCommandBuffer cb)
{
  return static_cast<uint32_t>(cbCommandInfo[cb].size());
}
uint32_t ShimVkTraceResources::queryCount(VkCommandBuffer cb)
{
  uint32_t count = cmdCount(cb);
  // Command buffer queries are written like this:
  // vkBeginCommandBuffer
  // vkCmd* Start Timestamp
  // vkCmd* End Timestamp
  // vkEndCommandBuffer
  // Therefore for each command there are 2 timestamp queries
  // except for Begin/End command buffers which have just 1.
  count = (count - 1) * 2;
  return count;
}
uint32_t ShimVkTraceResources::totalQueryCount()
{
  uint32_t totalCount = 0;
  for(auto cbQuery : cbCommandInfo)
  {
    totalCount += queryCount(cbQuery.first);
  }
  return totalCount;
}
uint32_t ShimVkTraceResources::assignQueryRange()
{
  uint32_t totalCount = totalQueryCount();
  uint32_t offset = 0;
  for(auto ci : cbCommandInfo)
  {
    uint32_t count = queryCount(ci.first);
    cbQueryRange[ci.first][0] = offset;
    cbQueryRange[ci.first][1] = count;
    offset += count;
  }
  assert(offset == totalCount);
  return totalCount;
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
    exec.push_back(ExecuteCommandBuffer{currentCB, idx});
    for(auto execCB : cbExecCmdBufs[currentCB])
    {
      cbContext[execCB.cb].queue = queue;
    }
  }
}

void ShimVkTraceResources::addExecCmdBufRelation(VkCommandBuffer cb, VkCommandBuffer exec, uint32_t offset)
{
  cbExecCmdBufs[cb].push_back(ExecuteCommandBuffer{exec, offset});
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
    cbAccumTimestamps[cb].resize(cbCommandInfo[cb].size(), {0.0, 0.0f});
    context.mask = getTimestampValidBits(queue);
    context.period = physDeviceProperties.limits.timestampPeriod;
    context.accum = 0;
  }

  double elapsedTimeNsec =
      double(((data.back() & context.mask) - (data.front() & context.mask)) * context.period);
  uint32_t accum = frameID % 2;
  cbAccumTimestamps[cb].front()[accum] +=
      elapsedTimeNsec;    // this is the total time for the command buffer submission.
  cbAccumTimestamps[cb].back()[accum] += elapsedTimeNsec;

  for(uint32_t i = 1; i < data.size() - 1; i += 2)
  {
    double elapsedTimeNsec =
        double(((data[i + 1] & context.mask) - (data[i] & context.mask)) * context.period);
    cbAccumTimestamps[cb][(i + 1) / 2][accum] += elapsedTimeNsec;
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
  cbExecCmdBufs[cb].clear();
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

void ShimVkTraceResources::writeCSV(const char *name)
{
  FILE *csv = OpenFile(name, "wt");

  fprintf(csv, "%s\n",
          "Command Buffer Index, Command Name, Command Info, Command Index, Elapsed Time (ms)");

  for(auto submit : cbSubmitOrder)
  {
    ExecCmdBufVec &order = submit.second;
    for(uint32_t i = 0; i < order.size(); i++)
    {
      VkCommandBuffer cb = order[i].cb;
      TupleVec accum = cbAccumTimestamps[cb];
      for(uint32_t j = 0; j < cbCommandInfo[cb].size(); j++)
      {
        double ts = (accum[j][1] + accum[j][0]) / (1000000.0 * cbContext[cb].accum);
        fprintf(csv, "%d, %s, %s, %d, %f\n", i, cbCommandInfo[cb][j].name.c_str(),
                cbCommandInfo[cb][j].info.c_str(), j, ts);
      }
    }
  }

  fclose(csv);
}