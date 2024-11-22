// enables vulkan asserts and validation layers
#define R_VULKAN_DEBUG 1
#define R_VULKAN_FRAMES 3
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>

#define CAMERA_UP ((V3F){{0, -1, 0}})
#define CAMERA_FRONT ((V3F){{0, 0, -1}})

typedef struct Camera Camera;
struct Camera
{ 
	V3F pos;	
	V3F mv;
	f32 pitch;
	f32 yaw;
	V2F old_mpos;
	b32 enable;
};

function void camUpdate(Camera *camera, OS_EventList *list, f32 delta)
{
	if(os_event(list, OS_Key_SPACE, OS_EventKind_Pressed))
	{
		if(camera->enable)
		{
			os_setCursorMode(OS_CursorMode_Normal);
		}
		else
		{
			os_setCursorMode(OS_CursorMode_Disabled);
		}
		
		camera->enable = !camera->enable;
		camera->mv = (V3F){0};
		os_event(list, OS_Key_SPACE, OS_EventKind_Released);
	}
	
	if(camera->enable)
	{
		if(os_event(list, OS_Key_W, OS_EventKind_Pressed))
		{
			camera->mv.z += -1;
		}
		else if(os_event(list, OS_Key_W, OS_EventKind_Released))
		{
			camera->mv.z -= -1;
		}
		
		if(os_event(list, OS_Key_A, OS_EventKind_Pressed))
		{
			camera->mv.x += 1;
		}
		else if(os_event(list, OS_Key_A, OS_EventKind_Released))
		{
			camera->mv.x -= 1;
		}
		
		if(os_event(list, OS_Key_D, OS_EventKind_Pressed))
		{
			camera->mv.x += -1;
		}
		else if(os_event(list, OS_Key_D, OS_EventKind_Released))
		{
			camera->mv.x -= -1;
		}
		
		if(os_event(list, OS_Key_S, OS_EventKind_Pressed))
		{
			camera->mv.z += 1;
		}
		else if(os_event(list, OS_Key_S, OS_EventKind_Released))
		{
			camera->mv.z -= 1;
		}
		
		if(os_event(list, OS_Key_LSHIFT, OS_EventKind_Pressed))
		{
			camera->mv.y += 1;
		}
		else if(os_event(list, OS_Key_LSHIFT, OS_EventKind_Released))
		{
			camera->mv.y -= 1;
		}
		
		V3F dir = {0};
		dir.x = cos(degToRad(camera->yaw)) * cos(degToRad(camera->pitch)) * 0.001;
		dir.y = sin(degToRad(camera->pitch)) * 0.001;
		dir.z = sin(degToRad(camera->yaw)) * cos(degToRad(camera->pitch)) * 0.001;
		dir = v3f_normalize(dir);
		
		if(camera->mv.z > 0)
		{
			V3F vel = dir;
			vel.x *= delta;
			vel.y *= delta;
			vel.z *= delta;
			
			camera->pos = v3f_sub(camera->pos, vel);
		}
		else if(camera->mv.z < 0)
		{
			V3F vel = dir;
			vel.x *= delta;
			vel.y *= delta;
			vel.z *= delta;
			
			camera->pos = v3f_add(camera->pos, vel);
		}
		
		V3F up = v3f(0.0f, 1.0f, 0.0f); 
		V3F cameraRight = v3f_normalize(v3f_cross(up, dir));
		V3F cameraUp = v3f_cross(dir, cameraRight);
		V3F vel = v3f_normalize(v3f_cross(dir, cameraUp));
		vel.x *= delta;
		vel.y *= delta;
		vel.z *= delta;
		
		if(camera->mv.x > 0)
		{
			camera->pos = v3f_sub(camera->pos, vel);
		}
		else if(camera->mv.x < 0)
		{
			camera->pos = v3f_add(camera->pos, vel);
		}
		
		
		OS_Event *mpos = os_event(list, OS_Key_NULL, OS_EventKind_MouseMove);
		if(mpos)
		{
			camera->yaw += mpos->mpos.x - camera->old_mpos.x;
			camera->pitch += camera->old_mpos.y - mpos->mpos.y;
			camera->old_mpos = mpos->mpos;
		}
	}
	else
	{
		camera->old_mpos = os_getCursorPos();
	}
}

function M4F camGetView(Camera *camera)
{
	V3F dir = {0};
	dir.x = cos(degToRad(camera->yaw)) * cos(degToRad(camera->pitch)) * 0.001;
	dir.y = sin(degToRad(camera->pitch)) * 0.001;
	dir.z = sin(degToRad(camera->yaw)) * cos(degToRad(camera->pitch)) * 0.001;
	dir = v3f_normalize(dir);
	
	return m4f_lookAt(camera->pos, v3f_add(camera->pos, dir), CAMERA_UP);
}

#define NUM_OBJECTS 256

typedef struct R_VULKAN_SceneData R_VULKAN_SceneData;
struct R_VULKAN_SceneData
{
	M4F proj;
	M4F view;
	M4F model[NUM_OBJECTS];
	u32 tex_id[NUM_OBJECTS];
	u32 pad;
};

typedef struct R_VULKAN_Image R_VULKAN_Image;
struct R_VULKAN_Image
{
	VkSampler sampler;
	VkImage image;
	VkImageView view;
	VmaAllocation memory;
	VkExtent3D extent;
	VkFormat format;
};

typedef struct R_VULKAN_Buffer R_VULKAN_Buffer;
struct R_VULKAN_Buffer
{
	VkBuffer buffer;
	VmaAllocation alloc;
	VmaAllocationInfo info;
};

typedef struct R_VULKAN_FrameData R_VULKAN_FrameData;
struct R_VULKAN_FrameData
{
	VkCommandPool cmd_pool;
	VkCommandBuffer cmd_buffer;
	
	VkFence fence;
	VkSemaphore image_ready;
	VkSemaphore render_finished;
	
	R_VULKAN_Buffer scene_buffer;
	VkDescriptorSet scene_set;
};

typedef struct R_VULKAN_State R_VULKAN_State;
struct R_VULKAN_State
{
	OS_Handle vkdll;
	V2S last_frame_window_size;
	
	Arena *arena;
	VkInstance instance;
	
	VkPhysicalDevice phys_device;
	VkDevice device;
	u32 q_main_family;
	VkQueue q_main;
	VkSurfaceKHR surface;
	VkSurfaceFormatKHR surface_format;
	VkExtent2D surface_extent;
	VkSwapchainKHR swapchain;
	
	VmaAllocator vma;
	
	VkImage *swapchain_images;
	VkImageView *swapchain_image_views;
	u32 swapchain_image_count;
	
	VkViewport viewport;
	VkRect2D scissor;
	
	VkFormat draw_image_format;
	VkImage draw_image;
	VkImageView draw_image_view;
	VkExtent2D draw_image_extent;
	VmaAllocation draw_image_memory;
	
	VkFormat depth_image_format;
	VkImage depth_image;
	VkImageView depth_image_view;
	VkExtent2D depth_image_extent;
	VmaAllocation depth_image_memory;
	
	VkPipelineLayout hello_triangle_pipeline_layout;
	VkDescriptorPool hello_triangle_descriptor_pool;
	VkPipeline hello_triangle_pipeline;
	VkDescriptorSetLayout hello_triangle_descriptor_set_layout;
	
	R_VULKAN_FrameData frames[R_VULKAN_FRAMES];
	u32 current_frame_index;
	
	VkCommandPool im_cmd_pool;
	VkCommandBuffer im_cmd_buffer;
	VkFence im_fence;
	
	R_VULKAN_Image textures[10];
};

global R_VULKAN_State *r_vulkan_state;

