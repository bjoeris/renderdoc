import rdtest
import renderdoc as rd


class VK_Separate_Depth_Stencil_Layouts(rdtest.TestCase):
    demos_test_name = 'VK_Separate_Depth_Stencil_Layouts'

    def check_image_subresource_layouts(self, name, img, layouts):
        if len(img.layouts) != len(layouts):
            raise rdtest.TestFailureException("{} image has {} subresource layouts ({} expected)".format(name, len(img.layouts), len(layouts)))
        for i in range(len(img.layouts)):
            layout = layouts[i]
            if img.layouts[i].name != layouts[i]:
                raise rdtest.TestFailureException("{} image layer {} is in {} layout ({} expected)".format(name, i, img.layouts[i].name, layouts[i]))


    def check_image_layouts(self, layouts):
        pipe: rd.VKState = self.controller.GetVulkanPipelineState()
        for img in pipe.images:
            img: rd.VKImageData
            res = self.get_resource(img.resourceId)
            if res.name.startswith("Image:"):
                name = res.name[len("Image:"):]
                if name in layouts:
                    self.check_image_subresource_layouts(name, img, layouts[name])


    def check_capture(self):
        layouts = {
            'DepthStencil': ['VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL', 'VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL'],
        }

        self.controller.SetFrameEvent(0, False)
        self.check_image_layouts(layouts)

        draw = self.find_draw("After Transition")
        self.check(draw is not None)
        self.controller.SetFrameEvent(draw.eventId, False)
        layouts['DepthStencil'] = ['VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL', 'VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL']
        self.check_image_layouts(layouts)

        draw = self.find_draw("Stencil only")
        self.check(draw is not None)
        self.controller.SetFrameEvent(draw.eventId, False)
        self.check_image_layouts(layouts)

        draw = self.find_draw("Depth write, stencil read")
        self.check(draw is not None)
        self.controller.SetFrameEvent(draw.eventId, False)
        layouts['DepthStencil'] = ['VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL', 'VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL']
        self.check_image_layouts(layouts)

        draw = self.find_draw("Depth input attachment")
        self.check(draw is not None)
        self.controller.SetFrameEvent(draw.eventId, False)
        layouts['DepthStencil'] = ['VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL', 'VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL']
        self.check_image_layouts(layouts)

        draw = self.find_draw("Depth/stencil read")
        self.check(draw is not None)
        self.controller.SetFrameEvent(draw.eventId, False)
        layouts['DepthStencil'] = ['VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL', 'VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL']
        self.check_image_layouts(layouts)


        draw = self.find_draw("After Render Pass")
        self.check(draw is not None)
        self.controller.SetFrameEvent(draw.eventId, False)
        self.check_image_layouts(layouts)
