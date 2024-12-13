function s32 os_vulkan_getPlatformExtentions(char *extentions[])
{
	uint32_t count;
	char** extensions = glfwGetRequiredInstanceExtensions(&count);
	
	if(extentions)
	{
		for(u32 i = 0; i < count; i++)
		{
			extentions[i] = extensions[i];
		}
	}
	
	return count;
}

function VkResult os_vulkan_createSurface(OS_Handle handle, VkInstance instance, VkSurfaceKHR *surface)
{
	VkResult res = glfwCreateWindowSurface(instance, os_windowFromHandle(handle)->v, 0, surface);
	return res;
}