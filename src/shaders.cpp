module;

#include <array>
#include <cstdint>

module dreamrender;

import :utils;
import vulkan_hpp;

namespace dreamrender::shaders {

namespace font_renderer {
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wc23-extensions"
    constexpr char vert_array[] = {
    #embed "shaders/font_renderer.vert.spv"
    };
    constexpr char geom_array[] = {
    #embed "shaders/font_renderer.geom.spv"
    };
    constexpr char frag_array[] = {
    #embed "shaders/font_renderer.frag.spv"
    };
    #pragma clang diagnostic pop

    constexpr std::array vert_shader = convert<std::to_array(vert_array), uint32_t>();
    constexpr std::array geom_shader = convert<std::to_array(geom_array), uint32_t>();
    constexpr std::array frag_shader = convert<std::to_array(frag_array), uint32_t>();

    vk::UniqueShaderModule vert(vk::Device device) {
        return createShader(device, vert_shader);
    }
    vk::UniqueShaderModule geom(vk::Device device) {
        return createShader(device, geom_shader);
    }
    vk::UniqueShaderModule frag(vk::Device device) {
        return createShader(device, frag_shader);
    }
}

namespace image_renderer {
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wc23-extensions"
    constexpr char vert_array[] = {
    #embed "shaders/image_renderer.vert.spv"
    };
    constexpr char frag_array[] = {
    #embed "shaders/image_renderer.frag.spv"
    };
    #pragma clang diagnostic pop

    constexpr std::array vert_shader = convert<std::to_array(vert_array), uint32_t>();
    constexpr std::array frag_shader = convert<std::to_array(frag_array), uint32_t>();

    vk::UniqueShaderModule vert(vk::Device device) {
        return createShader(device, vert_shader);
    }
    vk::UniqueShaderModule frag(vk::Device device) {
        return createShader(device, frag_shader);
    }
}

}