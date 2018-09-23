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

struct CommandInfo {
  std::string name;
  std::string info;
};

typedef std::array<uint32_t, 3> TripleU32;
typedef std::vector<std::array<double, 2>> TupleVec;
typedef std::vector<CommandInfo> CommandInfoVec;
typedef std::vector<ExecuteCommandBuffer> ExecCmdBufVec;

struct ShimVkTraceResources: public AuxVkTraceResources {
  std::map<VkQueue, ExecCmdBufVec> cbSubmitOrder;
  std::map<VkCommandBuffer, CommandInfoVec> cbCommandInfo;
  std::map<VkCommandBuffer, TupleVec> cbAccumTimestamps;
  std::map<VkCommandBuffer, TripleU32 > cbQueryRange;
  std::map<VkCommandBuffer, ExecCmdBufVec> cbExecCmdBufs;
  std::map<VkCommandBuffer, TimestampContext > cbContext;

  uint32_t cmdCount(VkCommandBuffer cb);
  uint32_t queryCount(VkCommandBuffer cb);
  uint32_t totalQueryCount();
  uint32_t assignQueryRange();
  uint32_t queryOffset(VkCommandBuffer cb);
  uint32_t resetQueries(VkCommandBuffer cb);
  uint32_t queryInc(VkCommandBuffer cb);

  void queueSubmit(VkQueue queue, uint32_t cbCount, VkCommandBuffer * cbList);
  void addExecCmdBufRelation(VkCommandBuffer cb, VkCommandBuffer exec, uint32_t offset);
  bool isPresent(VkCommandBuffer cb);
  double accumTimestamps(VkCommandBuffer cb, const std::vector<uint64_t> &data, uint64_t frameID);
  void accumulateAllTimestamps(VkCommandBuffer cb, uint64_t frameID);

  VkQueryPool queryPool(VkCommandBuffer cb);
  VkQueue queryQueue(VkCommandBuffer cb);
  VkResult createQueryPools();

  void writeCSV(const char *name);
};

extern ShimVkTraceResources aux;
extern int presentIndex;