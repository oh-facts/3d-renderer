function OS_Handle os_vulkan_loadLibrary()
{
#if defined(OS_LINUX)
    
    OS_Handle out = os_loadLibrary("libvulkan.so.1"); 
    
    return out;
    
#elif defined(OS_APPLE)
    
    OS_Handle out = os_loadLibrary("/usr/local/lib/libvulkan.dylib");
	
    return out;
    
#endif
}