function void r_vulkanAssertImpl(VkResult res)
{
	if(res != 0)
	{
		printf("VkResult is not great success; code: %d\n", res);
		INVALID_CODE_PATH();
	}
}

#if R_VULKAN_DEBUG
#define r_vulkanAssert(res) r_vulkanAssertImpl(res)
#else
#define r_vulkanAssert(res)
#endif

// instance
global PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
global PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
global PFN_vkCreateInstance vkCreateInstance;
global PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
global PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;

// device
global PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
global PFN_vkCreateDevice vkCreateDevice;
global PFN_vkDeviceWaitIdle vkDeviceWaitIdle;

// queue
global PFN_vkGetDeviceQueue vkGetDeviceQueue;
global PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2;

// surface / swapchain / image / views
global PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
global PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;

global PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
global PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;

global PFN_vkCreateImageView vkCreateImageView;
global PFN_vkCreateImage vkCreateImage;
global PFN_vkCreateSampler vkCreateSampler;

global PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;

global PFN_vkDestroyImageView vkDestroyImageView;
global PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;

// shaders / pipelines
global PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
global PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
global PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
global PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;

global PFN_vkCreateShaderModule vkCreateShaderModule;
global PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
global PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;

// command buffers
global PFN_vkCreateCommandPool vkCreateCommandPool;
global PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
global PFN_vkResetCommandBuffer vkResetCommandBuffer;

// sync
global PFN_vkCreateSemaphore vkCreateSemaphore;
global PFN_vkCreateFence vkCreateFence;
global PFN_vkWaitForFences vkWaitForFences;
global PFN_vkResetFences vkResetFences;

// commands
global PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR;
global PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR;
global PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
global PFN_vkEndCommandBuffer vkEndCommandBuffer;
global PFN_vkCmdBindPipeline vkCmdBindPipeline;
global PFN_vkCmdSetViewport vkCmdSetViewport;
global PFN_vkCmdSetScissor vkCmdSetScissor;
global PFN_vkCmdDraw vkCmdDraw;
global PFN_vkCmdBlitImage2KHR vkCmdBlitImage2KHR;
global PFN_vkCmdPipelineBarrier2KHR vkCmdPipelineBarrier2KHR;
global PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
global PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;

// submit
global PFN_vkQueueSubmit2KHR vkQueueSubmit2KHR;

// present
global PFN_vkQueuePresentKHR vkQueuePresentKHR;

function R_VULKAN_Buffer r_vulkan_createBuffer(u64 size, VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage)
{
	R_VULKAN_Buffer out = {0};
	
	VkBufferCreateInfo buffer_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = 0,
		.flags = 0,
		.size = size,
		.usage = buffer_usage,
		.sharingMode = 0,
		.queueFamilyIndexCount = 0,
	};
	
	VmaAllocationCreateInfo vma_info = {0};
	vma_info.usage = memory_usage;
	vma_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
	
	vmaCreateBuffer(r_vulkan_state->vma, &buffer_info, &vma_info, &out.buffer, &out.alloc, &out.info);
	return out;
}

function void r_vulkan_imBeginSubmit()
{
	vkResetFences(r_vulkan_state->device, 1, &r_vulkan_state->im_fence);
	vkResetCommandBuffer(r_vulkan_state->im_cmd_buffer, 0);
	
	VkCommandBufferBeginInfo info = {0};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	
	vkBeginCommandBuffer(r_vulkan_state->im_cmd_buffer, &info);
}

function void r_vulkan_imEndSubmit()
{
	vkEndCommandBuffer(r_vulkan_state->im_cmd_buffer);
	
	VkCommandBufferSubmitInfo buffer_submit_info = {0};
	buffer_submit_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	buffer_submit_info.commandBuffer = r_vulkan_state->im_cmd_buffer;
	
	VkSubmitInfo2 submit_info = {0};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	submit_info.pCommandBufferInfos = &buffer_submit_info;
	submit_info.commandBufferInfoCount = 1;
	
	vkQueueSubmit2KHR(r_vulkan_state->q_main, 1, &submit_info, r_vulkan_state->im_fence);    
	vkWaitForFences(r_vulkan_state->device, 1, &r_vulkan_state->im_fence, VK_TRUE, UINT64_MAX);
}

function R_VULKAN_Image r_vulkan_image(Bitmap bmp)
{
	R_VULKAN_Image out = {0};
	
	VmaAllocationCreateInfo alloc_info = {0};
	alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	
	VkFormat format = {0};
	format = VK_FORMAT_R8G8B8A8_SRGB;
	
	VkImageCreateInfo img_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = 0,
		
		.imageType = VK_IMAGE_TYPE_2D,
		
		.format = format,
		.extent = {.width = bmp.w, .height = bmp.h, .depth = 1},
		
		.mipLevels = 1,
		.arrayLayers = 1,
		
		.samples = VK_SAMPLE_COUNT_1_BIT,
		
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
	};
	
	vmaCreateImage(r_vulkan_state->vma, &img_info, &alloc_info, &out.image, &out.memory, 0);
	
	VkImageViewCreateInfo view_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = 0,
		
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.image = out.image,
		.format = img_info.format,
		.subresourceRange.baseMipLevel = 0,
		.subresourceRange.levelCount = 1,
		.subresourceRange.baseArrayLayer = 0,
		.subresourceRange.layerCount = 1,
		.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	};
	
	vkCreateImageView(r_vulkan_state->device, &view_info, 0, &out.view);
	
	size_t data_size = (size_t)img_info.extent.width * img_info.extent.height * img_info.extent.depth * 4;
	
	R_VULKAN_Buffer copy_buffer = r_vulkan_createBuffer(data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	
	memcpy(copy_buffer.info.pMappedData, bmp.data, data_size);
	
	r_vulkan_imBeginSubmit();
	
	{
		VkImageMemoryBarrier2 barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = 0,
			.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			.srcAccessMask = VK_ACCESS_NONE,
			.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = out.image,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = VK_REMAINING_MIP_LEVELS,
				.baseArrayLayer = 0,
				.layerCount = VK_REMAINING_ARRAY_LAYERS,
			},
		};
		
		VkDependencyInfo dep_info = {
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = 0,
			.dependencyFlags = 0,
			.memoryBarrierCount = 0,
			.pMemoryBarriers = 0,
			.bufferMemoryBarrierCount = 0,
			.pBufferMemoryBarriers = 0,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier,
		};
		
		vkCmdPipelineBarrier2KHR(r_vulkan_state->im_cmd_buffer, &dep_info);
	}
	
	VkBufferImageCopy copy_region = {
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		
		.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.imageSubresource.mipLevel = 0,
		.imageSubresource.baseArrayLayer = 0,
		.imageSubresource.layerCount = 1,
		.imageExtent = img_info.extent,
	};
	
	vkCmdCopyBufferToImage(r_vulkan_state->im_cmd_buffer, copy_buffer.buffer, out.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
	
	{
		VkImageMemoryBarrier2 barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = 0,
			.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = out.image,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = VK_REMAINING_MIP_LEVELS,
				.baseArrayLayer = 0,
				.layerCount = VK_REMAINING_ARRAY_LAYERS,
			},
		};
		
		VkDependencyInfo dep_info = {
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = 0,
			.dependencyFlags = 0,
			.memoryBarrierCount = 0,
			.pMemoryBarriers = 0,
			.bufferMemoryBarrierCount = 0,
			.pBufferMemoryBarriers = 0,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier,
		};
		
		vkCmdPipelineBarrier2KHR(r_vulkan_state->im_cmd_buffer, &dep_info);
	}
	
	r_vulkan_imEndSubmit();
	
	VkSamplerCreateInfo sampler_info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		
		// set anistoropyptosy
		
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE,
		
	};
	
	vkCreateSampler(r_vulkan_state->device, &sampler_info, 0, &out.sampler);
	
	//ykr_destroy_buffer(renderer->vma_allocator, &copy_data.copy_buffer);
	
	return out;
}

