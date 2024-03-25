#include "DeviceHandler.h"
#include <stdexcept>
#include <string>
#include <set>
#include <SDL_vulkan.h>

#include <optional>
#include <cassert>

#include <vulkan/vk_enum_string_helper.h>

#include "Logger.h"

DeviceHandler::DeviceHandler(SDL_Window* window)
    : m_window(window)
{
#ifdef VK_KHR_SWAPCHAIN_EXTENSION_NAME
    m_info.requiredPhysicalDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#endif
// need this one for macos
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
    m_info.requiredPhysicalDeviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

    Logger::Instance()->log("DeviceHandler Constructor start");
    createInstance();
    Logger::Instance()->logVerbose("DeviceHandler createInstance done");
    setupDebugMessenger();
    Logger::Instance()->logVerbose("DeviceHandler setupDebugMessenger done");
    createSurface();
    Logger::Instance()->logVerbose("DeviceHandler createSurface done");
    pickPhysicalDevice();
    Logger::Instance()->logVerbose("DeviceHandler pickPhysicalDevice done");
    createLogicalDevice();
    Logger::Instance()->logVerbose("DeviceHandler createLogicalDevice done");
    createSwapchain();
    Logger::Instance()->logVerbose("DeviceHandler createSpawchain done");
    createSwapchainImageViews();
    Logger::Instance()->logVerbose("DeviceHandler createSpawchain done");
    createRenderPass();
    Logger::Instance()->logVerbose("DeviceHandler createRenderPass done");
    createSwapchainFramebuffers();
    Logger::Instance()->logVerbose("DeviceHandler createSwapchainFramebuffers done");
    createCommandPool();
    Logger::Instance()->logVerbose("DeviceHandler createCommandPool done");
    //createCommandBuffer();
    Logger::Instance()->logVerbose("DeviceHandler createCommandBuffer done");
    createSyncObjects();
    Logger::Instance()->logVerbose("DeviceHandler createSyncObjects done");
    createTextureSampler();
    Logger::Instance()->logVerbose("DeviceHandler createTextureSampler done");
    Logger::Instance()->log("DeviceHandler Constructor end");
}

DeviceHandler::~DeviceHandler()
{
    Logger::Instance()->log("DeviceHandler Destructor start");
    checkVkResult(vkDeviceWaitIdle(m_logicalDevice));
    Logger::Instance()->logVerbose("DeviceHandler vkDeviceWaitIdle done");
    vkDestroySampler(m_logicalDevice, m_sampler, VK_NULL_HANDLE);
    Logger::Instance()->logVerbose("DeviceHandler vkDestroySampler done");
    vkDestroyFence(m_logicalDevice, m_renderFinishedFence, VK_NULL_HANDLE);
    Logger::Instance()->logVerbose("DeviceHandler vkDestroyFence done");
    vkDestroySemaphore(m_logicalDevice, m_renderFinishedSemaphore, VK_NULL_HANDLE);
    Logger::Instance()->logVerbose("DeviceHandler vkDestroySemaphore done");
    vkDestroySemaphore(m_logicalDevice, m_imageAvailableSemaphore, VK_NULL_HANDLE);
    Logger::Instance()->logVerbose("DeviceHandler vkDestroySemaphore done");
    vkDestroyCommandPool(m_logicalDevice, m_commandPool, VK_NULL_HANDLE);
    Logger::Instance()->logVerbose("DeviceHandler vkDestroyCommandPool done");
    for (auto f: m_swapChainFrameBuffers)
    { vkDestroyFramebuffer(m_logicalDevice, f, VK_NULL_HANDLE); }
    Logger::Instance()->logVerbose("DeviceHandler vkDestroyFramebuffer done");
    vkDestroyRenderPass(m_logicalDevice, m_renderPass, VK_NULL_HANDLE);
    Logger::Instance()->logVerbose("DeviceHandler vkDestroyRenderPass done");
    for (auto v: m_swapChainImageViews)
    { vkDestroyImageView(m_logicalDevice, v, VK_NULL_HANDLE); }
    Logger::Instance()->logVerbose("DeviceHandler vkDestroyImageView done");
    vkDestroySwapchainKHR(m_logicalDevice, m_swapchain, VK_NULL_HANDLE);
    Logger::Instance()->logVerbose("DeviceHandler vkDestroySwapchainKHR done");
    vkDestroyDevice(m_logicalDevice, VK_NULL_HANDLE);
    Logger::Instance()->logVerbose("DeviceHandler vkDestroyDevice done");
    vkDestroySurfaceKHR(m_instance, m_surface, VK_NULL_HANDLE);
    Logger::Instance()->logVerbose("DeviceHandler destroySurface done");
    teardownDebugMessenger();
    Logger::Instance()->logVerbose("DeviceHandler teardownDebugMessenger done");
    vkDestroyInstance(m_instance, VK_NULL_HANDLE);
    Logger::Instance()->log("DeviceHandler Destructor end");
}

