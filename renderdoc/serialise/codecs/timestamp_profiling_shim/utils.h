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

  ExecuteCommandBuffer(VkCommandBuffer acb, uint32_t r = 0)
  {
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

struct PipelineStatsContext {
  bool isGraphics = false;
  uint32_t stats = 1;
  VkQueryPool pool = NULL;
};

struct AllPipelineStats {
  uint64_t inputAssemblyVertices;
  uint64_t inputAssemblyPrimitive;
  uint64_t vertexInvocations;
  uint64_t geometryInvocations;
  uint64_t geometryPrimitives;
  uint64_t clippingInvocations;
  uint64_t clippingPrimitives;
  uint64_t fragmentInvocations;
  uint64_t controlInvocations;
  uint64_t evaluationInvocations;
  uint64_t computeInvocations;
};

struct AllComputePipelineStats {
  uint64_t computeInvocations;
};

struct CommandInfo
{
  std::string name;
  std::string info;
  uint32_t timestamps = 2;
  bool isInlinedSubpass = true;
  bool hasStats = false;
  AllPipelineStats stats = {};

  CommandInfo(std::string n) { name = n; }
  CommandInfo(std::string n, std::string i)
  {
    name = n;
    info = i;
  }
  CommandInfo(std::string n, std::string i, uint32_t ts)
  {
    name = n;
    info = i;
    timestamps = ts;
  }
  CommandInfo(std::string n, std::string i, uint32_t ts, bool inl)
  {
    name = n;
    info = i;
    timestamps = ts;
    isInlinedSubpass = inl;
  }
};

typedef std::array<uint32_t, 3> TripleU32;
typedef std::vector<std::array<double, 2>> TupleVec;
typedef std::vector<CommandInfo> CommandInfoVec;

struct CommandInfoDesc
{
  CommandInfoVec vec;
  bool isInlinedSubpass = false;
  uint32_t pipelineQueryCount = 0;
  uint32_t executeCommands = 0;
};
typedef std::vector<ExecuteCommandBuffer> ExecCmdBufVec;

struct ShimVkTraceResources : public AuxVkTraceResources
{
  std::map<VkQueue, ExecCmdBufVec> cbSubmitOrder;
  std::map<VkCommandBuffer, CommandInfoDesc> cbCommandInfo;
  std::map<VkCommandBuffer, TupleVec> cbAccumTimestamps;
  std::map<VkCommandBuffer, TripleU32> cbTimestampQueryRange;
  std::map<VkCommandBuffer, TripleU32> cbPipelineStatQueryRange;
  std::map<VkCommandBuffer, ExecCmdBufVec> cbExecCmdBufs;
  std::map<VkCommandBuffer, TimestampContext> cbTimestampContext;
  std::map<VkCommandBuffer, PipelineStatsContext> cbPipelineStatsContext;
  std::map<VkCommandBuffer, bool> cbSecondary;

  uint32_t timestampReportCommandCount(VkCommandBuffer);
  uint32_t timestampCollectCommandCount(VkCommandBuffer cb);
  uint32_t timestampQueryCount(VkCommandBuffer cb);
  uint32_t timestampQueryOffset(VkCommandBuffer cb);
  uint32_t timestampQueryInc(VkCommandBuffer cb);
  uint32_t resetTimestampQueries(VkCommandBuffer cb);
  VkQueryPool timestampQueryPool(VkCommandBuffer cb);

  uint32_t pipelinestatsQueryCount(VkCommandBuffer cb);
  uint32_t pipelinestatsQueryUnitsCount(VkCommandBuffer cb);
  uint32_t pipelinestatsQueryOffset(VkCommandBuffer cb);
  uint32_t pipelinestatsQueryInc(VkCommandBuffer cb);
  uint32_t resetPipelinestatsQueries(VkCommandBuffer cb);
  VkQueryPool pipelinestatsQueryPool(VkCommandBuffer cb);

  bool isPresent(VkCommandBuffer cb);

  void addCommandInfo(VkCommandBuffer cb, const CommandInfo &info);
  bool isInline(VkCommandBuffer cb);
  void isInline(VkCommandBuffer cb, bool current);
  void secondaryCB(VkCommandBuffer cb);
  bool isSecondary(VkCommandBuffer cb);

  void queueSubmit(VkQueue queue, uint32_t cbCount, VkCommandBuffer *cbList);
  void executeCommands(VkCommandBuffer cb, VkCommandBuffer exec, uint32_t offset);

  float commandTime(VkCommandBuffer cb, uint32_t cmd_index);
  double accumTimestamps(VkCommandBuffer cb, const std::vector<uint64_t> &data, uint64_t frameID,
                         VkQueue queue);
  void accumulateAllTimestamps(VkCommandBuffer cb, uint64_t frameID);
  void fetchPipelineStats(uint64_t frameID);

  VkResult createQueryPools();

  void writeCSV(FILE * f, VkCommandBuffer cb, const char * cb_name,
    uint32_t cb_index, float cb_time, uint32_t cmd_index);
  void writeAllCSV(const char *name);
};

extern ShimVkTraceResources aux;
extern int presentIndex;
