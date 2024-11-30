// TODO(mizu): use ssbo array and pass an id through push constants
// TODO(mizu): delete old swapchain

// TODO(mizu): upload mesh, upload materials
// TODO(mizu): texture cache, vertex / index cache
// TODO(mizu): pipeline builder. But do it after you have more pipelines.

// STUDY(mizu): How are update descriptor sets supposed to be handled. RN, after loading all textures (from meshes, debug textures, sprites, etc.), I call descriptor write once. How do engines usually handle this? What if I am unloading and loading assets on the fly. I guess I can update free slots. HMMM. Anyways, see if you can figure out how its done.

// STUDY(mizu): Also, how do I want to handle resources? Should uploadxxx return a resource and then that is added to the renderer's res table, or does it directly add to the renderer's res table? The reason why I returned the resource, then added to the table was because I thought materials might want to own those resources, so I might just have arrays of materials, and arrays of meshes. But now that I think about it, thats silly. All basic resources are their own unit. Thats how the gpu sees it. A material would have an id that indexes maps to the texture table. Perhaps I could have a staging table where resources are added. Then a main table where resources go after they have been descriptor written? Or should this be one step? IDK, I'm sure Ill figure something out.

// I am thinking of a system like this - All meshes, models, resources, etc. that are loaded are owned by the engine. A handle is returned to refer to them. "Loading" means storing on the gpu since I can't think of good reason to have it around in ram. Anyways, when rendering, you'd do something like draw_mesh(mesh handle, material handle) and then the backend would map the handle to whatever mesh is being requested to draw, and it would add it to render context that is generated per frame. Maybe for streaming what I could try is that if the handle doesn't exist, it makes it exist and if a handle isn't requested for long enough, it unloads the resource? I did something similar (identical) to this in my 2d opengl engine. Only a texture cache had purpose there since there was no need for vertex or index buffers. One glaring difference was the state didn't own resources. The texture cache owned and flushed resources. So in the vulkan engine, I would ideally see what all textures are being requested by the cache and the descriptor write them. This is just an idea btw. I will cross this bridge when I get there. I am just dumping ideas. Also, word wrap is objectively superior. I fit 4 panels on my 15 inch laptop screen. 80 col limit is bs. For anyone reading this, word wrap is better because regardless of screen size, things will always fit cleanly.



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

#define MAX_RECT3 256

typedef struct R_VULKAN_SceneData R_VULKAN_SceneData;
struct R_VULKAN_SceneData
{
	M4F proj;
	M4F view;
};

typedef struct R_VULKAN_Rect3InstanceData R_VULKAN_Rect3InstanceData;
struct R_VULKAN_Rect3InstanceData
{
	M4F model[MAX_RECT3];
	u32 tex_id[MAX_RECT3];
};

typedef struct R_VULKAN_Rect3PushConstants R_VULKAN_Rect3PushConstants;
struct R_VULKAN_Rect3PushConstants
{
	VkDeviceAddress scene;
	VkDeviceAddress instance;
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
	VkDeviceAddress address;
	VmaAllocation alloc;
	VmaAllocationInfo info;
};

typedef struct R_Handle R_Handle;
struct R_Handle
{
	u64 u64[2];
};

typedef struct GLTF_Vertex GLTF_Vertex;
struct GLTF_Vertex
{
	V3F pos;
	f32 uv_x;
	V3F normal;
	f32 uv_y;
	V4F color;
};

typedef struct GLTF_Primitive GLTF_Primitive;
struct GLTF_Primitive
{
	u32 start;
	u32 count;
	
	Str8 base_tex;
};

typedef struct GLTF_Mesh GLTF_Mesh;
struct GLTF_Mesh
{
	GLTF_Primitive *primitives;
	u64 num_primitives;
	
	u32 *indices;
	u32 num_indices;
	
	GLTF_Vertex *vertices;
	u32 num_vertices;
};

typedef struct GLTF_Model GLTF_Model;
struct GLTF_Model
{
	Bitmap *textures;
	u32 num_textures;
	
	GLTF_Mesh *meshes;
	u64 num_meshes;
};

// TODO(mizu): textures go inside engine. an id is held for it
// other fields 

typedef struct R_VULKAN_Primitive R_VULKAN_Primitive;
struct R_VULKAN_Primitive
{
	u32 start;
	u32 count;
	
	u32 base_tex_id;
};

typedef struct R_VULKAN_Mesh R_VULKAN_Mesh;
struct R_VULKAN_Mesh
{
	R_VULKAN_Primitive *primitives;
	u64 num_primitives;
	