typedef struct R_VULKAN_ExtentionList R_VULKAN_ExtentionList;
struct R_VULKAN_ExtentionList
{
	char **v;
	u32 num;
	u32 cap;
};

function char **r_vulkan_pushExtention(R_VULKAN_ExtentionList *list, u32 num)
{
	char **out = list->v + list->num;
	list->num += num;
	if(list->num > list->cap)
	{
		INVALID_CODE_PATH();
	}
	return out;
}

// section: swapchain ===============================

function void r_vulkan_cleanupSwapchain()
{
	for (u32 i = 0; i < r_vulkan_state->swapchain_image_count; i++)
	{
		vkDestroyImageView(r_vulkan_state->device, r_vulkan_state->swapchain_image_views[i], 0);
	}
	
	//draw image
	vkDestroyImageView(r_vulkan_state->device, r_vulkan_state->draw_image_view, 0);
	vmaDestroyImage(r_vulkan_state->vma, r_vulkan_state->draw_image, r_vulkan_state->draw_image_memory);
	
	//depth image
	//vkDestroyImageView(r_vulkan_state->device, r_vulkan_state->depth_image.imageView, 0);
	//vmaDestroyImage(renderer->vma_allocator, renderer->depth_image.image, renderer->depth_image.allocation);
	
	//vkDestroySwapchainKHR(r_vulkan_state->device, r_vulkan_state->swapchain, 0);
}

