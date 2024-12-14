typedef struct GLTF_Primitive GLTF_Primitive;
struct GLTF_Primitive
{
	u32 start;
	u32 count;
	
	u32 base_tex_index;
	u32 normal_tex_index;
};

typedef struct GLTF_Mesh GLTF_Mesh;
struct GLTF_Mesh
{
	GLTF_Primitive *primitives;
	u64 num_primitives;
	u32 *indices;
	u32 num_indices;
	
	R_Vertex *vertices;
	u32 num_vertices;

	R_Handle gpu_buffer;
	
	M4F transform;
};

typedef struct GLTF_Scene GLTF_Scene;
struct GLTF_Scene
{
	R_Handle *gpu_textures;
	Bitmap *textures;
	u32 num_textures;
	
	GLTF_Mesh *meshes;
	u64 num_meshes;
};

typedef struct GLTF_It GLTF_It;
struct GLTF_It
{
	Arena *arena;
	u64 mesh_index;
	GLTF_Scene *scene;
	cgltf_data *data;
	Str8 dir;
};

function void gltf_traverseNode(GLTF_It *it, cgltf_node *node)
{
	if(node->mesh)
	{
		cgltf_mesh *node_mesh = node->mesh;
		
		GLTF_Mesh *mesh = it->scene->meshes + it->mesh_index;
		mesh->num_primitives = node_mesh->primitives_count;
		mesh->primitives = pushArray(it->arena, GLTF_Primitive, mesh->num_primitives);
		
		size_t m_vertex_num = 0;
		size_t m_index_num = 0;
		
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
		mesh->vertices = pushArray(it->arena, R_Vertex, mesh->num_vertices);
		
		for(u32 i = 0; i < mesh->num_primitives; i++)
		{
			u64 init_vtx = m_vertex_num;
			u64 init_index = m_index_num;
			
			cgltf_primitive *node_prim = node->mesh->primitives + i;
			
			GLTF_Primitive *p = mesh->primitives + i;
			
			if(node_prim->material->pbr_metallic_roughness.base_color_texture.texture)
			{
				p->base_tex_index = cgltf_texture_index(it->data, node_prim->material->pbr_metallic_roughness.base_color_texture.texture); 
			}
			
			if(node_prim->material->normal_texture.texture)
			{
				p->normal_tex_index = cgltf_texture_index(it->data, node_prim->material->normal_texture.texture);
			}
   
			cgltf_accessor *index_attrib = node_prim->indices;
			
			p->start = init_index;
			p->count = index_attrib->count;
			
			// indices
			{
				for (u32 j = 0; j < index_attrib->count; j++)
				{
					size_t index = cgltf_accessor_read_index(index_attrib, j);
					
					mesh->indices[j + m_index_num] = index + init_vtx;
				}
				
				m_index_num += index_attrib->count;
			}
			
			// vertices
			for(u32 j = 0; j < node_prim->attributes_count; j++)
			{
				cgltf_attribute *attrib = node_prim->attributes + j;
				
				if(attrib->type == cgltf_attribute_type_position)
				{
					cgltf_accessor *vert_attrib = attrib->data;
					m_vertex_num += vert_attrib->count;
					
					for(u32 k = 0; k < vert_attrib->count; k++)
					{
						cgltf_accessor_read_float(vert_attrib, k, mesh->vertices[k + init_vtx].pos.e, sizeof(f32));
					}
				}
				
				if(attrib->type == cgltf_attribute_type_normal)
				{
					cgltf_accessor *norm_attrib = attrib->data;
					
					for(u32 k = 0; k < norm_attrib->count; k++)
					{
						cgltf_accessor_read_float(norm_attrib, k, mesh->vertices[k + init_vtx].normal.e, sizeof(f32));
					}
				}
				
				if(attrib->type == cgltf_attribute_type_color)
				{
					cgltf_accessor *color_attrib = attrib->data;
					for (u32 k = 0; k < color_attrib->count; k++)
					{
						cgltf_accessor_read_float(color_attrib, k, mesh->vertices[k + init_vtx].color.e, sizeof(f32));
					}
				}
				
				if(attrib->type == cgltf_attribute_type_texcoord)
				{
					cgltf_accessor *tex_attrib = attrib->data;
					
					// TODO(mizu):  difference b/w attrib index 0 and 1
					if (attrib->index == 0)
					{
						for(u32 k = 0; k < tex_attrib->count; k++)
						{
							f32 tex[2] = {0};
							
							cgltf_accessor_read_float(tex_attrib, k, tex, sizeof(f32));
							mesh->vertices[k + init_vtx].uv_x = tex[0];
							mesh->vertices[k + init_vtx].uv_y = 1 - tex[1];
						}
					}
				}
				
				if(attrib->type == cgltf_attribute_type_tangent)
				{
					cgltf_accessor *tangent_attrib = attrib->data;
					for (u32 k = 0; k < tangent_attrib->count; k++)
					{
						cgltf_accessor_read_float(tangent_attrib, k, mesh->vertices[k + init_vtx].tangent.e, sizeof(f32));
					}
				}
			}
			
			
		}
		
		f32 mat[16] = {0};
		cgltf_node_transform_world(node, mat);
		
		for (u32 i = 0; i < 4; i++)
		{
			for (u32 j = 0; j < 4; j++)
			{
				mesh->transform.v[i][j] = mat[i * 4 + j];
			}
		}
		
		it->mesh_index++;
	}
	
	for (u32 i = 0; i < node->children_count; ++i) 
	{
		gltf_traverseNode(it, node->children[i]);
	}
}