	R_VULKAN_Buffer i_buffer;
	u32 num_indices;
	
	R_VULKAN_Buffer v_buffer;
	u32 num_vertices;
};

typedef struct R_VULKAN_Model R_VULKAN_Model;
struct R_VULKAN_Model 
{
	R_VULKAN_Mesh *meshes;
	u64 num_meshes;
};

typedef struct GLTF_It GLTF_It;
struct GLTF_It
{
	Arena *arena;
	u64 mesh_index;
	GLTF_Model *model;
	cgltf_data *data;
	Str8 abs_path;
	Str8 dir;
};

function void gltf_traverse_node(GLTF_It *it, cgltf_node *node)
{
	if(node->mesh)
	{
		cgltf_mesh *node_mesh = node->mesh;
		
		GLTF_Mesh *mesh = it->model->meshes + it->mesh_index;
		mesh->num_primitives = node_mesh->primitives_count;
		mesh->primitives = pushArray(it->arena, GLTF_Primitive, mesh->num_primitives);
		
		for(u32 i = 0; i < mesh->num_primitives; i++)
		{
			cgltf_primitive *node_prim = node->mesh->primitives + i;
			cgltf_accessor *index_attrib = node_prim->indices;
			
			mesh->num_indices += index_attrib->count;
		}
		
		for(u32 i = 0; i < mesh->num_primitives; i++)
		{
			cgltf_primitive *node_prim = node->mesh->primitives + i;
			
			for(u32 j = 0; j < node_prim->attributes_count; j++)
			{
				cgltf_attribute *attrib = node_prim->attributes + j;
				
				if(attrib->type == cgltf_attribute_type_position)
				{
					cgltf_accessor *vert_attrib = attrib->data;
					mesh->num_vertices += vert_attrib->count;
				}
			}
		}
		
		mesh->indices = pushArray(it->arena, u32, mesh->num_indices);
		mesh->vertices = pushArray(it->arena, GLTF_Vertex, mesh->num_vertices);
		
		u64 init_vtx = 0;
		u64 init_index = 0;
		
		for(u32 i = 0; i < mesh->num_primitives; i++)
		{
			cgltf_primitive *node_prim = node->mesh->primitives + i;
			
			GLTF_Primitive *p = mesh->primitives + i;
			
			char *thing = node_prim->material->pbr_metallic_roughness.base_color_texture.texture->image->uri;
			
			Str8 uri_str =  str8((u8*)thing, strlen(thing));
			p->base_tex = str8_join(it->arena, it->dir, uri_str);
			pushArray(it->arena, u8, 1);
			
			cgltf_accessor *index_attrib = node_prim->indices;
			
			p->start = init_index;
			p->count = index_attrib->count;
			
			// indices
			{
				for (u32 j = 0; j < index_attrib->count; j++)
				{
					size_t index = cgltf_accessor_read_index(index_attrib, j);
					
					mesh->indices[j + init_index] = index + init_vtx;
				}
				
				init_index += index_attrib->count;
			}
			
			// vertices
			for(u32 j = 0; j < node_prim->attributes_count; j++)
			{
				cgltf_attribute *attrib = node_prim->attributes + j;
				
				if(attrib->type == cgltf_attribute_type_position)
				{
					init_vtx = 0;
					cgltf_accessor *vert_attrib = attrib->data;
					
					for(u32 k = 0; k < vert_attrib->count; k++)
					{
						cgltf_accessor_read_float(vert_attrib, k, mesh->vertices[k + init_vtx].pos.e, sizeof(f32));
					}
					
					init_vtx += vert_attrib->count;
				}
				
				// NOTE(mizu): stop cheezing init vtx;
				
				if(attrib->type == cgltf_attribute_type_normal)
				{
					init_vtx = 0;
					cgltf_accessor *norm_attrib = attrib->data;
					
					for(u32 k = 0; k < norm_attrib->count; k++)
					{
						cgltf_accessor_read_float(norm_attrib, k, mesh->vertices[k + init_vtx].normal.e, sizeof(f32));
					}
					init_vtx += norm_attrib->count;
				}
				
				if(attrib->type == cgltf_attribute_type_color)
				{
					init_vtx = 0;
					
					cgltf_accessor *color_attrib = attrib->data;
					for (u32 k = 0; k < color_attrib->count; k++)
					{
						cgltf_accessor_read_float(color_attrib, k, mesh->vertices[k + init_vtx].color.e, sizeof(f32));
						
						//printf("%f %f %f %f\n", mesh->vertices[k + init_vtx].color.e[0], mesh->vertices[k + init_vtx].color.e[1], mesh->vertices[k + init_vtx].color.e[2], mesh->vertices[k + init_vtx].color.e[3]);
						
						
					}
					init_vtx += color_attrib->count;
				}
				
				if(attrib->type == cgltf_attribute_type_texcoord)
				{
					cgltf_accessor *tex_attrib = attrib->data;
					
					// TODO(mizu):  difference b/w attrib index 0 and 1
					if (attrib->index == 0)
					{
						init_vtx = 0;
						
						for(u32 k = 0; k < tex_attrib->count; k++)
						{
							f32 tex[2] = {0};
							
							cgltf_accessor_read_float(tex_attrib, k, tex, sizeof(f32));
							mesh->vertices[k + init_vtx].uv_x = tex[0];
							mesh->vertices[k + init_vtx].uv_y = 1 - tex[1];
						}
						init_vtx += tex_attrib->count;
					}
				}
				
			}
			
			
		}
		
		it->mesh_index++;
	}
	
	for (u32 i = 0; i < node->children_count; ++i) 
	{
		gltf_traverse_node(it, node->children[i]);
	}
}

