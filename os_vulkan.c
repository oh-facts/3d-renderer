// OS / vulkan hooks ==================
function OS_Handle os_vulkan_loadLibrary();
function void os_vulkan_loadSurfaceFunction(OS_Handle vkdll);
function s32 os_vulkan_getPlatformExtentions(char *extentions[]);
function VkResult os_vulkan_createSurface(OS_Handle handle, VkInstance instance, VkSurfaceKHR *surface);