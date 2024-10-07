module;

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <span>
#include <vector>

#ifdef __GNUG__
#include <cxxabi.h>
#endif

export module dreamrender:utils;

import :debug;
import vulkan_hpp;

namespace dreamrender {

export template<std::array src, typename ToType, typename FromType = typename decltype(src)::value_type>
constexpr auto convert() requires (std::integral<FromType> && std::integral<ToType>) {
    static_assert(std::is_same_v<
        std::remove_cv_t<FromType>, 
        std::remove_cv_t<typename decltype(src)::value_type>
        >, "Source type must match the type of the input array");
    
    constexpr unsigned int from_size = sizeof(FromType);
    constexpr unsigned int to_size = sizeof(ToType);
    if constexpr (from_size < to_size) {
        static_assert(to_size % from_size == 0, "Cannot convert to a type that is not a multiple of the source type");
        constexpr auto factor = to_size/from_size;
        std::array<ToType, src.size()/factor> result;
        for(unsigned int i=0; i<result.size(); ++i) {
            ToType value = 0;
            for(unsigned int j=0; j<factor; ++j) {
                value |= static_cast<ToType>(src[i*to_size/from_size+j]) << (from_size*8*j);
            }
            result[i] = value;
        }
        return result;
    } else if constexpr (from_size > to_size) {
        static_assert(from_size % to_size == 0, "annot convert from a type that is not a multiple of the destination type");
        constexpr auto factor = from_size/to_size;
        std::array<ToType, src.size()*factor> result;
        for(unsigned int i=0; i<src.size(); ++i) {
            for(unsigned int j=0; j<factor; ++j) {
                result[i*factor+j] = (src[i] >> (to_size*8*j)) & ((1 << (to_size*8))-1);
            }
        }
        return result;
    } else {
        std::array<ToType, src.size()> result;
        for(unsigned int i=0; i<src.size(); ++i) {
            result[i] = static_cast<ToType>(src[i]);
        }
        return result;
    }
}

// Some constexpr tests
namespace {
    constexpr std::array<uint8_t, 4> test_array1 = {0x12, 0x34, 0x56, 0x78};
    static_assert(convert<test_array1, uint16_t>() == std::array<uint16_t, 2>{0x3412, 0x7856});
    static_assert(convert<test_array1, uint32_t>() == std::array<uint32_t, 1>{0x78563412});
    static_assert(convert<test_array1, uint8_t>() == test_array1);

    constexpr std::array<uint16_t, 2> test_array2 = {0x3412, 0x7856};
    static_assert(convert<test_array2, uint8_t>() == std::array<uint8_t, 4>{0x12, 0x34, 0x56, 0x78});
    static_assert(convert<test_array2, uint32_t>() == std::array<uint32_t, 1>{0x78563412});
    static_assert(convert<test_array2, uint16_t>() == test_array2);
}

export vk::UniqueShaderModule createShader(vk::Device device, std::span<const uint32_t> code) {
    vk::ShaderModuleCreateInfo shader_info({}, code.size()*sizeof(decltype(code)::element_type), code.data());
    vk::UniqueShaderModule shader = device.createShaderModuleUnique(shader_info);
    return std::move(shader);
}
export vk::UniqueShaderModule createShader(vk::Device device, const std::filesystem::path& path) {
    std::ifstream in(path, std::ios_base::binary | std::ios_base::ate);

    size_t size = in.tellg();
    std::vector<uint32_t> buf(size/sizeof(uint32_t));
    in.seekg(0);
    in.read(reinterpret_cast<char*>(buf.data()), size);

    vk::UniqueShaderModule shader = createShader(device, buf);
    debugName(device, shader.get(), "Shader Module \""+path.filename().string()+"\"");
    return shader;
}

export using UniquePipelineMap = std::map<vk::RenderPass, vk::UniquePipeline>;

export UniquePipelineMap createPipelines(
    vk::Device device,
    vk::PipelineCache pipelineCache,
    const vk::GraphicsPipelineCreateInfo& createInfo,
    const std::vector<vk::RenderPass>& renderPasses,
    const std::string& debugName)
{
    std::vector<vk::GraphicsPipelineCreateInfo> createInfos(renderPasses.size(), createInfo);
    createInfos[0].flags |= vk::PipelineCreateFlagBits::eAllowDerivatives;
    createInfos[0].renderPass = renderPasses[0];
    createInfos[0].basePipelineIndex = -1;

    for (size_t i = 1; i < renderPasses.size(); ++i)
    {
        createInfos[i].flags |= vk::PipelineCreateFlagBits::eDerivative;
        createInfos[i].renderPass = renderPasses[i];
        createInfos[i].basePipelineHandle = nullptr;
        createInfos[i].basePipelineIndex = 0;
    }
    auto result = device.createGraphicsPipelinesUnique(
        pipelineCache, createInfos, nullptr);
    if(result.result != vk::Result::eSuccess)
        throw std::runtime_error("Failed to create graphics pipeline(s) (error code: "+to_string(result.result)+")");

    UniquePipelineMap pipelines;
    for (size_t i = 0; i < renderPasses.size(); ++i)
    {
        dreamrender::debugName(device, result.value[i].get(), debugName);
        pipelines[renderPasses[i]] = std::move(result.value[i]);
    }
    return pipelines;
}

#ifdef __GNUG__
std::string demangle(const char *name) {
    int status = -4;
    std::unique_ptr<char, void(*)(void*)> res{
        abi::__cxa_demangle(name, nullptr, nullptr, &status),
        std::free
    };
    return (status==0) ? res.get() : name;
}
#else
std::string demangle(const char *name) {
    return std::string(name);
}
#endif

export template <class T>
std::string type_name(const T& t) {
    return demangle(typeid(t).name());
}

}