function void gltf_print(GLTF_Model *model)
{
	for(u32 i = 0; i < model->num_meshes; i++)
	{
		GLTF_Mesh *mesh = model->meshes + i;
		
		printf("indices %u\n", i);
		for(u32 j = 0; j < mesh->num_indices; j++)
		{
			printf("%u, ", mesh->indices[j]);
		}
		printf("\n");
		
		printf("verticess %u\n", i);
		for(u32 j = 0; j < mesh->num_vertices; j++)
		{
			GLTF_Vertex *vert = mesh->vertices + j;
			printf("[%f, %f, %f]", vert->pos.x, vert->pos.y, vert->pos.z);
		}
		printf("\n");
		
		for(u32 j = 0; j < mesh->num_primitives; j++)
		{
			GLTF_Primitive *p = mesh->primitives;
			
			printf("start: %u\n", p->start);
			printf("count: %u\n", p->count);
		}
		
		printf("\n");
	}
}

function GLTF_Model gltf_load_mesh(Arena *arena, Arena *scratch, Str8 filepath)
{
	GLTF_Model out = {0};
	
	GLTF_It it = {0};
	
	Str8 app_dir = os_getAppDir(scratch);
	
	app_dir = str8_join(scratch, app_dir, str8_lit("../res/"));
	it.abs_path = str8_join(scratch, app_dir, filepath);
	pushArray(scratch, u8, 1);
	
	cgltf_options options = {0};
	cgltf_data *data = 0;
	
	if(cgltf_parse_file(&options, (char*)it.abs_path.c, &data) == cgltf_result_success)
	{
		if(cgltf_load_buffers(&options, data, (char*)it.abs_path.c) == cgltf_result_success)
		{
			// load textures
			out.num_textures = data->textures_count;
			out.textures = pushArray(arena, Bitmap, data->textures_count);
			
			for(u32 i = 0; i < data->textures_count; i++)
			{
				Str8 uri_str = str8((u8*)data->textures[i].image->uri, strlen(data->textures[i].image->uri));
				Str8 pather = str8_lit("asuka/");
				
				pather = str8_join(arena, pather, uri_str);
				pushArray(arena, u8, 1);
				
				out.textures[i] = bitmap(pather);
			}
			
			// load meshes
			out.num_meshes = data->meshes_count;
			out.meshes = pushArray(arena, GLTF_Mesh, out.num_meshes);
			
			it.model = &out;
			it.arena = arena;
			it.data = data;
			
			for(u32 i = 0; i < data->scenes_count; i++)
			{
				cgltf_scene *scene = data->scenes + i;
				
				for(u32 j = 0; j < scene->nodes_count; j++)
				{
					cgltf_node *node = scene->nodes[j];
					
					gltf_traverse_node(&it, node);
				}
			}
			
			//gltf_print(it.model);
			
		}
	}
	
	cgltf_free(data);
	
	return out;
}

typedef struct R_VULKAN_FrameData R_VULKAN_FrameData;
struct R_VULKAN_FrameData
{
	VkCommandPool cmd_pool;
	VkCommandBuffer cmd_buffer;
	
	VkFence fence;
	VkSemaphore image_ready;
	VkSemaphore render_finished;
	
	VkDescriptorSet scene_set;
	R_VULKAN_Buffer scene_buffer;
	R_VULKAN_Buffer rect3_inst_buffer;
};

