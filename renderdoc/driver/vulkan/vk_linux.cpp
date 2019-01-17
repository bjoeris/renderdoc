/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2018 Baldur Karlsson
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

#include "api/replay/renderdoc_replay.h"

#include "vk_core.h"
#include "vk_replay.h"

#if defined(VK_USE_PLATFORM_XCB_KHR)

VkBool32 WrappedVulkan::vkGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                     uint32_t queueFamilyIndex,
                                                                     xcb_connection_t *connection,
                                                                     xcb_visualid_t visual_id)
{
  return ObjDisp(physicalDevice)
      ->GetPhysicalDeviceXcbPresentationSupportKHR(Unwrap(physicalDevice), queueFamilyIndex,
                                                   connection, visual_id);
}

namespace Keyboard
{
void UseConnection(xcb_connection_t *conn);
}

VkResult WrappedVulkan::vkCreateXcbSurfaceKHR(VkInstance instance,
                                              const VkXcbSurfaceCreateInfoKHR *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator,
                                              VkSurfaceKHR *pSurface)
{
  // should not come in here at all on replay
  RDCASSERT(IsCaptureMode(m_State));

  VkResult ret =
      ObjDisp(instance)->CreateXcbSurfaceKHR(Unwrap(instance), pCreateInfo, pAllocator, pSurface);

  if(ret == VK_SUCCESS)
  {
    GetResourceManager()->WrapResource(Unwrap(instance), *pSurface);

    WrappedVkSurfaceKHR *wrapped = GetWrapped(*pSurface);

    // since there's no point in allocating a full resource record and storing the window
    // handle under there somewhere, we just cast. We won't use the resource record for anything
    wrapped->record = (VkResourceRecord *)(uintptr_t)pCreateInfo->window;

    Keyboard::UseConnection(pCreateInfo->connection);
  }

  return ret;
}

#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)

VkBool32 WrappedVulkan::vkGetPhysicalDeviceXlibPresentationSupportKHR(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display *dpy, VisualID visualID)
{
  return ObjDisp(physicalDevice)
      ->GetPhysicalDeviceXlibPresentationSupportKHR(Unwrap(physicalDevice), queueFamilyIndex, dpy,
                                                    visualID);
}

namespace Keyboard
{
void CloneDisplay(Display *dpy);
}

VkResult WrappedVulkan::vkCreateXlibSurfaceKHR(VkInstance instance,
                                               const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator,
                                               VkSurfaceKHR *pSurface)
{
  // should not come in here at all on replay
  RDCASSERT(IsCaptureMode(m_State));

  VkResult ret =
      ObjDisp(instance)->CreateXlibSurfaceKHR(Unwrap(instance), pCreateInfo, pAllocator, pSurface);

  if(ret == VK_SUCCESS)
  {
    GetResourceManager()->WrapResource(Unwrap(instance), *pSurface);

    WrappedVkSurfaceKHR *wrapped = GetWrapped(*pSurface);

    // since there's no point in allocating a full resource record and storing the window
    // handle under there somewhere, we just cast. We won't use the resource record for anything
    wrapped->record = (VkResourceRecord *)pCreateInfo->window;

    Keyboard::CloneDisplay(pCreateInfo->dpy);
  }

  return ret;
}

VkResult WrappedVulkan::vkAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display *dpy,
                                                VkDisplayKHR display)
{
  // display is not wrapped so we can pass straight through
  return ObjDisp(physicalDevice)->AcquireXlibDisplayEXT(Unwrap(physicalDevice), dpy, display);
}

VkResult WrappedVulkan::vkGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display *dpy,
                                                   RROutput rrOutput, VkDisplayKHR *pDisplay)
{
  // display is not wrapped so we can pass straight through
  return ObjDisp(physicalDevice)
      ->GetRandROutputDisplayEXT(Unwrap(physicalDevice), dpy, rrOutput, pDisplay);
}

#endif

#if defined(VK_USE_PLATFORM_YETI_GOOGLE)

VkBool32 WrappedVulkan::vkGetPhysicalDeviceYetiPresentationSupportGOOGLE(
  VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
  int32_t streamIndex) {
  return ObjDisp(physicalDevice)
    ->GetPhysicalDeviceYetiPresentationSupportGOOGLE(
      Unwrap(physicalDevice), queueFamilyIndex, streamIndex);
}

VkResult WrappedVulkan::vkCreateYetiSurfaceGOOGLE(VkInstance instance,
  const VkYetiSurfaceCreateInfoGOOGLE *pCreateInfo,
  const VkAllocationCallbacks *pAllocator,
  VkSurfaceKHR *pSurface) {
  // should not come in here at all on replay
  RDCASSERT(IsCaptureMode(m_State));

  VkResult ret =
    ObjDisp(instance)->CreateYetiSurfaceGOOGLE(Unwrap(instance), pCreateInfo, pAllocator, pSurface);

  if(ret == VK_SUCCESS)
  {
    GetResourceManager()->WrapResource(Unwrap(instance), *pSurface);

    WrappedVkSurfaceKHR *wrapped = GetWrapped(*pSurface);

    // YETI: Copy the XCB hack and fudge our streamIndex into the record.  Add
    // one so that null checks still work for the streamIndex = 0 case.
    wrapped->record = (VkResourceRecord *) (uintptr_t) pCreateInfo->streamIndex + 1;
  }
  return ret;
}
#endif // defined(VK_USE_PLATFORM_YETI_GOOGLE)

