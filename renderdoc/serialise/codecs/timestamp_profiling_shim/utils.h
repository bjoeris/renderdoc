#pragma once
#include <array>
#include <cmath>
#include <fstream>
#include <string>

#include <assert.h>

#include "helper/helper.h"

struct ExecuteCommandBuffer
{
  VkCommandBuffer cb;
  uint32_t remaining = 0;

  ExecuteCommandBuffer(VkCommandBuffer acb, uint32_t r = 0) {
    cb = acb;
    remaining = r;
  }
};

struct TimestampContext
{
  VkQueue queue;
  VkQueryPool pool;
  uint64_t mask;
  float period;
  uint32_t accum;
};

struct CommandInfo
{
  std::string name;
  std::string info;
  uint32_t timestamps = 2;
  bool isInlinedSubpass = true;

  CommandInfo(std::string n) {
    name = n;
  }
  CommandInfo(std::string n, std::string i) {
    name = n; info = i;
  }
  CommandInfo(std::string n, std::string i, uint32_t ts) {
    name = n; info = i; timestamps = ts; 
  }
  CommandInfo(std::string n, std::string i, uint32_t ts, bool inl) { 
    name = n; info = i; timestamps = ts; isInlinedSubpass = inl;
  }
};

typedef std::array<uint32_t, 3> TripleU32;
typedef std::vector<std::array<double, 2>> TupleVec;
typedef std::vector<CommandInfo> CommandInfoVec;

struct CommandInfoDesc {
  CommandInfoVec vec;
  bool isInlinedSubpass = false;
  uint32_t executeCommands = 0;
};
typedef std::vector<ExecuteCommandBuffer> ExecCmdBufVec;

struct ShimVkTraceResources : public AuxVkTraceResources
{
  std::map<VkQueue, ExecCmdBufVec> cbSubmitOrder;
  std::map<VkCommandBuffer, CommandInfoDesc> cbCommandInfo;
  std::map<VkCommandBuffer, TupleVec> cbAccumTimestamps;
  std::map<VkCommandBuffer, TripleU32> cbQueryRange;
  std::map<VkCommandBuffer, ExecCmdBufVec> cbExecCmdBufs;
  std::map<VkCommandBuffer, TimestampContext> cbContext;
  std::map<VkCommandBuffer, bool> cbSecondary;

  uint32_t cmdCount(VkCommandBuffer cb);
  uint32_t queryCount(VkCommandBuffer cb);
  
  uint32_t queryOffset(VkCommandBuffer cb);
  uint32_t resetQueries(VkCommandBuffer cb);
  uint32_t queryInc(VkCommandBuffer cb);

  void addCommandInfo(VkCommandBuffer cb, const CommandInfo &info);
  bool isInline(VkCommandBuffer cb);
  void isInline(VkCommandBuffer cb, bool current);

  void queueSubmit(VkQueue queue, uint32_t cbCount, VkCommandBuffer *cbList);
  void markExecCmdBufRelation(VkCommandBuffer cb, VkCommandBuffer exec, uint32_t offset);
  bool isPresent(VkCommandBuffer cb);
  double accumTimestamps(VkCommandBuffer cb, const std::vector<uint64_t> &data, uint64_t frameID, VkQueue queue);
  void accumulateAllTimestamps(VkCommandBuffer cb, uint64_t frameID);

  void secondaryCB(VkCommandBuffer cb) {
    cbSecondary[cb] = true;
  }

  bool isSecondary(VkCommandBuffer cb) {
    return cbSecondary.count(cb) > 0;
  }

  VkQueryPool queryPool(VkCommandBuffer cb);
  VkQueue queryQueue(VkCommandBuffer cb);
  VkResult createQueryPools();

  uint32_t commandCount(VkCommandBuffer);
  float commandTime(VkCommandBuffer cb, uint32_t cmd_index);
  void writeCSV(FILE * f, VkCommandBuffer cb,
    uint32_t cmd_index, uint32_t cb_index, float cb_time);
  void writeAllCSV(const char *name);
};

extern ShimVkTraceResources aux;
extern int presentIndex;