void DeviceHandler::checkVkResult(const VkResult &res)
{
    if (res == VK_SUCCESS)
        return;

    //if (res < 0)// <- error
    Logger::Instance()->logError("Vulkan error with VkResult code: " + std::string(string_VkResult(res)));
}

bool DeviceHandler::IsExtensionSupported(const std::vector<VkExtensionProperties>& supportedExtensions, const char* extension)
{
    for (auto& se: supportedExtensions)
    {
        if (strcmp(se.extensionName, extension) == 0)
            return true;
    }
    return false;
}

void DeviceHandler::createInstance() {

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "vulkanRenderer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "theGame";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    createInfo.pApplicationInfo = &appInfo;

    // supported extension part
    uint32_t supportedExtensionCount{0};
    checkVkResult(vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr));
    m_info.supportedInstanceExtensions.resize(supportedExtensionCount);
    checkVkResult(vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, m_info.supportedInstanceExtensions.data()));

    // required extension part
    unsigned int requiredExtensionCount{0};
    // get the required instance extensions for an SDL window
    SDL_Vulkan_GetInstanceExtensions(m_window, &requiredExtensionCount, nullptr);
    m_info.requiredInstanceExtensions.resize(requiredExtensionCount);
    SDL_Vulkan_GetInstanceExtensions(m_window, &requiredExtensionCount, m_info.requiredInstanceExtensions.data());
    // TODO: maybe we need to check the extensions from SDL that all of them are supported or not
    // need this extension for debug messaging
    // TODO: for production code this is not required; only for development code
    if (IsExtensionSupported(m_info.supportedInstanceExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
    {
        m_info.requiredInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        m_info.instanceValidationLayers.push_back("VK_LAYER_KHRONOS_validation");
    }
// need this one for MacOS compatibility
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    if (IsExtensionSupported(m_info.supportedInstanceExtensions, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
    {
        m_info.requiredInstanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif
// need this one for MacOS compatibility
#ifdef VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
    if (IsExtensionSupported(m_info.supportedInstanceExtensions, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
        m_info.requiredInstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

    createInfo.enabledExtensionCount = (uint32_t)m_info.requiredInstanceExtensions.size();
    createInfo.ppEnabledExtensionNames = m_info.requiredInstanceExtensions.data();

    // debuging part
    createInfo.enabledLayerCount = (uint32_t)m_info.instanceValidationLayers.size();
    createInfo.ppEnabledLayerNames = m_info.instanceValidationLayers.data();

    checkVkResult(vkCreateInstance(&createInfo, nullptr, &m_instance));
}

VKAPI_ATTR VkBool32 VKAPI_CALL DeviceHandler::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* pUserData
    ) {
    char prefix[64];
    char *message = (char *)malloc(strlen(callbackData->pMessage) + 500);
    assert(message);
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        strcpy(prefix, "VERBOSE : ");
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        strcpy(prefix, "INFO : ");
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        strcpy(prefix, "WARNING : ");
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        strcpy(prefix, "ERROR : ");
    }
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        strcat(prefix, "GENERAL");
    } else {
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
            strcat(prefix, "VALIDATION");
            //validation_error = 1;
        }
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
            if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
                strcat(prefix, "|");
            }
            strcat(prefix, "PERF");
        }
    }
    sprintf(message,
    "%s - Message ID Number %d, Message ID String :%s\n%s",
    prefix,
    callbackData->messageIdNumber,
    callbackData->pMessageIdName,
    callbackData->pMessage);
    Logger::Instance()->logCritical(message);
    free(message);
    return VK_FALSE;
}

