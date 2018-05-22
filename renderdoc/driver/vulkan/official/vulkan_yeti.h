#ifndef VULKAN_YETI_H_
#define VULKAN_YETI_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

/*
** Copyright (c) 2015-2018 The Khronos Group Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

/*
** This header is generated from the Khronos Vulkan XML API Registry.
**
*/


#define VK_GOOGLE_yeti_surface 1
#define VK_GOOGLE_YETI_SURFACE_SPEC_VERSION 3
#define VK_GOOGLE_YETI_SURFACE_EXTENSION_NAME "VK_GOOGLE_yeti_surface"

typedef struct VkYetiSurfaceCreateInfoGOOGLE {
    VkStructureType    sType;
    const void*        pNext;
    int32_t            streamIndex;
} VkYetiSurfaceCreateInfoGOOGLE;

typedef struct VkPresentYetiFrameTokenGOOGLE {
    VkStructureType    sType;
    const void*        pNext;
    uint64_t           frameToken;
} VkPresentYetiFrameTokenGOOGLE;


typedef VkResult (VKAPI_PTR *PFN_vkCreateYetiSurfaceGOOGLE)(VkInstance instance, const VkYetiSurfaceCreateInfoGOOGLE* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
typedef VkBool32 (VKAPI_PTR *PFN_vkGetPhysicalDeviceYetiPresentationSupportGOOGLE)(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, int32_t streamIndex);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR VkResult VKAPI_CALL vkCreateYetiSurfaceGOOGLE(
    VkInstance                                  instance,
    const VkYetiSurfaceCreateInfoGOOGLE*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface);

VKAPI_ATTR VkBool32 VKAPI_CALL vkGetPhysicalDeviceYetiPresentationSupportGOOGLE(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    int32_t                                     streamIndex);
#endif

#ifdef __cplusplus
}
#endif

#endif
