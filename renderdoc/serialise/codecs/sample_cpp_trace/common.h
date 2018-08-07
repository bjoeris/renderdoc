#pragma once

#include <assert.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include "helper/helper.h"
#include "sample_cpp_shim/shim_vulkan.h"

std::string PostStageProgress(const char *stage, uint32_t i, uint32_t N);