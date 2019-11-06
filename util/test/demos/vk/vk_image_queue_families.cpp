/******************************************************************************
* The MIT License (MIT)
*
* Copyright (c) 2018-2019 Baldur Karlsson
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

#include <cassert>
#include "vk_test.h"

RD_TEST(VK_Image_Queue_Families, VulkanGraphicsTest)
{
  static constexpr const char *Description =
      "Tests edge-cases of image queue family ownership transfers, including implicit ownership "
      "transfers.";

  std::string common = R"EOSHADER(

#version 450

struct v2f
{
	vec4 pos;
	vec4 col;
	vec4 uv;
};

)EOSHADER";

  const std::string vertex = R"EOSHADER(

layout(location = 0) in vec3 Position;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec2 UV;

layout(location = 0) out v2f vertOut;

void main()
{
	vertOut.pos = vec4(Position.xyz*vec3(1,-1,1), 1);
	gl_Position = vertOut.pos;
	vertOut.col = Color;
	vertOut.uv = vec4(UV.xy, 0, 1);
}

)EOSHADER";

  const std::string pixel = R"EOSHADER(

layout(location = 0) in v2f vertIn;

layout(location = 0, index = 0) out vec4 Color;

void main()
{
	Color = vertIn.col;
}

)EOSHADER";

  const std::string compute = R"EOSHADER(

layout(binding = 0, rgba8) uniform readonly image2D inputImage;
layout(binding = 1, rgba8) uniform image2D resultImage;

void main()
{
  ivec2 pos = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
  vec3 rgb = imageLoad(inputImage, pos).rgb;
  vec4 res = vec4(1.0 - rgb, 1.0);
  imageStore(resultImage, pos, res);
}

)EOSHADER";

  struct QueueFamilyImageOwnershipTransfer
  {
    enum class State
    {
      UNINITIALIZED,
      RELEASED,
      ACQUIRED,
    };
    State state = State::UNINITIALIZED;
    VkImage image;
    VkImageSubresourceRange range;
    uint32_t srcQueueFamilyIndex;
    uint32_t dstQueueFamilyIndex;
    VkImageLayout oldLayout;
    VkImageLayout newLayout;
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;
    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;

    // semaphore that ensures the `release` barrier is executed before the `acquire` barrier
    VkSemaphore transferSemaphore = VK_NULL_HANDLE;
    // semaphore that ensures `dstQueueFamilyIndex` has finished processing the image, and that
    // `srcQueueFamilyIndex` can implicitly re-acquire the image.
    VkSemaphore returnSemaphore = VK_NULL_HANDLE;

    QueueFamilyImageOwnershipTransfer(VK_Image_Queue_Families &test, VkImage image,
                                      uint32_t arrayLayer, uint32_t srcQueueFamilyIndex,
                                      uint32_t dstQueueFamilyIndex, VkImageLayout oldLayout,
                                      VkImageLayout newLayout, VkAccessFlags srcAccessMask,
                                      VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStageMask,
                                      VkPipelineStageFlags dstStageMask)
        : image(image),
          srcQueueFamilyIndex(srcQueueFamilyIndex),
          dstQueueFamilyIndex(dstQueueFamilyIndex),
          oldLayout(oldLayout),
          newLayout(newLayout),
          srcAccessMask(srcAccessMask),
          dstAccessMask(dstAccessMask),
          srcStageMask(srcStageMask),
          dstStageMask(dstStageMask)
    {
      range = vkh::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, arrayLayer, 1u);
      vkCreateSemaphore(test.device, vkh::SemaphoreCreateInfo(), NULL, &transferSemaphore);
      vkCreateSemaphore(test.device, vkh::SemaphoreCreateInfo(), NULL, &returnSemaphore);
    }

    void acquireOwnership(VkCommandBuffer cmds)
    {
      uint32_t actualSrcQueueFamilyIndex = srcQueueFamilyIndex;
      VkAccessFlags actualSrcAccessMask = srcAccessMask;
      VkPipelineStageFlags actualSrcStageMask = srcStageMask;
      VkImageLayout actualOldLayout = oldLayout;
      if(state == State::UNINITIALIZED)
      {
        actualSrcQueueFamilyIndex = dstQueueFamilyIndex;
        actualOldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      }
      else if(actualSrcQueueFamilyIndex == dstQueueFamilyIndex)
      {
        actualOldLayout = newLayout;
      }
      if(srcQueueFamilyIndex != dstQueueFamilyIndex)
      {
        actualSrcAccessMask = 0u;
        actualSrcStageMask = dstStageMask;
      }
      vkh::cmdPipelineBarrier(
          cmds,
          {
              vkh::ImageMemoryBarrier(actualSrcAccessMask, dstAccessMask, actualOldLayout, newLayout,
                                      image, range, actualSrcQueueFamilyIndex, dstQueueFamilyIndex),
          },
          {}, {}, actualSrcStageMask, dstStageMask, 0u);
      state = State::ACQUIRED;
    }
    void releaseOwnership(VkCommandBuffer cmds)
    {
      VkAccessFlags actualDstAccessMask = dstAccessMask;
      VkPipelineStageFlags actualDstStageMask = dstStageMask;
      if(srcQueueFamilyIndex != dstQueueFamilyIndex)
      {
        actualDstAccessMask = 0u;
        actualDstStageMask = srcStageMask;
      }
      vkh::cmdPipelineBarrier(
          cmds,
          {
              vkh::ImageMemoryBarrier(srcAccessMask, actualDstAccessMask, oldLayout, newLayout,
                                      image, range, srcQueueFamilyIndex, dstQueueFamilyIndex),
          },
          {}, {}, srcStageMask, actualDstStageMask, 0u);
      state = State::RELEASED;
    }
  };

  struct FrameData
  {
    VK_Image_Queue_Families *test;
    uint32_t queueFamilyIndex;
    VkQueue queue;
    VkCommandPool cmdPool;
    VkCommandBuffer cmds;
    VkFence fence;

    FrameData(VK_Image_Queue_Families &test, uint32_t queueFamilyIndex)
        : test(&test), queueFamilyIndex(queueFamilyIndex)
    {
      vkGetDeviceQueue(test.device, queueFamilyIndex, 0, &queue);
      CHECK_VKR(vkCreateCommandPool(
          test.device, vkh::CommandPoolCreateInfo(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                                  queueFamilyIndex),
          NULL, &cmdPool));
      CHECK_VKR(
          vkAllocateCommandBuffers(test.device, vkh::CommandBufferAllocateInfo(cmdPool, 1), &cmds));

      vkh::FenceCreateInfo fenceCreateInfo;
      fenceCreateInfo.flags |= VK_FENCE_CREATE_SIGNALED_BIT;
      vkCreateFence(test.device, fenceCreateInfo, NULL, &fence);
    }

    void BeginCommandBuffer()
    {
      vkWaitForFences(test->device, 1, &fence, VK_TRUE, UINT64_MAX);
      vkResetFences(test->device, 1, &fence);
      vkBeginCommandBuffer(cmds, vkh::CommandBufferBeginInfo());
    }

    void Submit(const std::vector<std::pair<VkSemaphore, VkPipelineStageFlags>> &waitSemaphores,
                const std::vector<VkSemaphore> &signalSemaphores)
    {
      vkQueueSubmit(queue, 1, vkh::SubmitInfo({cmds}, waitSemaphores, signalSemaphores), fence);
    }
  };

  struct RenderFrameData : public FrameData
  {
    VkImage image;
    VkImageSubresourceRange range;
    VkImageView imageView;
    VkRenderPass renderPass;
    VkFramebuffer framebuffer;
    VkPipelineLayout pipeLayout;
    VkPipeline pipe;
    VkBuffer vertexBuffer;
    QueueFamilyImageOwnershipTransfer *imageOwnership;

    RenderFrameData(VK_Image_Queue_Families &test, uint32_t graphicsQueueFamilyIndex,
                    VkFormat format, VkImage image, uint32_t arrayLayer, VkBuffer vertexBuffer,
                    QueueFamilyImageOwnershipTransfer &imageOwnership)
        : FrameData(test, graphicsQueueFamilyIndex),
          image(image),
          vertexBuffer(vertexBuffer),
          imageOwnership(&imageOwnership)
    {
      range = vkh::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, arrayLayer, 1u);
      vkh::ImageViewCreateInfo viewCreateInfo(image, VK_IMAGE_VIEW_TYPE_2D, format);
      viewCreateInfo.subresourceRange = range;

      pipeLayout = test.createPipelineLayout(vkh::PipelineLayoutCreateInfo());

      vkh::RenderPassCreator renderPassCreateInfo;
      renderPassCreateInfo.attachments.push_back(vkh::AttachmentDescription(
          VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR,
          VK_ATTACHMENT_STORE_OP_STORE));
      renderPassCreateInfo.addSubpass(
          {VkAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL})});
      renderPass = test.createRenderPass(renderPassCreateInfo);

      vkh::GraphicsPipelineCreateInfo pipeCreateInfo;
      pipeCreateInfo.layout = pipeLayout;
      pipeCreateInfo.renderPass = renderPass;
      pipeCreateInfo.vertexInputState.vertexBindingDescriptions = {vkh::vertexBind(0, DefaultA2V)};
      pipeCreateInfo.vertexInputState.vertexAttributeDescriptions = {
          vkh::vertexAttr(0, 0, DefaultA2V, pos), vkh::vertexAttr(1, 0, DefaultA2V, col),
          vkh::vertexAttr(2, 0, DefaultA2V, uv),
      };
      pipeCreateInfo.stages = {
          test.CompileShaderModule(test.common + test.vertex, ShaderLang::glsl, ShaderStage::vert,
                                   "main"),
          test.CompileShaderModule(test.common + test.pixel, ShaderLang::glsl, ShaderStage::frag,
                                   "main"),
      };
      pipe = test.createGraphicsPipeline(pipeCreateInfo);

      imageView = test.createImageView(viewCreateInfo);
      framebuffer = test.createFramebuffer(
          vkh::FramebufferCreateInfo(renderPass, {imageView}, test.mainWindow->scissor.extent));
    }

    void Run()
    {
      std::vector<std::pair<VkSemaphore, VkPipelineStageFlags>> waitSemaphores;
      if(imageOwnership->state != QueueFamilyImageOwnershipTransfer::State::UNINITIALIZED)
        waitSemaphores.push_back(
            {imageOwnership->returnSemaphore, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT});
      std::vector<VkSemaphore> signalSemaphores = {imageOwnership->transferSemaphore};

      RecordCommandBuffer();
      Submit(waitSemaphores, signalSemaphores);
    }

    void RecordCommandBuffer()
    {
      BeginCommandBuffer();
      vkh::cmdPipelineBarrier(
          cmds,
          {
              vkh::ImageMemoryBarrier(0u, 0u, VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, image, range),
          },
          {}, {}, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0u);

      vkCmdBeginRenderPass(
          cmds, vkh::RenderPassBeginInfo(renderPass, framebuffer, test->mainWindow->scissor,
                                         {vkh::ClearValue(0.4f, 0.5f, 0.6f, 1.0f)}),
          VK_SUBPASS_CONTENTS_INLINE);

      vkCmdBindPipeline(cmds, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
      vkCmdSetViewport(cmds, 0, 1, &test->mainWindow->viewport);
      vkCmdSetScissor(cmds, 0, 1, &test->mainWindow->scissor);
      vkh::cmdBindVertexBuffers(cmds, 0, {vertexBuffer}, {0});
      vkCmdDraw(cmds, 3, 1, 0, 0);
      vkCmdEndRenderPass(cmds);

      imageOwnership->releaseOwnership(cmds);

      vkEndCommandBuffer(cmds);
    }
  };

  struct PostProcessFrameData : public FrameData
  {
    VkImage inputImage;
    VkImage outputImage;
    VkImageView inputImageView;
    VkImageView outputImageView;
    VkImageSubresourceRange range;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;
    VkPipelineLayout pipeLayout;
    VkPipeline pipe;
    QueueFamilyImageOwnershipTransfer *inputImageOwnership;
    QueueFamilyImageOwnershipTransfer *outputImageOwnership;

    PostProcessFrameData(VK_Image_Queue_Families &test, uint32_t computeQueueFamilyIndex,
                         VkFormat format, VkImage inputImage, VkImage outputImage,
                         uint32_t arrayLayer, QueueFamilyImageOwnershipTransfer &inputImageOwnership,
                         QueueFamilyImageOwnershipTransfer &outputImageOwnership)
        : FrameData(test, computeQueueFamilyIndex),
          inputImage(inputImage),
          outputImage(outputImage),
          inputImageOwnership(&inputImageOwnership),
          outputImageOwnership(&outputImageOwnership)
    {
      range = vkh::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, arrayLayer, 1u);

      std::vector<VkDescriptorSetLayoutBinding> bindings = {
          // Binding 0: Input image (read-only)
          {
              /* binding = */ 0,
              /* descriptorType = */ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
              /* descriptorCount = */ 1,
              /* stageFlags = */ VK_SHADER_STAGE_COMPUTE_BIT,
              /* pImmutableSamplers = */ NULL,
          },
          // Binding 1: Output image (write)
          {
              /* binding = */ 1,
              /* descriptorType = */ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
              /* descriptorCount = */ 1,
              /* stageFlags = */ VK_SHADER_STAGE_COMPUTE_BIT,
              /* pImmutableSamplers = */ NULL,
          },
      };
      descriptorSetLayout =
          test.createDescriptorSetLayout(vkh::DescriptorSetLayoutCreateInfo(bindings));
      pipeLayout = test.createPipelineLayout(vkh::PipelineLayoutCreateInfo({descriptorSetLayout}));
      pipe = test.createComputePipeline(vkh::ComputePipelineCreateInfo(
          pipeLayout, test.CompileShaderModule(test.common + test.compute, ShaderLang::glsl,
                                               ShaderStage::comp, "main")));
      descriptorSet = test.allocateDescriptorSet(descriptorSetLayout);

      vkh::ImageViewCreateInfo imageViewCreateInfo(inputImage, VK_IMAGE_VIEW_TYPE_2D, format);
      imageViewCreateInfo.subresourceRange = range;
      inputImageView = test.createImageView(imageViewCreateInfo);
      imageViewCreateInfo.image = outputImage;
      outputImageView = test.createImageView(imageViewCreateInfo);

      std::vector<std::vector<VkDescriptorImageInfo>> imageInfos = {
          {vkh::DescriptorImageInfo(inputImageView, VK_IMAGE_LAYOUT_GENERAL)},
          {vkh::DescriptorImageInfo(outputImageView, VK_IMAGE_LAYOUT_GENERAL)},
      };
      std::vector<vkh::WriteDescriptorSet> descWrites = {
          vkh::WriteDescriptorSet(descriptorSet, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                  imageInfos[0]),
          vkh::WriteDescriptorSet(descriptorSet, 1, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                  imageInfos[1]),
      };
      vkUpdateDescriptorSets(test.device, (uint32_t)descWrites.size(), descWrites.data(), 0, NULL);
    }

    void Run()
    {
      std::vector<std::pair<VkSemaphore, VkPipelineStageFlags>> waitSemaphores;
      if(inputImageOwnership->state != QueueFamilyImageOwnershipTransfer::State::UNINITIALIZED)
        waitSemaphores.push_back(
            {inputImageOwnership->transferSemaphore, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT});
      if(outputImageOwnership->state != QueueFamilyImageOwnershipTransfer::State::UNINITIALIZED)
        waitSemaphores.push_back(
            {outputImageOwnership->returnSemaphore, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT});
      std::vector<VkSemaphore> signalSemaphores = {inputImageOwnership->returnSemaphore,
                                                   outputImageOwnership->transferSemaphore};

      RecordCommandBuffer();
      Submit(waitSemaphores, signalSemaphores);
    }

    void RecordCommandBuffer()
    {
      BeginCommandBuffer();

      inputImageOwnership->acquireOwnership(cmds);

      vkh::cmdPipelineBarrier(
          cmds,
          {
              vkh::ImageMemoryBarrier(0u, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_GENERAL, outputImage, range),
          },
          {}, {}, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0u);

      vkCmdBindPipeline(cmds, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
      vkCmdBindDescriptorSets(cmds, VK_PIPELINE_BIND_POINT_COMPUTE, pipeLayout, 0, 1,
                              &descriptorSet, 0, NULL);
      VkExtent2D &extent = test->mainWindow->scissor.extent;
      vkCmdDispatch(cmds, extent.width, extent.height, 1u);

      outputImageOwnership->releaseOwnership(cmds);

      vkEndCommandBuffer(cmds);
    }
  };

  struct PresentFrameData : public FrameData
  {
    VkImage image;
    VkImageSubresourceRange range;
    QueueFamilyImageOwnershipTransfer *imageOwnership;

    PresentFrameData(VK_Image_Queue_Families &test, uint32_t graphicsQueueFamilyIndex, VkImage image,
                     uint32_t arrayLayer, QueueFamilyImageOwnershipTransfer &imageOwnership)
        : FrameData(test, graphicsQueueFamilyIndex), image(image), imageOwnership(&imageOwnership)
    {
      range = vkh::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, arrayLayer, 1u);
    }

    void Run()
    {
      std::vector<std::pair<VkSemaphore, VkPipelineStageFlags>> waitSemaphores;
      if(imageOwnership->state != QueueFamilyImageOwnershipTransfer::State::UNINITIALIZED)
        waitSemaphores.push_back(
            {imageOwnership->transferSemaphore, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT});
      std::vector<VkSemaphore> signalSemaphores = {imageOwnership->returnSemaphore};

      RecordCommandBuffer();
      test->Submit(0, 1, {cmds}, {}, NULL, VK_NULL_HANDLE, waitSemaphores, signalSemaphores);
      test->Present();
    }

    void RecordCommandBuffer()
    {
      cmds = test->GetCommandBuffer();
      vkBeginCommandBuffer(cmds, vkh::CommandBufferBeginInfo());

      imageOwnership->acquireOwnership(cmds);

      VkImage swapimg = test->StartUsingBackbuffer(cmds, VK_ACCESS_TRANSFER_WRITE_BIT,
                                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
      VkImageBlit blit = {};
      blit.srcSubresource = {
          /* aspectMask = */ VK_IMAGE_ASPECT_COLOR_BIT,
          /* mipLevel = */ 0u,
          /* baseArrayLayer = */ range.baseArrayLayer,
          /* layerCount = */ 1u,
      };
      blit.dstSubresource = {
          /* aspectMask = */ VK_IMAGE_ASPECT_COLOR_BIT,
          /* mipLevel = */ 0u,
          /* baseArrayLayer = */ 0u,
          /* layerCount = */ 1u,
      };
      VkExtent2D extent = test->mainWindow->scissor.extent;
      blit.srcOffsets[1] = {
          /* x = */ (int32_t)extent.width,
          /* y = */ (int32_t)extent.height,
          /* z = */ 1,
      };
      blit.dstOffsets[1] = blit.srcOffsets[1];
      vkCmdBlitImage(cmds, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapimg,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_NEAREST);

      test->FinishUsingBackbuffer(cmds, VK_ACCESS_TRANSFER_WRITE_BIT,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

      vkEndCommandBuffer(cmds);
    }
  };

  int main()
  {
    std::vector<VkQueueFamilyProperties> queueProps;
    vkh::getQueueFamilyProperties(queueProps, phys);
    std::vector<vkh::DeviceQueueCreateInfo> requestedComputeQueues;

    for(uint32_t q = 0; q < queueProps.size(); q++)
    {
      if(queueProps[q].queueFlags & VK_QUEUE_COMPUTE_BIT)
        requestedComputeQueues.push_back(vkh::DeviceQueueCreateInfo(q, 1));
    }

    // initialise, create window, create context, etc
    if(!Init(requestedComputeQueues))
      return 3;

    uint32_t computeQueueFamilyIndex = ~0u;
    for(auto it = requestedComputeQueues.begin(); it != requestedComputeQueues.end(); ++it)
    {
      if(it->queueFamilyIndex != queueFamilyIndex)
      {
        computeQueueFamilyIndex = it->queueFamilyIndex;
      }
    }
    uint32_t graphicsQueueFamilyIndex = queueFamilyIndex;

    AllocatedBuffer vb(
        this, vkh::BufferCreateInfo(sizeof(DefaultTri), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                                            VK_BUFFER_USAGE_TRANSFER_DST_BIT),
        VmaAllocationCreateInfo({0, VMA_MEMORY_USAGE_CPU_TO_GPU}));

    vb.upload(DefaultTri);

    const uint32_t imageCount = 3;

    AllocatedImage renderImage(
        this, vkh::ImageCreateInfo(mainWindow->scissor.extent.width,
                                   mainWindow->scissor.extent.height, 0, VK_FORMAT_R8G8B8A8_UNORM,
                                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                                   1u, imageCount),
        VmaAllocationCreateInfo({0, VMA_MEMORY_USAGE_GPU_ONLY}));
    AllocatedImage postProcessImage(
        this, vkh::ImageCreateInfo(mainWindow->scissor.extent.width,
                                   mainWindow->scissor.extent.height, 0, VK_FORMAT_R8G8B8A8_UNORM,
                                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, 1u,
                                   imageCount),
        VmaAllocationCreateInfo({0, VMA_MEMORY_USAGE_GPU_ONLY}));

    std::vector<QueueFamilyImageOwnershipTransfer> renderImageToCompute;
    std::vector<QueueFamilyImageOwnershipTransfer> postProcessImageToGraphics;
    for(uint32_t i = 0; i < imageCount; ++i)
    {
      renderImageToCompute.push_back(QueueFamilyImageOwnershipTransfer(
          *this, renderImage.image, i, graphicsQueueFamilyIndex, computeQueueFamilyIndex,
          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
          VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT));

      postProcessImageToGraphics.push_back(QueueFamilyImageOwnershipTransfer(
          *this, postProcessImage.image, i, computeQueueFamilyIndex, graphicsQueueFamilyIndex,
          VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_SHADER_WRITE_BIT,
          VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
          VK_PIPELINE_STAGE_TRANSFER_BIT));
    }

    std::vector<RenderFrameData> renderFrameData;
    std::vector<PostProcessFrameData> postProcessFrameData;
    std::vector<PresentFrameData> presentFrameData;

    for(uint32_t i = 0; i < imageCount; ++i)
    {
      renderFrameData.push_back(RenderFrameData(*this, graphicsQueueFamilyIndex,
                                                VK_FORMAT_R8G8B8A8_UNORM, renderImage.image, i,
                                                vb.buffer, renderImageToCompute[i]));
      postProcessFrameData.push_back(PostProcessFrameData(
          *this, computeQueueFamilyIndex, VK_FORMAT_R8G8B8A8_UNORM, renderImage.image,
          postProcessImage.image, i, renderImageToCompute[i], postProcessImageToGraphics[i]));
      presentFrameData.push_back(PresentFrameData(
          *this, graphicsQueueFamilyIndex, postProcessImage.image, i, postProcessImageToGraphics[i]));
    }

    uint32_t imageIndex = 0u;
    while(Running())
    {
      uint32_t presentImageIndex = imageIndex;
      uint32_t postProcessImageIndex = (imageIndex + 1) % imageCount;
      uint32_t renderImageIndex = (imageIndex + 2) % imageCount;

      renderFrameData[renderImageIndex].Run();
      postProcessFrameData[postProcessImageIndex].Run();
      presentFrameData[presentImageIndex].Run();

      imageIndex = (imageIndex + 1) % imageCount;
    }

    return 0;
  }
};

REGISTER_TEST();
