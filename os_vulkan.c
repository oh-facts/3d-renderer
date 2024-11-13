// OS / vulkan hooks ==================
function OS_Handle os_vulkan_loadLibrary();
function void os_vulkan_loadSurfaceFunction(OS_Handle vkdll);
function char *os_vulkan_surfaceExtentionName();
function VkResult os_vulkan_createSurface(OS_Handle handle, VkInstance instance, VkSurfaceKHR *surface);