function void gltf_print(GLTF_Scene *scene)
{
	for(u32 i = 0; i < scene->num_meshes; i++)
	{
		GLTF_Mesh *mesh = scene->meshes + i;
		
		printf("indices %u\n", i);
		for(u32 j = 0; j < mesh->num_indices; j++)
		{
			printf("%u, ", mesh->indices[j]);
		}
		printf("\n");
		
		printf("verticess %u\n", i);
		for(u32 j = 0; j < mesh->num_vertices; j++)
		{
			R_Vertex *vert = mesh->vertices + j;
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

function GLTF_Scene gltf_loadMesh(Arena *arena, Arena *scratch, Str8 filepath)
{
	GLTF_Scene out = {0};
	
	GLTF_It it = {0};
	
	it.dir = os_dirFromFile(scratch, filepath);
	
	cgltf_options options = {0};
	cgltf_data *data = 0;
	
	if(cgltf_parse_file(&options, filepath.c, &data) != cgltf_result_success)
	{
		printf("Couldnt find file\n");
		INVALID_CODE_PATH();
	}
	
	if(cgltf_load_buffers(&options, data, filepath.c) == cgltf_result_success)
	{
		Str8 dir = os_dirFromFile(scratch, filepath);

		// load textures
		out.num_textures = data->textures_count;
		out.textures = pushArray(arena, Bitmap, data->textures_count);
	
		for(u32 i = 0; i < data->textures_count; i++)
		{
			Str8 uri_str = {0};
			uri_str.c = pushArray(scratch, u8, strlen(data->textures[i].image->uri));
			uri_str.len = strlen(data->textures[i].image->uri);
			
			memcpy(uri_str.c, data->textures[i].image->uri, uri_str.len);

			Str8 texture_abs_path = str8_join(scratch, dir, uri_str);

			out.textures[i] = bitmap(arena, texture_abs_path);
		}
		
		// load meshes
		out.num_meshes = data->meshes_count;
		out.meshes = pushArray(arena, GLTF_Mesh, out.num_meshes);
		
		it.scene = &out;
		it.arena = arena;
		it.data = data;
		
		for(u32 i = 0; i < data->scenes_count; i++)
		{
			cgltf_scene *scene = data->scenes + i;
			
			for(u32 j = 0; j < scene->nodes_count; j++)
			{
				cgltf_node *node = scene->nodes[j];
				
				gltf_traverseNode(&it, node);
			}
		}
		
		//gltf_print(it.scene);
		
	}

	cgltf_free(data);
	
	return out;
}

function void gltf_upload(Arena *arena, GLTF_Scene *scene)
{
	scene->gpu_textures = pushArray(arena, R_Handle, scene->num_textures);
	for(u32 i = 0; i < scene->num_textures; i++)
	{
		scene->gpu_textures[i] = r_image(scene->textures[i]);
	}

	for(u32 i = 0; i < scene->num_meshes; i++)
	{
		GLTF_Mesh *mesh = scene->meshes + i;
		mesh->gpu_buffer = r_meshBuffer(mesh->vertices, mesh->num_vertices, mesh->indices, mesh->num_indices);
	}
}

function void gltf_draw(R_Batch *batch, M4F transform, GLTF_Scene *scene)
{
	for(u32 i = 0; i < scene->num_meshes; i++)
	{
		GLTF_Mesh *mesh = scene->meshes + i;
		r_beginMesh(batch, mesh->num_primitives, mesh->gpu_buffer);

		for(u32 j = 0; j < mesh->num_primitives; j++)
		{
			GLTF_Primitive *primitive = mesh->primitives + j;
			R_Handle base_tex = scene->gpu_textures[primitive->base_tex_index];
			R_Handle normal_tex = scene->gpu_textures[primitive->normal_tex_index];
			
			r_pushMesh(batch, primitive->start, primitive->count, base_tex, normal_tex, m4f_mul(mesh->transform, transform));
		}
	}
}