void DeviceHandler::setupDebugMessenger() {
    CreateDebugUtilsMessengerEXT =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        m_instance,
        "vkCreateDebugUtilsMessengerEXT");

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    checkVkResult(CreateDebugUtilsMessengerEXT(
        m_instance,
        &createInfo,
        NULL,
        &m_debugMessenger
        ));
}

void DeviceHandler::teardownDebugMessenger() {
    DestroyDebugUtilsMessengerEXT =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        m_instance,
        "vkDestroyDebugUtilsMessengerEXT");

    DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, NULL);
}

void DeviceHandler::createSurface()
{
    if (!SDL_Vulkan_CreateSurface(m_window, m_instance, &m_surface)) {
        Logger::Instance()->logCritical("SDL Vulkan failed to create a Surface!");
    }
}

void DeviceHandler::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    checkVkResult(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr));

    if (deviceCount == 0)
        Logger::Instance()->logCritical("Vulkan failed to find any GPU!");

    m_info.gpus.resize(deviceCount);
    checkVkResult(vkEnumeratePhysicalDevices(m_instance, &deviceCount, m_info.gpus.data()));
    Logger::Instance()->logVerbose("Vulkan: pickPhysicalDevice 1");

    // TODO: design a GPU selection method; score system; fallback to integrated from dedicated etc...
    // and this can be implemented here; I think I just get back here when I require new stuff from the GPU

    for (const auto& device : m_info.gpus) {
        Logger::Instance()->logVerbose("Vulkan: pickPhysicalDevice 2");
        if (isDeviceGoodForUs(device)) {
            Logger::Instance()->logVerbose("Vulkan: pickPhysicalDevice 3");
            m_physicalDevice = device;
            break;
        }
    }

    Logger::Instance()->logVerbose("Vulkan: pickPhysicalDevice 4");
    if (m_physicalDevice == VK_NULL_HANDLE)
        Logger::Instance()->logCritical("Vulkan failed to find a good GPU ABORT!");

    Logger::Instance()->logVerbose("Vulkan: pickPhysicalDevice 5");
}