function VkResult r_vulkan_createSwapchain(OS_Handle win)
{
	// swapchain
	VkSurfaceCapabilitiesKHR surface_cap = {0};
	VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(r_vulkan_state->phys_device, r_vulkan_state->surface, &surface_cap);
	r_vulkanAssert(res);
	
	if(surface_cap.currentExtent.width == UINT32_MAX)
	{
		V2S size = os_getWindowSize(win);
		surface_cap.currentExtent.width = size.x;
		surface_cap.currentExtent.height = size.y;
	}
	
	r_vulkan_state->surface_extent = surface_cap.currentExtent;
	
	u32 format_count = 0;
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(r_vulkan_state->phys_device, r_vulkan_state->surface, &format_count, 0);
	r_vulkanAssert(res);
	
	VkSurfaceFormatKHR *surface_format_list = pushArray(r_vulkan_state->arena, VkSurfaceFormatKHR, format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(r_vulkan_state->phys_device, r_vulkan_state->surface, &format_count, surface_format_list);
	
	// get format
	b32 found_format = 0;
	
	for(u32 i = 0; i < format_count; i++)
	{
		VkSurfaceFormatKHR *cur = surface_format_list + i;
		if((cur->format == VK_FORMAT_B8G8R8A8_SRGB) && (cur->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
		{
			r_vulkan_state->surface_format = *cur;
			found_format = 1;
		}
	}
	Assert(found_format);
	
	VkSwapchainCreateInfoKHR swapchain_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = 0,
		.flags = 0,
		.surface = r_vulkan_state->surface,
		.minImageCount = surface_cap.minImageCount,
		.imageFormat = r_vulkan_state->surface_format.format,
		.imageColorSpace = r_vulkan_state->surface_format.colorSpace,
		.imageExtent = r_vulkan_state->surface_extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices = &r_vulkan_state->q_main_family,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR,
		.oldSwapchain = r_vulkan_state->swapchain,
		.clipped = VK_TRUE,
	};
	
	res = vkCreateSwapchainKHR(r_vulkan_state->device, &swapchain_info, 0, &r_vulkan_state->swapchain);
	r_vulkanAssert(res);
	
	// allocate swapchain images & views
	{
		u32 image_count = 0;
		vkGetSwapchainImagesKHR(r_vulkan_state->device, r_vulkan_state->swapchain, &image_count, 0);
		
		r_vulkan_state->swapchain_images = pushArray(r_vulkan_state->arena, VkImage, image_count);
		r_vulkan_state->swapchain_image_views = pushArray(r_vulkan_state->arena, VkImageView, image_count);
		
		r_vulkan_state->swapchain_image_count = image_count;
	}
	
	res = vkGetSwapchainImagesKHR(r_vulkan_state->device, r_vulkan_state->swapchain, &r_vulkan_state->swapchain_image_count, r_vulkan_state->swapchain_images);
	r_vulkanAssert(res);
	
	for (u32 i = 0; i < r_vulkan_state->swapchain_image_count; i++)
	{
		VkImageViewCreateInfo image_view_create_info = { 
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = 0,
			.flags = 0,
			.image = r_vulkan_state->swapchain_images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = r_vulkan_state->surface_format.format,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
		
		res = vkCreateImageView(r_vulkan_state->device, &image_view_create_info, 0, &r_vulkan_state->swapchain_image_views[i]);
		
		r_vulkanAssert(res);
	}
	
	// draw image
	{
		r_vulkan_state->draw_image_format = VK_FORMAT_R16G16B16A16_SFLOAT; 
		r_vulkan_state->draw_image_extent = r_vulkan_state->surface_extent;
		
		VkImageCreateInfo image_create_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = 0,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = r_vulkan_state->draw_image_format,
			.extent = 
			{
				.width = r_vulkan_state->draw_image_extent.width,
				.height = r_vulkan_state->draw_image_extent.height,
				.depth = 1
			},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &r_vulkan_state->q_main_family,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};
		
		VmaAllocationCreateInfo image_alloc_info = {0};
		image_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		image_alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		
		res = vmaCreateImage(r_vulkan_state->vma, &image_create_info, &image_alloc_info, &r_vulkan_state->draw_image, &r_vulkan_state->draw_image_memory, 0);
		r_vulkanAssert(res);
		
		VkImageViewCreateInfo image_view_create_info = { 
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = 0,
			.flags = 0,
			.image = r_vulkan_state->draw_image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = r_vulkan_state->draw_image_format,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
		
		res = vkCreateImageView(r_vulkan_state->device, &image_view_create_info, 0, &r_vulkan_state->draw_image_view);
		
		r_vulkanAssert(res);
	}
	
	// depth image
	{
		r_vulkan_state->depth_image_format = VK_FORMAT_D32_SFLOAT; 
		VkImageCreateInfo image_create_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = 0,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = r_vulkan_state->depth_image_format,
			.extent = 
			{
				.width = r_vulkan_state->surface_extent.width,
				.height = r_vulkan_state->surface_extent.height,
				.depth = 1
			},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &r_vulkan_state->q_main_family,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};
		
		VmaAllocationCreateInfo image_alloc_info = {0};
		image_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		image_alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		
		res = vmaCreateImage(r_vulkan_state->vma, &image_create_info, &image_alloc_info, &r_vulkan_state->depth_image, &r_vulkan_state->depth_image_memory, 0);
		r_vulkanAssert(res);
		
		VkImageViewCreateInfo image_view_create_info = { 
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = 0,
			.flags = 0,
			.image = r_vulkan_state->depth_image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = r_vulkan_state->depth_image_format,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
		
		res = vkCreateImageView(r_vulkan_state->device, &image_view_create_info, 0, &r_vulkan_state->depth_image_view);
		r_vulkanAssert(res);
		
	}
	
	r_vulkan_state->viewport.x = 0;
	r_vulkan_state->viewport.y = 0;
	r_vulkan_state->viewport.width = r_vulkan_state->surface_extent.width;
	r_vulkan_state->viewport.height = r_vulkan_state->surface_extent.height;
	r_vulkan_state->viewport.minDepth = 0;
	r_vulkan_state->viewport.maxDepth = 1;
	
	r_vulkan_state->scissor.offset = (VkOffset2D){0};
	r_vulkan_state->scissor.extent = r_vulkan_state->surface_extent;
	
	return res;
}

function VkResult r_vulkan_recreateSwapchain(OS_Handle win)
{
	//printf("recreated swapchain\n");
	vkDeviceWaitIdle(r_vulkan_state->device);
	r_vulkan_cleanupSwapchain();
	r_vulkan_createSwapchain(win);
	
	return 0;
}

// ======================================

function VkDescriptorSet r_vulkan_allocDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout* layouts)
{
	VkDescriptorSetAllocateInfo info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = 0,
		.descriptorPool = pool,
		.descriptorSetCount = 1,
		.pSetLayouts = layouts,
	};
	
	VkDescriptorSet set = {0};
	VkResult res = vkAllocateDescriptorSets(r_vulkan_state->device, &info, &set);
	r_vulkanAssert(res);
	
	return set;
}

/*
void ubo_update(VmaAllocator allocator, YkBuffer* buffer, void* ubo, size_t size)
{
    void* data = 0;
    vmaMapMemory(allocator, buffer->alloc, &data);

    memcpy(data, ubo, size);

    vmaUnmapMemory(allocator, buffer->alloc);
}
*/

function void r_vulkanInnit(OS_Handle win)
{
	Arena *arena = arenaAlloc();
	r_vulkan_state = pushArray(arena, R_VULKAN_State, 1);
	r_vulkan_state->arena = arena;
	
	r_vulkan_state->vkdll = os_vulkan_loadLibrary();
	
	// load instance functions
	vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)os_loadFunction(r_vulkan_state->vkdll, "vkGetInstanceProcAddr");
	
	// instance / device / queues
	vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(0, "vkCreateInstance");
	vkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)vkGetInstanceProcAddr(0, "vkEnumerateInstanceVersion");
	
	VkResult res;
	
	// instance creation
	{
		u32 version = 0;
		
		vkEnumerateInstanceVersion(&version);
		
		printf("\nInstance Version: %d.%d.%d\n\n"
									,VK_VERSION_MAJOR(version)
									,VK_VERSION_MINOR(version)
									,VK_VERSION_PATCH(version)
									);
		
		char *validation_layers[1] = {0};
		u32 validation_layers_num = 0;
		
		R_VULKAN_ExtentionList extentions = {
			.v = pushArray(arena, char*, 10),
			.cap = 10,
		};
		
		{
			u32 platform_ext_num = os_vulkan_getPlatformExtentions(0);
			char **platform_ext = r_vulkan_pushExtention(&extentions, platform_ext_num);
			os_vulkan_getPlatformExtentions(platform_ext);
		}
		{
			char **ext = r_vulkan_pushExtention(&extentions, 1);
			*ext = VK_KHR_SURFACE_EXTENSION_NAME;
		}
#if defined(OS_APPLE)
		{
			char **ext = r_vulkan_pushExtention(&extentions, 1);
			*ext = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
		}
		
		{
			char **ext = r_vulkan_pushExtention(&extentions, 1);
			*ext = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
		}
#endif
#if R_VULKAN_DEBUG
		
		validation_layers[validation_layers_num++] = (char*){
			"VK_LAYER_KHRONOS_validation"
		};
		
		{
			char **ext = r_vulkan_pushExtention(&extentions, 1);
			*ext = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		}
		
#endif
		
		VkApplicationInfo app_info = {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pNext = 0,
			.pApplicationName = "Alfia",
			.applicationVersion = 1,
			.pEngineName = "Saoirse",
			.engineVersion = 1,
			.apiVersion = VK_API_VERSION_1_2
		};
		
		VkInstanceCreateInfo inst_info = {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = 0,
			
#if defined(OS_APPLE)
			.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
#else
			.flags = 0,
#endif
			
			.pApplicationInfo = &app_info,
			.enabledLayerCount = validation_layers_num,
			.ppEnabledLayerNames = validation_layers,
			.enabledExtensionCount = extentions.num,
			.ppEnabledExtensionNames = extentions.v
		};
		
		res = vkCreateInstance(&inst_info, 0, &r_vulkan_state->instance);
		r_vulkanAssert(res);
	}
	
	// load more instance functions
	vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)vkGetInstanceProcAddr(r_vulkan_state->instance, "vkEnumeratePhysicalDevices");
	vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)vkGetInstanceProcAddr(r_vulkan_state->instance, "vkGetPhysicalDeviceProperties");
	vkCreateDevice = (PFN_vkCreateDevice)vkGetInstanceProcAddr(r_vulkan_state->instance, "vkCreateDevice");
	vkGetPhysicalDeviceFeatures2 = (PFN_vkCreateDevice)vkGetInstanceProcAddr(r_vulkan_state->instance, "vkGetPhysicalDeviceFeatures2");
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR = vkGetInstanceProcAddr(r_vulkan_state->instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	
	vkGetPhysicalDeviceSurfaceFormatsKHR = vkGetInstanceProcAddr(r_vulkan_state->instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
	
	vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)vkGetInstanceProcAddr(r_vulkan_state->instance, "vkGetDeviceProcAddr");
	// device selection
	{
		u32 count = 0;
		res = vkEnumeratePhysicalDevices(r_vulkan_state->instance, &count, 0);
		r_vulkanAssert(res);
		
		VkPhysicalDevice *phys_devices = pushArray(arena, VkPhysicalDevice, count);
		VkPhysicalDeviceFeatures2 *features = pushArray(arena, VkPhysicalDeviceFeatures2, count);
		
		res = vkEnumeratePhysicalDevices(r_vulkan_state->instance, &count, phys_devices);
		
		r_vulkanAssert(res);
		
		typedef struct GpuStat
		{
			b32 discrete;
			b32 good;
		}GpuStat;
		
		GpuStat *gpus = pushArray(arena, GpuStat, count);
		
		for(u32 i = 0; i < count; i ++)
		{
			// we care for
			//dyn rendering, sync2, bda and descr indexing
			VkPhysicalDeviceDynamicRenderingFeaturesKHR dyn_ren = 
			{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
			};
			
			VkPhysicalDeviceSynchronization2FeaturesKHR sync2 = 
			{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
				.pNext = &dyn_ren
			};
			
			VkPhysicalDeviceVulkan12Features vk12_feat = {
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
				.pNext = &sync2
			};
			
			features[i].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			features[i].pNext = &vk12_feat;
			
			VkPhysicalDeviceProperties props = {0};
			vkGetPhysicalDeviceProperties(phys_devices[i], &props);
			
			// print gpu details
			printf("--------index %d--------\n", i);
			printf("Name: %s\n",props.deviceName);
			printf("Driver Version: %u.%u.%u\n",
										VK_VERSION_MAJOR(props.driverVersion),
										VK_VERSION_MINOR(props.driverVersion),
										VK_VERSION_PATCH(props.driverVersion));
			
			printf("Api Version: %u.%u.%u\n",
										VK_VERSION_MAJOR(props.apiVersion),
										VK_VERSION_MINOR(props.apiVersion),
										VK_VERSION_PATCH(props.apiVersion));
			printf("------------------------\n\n");
			
			vkGetPhysicalDeviceFeatures2(phys_devices[i], &features[i]);
			
			if (dyn_ren.dynamicRendering && sync2.synchronization2 && vk12_feat.bufferDeviceAddress && vk12_feat.descriptorIndexing)
			{
				gpus[i].good = 1;
			}
			
			if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				gpus[i].discrete = 1;
			}
		}
		
		printf("Selected GPU\n");
		b32 found_gpu = 0;
		
		for(u32 i = 0; i < count; i ++)
		{
			printf("=========\n");
			printf("index: %d\n", i);
			printf("discrete: %d\n", gpus[i].discrete);
			printf("good: %d\n", gpus[i].good);
			printf("========\n");
			
			if(gpus[i].discrete && gpus[i].good)
			{
				found_gpu = 1;
				r_vulkan_state->phys_device = phys_devices[i];
				break;
			}
		}
		
		if(!found_gpu)
		{
			for(u32 i = 0; i < count; i ++)
			{
				if(gpus[i].good)
				{
					found_gpu = 1;
					r_vulkan_state->phys_device = phys_devices[i];
					break;
				}
			}
		}
		
		if(!found_gpu)
		{
			printf("Quitting. No good gpu\n");
			INVALID_CODE_PATH();
		}
	}
	
	res = os_vulkan_createSurface(win, r_vulkan_state->instance, &r_vulkan_state->surface);
	r_vulkanAssert(res);
	
	// logical device
	{
		char* device_extention_names[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
			VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
			VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
			VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME,
#if defined(OS_APPLE)
			VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
#endif
		};
		
		f32 q_priorities[1] = {1.f};
		
		VkDeviceQueueCreateInfo q_info = 
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = r_vulkan_state->q_main_family,
			.queueCount = 1,
			.pQueuePriorities = q_priorities,
		};
		
		VkPhysicalDeviceSynchronization2FeaturesKHR sync2 = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
			.synchronization2 = VK_TRUE,
		};
		
		VkPhysicalDeviceBufferDeviceAddressFeaturesKHR buffer_device_address = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR,
			.bufferDeviceAddress = VK_TRUE,
			.pNext = &sync2,
		};
		
		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature = 
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
			.dynamicRendering = VK_TRUE,
			.pNext = &buffer_device_address,
		};
		
		VkDeviceCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &dynamic_rendering_feature,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &q_info,
			.enabledLayerCount = 0,
			.ppEnabledLayerNames = 0,
			.enabledExtensionCount = arrayLen(device_extention_names),
			.ppEnabledExtensionNames = device_extention_names,
			.pEnabledFeatures = 0
		};
		
		res = vkCreateDevice(r_vulkan_state->phys_device, &info, 0, &r_vulkan_state->device);
		r_vulkanAssert(res);
	}
	
	// load device functions
	
	// queue
	vkDeviceWaitIdle = vkGetDeviceProcAddr(r_vulkan_state->device, "vkDeviceWaitIdle");
	vkGetDeviceQueue = vkGetDeviceProcAddr(r_vulkan_state->device, "vkGetDeviceQueue");
	vkGetPhysicalDeviceFeatures2 = vkGetDeviceProcAddr(r_vulkan_state->device, "vkGetPhysicalDeviceFeatures2");
	
	//swapchain / image / views
	vkCreateSwapchainKHR = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCreateSwapchainKHR");
	vkGetSwapchainImagesKHR = vkGetDeviceProcAddr(r_vulkan_state->device, "vkGetSwapchainImagesKHR");
	
	vkCreateImageView = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCreateImageView");
	vkCreateImage = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCreateImage");
	vkCreateSampler = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCreateSampler");
	
	vkAcquireNextImageKHR = vkGetDeviceProcAddr(r_vulkan_state->device, "vkAcquireNextImageKHR");
	
	vkDestroyImageView = vkGetDeviceProcAddr(r_vulkan_state->device, "vkDestroyImageView");
	vkDestroySwapchainKHR = vkGetDeviceProcAddr(r_vulkan_state->device, "vkDestroySwapchainKHR");
	
	// shaders / pipelines
	vkCreateDescriptorSetLayout = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCreateDescriptorSetLayout");
	vkCreateDescriptorPool = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCreateDescriptorPool");
	vkAllocateDescriptorSets = vkGetDeviceProcAddr(r_vulkan_state->device, "vkAllocateDescriptorSets");
	vkUpdateDescriptorSets = vkGetDeviceProcAddr(r_vulkan_state->device, "vkUpdateDescriptorSets");
	
	vkCreateShaderModule = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCreateShaderModule");
	vkCreatePipelineLayout = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCreatePipelineLayout");
	vkCreateGraphicsPipelines = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCreateGraphicsPipelines");
	
	// command buffers
	vkCreateCommandPool = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCreateCommandPool");
	vkAllocateCommandBuffers = vkGetDeviceProcAddr(r_vulkan_state->device, "vkAllocateCommandBuffers");
	vkResetCommandBuffer = vkGetDeviceProcAddr(r_vulkan_state->device, "vkResetCommandBuffer");
	
	// sync
	vkCreateSemaphore = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCreateSemaphore");
	vkCreateFence = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCreateFence");
	vkWaitForFences = vkGetDeviceProcAddr(r_vulkan_state->device, "vkWaitForFences");
	vkResetFences = vkGetDeviceProcAddr(r_vulkan_state->device, "vkResetFences");
	
	// commands
	vkCmdBeginRenderingKHR = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdBeginRenderingKHR");
	vkCmdEndRenderingKHR = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdEndRenderingKHR");
	vkBeginCommandBuffer = vkGetDeviceProcAddr(r_vulkan_state->device, "vkBeginCommandBuffer");
	vkEndCommandBuffer = vkGetDeviceProcAddr(r_vulkan_state->device, "vkEndCommandBuffer");
	vkCmdBindPipeline = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdBindPipeline");
	vkCmdSetViewport = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdSetViewport");
	vkCmdSetScissor = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdSetScissor");
	vkCmdDraw = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdDraw");
	vkCmdBlitImage2KHR = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdBlitImage2KHR");
	vkCmdPipelineBarrier2KHR = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdPipelineBarrier2KHR");
	vkCmdBindDescriptorSets = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdBindDescriptorSets");
	vkCmdCopyBufferToImage = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdCopyBufferToImage");
	
	// submit
	vkQueueSubmit2KHR = vkGetDeviceProcAddr(r_vulkan_state->device, "vkQueueSubmit2KHR");
	
	// present
	vkQueuePresentKHR = vkGetDeviceProcAddr(r_vulkan_state->device, "vkQueuePresentKHR");
	
	// vma allocator
	{
		VmaVulkanFunctions vma_vulkan_func = {
			.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
			.vkGetDeviceProcAddr = vkGetDeviceProcAddr
		};
		
		VmaAllocatorCreateInfo allocator_info = {
			.physicalDevice = r_vulkan_state->phys_device,
			.device = r_vulkan_state->device,
			.instance = r_vulkan_state->instance,
			.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
			.pVulkanFunctions = &vma_vulkan_func,
		};
		
		res = vmaCreateAllocator(&allocator_info, &r_vulkan_state->vma);
		r_vulkanAssert(res);
	}
	
	// queues
	{
		// NOTE(mizu): Consider doing it properly at some point.
		r_vulkan_state->q_main_family = 0;
		vkGetDeviceQueue(r_vulkan_state->device, r_vulkan_state->q_main_family, 0, &r_vulkan_state->q_main);
	}
	
	//vkCmdPipelineBarrier2KHR = (PFN_vkCmdPipelineBarrier2KHR)os_loadFunction(vkdll, "vkCmdPipelineBarrier2KHR");
	
	r_vulkan_createSwapchain(win);
	
	// hello triangle pipeline 
	{
		// pipeline shader stage
		FileData vert_src = readFile(r_vulkan_state->arena, "hello_triangle.vert.spv", FILE_TYPE_BINARY);
		
		VkShaderModuleCreateInfo vert_shader_module_info = {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = vert_src.size,
			.pCode = vert_src.bytes,
		};
		
		VkShaderModule vert_module = {0};
		res = vkCreateShaderModule(r_vulkan_state->device, &vert_shader_module_info, 0, &vert_module);
		r_vulkanAssert(res);
		
		FileData frag_src = readFile(r_vulkan_state->arena, "hello_triangle.frag.spv", FILE_TYPE_BINARY);
		
		VkShaderModuleCreateInfo frag_shader_module_info = {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = frag_src.size,
			.pCode = frag_src.bytes,
		};
		
		VkShaderModule frag_module = {0};
		res = vkCreateShaderModule(r_vulkan_state->device, &frag_shader_module_info, 0, &frag_module);
		r_vulkanAssert(res);
		
		VkPipelineShaderStageCreateInfo shader_stages[2] = {
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext = 0,
				.flags = 0,
				.stage = VK_SHADER_STAGE_VERTEX_BIT,
				.module = vert_module,
				.pName = "main",
				.pSpecializationInfo = 0,
			},
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext = 0,
				.flags = 0,
				.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
				.module = frag_module,
				.pName = "main",
				.pSpecializationInfo = 0,
			},
		};
		
		// vertex input
		VkPipelineVertexInputStateCreateInfo vertex_input = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.pNext = 0,
			.flags = 0,
			.vertexBindingDescriptionCount = 0,
			.pVertexBindingDescriptions = 0,
			.vertexAttributeDescriptionCount = 0,
			.pVertexAttributeDescriptions = 0
		};
		
		// Input assembly
		
		VkPipelineInputAssemblyStateCreateInfo input_assembly = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.pNext = 0,
			.flags = 0,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE,
		};
		
		// view port state
		VkPipelineViewportStateCreateInfo viewport = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.scissorCount = 1,
		};
		
		// rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = 0,
			.flags = 0,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			//.cullMode = VK_CULL_MODE_BACK_BIT,
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0,
			.depthBiasClamp = 0,
			.depthBiasSlopeFactor = 0,
			.lineWidth = 1.f,
		};
		
		// multisample
		VkPipelineMultisampleStateCreateInfo multisample = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.pNext = 0,
			.flags = 0,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
			.minSampleShading = 0,
			.pSampleMask = 0,
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE,
		};
		
		// depth stencil stage
		VkPipelineDepthStencilStateCreateInfo depth_stencil = {
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.front = {0},
			.back = {0},
			.minDepthBounds = 0.f,
			.maxDepthBounds = 0.f,
		};
		
		// color blend state
		VkPipelineColorBlendAttachmentState color_blend_attatchment = {
			.blendEnable = VK_TRUE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		};
		
		VkPipelineColorBlendStateCreateInfo color_blending = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &color_blend_attatchment,
			.blendConstants[0] = 0.0f,
			.blendConstants[1] = 0.0f,
			.blendConstants[2] = 0.0f,
			.blendConstants[3] = 0.0f,
		};
		
		// Dynamic state
		VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = 2,
			.pDynamicStates = dynamic_states,
		};
		
		// descriptor set layout
		
		VkDescriptorSetLayoutBinding descriptor_bindings[2] = {
			[0] = {
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
				.pImmutableSamplers = 0,
			},
			[1] = {
				.binding = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = 10,
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				.pImmutableSamplers = 0,
			},
			
		};
		
		VkDescriptorSetLayoutCreateInfo descriptor_layout_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = 0,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
			.bindingCount = 2,
			.pBindings = descriptor_bindings,
		};
		
		res = vkCreateDescriptorSetLayout(r_vulkan_state->device,
																																				&descriptor_layout_info,
																																				0,
																																				&r_vulkan_state->hello_triangle_descriptor_set_layout);
		r_vulkanAssert(res);
		
		// pipeline layout
		VkPipelineLayoutCreateInfo layout_info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &r_vulkan_state->hello_triangle_descriptor_set_layout,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = 0,
		};
		
		vkCreatePipelineLayout(r_vulkan_state->device, &layout_info, 0, &r_vulkan_state->hello_triangle_pipeline_layout);
		
		// pipeline
		
		VkPipelineRenderingCreateInfo render_info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &r_vulkan_state->draw_image_format,
			.depthAttachmentFormat = r_vulkan_state->depth_image_format,
			.stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
		};
		
		VkGraphicsPipelineCreateInfo pipeline_create_info = 
		{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &render_info,
			.flags = 0,
			.stageCount = 2,
			.pStages = shader_stages,
			.pVertexInputState = &vertex_input,
			.pInputAssemblyState = &input_assembly,
			.pTessellationState = 0,
			.pViewportState = &viewport,
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisample,
			.pDepthStencilState = &depth_stencil,
			.pColorBlendState = &color_blending,
			.pDynamicState = &dynamic_state_create_info,
			.layout = r_vulkan_state->hello_triangle_pipeline_layout,
			.renderPass = 0,
			.subpass = 0,
			.basePipelineHandle = 0,
			.basePipelineIndex = 0
		};
		
		res = vkCreateGraphicsPipelines(r_vulkan_state->device, 0, 1, &pipeline_create_info, 0, &r_vulkan_state->hello_triangle_pipeline);
		r_vulkanAssert(res);
	}
	
	// cmd buffers
	{
		VkCommandPoolCreateInfo cmd_pool_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = 0,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = r_vulkan_state->q_main_family,
		};
		
		for(s32 i = 0; i < R_VULKAN_FRAMES; i++)
		{
			R_VULKAN_FrameData *frame = r_vulkan_state->frames + i;
			res = vkCreateCommandPool(r_vulkan_state->device, &cmd_pool_info, 0, &frame->cmd_pool);
			
			r_vulkanAssert(res);
			
			VkCommandBufferAllocateInfo cmd_buffer_info = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.pNext = 0,
				.commandPool = frame->cmd_pool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};
			
			res = vkAllocateCommandBuffers(r_vulkan_state->device, &cmd_buffer_info, &frame->cmd_buffer);
			r_vulkanAssert(res);
		}
		
		// im commands
		{
			res = vkCreateCommandPool(r_vulkan_state->device, &cmd_pool_info, 0, &r_vulkan_state->im_cmd_pool);
			r_vulkanAssert(res);
			
			VkCommandBufferAllocateInfo cmd_buffer_info = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.pNext = 0,
				.commandPool = r_vulkan_state->im_cmd_pool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};
			
			res = vkAllocateCommandBuffers(r_vulkan_state->device, &cmd_buffer_info, &r_vulkan_state->im_cmd_buffer);
			r_vulkanAssert(res);
		}
	}
	
	// sync objects
	{
		VkSemaphoreCreateInfo semaphore_create_info = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = 0,
			.flags = 0,
		};
		
		VkFenceCreateInfo fence_create_info = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = 0,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};
		
		for(s32 i = 0; i < R_VULKAN_FRAMES; i++)
		{
			R_VULKAN_FrameData *frame = r_vulkan_state->frames + i;
			
			res = vkCreateSemaphore(r_vulkan_state->device, &semaphore_create_info, 0, &frame->image_ready);
			r_vulkanAssert(res);
			
			res = vkCreateSemaphore(r_vulkan_state->device, &semaphore_create_info, 0, &frame->render_finished);
			r_vulkanAssert(res);
			
			res = vkCreateFence(r_vulkan_state->device, &fence_create_info, 0, &frame->fence);
			r_vulkanAssert(res);
		}
		
		res = vkCreateFence(r_vulkan_state->device, &fence_create_info, 0, &r_vulkan_state->im_fence);
		r_vulkanAssert(res);
		
	}
	
	// resources
	{
		{
			Bitmap bmp = bitmap(str8_lit("scratch/ell"));
			r_vulkan_state->textures[0] = r_vulkan_image(bmp);
		}
		{
			Bitmap bmp = bitmap(str8_lit("scratch/marhall"));
			r_vulkan_state->textures[1] = r_vulkan_image(bmp);
		}
		{
			Bitmap bmp = bitmap(str8_lit("scratch/maruko"));
			r_vulkan_state->textures[2] = r_vulkan_image(bmp);
		}
		{
			Bitmap bmp = bitmap(str8_lit("scratch/ankha.png"));
			r_vulkan_state->textures[3] = r_vulkan_image(bmp);
		}
	}
	
	// descriptor set
	{
		VkDescriptorPoolSize sizes[2] = { 
			[0] = {
				.descriptorCount = R_VULKAN_FRAMES,
				.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			},
			[1] = {
				.descriptorCount = R_VULKAN_FRAMES,
				.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			},
		};
		
		VkDescriptorPoolCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = 0,
			.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
			.maxSets = R_VULKAN_FRAMES,
			.poolSizeCount = arrayLen(sizes),
			.pPoolSizes = sizes,
		};
		
		res = vkCreateDescriptorPool(r_vulkan_state->device, &info, 0, &r_vulkan_state->hello_triangle_descriptor_pool);
		r_vulkanAssert(res);
		
		// scene descriptor
		for (u32 i = 0; i < R_VULKAN_FRAMES; i++)
		{
			R_VULKAN_FrameData *frame = r_vulkan_state->frames + i;
			frame->scene_buffer = r_vulkan_createBuffer(sizeof(R_VULKAN_SceneData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			frame->scene_set = r_vulkan_allocDescriptorSet(r_vulkan_state->hello_triangle_descriptor_pool, &r_vulkan_state->hello_triangle_descriptor_set_layout);
			
			VkDescriptorBufferInfo buffer_info = {
				.buffer = frame->scene_buffer.buffer,
				.offset = 0,
				.range = sizeof(R_VULKAN_SceneData),
			};
			
			VkDescriptorImageInfo img_info = {
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.imageView = r_vulkan_state->textures[0].view,
				.sampler = r_vulkan_state->textures[0].sampler,
			};
			
			VkDescriptorImageInfo img_info2 = {
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.imageView = r_vulkan_state->textures[1].view,
				.sampler = r_vulkan_state->textures[1].sampler,
			};
			
			VkDescriptorImageInfo img_info3 = {
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.imageView = r_vulkan_state->textures[2].view,
				.sampler = r_vulkan_state->textures[2].sampler,
			};
			
			VkDescriptorImageInfo img_info4 = {
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.imageView = r_vulkan_state->textures[3].view,
				.sampler = r_vulkan_state->textures[3].sampler,
			};
			
			VkWriteDescriptorSet writes[5] = {
				[0] = {
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = frame->scene_set,
					.dstBinding = 0,
					.dstArrayElement = 0,
					.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
					.descriptorCount = 1,
					.pBufferInfo = &buffer_info,
				},
				[1] = {
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = frame->scene_set,
					.dstBinding = 1,
					.dstArrayElement = 0,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = 1,
					.pImageInfo = &img_info,
				},
				[2] = {
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = frame->scene_set,
					.dstBinding = 1,
					.dstArrayElement = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = 1,
					.pImageInfo = &img_info2,
				},
				[3] = {
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = frame->scene_set,
					.dstBinding = 1,
					.dstArrayElement = 2,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = 1,
					.pImageInfo = &img_info3,
				},
				[4] = {
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = frame->scene_set,
					.dstBinding = 1,
					.dstArrayElement = 3,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = 1,
					.pImageInfo = &img_info4,
				},
				
			};
			
			vkUpdateDescriptorSets(r_vulkan_state->device, 5, writes, 0, 0);
		}
		
		// texture descriptors
		
	}
	//arena_temp_end(&scratch);
}

