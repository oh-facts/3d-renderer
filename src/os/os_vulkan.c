// OS / vulkan hooks ==================
function OS_Handle os_vulkan_loadLibrary();
function s32 os_vulkan_getPlatformExtentions(char *extentions[]);
function VkResult os_vulkan_createSurface(OS_Handle handle, VkInstance instance, VkSurfaceKHR *surface);