bool DeviceHandler::isDeviceGoodForUs(const VkPhysicalDevice& device)
{
    bool boolRes{true};
    //TODO: gather all of the requirements that are present here and make them variables or smtg, so we can reuse them more easily later
    Logger::Instance()->logVerbose("Vulkan: isDeviceGoodForUs 1");
    // our custom method to figure out the gpu is fine for us or not
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    Logger::Instance()->logVerbose("Vulkan: isDeviceGoodForUs 2");

    // check the properties; like is it a discrete GPU or not
    // dont need this, M1 Air will fail here
    //bool isDiscreteGPU{false};
    //if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    //    isDiscreteGPU = true;

    Logger::Instance()->logVerbose("Vulkan: isDeviceGoodForUs 3");
    // check the available queueFamilies
    uint32_t queueFamilyCount{0};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    Logger::Instance()->logVerbose("Vulkan: isDeviceGoodForUs 4");
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    Logger::Instance()->logVerbose("Vulkan: isDeviceGoodForUs 5");
    // graphics and present queue
    VkBool32 hasPresentQueue{false};
    for (uint32_t i = 0; i < queueFamilies.size(); i++)
    {
        Logger::Instance()->logVerbose("Vulkan: isDeviceGoodForUs 6");
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            Logger::Instance()->logVerbose("Vulkan: isDeviceGoodForUs 7");
            checkVkResult(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &hasPresentQueue));
            if (hasPresentQueue)
            {
                Logger::Instance()->logVerbose("Vulkan: isDeviceGoodForUs 8");
                m_info.graphicsQueueIndex = i;
                m_info.presentQueueIndex = i;
                break;
            }
        }
    }
    boolRes = boolRes && m_info.graphicsQueueIndex.has_value() && hasPresentQueue;
    Logger::Instance()->logVerbose("Vulkan: 1 boolRes " + std::to_string(boolRes));
    Logger::Instance()->logVerbose("Vulkan: isDeviceGoodForUs 9");

    // swapchain support
    uint32_t physicalDeviceExtensionCount{0};
    checkVkResult(vkEnumerateDeviceExtensionProperties(device, nullptr, &physicalDeviceExtensionCount, nullptr));
    std::vector<VkExtensionProperties> physicalDeviceExtensions(physicalDeviceExtensionCount);
    checkVkResult(vkEnumerateDeviceExtensionProperties(device, nullptr, &physicalDeviceExtensionCount, physicalDeviceExtensions.data()));

    std::set<std::string> reqExt(m_info.requiredPhysicalDeviceExtensions.begin(), m_info.requiredPhysicalDeviceExtensions.end());
    for(const auto& e: physicalDeviceExtensions)
        reqExt.erase(e.extensionName);
    boolRes = boolRes && reqExt.empty();
    Logger::Instance()->logVerbose("Vulkan: 2 boolRes " + std::to_string(boolRes));

    // more swapchain checking before choosing the device
    // 1, swapChainCapabilities; like min/max number of images in swapchain; min/max width/height of image
    VkSurfaceCapabilitiesKHR surfaceCapa{};
    checkVkResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &surfaceCapa));
    Logger::Instance()->logCritical(std::to_string(surfaceCapa.currentExtent.width));
    Logger::Instance()->logCritical(std::to_string(surfaceCapa.currentExtent.height));

    // 2, supported surface formats; like pixel format, color space
    uint32_t surfaceFormatCount{0};
    checkVkResult(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &surfaceFormatCount, nullptr));
    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    checkVkResult(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &surfaceFormatCount, surfaceFormats.data()));
    bool surfaceFormatGood{false};
    for (auto& sf: surfaceFormats)
    {
        if (sf.format == VK_FORMAT_B8G8R8A8_SRGB && sf.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            m_info.swapChainSurfaceFormat = sf;
            surfaceFormatGood = true;
            break;
        }
    }
    boolRes = boolRes && surfaceFormatGood;
    Logger::Instance()->logVerbose("Vulkan: 3 boolRes " + std::to_string(boolRes));

    // 3, supported presentation modes; like fifo, mailbox: how to handle the full swapchain
    uint32_t surfacePresentCount{0};
    checkVkResult(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &surfacePresentCount, nullptr));
    std::vector<VkPresentModeKHR> surfacePresents(surfacePresentCount);
    checkVkResult(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &surfacePresentCount, surfacePresents.data()));
    bool surfacePresentGood{false};
    for (auto& sp: surfacePresents)
    {
        // first it will OK for us to use FIFO alias vertical sync, but can change later
        if (sp == VK_PRESENT_MODE_FIFO_KHR)
        {
            m_info.swapChainPresentMode = sp;
            surfacePresentGood = true;
            break;
        }
    }
    boolRes = boolRes && surfacePresentGood;
    Logger::Instance()->logVerbose("Vulkan: 4 boolRes " + std::to_string(boolRes));

    // just query the memory properties for later buffer allocation
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
    Logger::Instance()->logCritical("isDeviceGoodForUs 10");

    if (boolRes)
    {
        Logger::Instance()->logCritical("isDeviceGoodForUs 11");
        m_info.physicalDeviceProperties = deviceProperties;
        m_info.physicalDeviceFeatures = deviceFeatures;
        m_info.physicalDeviceQueueFamilyProperties = queueFamilies;
        m_info.physicalDeviceExtensionProperties = physicalDeviceExtensions;
        m_info.physicalDeviceSurfaceCapabilities = surfaceCapa;
        m_info.physicalDeviceSurfaceFormats = surfaceFormats;
        m_info.physicalDevicePresentModes = surfacePresents;
        m_info.physicalDeviceMemoryProperties = memProperties;
    }
    Logger::Instance()->logCritical("isDeviceGoodForUs 12");
    return boolRes;
}