#if defined(VK_USE_PLATFORM_GGP)

VkResult WrappedVulkan::vkCreateStreamDescriptorSurfaceGGP(
  const VkInstance                                  instance,
  const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
  const VkAllocationCallbacks*                pAllocator,
  VkSurfaceKHR*                               pSurface) {
  // should not come in here at all on replay
  RDCASSERT(IsCaptureMode(m_State));

  VkResult ret =
    ObjDisp(instance)->CreateStreamDescriptorSurfaceGGP(Unwrap(instance), pCreateInfo, pAllocator, pSurface);

  if(ret == VK_SUCCESS)
  {
    GetResourceManager()->WrapResource(Unwrap(instance), *pSurface);

    WrappedVkSurfaceKHR *wrapped = GetWrapped(*pSurface);

    // GGP: Copy the XCB hack and fudge our streamIndex into the record.  Add
    // one so that null checks still work for the streamIndex = 0 case.
    wrapped->record = (VkResourceRecord *) (uintptr_t) pCreateInfo->streamDescriptor;
  }
  return ret;
}
#endif // defined(VK_USE_PLATFORM_GGP)

void VulkanReplay::OutputWindow::SetWindowHandle(WindowingData window)
{
#if ENABLED(RDOC_XLIB)
  if(window.system == WindowingSystem::Xlib)
  {
    xlib.display = window.xlib.display;
    xlib.window = window.xlib.window;
    return;
  }
#endif

#if ENABLED(RDOC_XCB)
  if(window.system == WindowingSystem::XCB)
  {
    xcb.connection = window.xcb.connection;
    xcb.window = window.xcb.window;
    return;
  }
#endif

#if ENABLED(RDOC_GGP)
  RDCWARN("Window system is GGP, no OS handles for app window are needed");
  return; // there are no OS specific handles to save.
#endif

  RDCERR("Unrecognised/unsupported window system %d", window.system);
}

void VulkanReplay::OutputWindow::CreateSurface(VkInstance inst)
{
#if ENABLED(RDOC_XLIB)
  if(m_WindowSystem == WindowingSystem::Xlib)
  {
    VkXlibSurfaceCreateInfoKHR createInfo;

    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.dpy = xlib.display;
    createInfo.window = xlib.window;

    VkResult vkr = ObjDisp(inst)->CreateXlibSurfaceKHR(Unwrap(inst), &createInfo, NULL, &surface);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    return;
  }
#endif

#if ENABLED(RDOC_XCB)
  if(m_WindowSystem == WindowingSystem::XCB)
  {
    VkXcbSurfaceCreateInfoKHR createInfo;

    createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.connection = xcb.connection;
    createInfo.window = xcb.window;

    VkResult vkr = ObjDisp(inst)->CreateXcbSurfaceKHR(Unwrap(inst), &createInfo, NULL, &surface);
    RDCASSERTEQUAL(vkr, VK_SUCCESS);

    return;
  }
#endif

#if ENABLED(RDOC_GGP)
  VkStreamDescriptorSurfaceCreateInfoGGP createInfo;

  createInfo.sType = VK_STRUCTURE_TYPE_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP;
  createInfo.pNext = NULL;
  createInfo.streamDescriptor = 1;

  VkResult vkr = ObjDisp(inst)->CreateStreamDescriptorSurfaceGGP(Unwrap(inst), &createInfo, NULL, &surface);
  RDCASSERTEQUAL(vkr, VK_SUCCESS);

  return;
#endif // ENABLED(RDOC_GGP)

  RDCERR("Unrecognised/unsupported window system %d", m_WindowSystem);
}

void VulkanReplay::GetOutputWindowDimensions(uint64_t id, int32_t &w, int32_t &h)
{
  if(id == 0 || m_OutputWindows.find(id) == m_OutputWindows.end())
    return;

  OutputWindow &outw = m_OutputWindows[id];

#if ENABLED(RDOC_XLIB)
  if(outw.m_WindowSystem == WindowingSystem::Xlib)
  {
    XWindowAttributes attr = {};
    XGetWindowAttributes(outw.xlib.display, outw.xlib.window, &attr);

    w = (int32_t)attr.width;
    h = (int32_t)attr.height;

    return;
  }
#endif

#if ENABLED(RDOC_XCB)
  if(outw.m_WindowSystem == WindowingSystem::XCB)
  {
    xcb_get_geometry_cookie_t geomCookie =
        xcb_get_geometry(outw.xcb.connection, outw.xcb.window);    // window is a xcb_drawable_t
    xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(outw.xcb.connection, geomCookie, NULL);

    w = (int32_t)geom->width;
    h = (int32_t)geom->height;

    free(geom);

    return;
  }
#endif

#if ENABLED(RDOC_GGP)
  if(outw.m_WindowSystem == WindowingSystem::GGP)
  {
    RDCWARN("Window system is GGP (%d), size is %d, %d", outw.m_WindowSystem, outw.width, outw.height);
    // TODO(akharlamov) Can there be a better default resolution selection?
    // Because we don't have a window, outw dimensions can actually be (0,0) until the surface is created.
    // This will fail surface creation, so default to 1080p.
    w = outw.width != 0 ? outw.width : 1920;
    h = outw.height != 0 ? outw.height : 1080;
    return;
  }
#endif

  RDCERR("Unrecognised/unsupported window system %d", outw.m_WindowSystem);
}

void *LoadVulkanLibrary()
{
  return Process::LoadModule("libvulkan.so.1");
}
