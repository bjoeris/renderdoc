import rdtest
import renderdoc as rd


class VK_Image_Queue_Families(rdtest.TestCase):
    demos_test_name = 'VK_Image_Queue_Families'

    def check_image_subresource_states(self, name, img, states):
        if len(img.layouts) != len(states):
            raise rdtest.TestFailureException("{} image has {} subresource states (3 expected)".format(name, len(img.layouts)))
        for i in range(len(img.layouts)):
            layout = states[i]['layout']
            queueFamily = states[i]['queueFamily']
            srcQueueFamily = states[i].get('srcQueueFamily', 4294967295)
            srcLayout = states[i].get('srcLayout', '')
            if img.layouts[i].name != layout:
                raise rdtest.TestFailureException("{} image layer {} is in {} layout ({} expected)".format(name, i, img.layouts[i].name, layout))
            if img.layouts[i].queueFamily != queueFamily:
                raise rdtest.TestFailureException("{} image layer {} is owned by queue family {} ({} expected)".format(name, i, img.layouts[i].queueFamily, queueFamily))
            if img.layouts[i].srcQueueFamily != srcQueueFamily:
                raise rdtest.TestFailureException("{} image layer {} is transfering from queue family {} ({} expected)".format(name, i, img.layouts[i].srcQueueFamily, srcQueueFamily))
            if img.layouts[i].srcLayout != srcLayout:
                raise rdtest.TestFailureException("{} image layer {} is transfering from layout {} ({} expected)".format(name, i, img.layouts[i].srcLayout, srcLayout))
                


    def check_image_states(self, states):
        pipe: rd.VKState = self.controller.GetVulkanPipelineState()
        for img in pipe.images:
            img: rd.VKImageData
            res = self.get_resource(img.resourceId)
            if res.name.startswith("Image:"):
                name = res.name[len("Image:"):]
                if name in states:
                    self.check_image_subresource_states(name, img, states[name])


    def check_capture(self):
        states = {
            'Render': [
                {
                    'layout': 'VK_IMAGE_LAYOUT_GENERAL',
                    'queueFamily': 1,
                    'srcLayout': 'VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL',
                    'srcQueueFamily': 0,
                },
                {
                    'layout': 'VK_IMAGE_LAYOUT_GENERAL',
                    'queueFamily': 1,
                },
                {
                    'layout': 'VK_IMAGE_LAYOUT_GENERAL',
                    'queueFamily': 1,
                },
            ],
            'PostProcess': [
                {
                    'layout': 'VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL',
                    'queueFamily': 0,
                },
                {
                    'layout': 'VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL',
                    'queueFamily': 0,
                },
                {
                    'layout': 'VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL',
                    'queueFamily': 0,
                    'srcLayout': 'VK_IMAGE_LAYOUT_GENERAL',
                    'srcQueueFamily': 1,
                },
            ],
        }

        self.controller.SetFrameEvent(0, False)
        self.check_image_states(states)

        draw = self.find_draw("After Pre-Render Barrier 1")
        self.check(draw is not None)
        self.controller.SetFrameEvent(draw.eventId, False)
        states['Render'][1] = {
            'layout': 'VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL',
            'queueFamily': 0,
        }
        self.check_image_states(states)

        draw = self.find_draw("After Post-Render Barrier 1")
        self.check(draw is not None)
        self.controller.SetFrameEvent(draw.eventId, False)
        states['Render'][1] = {
            'layout': 'VK_IMAGE_LAYOUT_GENERAL',
            'queueFamily': 1,
            'srcLayout': 'VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL',
            'srcQueueFamily': 0,
        }
        self.check_image_states(states)

        draw = self.find_draw("After Post-Process Image Acquired 0")
        self.check(draw is not None)
        self.controller.SetFrameEvent(draw.eventId, False)
        states['Render'][0] = {
            'layout': 'VK_IMAGE_LAYOUT_GENERAL',
            'queueFamily': 1,
        }
        self.check_image_states(states)

        draw = self.find_draw("After Post-Process Pre-Dispatch Barrier 0")
        self.check(draw is not None)
        self.controller.SetFrameEvent(draw.eventId, False)
        states['PostProcess'][0] = {
            'layout': 'VK_IMAGE_LAYOUT_GENERAL',
            'queueFamily': 1,
        }
        self.check_image_states(states)

        draw = self.find_draw("After Post-Process Image Released 0")
        self.check(draw is not None)
        self.controller.SetFrameEvent(draw.eventId, False)
        states['PostProcess'][0] = {
            'layout': 'VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL',
            'queueFamily': 0,
            'srcLayout': 'VK_IMAGE_LAYOUT_GENERAL',
            'srcQueueFamily': 1,
        }
        self.check_image_states(states)
