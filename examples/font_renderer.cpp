#include <vector>

import dreamrender;
import spdlog;
import vulkan_hpp;

class simple_phase : public dreamrender::phase {
    public:
        simple_phase(dreamrender::window* win) : dreamrender::phase(win),
            fontRenderer{"/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 64, device, allocator, win->swapchainExtent, win->gpuFeatures}
        {}

        vk::UniqueRenderPass renderPass;
        std::vector<vk::UniqueFramebuffer> framebuffers;

        dreamrender::font_renderer fontRenderer;

        void preload() override {
            phase::preload();

            vk::AttachmentDescription attachment{{}, win->swapchainFormat.format, win->config.sampleCount,
                vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                vk::ImageLayout::eUndefined, win->swapchainFinalLayout};
            vk::AttachmentReference ref(0, vk::ImageLayout::eColorAttachmentOptimal);
            vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {}, ref);
            vk::SubpassDependency dependency(vk::SubpassExternal, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, vk::AccessFlagBits::eColorAttachmentWrite, {});

            renderPass = device.createRenderPassUnique(vk::RenderPassCreateInfo({}, attachment, subpass, dependency));

            add_task(fontRenderer.preload(loader, {renderPass.get()}, win->config.sampleCount));
        }
        void prepare(std::vector<vk::Image> swapchainImages, std::vector<vk::ImageView> swapchainViews) override {
            phase::prepare(swapchainImages, swapchainViews);

            framebuffers = createFramebuffers(renderPass.get());

            fontRenderer.prepare(swapchainImages.size());
        }
        void init() override {
            phase::init();
        }
        void render(int frame, vk::Semaphore imageAvailable, vk::Semaphore renderFinished, vk::Fence fence) override {
            phase::render(frame, imageAvailable, renderFinished, fence);

            vk::CommandBuffer& commandBuffer = commandBuffers[frame];
            commandBuffer.begin(vk::CommandBufferBeginInfo());
            vk::ClearValue clearValue(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f}));
            vk::RenderPassBeginInfo renderPassInfo(renderPass.get(), framebuffers[frame].get(), vk::Rect2D({0, 0}, win->swapchainExtent), clearValue);
            commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

            vk::Viewport viewport(0.0f, 0.0f, win->swapchainExtent.width, win->swapchainExtent.height, 0.0f, 1.0f);
            vk::Rect2D scissor({0,0}, win->swapchainExtent);
            commandBuffer.setViewport(0, viewport);
            commandBuffer.setScissor(0, scissor);

            fontRenderer.renderText(commandBuffer, frame, renderPass.get(), "Hello World!", 0.0f, 0.0f, 0.1f);
            fontRenderer.renderText(commandBuffer, frame, renderPass.get(), "Goodbye\nWorld!", 0.0f, 0.1f, 0.1f);

            commandBuffer.endRenderPass();

            fontRenderer.finish(frame);
            commandBuffer.end();

            vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            vk::SubmitInfo submitInfo(1, &imageAvailable, &waitStages, 1, &commandBuffer, 1, &renderFinished);
            graphicsQueue.submit(submitInfo, fence);
        }
};

int main() {
    spdlog::set_level(spdlog::level::debug);

    dreamrender::window_config config;
    config.title = "Simple Example";
    config.name = "simple-example";

    dreamrender::window window{config};
    window.init();
    window.set_phase(new simple_phase(&window));
    window.loop();
}