void DeviceHandler::createSwapchainFramebuffers()
{
    m_swapChainFrameBuffers.resize(m_swapChainImageViews.size());

    for (size_t i = 0; i < m_swapChainFrameBuffers.size(); i++) {
        std::vector<VkImageView> attachments = { m_swapChainImageViews[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = (uint32_t)attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_info.physicalDeviceSurfaceCapabilities.currentExtent.width;
        framebufferInfo.height = m_info.physicalDeviceSurfaceCapabilities.currentExtent.height;
        framebufferInfo.layers = 1;

        checkVkResult(vkCreateFramebuffer(m_logicalDevice, &framebufferInfo, nullptr, &m_swapChainFrameBuffers[i]));
    }
}

void DeviceHandler::createCommandPool()
{
    VkCommandPoolCreateInfo commandPoolCreateInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    /*
    VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
    */
    commandPoolCreateInfo.queueFamilyIndex = m_info.graphicsQueueIndex.value();

    checkVkResult(vkCreateCommandPool(m_logicalDevice, &commandPoolCreateInfo, nullptr, &m_commandPool));
}

void DeviceHandler::createCommandBuffer(VkCommandBuffer& buffer, VkCommandBufferLevel level)
{
    VkCommandBufferAllocateInfo createInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    createInfo.commandPool = m_commandPool;
    createInfo.level = level;
    /*
    VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
    VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command buffers.
    */
    createInfo.commandBufferCount = 1;

    checkVkResult(vkAllocateCommandBuffers(m_logicalDevice, &createInfo, &buffer));
}

void DeviceHandler::createCommandBuffer(std::vector<VkCommandBuffer>& buffer, VkCommandBufferLevel level)
{
    VkCommandBufferAllocateInfo createInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    createInfo.commandPool = m_commandPool;
    createInfo.level = level;
    /*
    VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
    VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command buffers.
    */
    createInfo.commandBufferCount = (uint32_t)buffer.size();

    checkVkResult(vkAllocateCommandBuffers(m_logicalDevice, &createInfo, buffer.data()));
}

void DeviceHandler::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    checkVkResult(vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore));
    checkVkResult(vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore));
    checkVkResult(vkCreateFence(m_logicalDevice, &fenceInfo, nullptr, &m_renderFinishedFence));
}

void DeviceHandler::recordCommand(VkCommandBuffer buffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    //checkVkResult(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
    /* flag
    without any setting; so neither of these; the commandbuffer will go back to executable state
    VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.
    VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
    VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already pending execution.
    */

    VkRenderPassBeginInfo renderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapChainFrameBuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0,0};
    renderPassInfo.renderArea.extent = m_info.physicalDeviceSurfaceCapabilities.currentExtent;

    VkClearValue clearColor = {{{1.f, 0.f, 1.f, 1.f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    /*
    VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
    VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass commands will be executed from secondary command buffers.
    */

    //vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_baseGraphicsPipeline.pipeline);

    //vkCmdSetViewport(buffer, 0, 1, &m_baseGraphicsPipeline.viewport);
    //vkCmdSetScissor(buffer, 0, 1, &m_baseGraphicsPipeline.scissor);

    vkCmdDraw(buffer, 4, 1, 0, 0);

    vkCmdEndRenderPass(buffer);

    checkVkResult(vkEndCommandBuffer(buffer));
}

void DeviceHandler::createLogicalDevice() {
    // we are just using one queue for graphics and present; but later maybe we need more
    // for example a good old compute shader for calculating a lot
    VkDeviceQueueCreateInfo queueCreateInfo[1] = {};
    queueCreateInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo[0].pNext = NULL;
    queueCreateInfo[0].queueFamilyIndex = m_info.graphicsQueueIndex.value();
    queueCreateInfo[0].queueCount = 1;
    const float prio{1.f};
    queueCreateInfo[0].pQueuePriorities = &prio;

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = NULL;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = (uint32_t)m_info.requiredPhysicalDeviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = m_info.requiredPhysicalDeviceExtensions.data();
    deviceCreateInfo.enabledLayerCount = 0;

    checkVkResult(vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_logicalDevice));

    vkGetDeviceQueue(m_logicalDevice, m_info.graphicsQueueIndex.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_logicalDevice, m_info.presentQueueIndex.value(), 0, &m_presentQueue);
}

void DeviceHandler::createSwapchain()
{
    uint32_t minImageCount{0};
    minImageCount = m_info.physicalDeviceSurfaceCapabilities.minImageCount + 1;
    if(m_info.physicalDeviceSurfaceCapabilities.maxImageCount > 0)
        minImageCount = std::min(m_info.physicalDeviceSurfaceCapabilities.maxImageCount, minImageCount);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.surface = m_surface;
    createInfo.minImageCount = minImageCount;
    createInfo.imageFormat = m_info.swapChainSurfaceFormat.format;
    createInfo.imageColorSpace = m_info.swapChainSurfaceFormat.colorSpace;
    createInfo.imageExtent = m_info.physicalDeviceSurfaceCapabilities.currentExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
    createInfo.preTransform = m_info.physicalDeviceSurfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = m_info.swapChainPresentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    checkVkResult(vkCreateSwapchainKHR(m_logicalDevice, &createInfo, nullptr, &m_swapchain));

    checkVkResult(vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain, &m_info.swapChainImageCount, nullptr));
    m_swapChainImages.resize(m_info.swapChainImageCount);
    checkVkResult(vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain, &m_info.swapChainImageCount, m_swapChainImages.data()));
}