function void r_vulkan_beginRendering()
{
	
}

function R_VULKAN_FrameData *r_vulkan_getCurrentFrame()
{
	return r_vulkan_state->frames + r_vulkan_state->current_frame_index;
}

function void r_vulkan_endRendering(OS_Handle win)
{
	r_vulkan_state->current_frame_index = (r_vulkan_state->current_frame_index + 1) % R_VULKAN_FRAMES;
	r_vulkan_state->last_frame_window_size = os_getWindowSize(win);
}

function void r_vulkanRender(OS_Handle win, OS_EventList *events, f32 delta)
{
	//printf("%f %f\n", r_vulkan_state->viewport.width, r_vulkan_state->viewport.height);
	R_VULKAN_FrameData *frame = r_vulkan_getCurrentFrame();
	
	VkResult res = vkWaitForFences(r_vulkan_state->device, 1, &frame->fence, VK_TRUE, UINT64_MAX);
	r_vulkanAssert(res);
	
	u32 image_index = -1;
	res = vkAcquireNextImageKHR(r_vulkan_state->device, r_vulkan_state->swapchain, UINT64_MAX, frame->image_ready,
																													0, &image_index);
	
	if((res == VK_ERROR_OUT_OF_DATE_KHR) || (res == VK_SUBOPTIMAL_KHR)) 
	{
		res = r_vulkan_createSwapchain(win);
		r_vulkanAssert(res);
		
		return;
	}
	
	res = vkResetFences(r_vulkan_state->device, 1, &frame->fence);
	r_vulkanAssert(res);
	
	VkImage swapchain_image = r_vulkan_state->swapchain_images[image_index]; 
	VkImage draw_image = r_vulkan_state->draw_image;
	
	//VkImage depth_image = r_vulkan_state->depth_image; 
	
	VkCommandBufferBeginInfo cmd_buffer_begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = 0,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = 0,
	};
	
	res = vkBeginCommandBuffer(frame->cmd_buffer, &cmd_buffer_begin_info);
	
	// transition from undefined to color
	{
		VkImageMemoryBarrier2 barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = 0,
			.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			.srcAccessMask = VK_ACCESS_NONE,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = draw_image,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = VK_REMAINING_MIP_LEVELS,
				.baseArrayLayer = 0,
				.layerCount = VK_REMAINING_ARRAY_LAYERS,
			},
		};
		
		VkDependencyInfo dep_info = {
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = 0,
			.dependencyFlags = 0,
			.memoryBarrierCount = 0,
			.pMemoryBarriers = 0,
			.bufferMemoryBarrierCount = 0,
			.pBufferMemoryBarriers = 0,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier,
		};
		
		vkCmdPipelineBarrier2KHR(frame->cmd_buffer, &dep_info);
	}
	
	// draw hello triangle
	
	VkRenderingAttachmentInfo color_attachment = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.pNext = 0,
		.imageView = r_vulkan_state->draw_image_view,
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.resolveMode = VK_RESOLVE_MODE_NONE,
		.resolveImageView = 0,
		.resolveImageLayout = 0,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = {{{0.02, 0, 0.02, 1}}},
	};
	
	VkRenderingAttachmentInfo depth_attachment = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
		.imageView = r_vulkan_state->depth_image_view,
		.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.clearValue.depthStencil.depth = 1.f,
	};
	
	VkRenderingInfoKHR rendering_info = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.pNext = 0,
		.flags = 0,
		.renderArea = r_vulkan_state->scissor,
		.layerCount = 1,
		.viewMask = 0,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment,
		.pDepthAttachment = &depth_attachment,
		.pStencilAttachment = 0,
	};
	
	vkCmdBeginRenderingKHR(frame->cmd_buffer, &rendering_info);
	
	vkCmdBindPipeline(frame->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r_vulkan_state->hello_triangle_pipeline);
	vkCmdSetViewport(frame->cmd_buffer, 0, 1, &r_vulkan_state->viewport);
	vkCmdSetScissor(frame->cmd_buffer, 0, 1, &r_vulkan_state->scissor);
	
	vkCmdBindDescriptorSets(frame->cmd_buffer,
																									VK_PIPELINE_BIND_POINT_GRAPHICS,
																									r_vulkan_state->hello_triangle_pipeline_layout,
																									0,
																									1,
																									&frame->scene_set,
																									0,
																									0);
	
	static f32 counter = 0;
	counter += delta;
	V2S win_size = os_getWindowSize(win);
	
	static Camera camera = {
		.yaw = 90,
	};
	
	camUpdate(&camera, events, delta);
	
	//M4F view = m4f_lookAt((V3F){.z = -3}, (V3F){.z = 0}, (V3F){.y = 1});
	
	R_VULKAN_SceneData data = {
		.proj = m4f_perspective(degToRad(90), win_size.x * 1.f / win_size.y, 0.01, 1000),
		.view = camGetView(&camera),
		//.view = view,
		//.model[0] = m4f_mul(m4f_translate(v3f(-3, 0, 3)), m4f_rotate(v3f(0, 1, 0), counter)),
		//.model = m4f_rotate(v3f(0, 1, 0), counter),
		//.model = m4f(1)
		//.model = m4f_translate(v3f(0, 0.2, 0))
	};
	
	srand(0);
	for(s32 i = 0; i < NUM_OBJECTS; i++)
	{
		M4F trans = m4f_translate(v3f(i * rand() % 16, i * rand() % 16, i * rand() % 16));
		data.model[i] = m4f_mul(trans, m4f_rotate(v3f(0, 1, 0), counter * (rand() % 16)));
		data.tex_id[i] = i % 4;
	}
	
	void* mappedData;
	vmaMapMemory(r_vulkan_state->vma, frame->scene_buffer.alloc, &mappedData);
	memcpy(mappedData, &data, sizeof(R_VULKAN_SceneData));
	vmaUnmapMemory(r_vulkan_state->vma, frame->scene_buffer.alloc);
	
	vkCmdDraw(frame->cmd_buffer, 6, NUM_OBJECTS, 0, 0);
	
	vkCmdEndRenderingKHR(frame->cmd_buffer);
	
	// draw image: color ---> transfer src 
	{
		VkImageMemoryBarrier2 barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
			.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = draw_image,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = VK_REMAINING_MIP_LEVELS,
				.baseArrayLayer = 0,
				.layerCount = VK_REMAINING_ARRAY_LAYERS,
			},
		};
		
		VkDependencyInfo dep_info = {
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = 0,
			.dependencyFlags = 0,
			.memoryBarrierCount = 0,
			.pMemoryBarriers = 0,
			.bufferMemoryBarrierCount = 0,
			.pBufferMemoryBarriers = 0,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier,
		};
		
		vkCmdPipelineBarrier2KHR(frame->cmd_buffer, &dep_info);
	}
	
	// swapchain: undefined ---> transfer dst 
	{
		VkImageMemoryBarrier2 barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = 0,
			.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			.srcAccessMask = VK_ACCESS_NONE,
			.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = swapchain_image,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = VK_REMAINING_MIP_LEVELS,
				.baseArrayLayer = 0,
				.layerCount = VK_REMAINING_ARRAY_LAYERS,
			},
		};
		
		VkDependencyInfo dep_info = {
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = 0,
			.dependencyFlags = 0,
			.memoryBarrierCount = 0,
			.pMemoryBarriers = 0,
			.bufferMemoryBarrierCount = 0,
			.pBufferMemoryBarriers = 0,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier,
		};
		
		vkCmdPipelineBarrier2KHR(frame->cmd_buffer, &dep_info);
	}
	
	// copy draw image ---> swapchain
	{
		VkImageBlit2 regions = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
			.pNext = 0,
			.srcSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.srcOffsets[0] = {
				0, 
				0, 
				0
			},
			.srcOffsets[1] = {
				r_vulkan_state->draw_image_extent.width,
				r_vulkan_state->draw_image_extent.height,
				1,
			},
			.dstSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.dstOffsets[0] = {
				0,
				0,
				0
			},
			.dstOffsets[1] = {
				r_vulkan_state->surface_extent.width,
				r_vulkan_state->surface_extent.height,
				1
			},
		};
		
		VkBlitImageInfo2 blit_info = {
			.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
			.pNext = 0,
			.srcImage = draw_image,
			.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.dstImage = swapchain_image,
			.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.regionCount = 1,
			.pRegions = &regions,
			.filter = VK_FILTER_LINEAR,
		};
		
		vkCmdBlitImage2KHR(frame->cmd_buffer, &blit_info);
	}
	
	// swapchain: transfer dst ---> present 
	{
		VkImageMemoryBarrier2 barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = 0,
			.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			.dstAccessMask =VK_ACCESS_NONE,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = swapchain_image,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = VK_REMAINING_MIP_LEVELS,
				.baseArrayLayer = 0,
				.layerCount = VK_REMAINING_ARRAY_LAYERS,
			},
		};
		
		VkDependencyInfo dep_info = {
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = 0,
			.dependencyFlags = 0,
			.memoryBarrierCount = 0,
			.pMemoryBarriers = 0,
			.bufferMemoryBarrierCount = 0,
			.pBufferMemoryBarriers = 0,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier,
		};
		
		vkCmdPipelineBarrier2KHR(frame->cmd_buffer, &dep_info);
	}
	
	vkEndCommandBuffer(frame->cmd_buffer);
	
	// submit
	VkSemaphoreSubmitInfo wait_semaphore_info = 
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.pNext = 0,
		.semaphore = frame->image_ready,
		.value = 0,
		.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		.deviceIndex = 0,
	};
	
	VkSemaphoreSubmitInfo signal_semaphore_info = 
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.pNext = 0,
		.semaphore = frame->render_finished,
		.value = 0,
		.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
		.deviceIndex = 0,
	};
	
	VkCommandBufferSubmitInfo cmd_buffer_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.pNext = 0,
		.commandBuffer = frame->cmd_buffer,
		.deviceMask = 0,
	};
	
	VkSubmitInfo2 submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.pNext = 0,
		.flags = 0,
		.waitSemaphoreInfoCount = 1,
		.pWaitSemaphoreInfos = &wait_semaphore_info,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cmd_buffer_info,
		.signalSemaphoreInfoCount = 1,
		.pSignalSemaphoreInfos = &signal_semaphore_info
	};
	
	res = vkQueueSubmit2KHR(r_vulkan_state->q_main, 1, &submit_info, frame->fence);
	r_vulkanAssert(res);
	
	VkResult sc_res = {0};
	// present
	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = 0,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &frame->render_finished,
		.swapchainCount = 1,
		.pSwapchains = &r_vulkan_state->swapchain,
		.pImageIndices = &image_index,
		.pResults = &sc_res,
	};
	
	res = vkQueuePresentKHR(r_vulkan_state->q_main, &present_info);
	
	if((res == VK_ERROR_OUT_OF_DATE_KHR) || (res == VK_SUBOPTIMAL_KHR) || !v2s_equals(os_getWindowSize(win), r_vulkan_state->last_frame_window_size))
	{
		res = r_vulkan_recreateSwapchain(win);
		return;
	}
	
	r_vulkanAssert(res);
}