typedef struct R_VULKAN_State R_VULKAN_State;
struct R_VULKAN_State
{
	Arena *arena;
	VmaAllocator vma;
	
	OS_Handle vkdll;
	
	// vulkan data
	VkInstance instance;
	VkPhysicalDevice phys_device;
	VkDevice device;
	u32 q_main_family;
	VkQueue q_main;
	
	V2S last_frame_window_size;
	VkSurfaceKHR surface;
	VkSurfaceFormatKHR surface_format;
	VkExtent2D surface_extent;
	VkSwapchainKHR swapchain;
	
	VkImage *swapchain_images;
	VkImageView *swapchain_image_views;
	u32 swapchain_image_count;
	
	// render target
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
	
	// layouts
	VkPipelineLayout pipeline_layout;
	VkDescriptorPool descriptor_pool;
	VkDescriptorSetLayout descriptor_set_layout;
	
	// pipelines
	VkPipeline rect3_pipeline;
	VkPipeline mesh_pipeline;
	
	// frame render data
	R_VULKAN_FrameData frames[R_VULKAN_FRAMES];
	u32 current_frame_index;
	
	VkCommandPool im_cmd_pool;
	VkCommandBuffer im_cmd_buffer;
	VkFence im_fence;
	
	// resources
	R_VULKAN_Model model;
	R_VULKAN_Image *textures;
	u32 num_textures;
};

global R_VULKAN_State *r_vulkan_state;

typedef struct R_VULKAN_MeshPushConstants R_VULKAN_MeshPushConstants;
struct R_VULKAN_MeshPushConstants
{
	M4F model;
	VkDeviceAddress scene_buffer;
	VkDeviceAddress v_buffer;
};

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
global PFN_vkCmdPushConstants vkCmdPushConstants;
global PFN_vkCmdBindPipeline vkCmdBindPipeline;
global PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
global PFN_vkCmdSetViewport vkCmdSetViewport;
global PFN_vkCmdSetScissor vkCmdSetScissor;
global PFN_vkCmdDraw vkCmdDraw;
global PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
global PFN_vkCmdBlitImage2KHR vkCmdBlitImage2KHR;
global PFN_vkCmdPipelineBarrier2KHR vkCmdPipelineBarrier2KHR;
global PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
global PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
global PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
global PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddress;
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