void DeviceHandler::createSwapchainImageViews()
{
    m_swapChainImageViews.resize(m_swapChainImages.size());

    for(size_t i = 0; i < m_swapChainImageViews.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = NULL;
        createInfo.image = m_swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_info.swapChainSurfaceFormat.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        checkVkResult(vkCreateImageView(m_logicalDevice, &createInfo, nullptr, &m_swapChainImageViews[i]));
    }
}

void DeviceHandler::createRenderPass()
{
    // renderPass is independent from the pipeline in use, it is only depends on the swapchain, the images we are trying to render into
    // so we can create this after swapchain creation
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_info.swapChainSurfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    /*
    VK_ATTACHMENT_LOAD_OP_LOAD: Preserve the existing contents of the attachment
    VK_ATTACHMENT_LOAD_OP_CLEAR: Clear the values to a constant at the start
    VK_ATTACHMENT_LOAD_OP_DONT_CARE: Existing contents are undefined; we don't care about them
    */
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    /*
    VK_ATTACHMENT_STORE_OP_STORE: Rendered contents will be stored in memory and can be read later
    VK_ATTACHMENT_STORE_OP_DONT_CARE: Contents of the framebuffer will be undefined after the rendering operation
    */
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    /*
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color attachment
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Images to be presented in the swap chain
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: Images to be used as destination for a memory copy operation
    */

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    /*
    The following other types of attachments can be referenced by a subpass:
    pInputAttachments: Attachments that are read from a shader
    pResolveAttachments: Attachments used for multisampling color attachments
    pDepthStencilAttachment: Attachment for depth and stencil data
    pPreserveAttachments: Attachments that are not used by this subpass, but for which the data must be preserved
    */
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    //VkRenderPassCreateInfo
    VkRenderPassCreateInfo renderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;

    checkVkResult(vkCreateRenderPass(m_logicalDevice, &renderPassCreateInfo, nullptr, &m_renderPass));
}

void DeviceHandler::drawFrame(VkCommandBuffer& buffer)
{
    vkWaitForFences(m_logicalDevice, 1, &m_renderFinishedFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_logicalDevice, 1, &m_renderFinishedFence);

    vkAcquireNextImageKHR(
        m_logicalDevice,
        m_swapchain,
        UINT64_MAX,
        m_imageAvailableSemaphore,
        VK_NULL_HANDLE,
        &m_currentImageIndex
        );

    //printf("imageindex %d\n", imageIndex);
    //vkResetCommandBuffer(m_commandBuffer, 0);

    //recordCommand(m_commandBuffer, imageIndex);

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};

    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buffer;
    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    checkVkResult(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_renderFinishedFence));

    VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {m_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &m_currentImageIndex;

    checkVkResult(vkQueuePresentKHR(m_presentQueue, &presentInfo));
    checkVkResult(vkQueueWaitIdle(m_presentQueue));
}

