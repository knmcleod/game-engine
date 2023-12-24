VULKAN_SDK = os.getenv("VulkanSDK\1.3.268.0");

IncludeDir = {}
IncludeDir["glfw"] = "game-engine/vender/glfw/include"
IncludeDir["GLAD"] = "game-engine/vender/GLAD/include"
IncludeDir["ImGui"] = "game-engine/vender/imgui"
IncludeDir["glm"] = "game-engine/vender/glm"
IncludeDir["spdlog"] = "game-engine/vender/spdlog/include"
IncludeDir["stb"] = "game-engine/vender/stb_image"
IncludeDir["entt"] = "game-engine/vender/entt/single_include"
IncludeDir["yaml_cpp"] = "game-engine/vender/yaml-cpp/include"
IncludeDir["shaderc"] = "game-engine/vendor/shaderc/libshaderc/include"
IncludeDir["SPIRV_Cross"] = "game-engine/vendor/SPIRV-Cross"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"

LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"] = "game-engine/vendor/VulkanSDK/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"