function void r_vulkan_destroyBuffer(R_VULKAN_Buffer *buffer)
{
	vmaDestroyBuffer(r_vulkan_state->vma, buffer->buffer, buffer->alloc);
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

function R_VULKAN_Image r_vulkan_uploadImage(Bitmap bmp)
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
	
	R_VULKAN_Buffer staging = r_vulkan_createBuffer(data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	
	memcpy(staging.info.pMappedData, bmp.data, data_size);
	
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
	
	vkCmdCopyBufferToImage(r_vulkan_state->im_cmd_buffer, staging.buffer, out.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
	
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
	r_vulkan_destroyBuffer(&staging);
	
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
	
	return out;
}

function void r_vulkan_uploadTexture()
{
	
}

function void r_vulkan_uploadVertexIndexData(Arena *scratch)
{
	GLTF_Model model = gltf_load_mesh(scratch, scratch, str8_lit("asuka/scene.gltf"));
	
	for(u32 j = 0; j < model.num_textures; j++)
	{
		R_VULKAN_Image *image = r_vulkan_state->textures + r_vulkan_state->num_textures++;
		
		*image = r_vulkan_uploadImage(model.textures[j]);
	}
	
	r_vulkan_state->model.num_meshes = model.num_meshes;
	r_vulkan_state->model.meshes = pushArray(r_vulkan_state->arena, R_VULKAN_Mesh, r_vulkan_state->model.num_meshes);
	
	r_vulkan_state->model.num_meshes = model.num_meshes;
	r_vulkan_state->model.meshes = pushArray(r_vulkan_state->arena, R_VULKAN_Mesh, r_vulkan_state->model.num_meshes);
	
	for(u32 i = 0; i < model.num_meshes; i++)
	{
		GLTF_Mesh *gltf_mesh = model.meshes + i;
		R_VULKAN_Mesh *vk_mesh = r_vulkan_state->model.meshes + i;
		
		vk_mesh->num_primitives = gltf_mesh->num_primitives;
		vk_mesh->primitives = pushArray(r_vulkan_state->arena, R_VULKAN_Primitive, vk_mesh->num_primitives);
		
		for(u32 j = 0; j < gltf_mesh->num_primitives; j++)
		{
			vk_mesh->primitives[j].start = gltf_mesh->primitives[j].start;
			vk_mesh->primitives[j].count = gltf_mesh->primitives[j].count;
		}
	}
	
	for(u32 i = 0; i < model.num_meshes; i++)
	{
		GLTF_Mesh *gltf_mesh = model.meshes + i;
		R_VULKAN_Mesh *vk_mesh = r_vulkan_state->model.meshes + i;
		
		u64 vertices_size = sizeof(GLTF_Vertex) * gltf_mesh->num_vertices;
		u64 indices_size = sizeof(u32) * gltf_mesh->num_indices;
		
		vk_mesh->v_buffer = r_vulkan_createBuffer(vertices_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
																							VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
																							VMA_MEMORY_USAGE_GPU_ONLY);
		
		VkBufferDeviceAddressInfo device_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.buffer = vk_mesh->v_buffer.buffer,
		};
		
		vk_mesh->v_buffer.address = vkGetBufferDeviceAddress(r_vulkan_state->device, &device_info);
		
		vk_mesh->i_buffer = r_vulkan_createBuffer(indices_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
																							VMA_MEMORY_USAGE_GPU_ONLY);
		
		R_VULKAN_Buffer staging =
			r_vulkan_createBuffer(vertices_size + indices_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
														VMA_MEMORY_USAGE_CPU_TO_GPU);
		
		void *data = 0;
		vmaMapMemory(r_vulkan_state->vma, staging.alloc, &data);
		
		memcpy(data, gltf_mesh->vertices, vertices_size);
		memcpy((u8*)data + vertices_size, gltf_mesh->indices, indices_size);
		
		vmaUnmapMemory(r_vulkan_state->vma, staging.alloc);
		
		r_vulkan_imBeginSubmit();
		
		VkBufferCopy vert_copy = {
			.size = vertices_size,
		};
		
		vkCmdCopyBuffer(r_vulkan_state->im_cmd_buffer, staging.buffer, vk_mesh->v_buffer.buffer, 1, &vert_copy);
		
		VkBufferCopy index_copy = {
			.srcOffset = vertices_size,
			.size = indices_size,
		};
		
		vkCmdCopyBuffer(r_vulkan_state->im_cmd_buffer, staging.buffer, vk_mesh->i_buffer.buffer, 1, &index_copy);
		
		r_vulkan_imEndSubmit();
		r_vulkan_destroyBuffer(&staging);
	}
	
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
	vkDestroyImageView(r_vulkan_state->device, r_vulkan_state->depth_image_view, 0);
	vmaDestroyImage(r_vulkan_state->vma, r_vulkan_state->depth_image, r_vulkan_state->depth_image_memory);
	
	//vkDestroySwapchainKHR(r_vulkan_state->device, r_vulkan_state->swapchain, 0);
}

function VkResult r_vulkan_createSwapchain(OS_Handle win, Arena *scratch)
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
	
	VkSurfaceFormatKHR *surface_format_list = pushArray(scratch, VkSurfaceFormatKHR, format_count);
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
		local_persist b32 initialized = 0;
		if(!initialized)
		{
			initialized = 1;
			u32 image_count = 0;
			vkGetSwapchainImagesKHR(r_vulkan_state->device, r_vulkan_state->swapchain, &image_count, 0);
			
			r_vulkan_state->swapchain_images = pushArray(r_vulkan_state->arena, VkImage, image_count);
			r_vulkan_state->swapchain_image_views = pushArray(r_vulkan_state->arena, VkImageView, image_count);
			
			r_vulkan_state->swapchain_image_count = image_count;
		}
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

function VkResult r_vulkan_recreateSwapchain(OS_Handle win, Arena *scratch)
{
	//printf("recreated swapchain\n");
	vkDeviceWaitIdle(r_vulkan_state->device);
	r_vulkan_cleanupSwapchain();
	r_vulkan_createSwapchain(win, scratch);
	
	//vkDestroySwapchainKHR(r_vulkan_state->device, r_vulkan_state->swapchain, 0);
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

function void r_vulkan_init(OS_Handle win, Arena *scratch)
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
		
		char *validation_layers[] = {
#if R_VULKAN_DEBUG
			"VK_LAYER_KHRONOS_validation"
#endif
		};
		
		char *user_extentions[] = {
			VK_KHR_SURFACE_EXTENSION_NAME,
#if R_VULKAN_DEBUG
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
#if defined(OS_APPLE)
			VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
			VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
#endif
		};
		
		u32 platform_ext_num = os_vulkan_getPlatformExtentions(0);
		
		char **extentions = pushArray(scratch, char *, arrayLen(user_extentions) + platform_ext_num);
		
		u32 extention_num = 0;
		
		extention_num += os_vulkan_getPlatformExtentions(extentions);
		
		for(s32 i = 0; i < arrayLen(user_extentions); i++)
		{
			extentions[i + platform_ext_num] = user_extentions[i];
			extention_num += 1;
		}
		
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
			.enabledLayerCount = arrayLen(validation_layers),
			.ppEnabledLayerNames = validation_layers,
			.enabledExtensionCount = extention_num,
			.ppEnabledExtensionNames = extentions
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
		
		VkPhysicalDevice *phys_devices = pushArray(scratch, VkPhysicalDevice, count);
		VkPhysicalDeviceFeatures2 *features = pushArray(scratch, VkPhysicalDeviceFeatures2, count);
		
		res = vkEnumeratePhysicalDevices(r_vulkan_state->instance, &count, phys_devices);
		
		r_vulkanAssert(res);
		
		typedef struct GpuStat
		{
			b32 discrete;
			b32 good;
		}GpuStat;
		
		GpuStat *gpus = pushArray(scratch, GpuStat, count);
		
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
	vkCmdPushConstants = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdPushConstants");
	vkCmdBindPipeline = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdBindPipeline");
	vkCmdBindIndexBuffer = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdBindIndexBuffer");
	vkCmdSetViewport = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdSetViewport");
	vkCmdSetScissor = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdSetScissor");
	vkCmdDraw = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdDraw");
	vkCmdDrawIndexed = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdDrawIndexed");
	
	vkCmdBlitImage2KHR = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdBlitImage2KHR");
	vkCmdPipelineBarrier2KHR = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdPipelineBarrier2KHR");
	vkCmdBindDescriptorSets = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdBindDescriptorSets");
	vkCmdCopyBufferToImage = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdCopyBufferToImage");
	vkCmdCopyBuffer = vkGetDeviceProcAddr(r_vulkan_state->device, "vkCmdCopyBuffer");
	
	vkGetBufferDeviceAddress = vkGetDeviceProcAddr(r_vulkan_state->device, "vkGetBufferDeviceAddress");
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
	
	r_vulkan_createSwapchain(win, scratch);
	
	// pipeline layout and descriptor set layout
	{
		// descriptor set layout
		
		VkDescriptorSetLayoutBinding descriptor_bindings[1] = {
			[0] = {
				.binding = 0,
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
			.bindingCount = arrayLen(descriptor_bindings),
			.pBindings = descriptor_bindings,
		};
		
		res = vkCreateDescriptorSetLayout(r_vulkan_state->device,
																			&descriptor_layout_info,
																			0,
																			&r_vulkan_state->descriptor_set_layout);
		r_vulkanAssert(res);
		
		VkPushConstantRange range = {
			.offset = 0,
			//.size = sizeof(R_VULKAN_Rect3PushConstants),
			.size = 256,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		};
		
		// pipeline layout
		VkPipelineLayoutCreateInfo layout_info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &r_vulkan_state->descriptor_set_layout,
			.pushConstantRangeCount = 1,
			.pPushConstantRanges = &range,
		};
		
		vkCreatePipelineLayout(r_vulkan_state->device, &layout_info, 0, &r_vulkan_state->pipeline_layout);
	}
	
	// rect3 pipeline 
	{
		// pipeline shader stage
		FileData vert_src = readFile(scratch, str8_lit("rect3.vert.spv"), FILE_TYPE_BINARY);
		VkShaderModuleCreateInfo vert_shader_module_info = {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = vert_src.size,
			.pCode = vert_src.bytes,
		};
		
		VkShaderModule vert_module = {0};
		res = vkCreateShaderModule(r_vulkan_state->device, &vert_shader_module_info, 0, &vert_module);
		r_vulkanAssert(res);
		
		FileData frag_src = readFile(scratch, str8_lit("rect3.frag.spv"), FILE_TYPE_BINARY);
		
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
			.cullMode = VK_CULL_MODE_NONE,
			//.cullMode = VK_CULL_MODE_BACK_BIT,
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
			.layout = r_vulkan_state->pipeline_layout,
			.renderPass = 0,
			.subpass = 0,
			.basePipelineHandle = 0,
			.basePipelineIndex = 0
		};
		
		res = vkCreateGraphicsPipelines(r_vulkan_state->device, 0, 1, &pipeline_create_info, 0, &r_vulkan_state->rect3_pipeline);
		r_vulkanAssert(res);
	}
	
	// mesh pipeline 
	{
		// pipeline shader stage
		FileData vert_src = readFile(scratch, str8_lit("mesh.vert.spv"), FILE_TYPE_BINARY);
		VkShaderModuleCreateInfo vert_shader_module_info = {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = vert_src.size,
			.pCode = vert_src.bytes,
		};
		
		VkShaderModule vert_module = {0};
		res = vkCreateShaderModule(r_vulkan_state->device, &vert_shader_module_info, 0, &vert_module);
		r_vulkanAssert(res);
		
		FileData frag_src = readFile(scratch, str8_lit("mesh.frag.spv"), FILE_TYPE_BINARY);
		
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
			//.cullMode = VK_CULL_MODE_NONE,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
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
			.layout = r_vulkan_state->pipeline_layout,
			.renderPass = 0,
			.subpass = 0,
			.basePipelineHandle = 0,
			.basePipelineIndex = 0
		};
		
		res = vkCreateGraphicsPipelines(r_vulkan_state->device, 0, 1, &pipeline_create_info, 0, &r_vulkan_state->mesh_pipeline);
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
	
	// debug bitmaps
	{
		r_vulkan_state->textures = pushArray(r_vulkan_state->arena, R_VULKAN_Image, 10);
		{
			Bitmap bmp = bitmap(str8_lit("scratch/ell.png"));
			r_vulkan_state->textures[r_vulkan_state->num_textures++] = r_vulkan_uploadImage(bmp);
		}
		{
			Bitmap bmp = bitmap(str8_lit("scratch/marhall.png"));
			r_vulkan_state->textures[r_vulkan_state->num_textures++] = r_vulkan_uploadImage(bmp);
		}
		{
			Bitmap bmp = bitmap(str8_lit("scratch/maruko.png"));
			r_vulkan_state->textures[r_vulkan_state->num_textures++] = r_vulkan_uploadImage(bmp);
		}
		{
			Bitmap bmp = bitmap(str8_lit("scratch/ankha.png"));
			r_vulkan_state->textures[r_vulkan_state->num_textures++] = r_vulkan_uploadImage(bmp);
		}
	}
	
	// mega descriptor set
	{
		
		// allocate pools
		VkDescriptorPoolSize sizes[1] = {
			[0] = {
				.descriptorCount = 4,
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
		
		res = vkCreateDescriptorPool(r_vulkan_state->device, &info, 0, &r_vulkan_state->descriptor_pool);
		r_vulkanAssert(res);
		
		// allocate sets
		for (u32 i = 0; i < R_VULKAN_FRAMES; i++)
		{
			R_VULKAN_FrameData *frame = r_vulkan_state->frames + i;
			frame->scene_set = r_vulkan_allocDescriptorSet(r_vulkan_state->descriptor_pool, &r_vulkan_state->descriptor_set_layout);
			
			// scene buffer
			{
				frame->scene_buffer = r_vulkan_createBuffer(sizeof(R_VULKAN_SceneData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
				VkBufferDeviceAddressInfo device_info = {
					.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
					.buffer = frame->scene_buffer.buffer,
				};
				
				frame->scene_buffer.address = vkGetBufferDeviceAddress(r_vulkan_state->device, &device_info);
				
			}
			
			// 3d rect instance buffer
			{
				frame->rect3_inst_buffer = r_vulkan_createBuffer(sizeof(R_VULKAN_Rect3InstanceData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
				VkBufferDeviceAddressInfo device_info = {
					.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
					.buffer = frame->rect3_inst_buffer.buffer,
				};
				
				frame->rect3_inst_buffer.address = vkGetBufferDeviceAddress(r_vulkan_state->device, &device_info);
			}
		}
	}
}

function void r_vulkan_updateDescriptorSets(Arena *scratch)
{
	for (u32 i = 0; i < R_VULKAN_FRAMES; i++)
	{
		R_VULKAN_FrameData *frame = r_vulkan_state->frames + i;
		
		VkDescriptorImageInfo *img_info = pushArray(scratch, VkDescriptorImageInfo, r_vulkan_state->num_textures); 
		
		for(u32 j = 0; j < r_vulkan_state->num_textures; j++)
		{
			R_VULKAN_Image *image = r_vulkan_state->textures + j;
			img_info[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			img_info[j].imageView = image->view;
			img_info[j].sampler = image->sampler;
		}
		
		VkWriteDescriptorSet write = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = frame->scene_set,
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = r_vulkan_state->num_textures,
			.pImageInfo = img_info,
		};
		
		vkUpdateDescriptorSets(r_vulkan_state->device, 1, &write, 0, 0);
	}
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

function void r_vulkanRender(OS_Handle win, OS_EventList *events, f32 delta, Arena *scratch)
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
		res = r_vulkan_createSwapchain(win, scratch);
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
	
	// draw rect3
	
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
	vkCmdBindDescriptorSets(frame->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r_vulkan_state->pipeline_layout, 0, 1, &frame->scene_set, 0, 0);
	
	
	vkCmdSetViewport(frame->cmd_buffer, 0, 1, &r_vulkan_state->viewport);
	vkCmdSetScissor(frame->cmd_buffer, 0, 1, &r_vulkan_state->scissor);
	
	static f32 counter = 0;
	counter += delta;
	V2S win_size = os_getWindowSize(win);
	
	static Camera camera = {
		.yaw = 90,
	};
	
	camUpdate(&camera, events, delta);
	
	//M4F view = m4f_lookAt((V3F){.z = -3}, (V3F){.z = 0}, (V3F){.y = 1});
	
	R_VULKAN_SceneData scene_data = {
		.proj = m4f_perspective(degToRad(90), win_size.x * 1.f / win_size.y, 0.01, 1000),
		.view = camGetView(&camera),
		//.view = view,
		//.model[0] = m4f_mul(m4f_translate(v3f(-3, 0, 3)), m4f_rotate(v3f(0, 1, 0), counter)),
		//.model = m4f_rotate(v3f(0, 1, 0), counter),
		//.model = m4f(1)
		//.model = m4f_translate(v3f(0, 0.2, 0))
	};
	
	void* mappedData;
	vmaMapMemory(r_vulkan_state->vma, frame->scene_buffer.alloc, &mappedData);
	memcpy(mappedData, &scene_data, sizeof(R_VULKAN_SceneData));
	vmaUnmapMemory(r_vulkan_state->vma, frame->scene_buffer.alloc);
	
	R_VULKAN_Rect3InstanceData *rect3_inst_data = pushArray(scratch, R_VULKAN_Rect3InstanceData, 1);
	
	srand(0);
	for(s32 i = 0; i < MAX_RECT3; i++)
	{
		M4F trans = m4f_translate(v3f(i * rand() % 16, i * rand() % 16, i * rand() % 16));
		rect3_inst_data->model[i] = m4f_mul(trans, m4f_rotate(v3f(0, 1, 0), counter * (rand() % 16)));
		rect3_inst_data->tex_id[i] = i % 4;
	}
	
	mappedData;
	vmaMapMemory(r_vulkan_state->vma, frame->rect3_inst_buffer.alloc, &mappedData);
	memcpy(mappedData, rect3_inst_data, sizeof(R_VULKAN_Rect3InstanceData));
	vmaUnmapMemory(r_vulkan_state->vma, frame->rect3_inst_buffer.alloc);
	
	vkCmdBindPipeline(frame->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r_vulkan_state->rect3_pipeline);
	
	R_VULKAN_Rect3PushConstants push_constants = 
	{
		.scene = frame->scene_buffer.address,
		.instance = frame->rect3_inst_buffer.address,
	};
	
	vkCmdPushConstants(frame->cmd_buffer, r_vulkan_state->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(R_VULKAN_Rect3PushConstants), &push_constants);
	
	vkCmdDraw(frame->cmd_buffer, 6, MAX_RECT3, 0, 0);
	
#if 1
	vkCmdBindPipeline(frame->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r_vulkan_state->mesh_pipeline);
	
	for(u32 i = 0; i < r_vulkan_state->model.num_meshes; i++)
	{
		R_VULKAN_Mesh *mesh = r_vulkan_state->model.meshes + i;
		vkCmdBindIndexBuffer(frame->cmd_buffer, mesh->i_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		
		for(u32 j = 0; j < mesh->num_primitives; j++)
		{
			R_VULKAN_Primitive *prim = mesh->primitives + j;
			R_VULKAN_MeshPushConstants push_constants = 
			{
				.scene_buffer = frame->scene_buffer.address,
				.model = m4f_mul(m4f_translate(v3f(2.8, 0, 1.2)), m4f_rotate(v3f(1, 0, 0), degToRad(90))),
				.v_buffer = mesh->v_buffer.address,
			};
			
			vkCmdPushConstants(frame->cmd_buffer, r_vulkan_state->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(R_VULKAN_MeshPushConstants), &push_constants);
			
			vkCmdDrawIndexed(frame->cmd_buffer, prim->count, 1, prim->start, 0, 0);
		}
	}
#endif
	
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
		res = r_vulkan_recreateSwapchain(win, scratch);
		return;
	}
	
	r_vulkanAssert(res);
}