void DeviceHandler::recordRenderPrimaryCommandBuffer(
    VkCommandBuffer& buffer,
    std::vector<VkCommandBuffer>& secBuffers
    )
{
    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

    checkVkResult(vkBeginCommandBuffer(buffer, &beginInfo));

    VkRenderPassBeginInfo renderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapChainFrameBuffers[m_currentImageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_info.physicalDeviceSurfaceCapabilities.currentExtent;

    VkClearValue clearColor = {{{1.f, 1.f, 1.f, 1.f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    vkCmdExecuteCommands(buffer, secBuffers.size(), secBuffers.data());

    vkCmdEndRenderPass(buffer);

    recordEndCommandBuffer(buffer);
}

void DeviceHandler::recordRenderSecondaryCommandBufferStart(VkCommandBuffer& buffer)
{
    VkCommandBufferInheritanceInfo inheritanceInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};
    inheritanceInfo.renderPass = m_renderPass;
    inheritanceInfo.subpass = 0;
    inheritanceInfo.framebuffer = VK_NULL_HANDLE;

    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    checkVkResult(vkBeginCommandBuffer(buffer, &beginInfo));//this is also resetting back to recording state from executable

    //VkViewport
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) m_info.physicalDeviceSurfaceCapabilities.currentExtent.width;
    viewport.height = (float) m_info.physicalDeviceSurfaceCapabilities.currentExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(buffer, 0, 1, &viewport);

    //VkRect2D;
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_info.physicalDeviceSurfaceCapabilities.currentExtent;
    vkCmdSetScissor(buffer, 0, 1, &scissor);
}

void DeviceHandler::recordOneTimerCommandBufferStart(VkCommandBuffer& buffer)
{
    createCommandBuffer(buffer, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(buffer, &beginInfo);
}

void DeviceHandler::recordEndCommandBuffer(VkCommandBuffer& buffer)
{
    checkVkResult(vkEndCommandBuffer(buffer));
}

void DeviceHandler::submitAndDeleteCommandBuffer(VkCommandBuffer &buffer)
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buffer;

    checkVkResult(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
    checkVkResult(vkQueueWaitIdle(m_graphicsQueue));

    vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, &buffer);
}

void DeviceHandler::createTextureSampler()
{
    VkSamplerCreateInfo samplerInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;//if we need this later, I will turn ON
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    checkVkResult(vkCreateSampler(m_logicalDevice, &samplerInfo, VK_NULL_HANDLE, &m_sampler));
}

void DeviceHandler::copyBufferToGPU(VkBuffer& sourceBuffer, VkBuffer& destinationBuffer, VkDeviceSize& size)
{
    VkCommandBuffer copyCommand;

    recordOneTimerCommandBufferStart(copyCommand);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(copyCommand, sourceBuffer, destinationBuffer, 1, &copyRegion);

    recordEndCommandBuffer(copyCommand);

    submitAndDeleteCommandBuffer(copyCommand);
}

void DeviceHandler::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_logicalDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    checkVkResult(vkBindBufferMemory(m_logicalDevice, buffer, bufferMemory, 0));
}

void DeviceHandler::destroyBuffer(VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
    vkDestroyBuffer(m_logicalDevice, buffer, VK_NULL_HANDLE);
    vkFreeMemory(m_logicalDevice, bufferMemory, VK_NULL_HANDLE);
}

void DeviceHandler::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory)
{
    VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    checkVkResult(vkCreateImage(m_logicalDevice, &imageInfo, VK_NULL_HANDLE, &image));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_logicalDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    checkVkResult(vkAllocateMemory(m_logicalDevice, &allocInfo, VK_NULL_HANDLE, &imageMemory));

    checkVkResult(vkBindImageMemory(m_logicalDevice, image, imageMemory, 0));
}

void DeviceHandler::destroyImage(VkImage &image, VkDeviceMemory &imageMemory)
{
    vkDestroyImage(m_logicalDevice, image, VK_NULL_HANDLE);
    vkFreeMemory(m_logicalDevice, imageMemory, VK_NULL_HANDLE);
}

void DeviceHandler::changeImageLayout(VkImage &image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcMask, VkAccessFlags dstMask, VkPipelineStageFlags sourceFlags, VkPipelineStageFlags destFlags)
{
    VkCommandBuffer commandBuffer{};
    recordOneTimerCommandBufferStart(commandBuffer);

    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = srcMask;
    barrier.dstAccessMask = dstMask;

    VkPipelineStageFlags sourceStage{sourceFlags};
    VkPipelineStageFlags destinationStage{destFlags};

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    recordEndCommandBuffer(commandBuffer);

    submitAndDeleteCommandBuffer(commandBuffer);
}

void DeviceHandler::copyBufferToImage(VkBuffer &buffer, VkImage &image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer{};
    recordOneTimerCommandBufferStart(commandBuffer);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    recordEndCommandBuffer(commandBuffer);

    submitAndDeleteCommandBuffer(commandBuffer);
}

void DeviceHandler::createImageView(VkImageView& imageView, VkImage &image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    checkVkResult(vkCreateImageView(m_logicalDevice, &viewInfo, VK_NULL_HANDLE, &imageView));
}

void DeviceHandler::destroyImageView(VkImageView &imageView)
{
    vkDestroyImageView(m_logicalDevice, imageView, VK_NULL_HANDLE);
}

uint32_t DeviceHandler::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    for (uint32_t i = 0; i < m_info.physicalDeviceMemoryProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (m_info.physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}
