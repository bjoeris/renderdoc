#pragma once
#include <array>
#include <cmath>
#include <fstream>
#include <string>

#include <assert.h>

#include "helper/helper.h"

struct ExecuteCommandBuffer {
  VkCommandBuffer cb;
  uint32_t id;
};

struct TimestampContext {
  VkQueue queue;
  VkQueryPool pool;
  uint64_t mask;
  uint64_t period;
  uint32_t accum;
};

typedef std::array<uint32_t, 3> TripleU32;
typedef std::vector<std::array<double, 2>> TupleVec;
typedef std::vector<std::string> StringVec;
typedef std::vector<ExecuteCommandBuffer> ExecCmdBufVec;

struct ShimVkTraceResources: public AuxVkTraceResources {
  std::map<VkQueue, ExecCmdBufVec> cbSubmitOrder;
  std::map<VkCommandBuffer, StringVec> cbQueries;
  std::map<VkCommandBuffer, TupleVec> cbAccumTimestamps;
  std::map<VkCommandBuffer, TripleU32 > cbQueryRange;
  std::map<VkCommandBuffer, ExecCmdBufVec> cbExecCmdBufs;
  std::map<VkCommandBuffer, TimestampContext > cbContext;

  uint32_t cmdCount(VkCommandBuffer cb) {
    return cbQueries[cb].size();
  }
  uint32_t queryCount(VkCommandBuffer cb) {
    uint32_t count = cbQueries[cb].size();
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
  uint32_t totalQueryCount() {
    uint32_t totalCount = 0;
    for(auto cbQuery : cbQueries) {
      totalCount += queryCount(cbQuery.first);
    }
    return totalCount;
  }
  uint32_t assignQueryRange() {
    uint32_t totalCount = totalQueryCount();
    uint32_t offset = 0;
    for (auto cbQuery : cbQueries) {
      uint32_t count = queryCount(cbQuery.first);
      cbQueryRange[cbQuery.first][0] = offset;
      cbQueryRange[cbQuery.first][1] = count;
      offset += count;
    }
    assert(offset == totalCount);
    return totalCount;
  }
  uint32_t cbCount() {
    return cbQueries.size();
  }
  uint32_t cbQueryOffset(VkCommandBuffer cb) {
    return cbQueryRange[cb][0];
  }
  uint32_t cbQueryCount(VkCommandBuffer cb) {
    return cbQueryRange[cb][1];
  }
  uint32_t cbQueryReset(VkCommandBuffer cb) {
    return cbQueryRange[cb][2] = 0;
  }
  uint32_t cbQueryInc(VkCommandBuffer cb) {
    return cbQueryRange[cb][2]++;
  }

  void queueSubmit(VkQueue queue, uint32_t cbCount, VkCommandBuffer * cbList) {
    ExecCmdBufVec & exec = cbSubmitOrder[queue];
    for (uint32_t cbi = 0; cbi < cbCount; cbi++) {
      VkCommandBuffer currentCB = cbList[cbi];
      cbContext[currentCB].queue = queue;
      uint32_t idx = exec.size();
      exec.push_back(ExecuteCommandBuffer{currentCB, idx});
    }
  }

  void cbAddExecCmdBufRelation(VkCommandBuffer cb, VkCommandBuffer exec, uint32_t offset) {
    cbExecCmdBufs[cb].push_back(ExecuteCommandBuffer{exec, offset});
  }

  bool IsPresent(VkCommandBuffer cb) {
    return cbQueries.find(cb) != cbQueries.end();
  }

  double accumTimestamps(VkCommandBuffer cb, const std::vector<uint64_t> &data, uint64_t frameID) {
    TimestampContext &context = cbContext[cb];
    if (cbAccumTimestamps[cb].empty()) {
      cbAccumTimestamps[cb].resize(cbQueries[cb].size(), {0.0, 0.0f});
      context.mask = getTimestampValidBits(context.queue);
      context.period = physDeviceProperties.limits.timestampPeriod;
      context.accum = 0;
    }

    double elapsedTimeNsec = (data.back() & context.mask -
                              data.front() & context.mask) * context.period;
    uint32_t accum = frameID % 2;
    cbAccumTimestamps[cb].front()[accum] += elapsedTimeNsec; // this is the total time for the command buffer submission.
    cbAccumTimestamps[cb].back()[accum] += elapsedTimeNsec;

    for (uint32_t i = 1; i < data.size() - 1; i+=2) {
      double elapsedTimeNsec = (data[i+1] & context.mask -
                                data[i] & context.mask) * context.period;
      cbAccumTimestamps[cb][(i+1) / 2][accum] += elapsedTimeNsec;
    }

    context.accum++;

    return elapsedTimeNsec;
  }

  void accumulateAllTimestamps(VkCommandBuffer cb, uint64_t frameID) {
    std::vector<VkCommandBuffer> todo(1, cb);

    // If command buffer had indirect buffers, add them to
    // todo list of work.
    for (auto execCB : cbExecCmdBufs[cb])
      todo.push_back(execCB.cb);

    for (VkCommandBuffer cbi : todo) {
      uint32_t offset = cbQueryOffset(cbi);
      uint32_t count = cbQueryCount(cbi);
      std::vector<uint64_t> data(count);
      VkResult pollResult = vkGetQueryPoolResults(
        device, queryPool(cbi), offset, count,
        sizeof(uint64_t) * count, data.data(), sizeof(uint64_t),
        VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
      assert(pollResult == VK_SUCCESS);
      accumTimestamps(cbi, data, frameID);
    }
  }

  VkQueryPool queryPool(VkCommandBuffer cb) {
    return cbContext[cb].pool;
  }

  VkQueue queryQueue(VkCommandBuffer cb) {
    return cbContext[cb].queue;
  }

  VkResult createQueryPools() {
    for (auto cbQuery : cbQueries) {
      VkQueryPoolCreateInfo ci = {};
      ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
      ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
      ci.queryCount = queryCount(cbQuery.first);
      cbQueryRange[cbQuery.first][0] = 0;
      cbQueryRange[cbQuery.first][1] = ci.queryCount;
      VkQueryPool & qp = cbContext[cbQuery.first].pool;
      VK_CHECK_RESULT(vkCreateQueryPool(device, &ci, NULL, &qp));
    }
    return VK_SUCCESS;
  }

  void writeCSV(const char *name) {
    FILE *csv = fopen(name, "wt");

    fprintf(csv, "%s\n", "Command Buffer Index, Command Name, Command Index, Elapsed Time (ms)");

    for (auto submit : cbSubmitOrder) {
      ExecCmdBufVec & order = submit.second;
      for (uint32_t i = 0; i < order.size(); i++) {
        VkCommandBuffer cb = order[i].cb;
        TupleVec accum = cbAccumTimestamps[cb];
        for (uint32_t j = 0; j < cbQueries[cb].size(); j++) {
          double ts = (accum[j][1] + accum[j][0]) / (1000000.0 * cbContext[cb].accum);
          fprintf(csv, "%d, %s, %d, %f\n",
            i, cbQueries[cb][j].c_str(), j, ts);
        }
      }
    }

    fclose(csv);
  }
};

extern ShimVkTraceResources aux;
extern int presentIndex;