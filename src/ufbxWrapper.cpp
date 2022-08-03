

#define UFBX_MAX_BONES 64
#define UFBX_MAX_BLEND_SHAPES 64

um_vec2 ufbx_to_um_vec2(ufbx_vec2 v) { return um_v2((float)v.x, (float)v.y); }
um_vec3 ufbx_to_um_vec3(ufbx_vec3 v) { return um_v3((float)v.x, (float)v.y, (float)v.z); }
um_quat ufbx_to_um_quat(ufbx_quat v) { return um_quat_xyzw((float)v.x, (float)v.y, (float)v.z, (float)v.w); }
um_mat ufbx_to_um_mat(ufbx_matrix m) {
	return um_mat_rows(
		(float)m.m00, (float)m.m01, (float)m.m02, (float)m.m03,
		(float)m.m10, (float)m.m11, (float)m.m12, (float)m.m13,
		(float)m.m20, (float)m.m21, (float)m.m22, (float)m.m23,
		0, 0, 0, 1,
	);
}

typedef struct mesh_vertex {
	um_vec3 position;
	um_vec3 normal;
	um_vec2 uv;
	float f_vertex_index;
} mesh_vertex;

typedef struct skin_vertex {
	uint8_t bone_index[4];
	uint8_t bone_weight[4];
} skin_vertex;


void print_error(const ufbx_error *error, const char *description)
{
	char buffer[1024];
	ufbx_format_error(buffer, sizeof(buffer), error);
	fprintf(stderr, "%s\n%s\n", description, buffer);
}

void *alloc_imp(size_t type_size, size_t count)
{
	void *ptr = malloc(type_size * count);
	if (!ptr) {
		fprintf(stderr, "Out of memory\n");
		exit(1);
	}
	memset(ptr, 0, type_size * count);
	return ptr;
}

void *alloc_dup_imp(size_t type_size, size_t count, const void *data)
{
	void *ptr = malloc(type_size * count);
	if (!ptr) {
		fprintf(stderr, "Out of memory\n");
		exit(1);
	}
	memcpy(ptr, data, type_size * count);
	return ptr;
}

#define alloc(m_type, m_count) (m_type*)alloc_imp(sizeof(m_type), (m_count))
#define alloc_dup(m_type, m_count, m_data) (m_type*)alloc_dup_imp(sizeof(m_type), (m_count), (m_data))

size_t min_sz(size_t a, size_t b) { return a < b ? a : b; }
size_t max_sz(size_t a, size_t b) { return b < a ? a : b; }
size_t clamp_sz(size_t a, size_t min_a, size_t max_a) { return min_sz(max_sz(a, min_a), max_a); }

typedef struct viewer_node_anim {
	float time_begin;
	float framerate;
	size_t num_frames;
	um_quat const_rot;
	um_vec3 const_pos;
	um_vec3 const_scale;
	um_quat *rot;
	um_vec3 *pos;
	um_vec3 *scale;
} viewer_node_anim;

typedef struct viewer_blend_channel_anim {
	float const_weight;
	float *weight;
} viewer_blend_channel_anim;

typedef struct viewer_anim {
	const char *name;
	float time_begin;
	float time_end;
	float framerate;
	size_t num_frames;

	viewer_node_anim *nodes;
	viewer_blend_channel_anim *blend_channels;
} viewer_anim;

typedef struct viewer_node {
	int32_t parent_index;

	um_mat geometry_to_node;
	um_mat node_to_parent;
	um_mat node_to_world;
	um_mat geometry_to_world;
	um_mat normal_to_world;
} viewer_node;

typedef struct viewer_mesh_part {
	sg_buffer vertex_buffer;
	sg_buffer index_buffer;
	sg_buffer skin_buffer; // Optional

	size_t num_indices;
	int32_t material_index;
} viewer_mesh_part;

typedef struct viewer_mesh {

	int32_t *instance_node_indices;
	size_t num_instances;

	viewer_mesh_part *parts;
	size_t num_parts;

	bool aabb_is_local;
	um_vec3 aabb_min;
	um_vec3 aabb_max;

	// Skinning (optional)
	bool skinned;
	size_t num_bones;
	int32_t bone_indices[UFBX_MAX_BONES];
	um_mat bone_matrices[UFBX_MAX_BONES];

	// Blend shapes (optional)
	size_t num_blend_shapes;
	sg_image blend_shape_image;
	int32_t blend_channel_indices[UFBX_MAX_BLEND_SHAPES];

} viewer_mesh;

typedef struct viewer_scene {

	viewer_node *nodes;
	size_t num_nodes;

	viewer_mesh *meshes;
	size_t num_meshes;

	viewer_blend_channel *blend_channels;
	size_t num_blend_channels;

	viewer_anim *animations;
	size_t num_animations;

	um_vec3 aabb_min;
	um_vec3 aabb_max;

} viewer_scene;

typedef struct viewer {

	viewer_scene scene;
	float anim_time;

	sg_shader shader_mesh_lit_static;
	sg_shader shader_mesh_lit_skinned;
	sg_pipeline pipe_mesh_lit_static;
	sg_pipeline pipe_mesh_lit_skinned;
	sg_image empty_blend_shape_image;

	um_mat world_to_view;
	um_mat view_to_clip;
	um_mat world_to_clip;

	float camera_yaw;
	float camera_pitch;
	float camera_distance;
	uint32_t mouse_buttons;
} viewer;


void read_node(viewer_node *vnode, ufbx_node *node)
{
	vnode->parent_index = node->parent ? node->parent->typed_id : -1;
	vnode->node_to_parent = ufbx_to_um_mat(node->node_to_parent);
	vnode->node_to_world = ufbx_to_um_mat(node->node_to_world);
	vnode->geometry_to_node = ufbx_to_um_mat(node->geometry_to_node);
	vnode->geometry_to_world = ufbx_to_um_mat(node->geometry_to_world);
	vnode->normal_to_world = ufbx_to_um_mat(ufbx_matrix_for_normals(&node->geometry_to_world));
}


void read_mesh(viewer_mesh *vmesh, ufbx_mesh *mesh)
{
	// Count the number of needed parts and temporary buffers
	size_t max_parts = 0;
	size_t max_triangles = 0;

	// We need to render each material of the mesh in a separate part, so let's
	// count the number of parts and maximum number of triangles needed.
	for (size_t pi = 0; pi < mesh->materials.count; pi++) {
		ufbx_mesh_material *mesh_mat = &mesh->materials.data[pi];
		if (mesh_mat->num_triangles == 0) continue;
		max_parts += 1;
		max_triangles = max_sz(max_triangles, mesh_mat->num_triangles);
	}

	// Temporary buffers
	size_t num_tri_indices = mesh->max_face_triangles * 3;
	uint32_t *tri_indices = alloc(uint32_t, num_tri_indices);
	mesh_vertex *vertices = alloc(mesh_vertex, max_triangles * 3);
	skin_vertex *skin_vertices = alloc(skin_vertex, max_triangles * 3);
	skin_vertex *mesh_skin_vertices = alloc(skin_vertex, mesh->num_vertices);
	uint32_t *indices = alloc(uint32_t, max_triangles * 3);

	// Result buffers
	viewer_mesh_part *parts = alloc(viewer_mesh_part, max_parts);
	size_t num_parts = 0;

	// In FBX files a single mesh can be instanced by multiple nodes. ufbx handles the connection
	// in two ways: (1) `ufbx_node.mesh/light/camera/etc` contains pointer to the data "attribute"
	// that node uses and (2) each element that can be connected to a node contains a list of
	// `ufbx_node*` instances eg. `ufbx_mesh.instances`.
	vmesh->num_instances = mesh->instances.count;
	vmesh->instance_node_indices = alloc(int32_t, mesh->instances.count);
	for (size_t i = 0; i < mesh->instances.count; i++) {
		vmesh->instance_node_indices[i] = (int32_t)mesh->instances.data[i]->typed_id;
	}

	// Create the vertex buffers
	size_t num_blend_shapes = 0;
	ufbx_blend_channel *blend_channels[UFBX_MAX_BLEND_SHAPES];
	size_t num_bones = 0;
	ufbx_skin_deformer *skin = NULL;

	if (mesh->skin_deformers.count > 0) {
		vmesh->skinned = true;

		// Having multiple skin deformers attached at once is exceedingly rare so we can just
		// pick the first one without having to worry too much about it.
		skin = mesh->skin_deformers.data[0];

		// NOTE: A proper implementation would split meshes with too many bones to chunks but
		// for simplicity we're going to just pick the first `UFBX_MAX_BONES` ones.
		for (size_t ci = 0; ci < skin->clusters.count; ci++) {
			ufbx_skin_cluster *cluster = skin->clusters.data[ci];
			if (num_bones < UFBX_MAX_BONES) {
				vmesh->bone_indices[num_bones] = (int32_t)cluster->bone_node->typed_id;
				vmesh->bone_matrices[num_bones] = ufbx_to_um_mat(cluster->geometry_to_bone);
				num_bones++;
			}
		}
		vmesh->num_bones = num_bones;

		// Pre-calculate the skinned vertex bones/weights for each vertex as they will probably
		// be shared by multiple indices.
		for (size_t vi = 0; vi < mesh->num_vertices; vi++) {
			size_t num_weights = 0;
			float total_weight = 0.0f;
			float weights[4] = { 0.0f };
			uint8_t clusters[4] = { 0 };

			// `ufbx_skin_vertex` contains the offset and number of weights that deform the vertex
			// in a descending weight order so we can pick the first N weights to use and get a
			// reasonable approximation of the skinning.
			ufbx_skin_vertex vertex_weights = skin->vertices.data[vi];
			for (size_t wi = 0; wi < vertex_weights.num_weights; wi++) {
				if (num_weights >= 4) break;
				ufbx_skin_weight weight = skin->weights.data[vertex_weights.weight_begin + wi];

				// Since we only support a fixed amount of bones up to `UFBX_MAX_BONES` and we take the
				// first N ones we need to ignore weights with too high `cluster_index`.
				if (weight.cluster_index < UFBX_MAX_BONES) {
					total_weight += (float)weight.weight;
					clusters[num_weights] = (uint8_t)weight.cluster_index;
					weights[num_weights] = (float)weight.weight;
					num_weights++;
				}
			}

			// Normalize and quantize the weights to 8 bits. We need to be a bit careful to make
			// sure the _quantized_ sum is normalized ie. all 8-bit values sum to 255.
			if (total_weight > 0.0f) {
				skin_vertex *skin_vert = &mesh_skin_vertices[vi];
				uint32_t quantized_sum = 0;
				for (size_t i = 0; i < 4; i++) {
					uint8_t quantized_weight = (uint8_t)((float)weights[i] / total_weight * 255.0f);
					quantized_sum += quantized_weight;
					skin_vert->bone_index[i] = clusters[i];
					skin_vert->bone_weight[i] = quantized_weight;
				}
				skin_vert->bone_weight[0] += 255 - quantized_sum;
			}
		}
	}

	// Fetch blend channels from all attached blend deformers.
	for (size_t di = 0; di < mesh->blend_deformers.count; di++) {
		ufbx_blend_deformer *deformer = mesh->blend_deformers.data[di];
		for (size_t ci = 0; ci < deformer->channels.count; ci++) {
			ufbx_blend_channel *chan = deformer->channels.data[ci];
			if (chan->keyframes.count == 0) continue;
			if (num_blend_shapes < UFBX_MAX_BLEND_SHAPES) {
				blend_channels[num_blend_shapes] = chan;
				vmesh->blend_channel_indices[num_blend_shapes] = (int32_t)chan->typed_id;
				num_blend_shapes++;
			}
		}
	}
	if (num_blend_shapes > 0) {
		vmesh->blend_shape_image = pack_blend_channels_to_image(mesh, blend_channels, num_blend_shapes);
		vmesh->num_blend_shapes = num_blend_shapes;
	}

	// Our shader supports only a single material per draw call so we need to split the mesh
	// into parts by material. `ufbx_mesh_material` contains a handy compact list of faces
	// that use the material which we use here.
	for (size_t pi = 0; pi < mesh->materials.count; pi++) {
		ufbx_mesh_material *mesh_mat = &mesh->materials.data[pi];
		if (mesh_mat->num_triangles == 0) continue;

		viewer_mesh_part *part = &parts[num_parts++];
		size_t num_indices = 0;

		// First fetch all vertices into a flat non-indexed buffer, we also need to
		// triangulate the faces
		for (size_t fi = 0; fi < mesh_mat->num_faces; fi++) {
			ufbx_face face = mesh->faces[mesh_mat->face_indices[fi]];
			size_t num_tris = ufbx_triangulate_face(tri_indices, num_tri_indices, mesh, face);

			ufbx_vec2 default_uv = { 0 };

			// Iterate through every vertex of every triangle in the triangulated result
			for (size_t vi = 0; vi < num_tris * 3; vi++) {
				uint32_t ix = tri_indices[vi];
				mesh_vertex *vert = &vertices[num_indices];

				ufbx_vec3 pos = ufbx_get_vertex_vec3(&mesh->vertex_position, ix);
				ufbx_vec3 normal = ufbx_get_vertex_vec3(&mesh->vertex_normal, ix);
				ufbx_vec2 uv = mesh->vertex_uv.data ? ufbx_get_vertex_vec2(&mesh->vertex_uv, ix) : default_uv;

				vert->position = ufbx_to_um_vec3(pos);
				vert->normal = um_normalize3(ufbx_to_um_vec3(normal));
				vert->uv = ufbx_to_um_vec2(uv);
				vert->f_vertex_index = (float)mesh->vertex_indices[ix];

				// The skinning vertex stream is pre-calculated above so we just need to
				// copy the right one by the vertex index.
				if (skin) {
					skin_vertices[num_indices] = mesh_skin_vertices[mesh->vertex_indices[ix]];
				}

				num_indices++;
			}
		}

		ufbx_vertex_stream streams[2];
		size_t num_streams = 1;

		streams[0].data = vertices;
		streams[0].vertex_size = sizeof(mesh_vertex);

		if (skin) {
			streams[1].data = skin_vertices;
			streams[1].vertex_size = sizeof(skin_vertex);
			num_streams = 2;
		}

		// Optimize the flat vertex buffer into an indexed one. `ufbx_generate_indices()`
		// compacts the vertex buffer and returns the number of used vertices.
		ufbx_error error;
		size_t num_vertices = ufbx_generate_indices(streams, num_streams, indices, num_indices, NULL, &error);
		if (error.type != UFBX_ERROR_NONE) {
			print_error(&error, "Failed to generate index buffer");
			exit(1);
		}

		// To unify code we use `ufbx_load_opts.allow_null_material` to make ufbx create a
		// `ufbx_mesh_material` even if there are no materials, so it might be `NULL` here.
		part->num_indices = num_indices;
		if (mesh_mat->material) {
			part->material_index = (int32_t)mesh_mat->material->typed_id;
		} else {
			part->material_index = -1;
		}

		// Create the GPU buffers from the temporary `vertices` and `indices` arrays
		part->index_buffer = sg_make_buffer(&(sg_buffer_desc){
			.size = num_indices * sizeof(uint32_t),
			.type = SG_BUFFERTYPE_INDEXBUFFER,
			.data = { indices, num_indices * sizeof(uint32_t) },
		});
		part->vertex_buffer = sg_make_buffer(&(sg_buffer_desc){
			.size = num_vertices * sizeof(mesh_vertex),
			.type = SG_BUFFERTYPE_VERTEXBUFFER,
			.data = { vertices, num_vertices * sizeof(mesh_vertex) },
		});

		if (vmesh->skinned) {
			part->skin_buffer = sg_make_buffer(&(sg_buffer_desc){
				.size = num_vertices * sizeof(skin_vertex),
				.type = SG_BUFFERTYPE_VERTEXBUFFER,
				.data = { skin_vertices, num_vertices * sizeof(skin_vertex) },
			});
		}
	}

	// Free the temporary buffers
	free(tri_indices);
	free(vertices);
	free(skin_vertices);
	free(mesh_skin_vertices);
	free(indices);

	// Compute bounds from the vertices
	vmesh->aabb_is_local = mesh->skinned_is_local;
	vmesh->aabb_min = um_dup3(+INFINITY);
	vmesh->aabb_max = um_dup3(-INFINITY);
	for (size_t i = 0; i < mesh->num_vertices; i++) {
		um_vec3 pos = ufbx_to_um_vec3(mesh->skinned_position.data[i]);
		vmesh->aabb_min = um_min3(vmesh->aabb_min, pos);
		vmesh->aabb_max = um_max3(vmesh->aabb_max, pos);
	}

	vmesh->parts = parts;
	vmesh->num_parts = num_parts;
}


void read_anim_stack(viewer_anim *va, ufbx_anim_stack *stack, ufbx_scene *scene)
{
	const float target_framerate = 30.0f;
	const size_t max_frames = 4096;

	// Sample the animation evenly at `target_framerate` if possible while limiting the maximum
	// number of frames to `max_frames` by potentially dropping FPS.
	float duration = (float)stack->time_end - (float)stack->time_begin;
	size_t num_frames = clamp_sz((size_t)(duration * target_framerate), 2, max_frames);
	float framerate = (float)(num_frames - 1) / duration;

	va->name = alloc_dup(char, stack->name.length + 1, stack->name.data);
	va->time_begin = (float)stack->time_begin;
	va->time_end = (float)stack->time_end;
	va->framerate = framerate;
	va->num_frames = num_frames;

	// Sample the animations of all nodes and blend channels in the stack
	va->nodes = alloc(viewer_node_anim, scene->nodes.count);
	for (size_t i = 0; i < scene->nodes.count; i++) {
		ufbx_node *node = scene->nodes.data[i];
		read_node_anim(va, &va->nodes[i], stack, node);
	}

	va->blend_channels = alloc(viewer_blend_channel_anim, scene->blend_channels.count);
	for (size_t i = 0; i < scene->blend_channels.count; i++) {
		ufbx_blend_channel *chan = scene->blend_channels.data[i];
		read_blend_channel_anim(va, &va->blend_channels[i], stack, chan);
	}
}

void read_scene(viewer_scene *vs, ufbx_scene *scene)
{
	vs->num_nodes = scene->nodes.count;
	vs->nodes = alloc(viewer_node, vs->num_nodes);
	for (size_t i = 0; i < vs->num_nodes; i++) {
		read_node(&vs->nodes[i], scene->nodes.data[i]);
	}

	vs->num_meshes = scene->meshes.count;
	vs->meshes = alloc(viewer_mesh, vs->num_meshes);
	for (size_t i = 0; i < vs->num_meshes; i++) {
		read_mesh(&vs->meshes[i], scene->meshes.data[i]);
	}

	vs->num_blend_channels = scene->blend_channels.count;
	vs->blend_channels = alloc(viewer_blend_channel, vs->num_blend_channels);
	for (size_t i = 0; i < vs->num_blend_channels; i++) {
		read_blend_channel(&vs->blend_channels[i], scene->blend_channels.data[i]);
	}

	vs->num_animations = scene->anim_stacks.count;
	vs->animations = alloc(viewer_anim, vs->num_animations);
	for (size_t i = 0; i < vs->num_animations; i++) {
		read_anim_stack(&vs->animations[i], scene->anim_stacks.data[i], scene);
	}
}

void update_animation(viewer_scene *vs, viewer_anim *va, float time)
{
	float frame_time = (time - va->time_begin) * va->framerate;
	size_t f0 = min_sz((size_t)frame_time + 0, va->num_frames - 1);
	size_t f1 = min_sz((size_t)frame_time + 1, va->num_frames - 1);
	float t = um_min(frame_time - (float)f0, 1.0f);

	for (size_t i = 0; i < vs->num_nodes; i++) {
		viewer_node *vn = &vs->nodes[i];
		viewer_node_anim *vna = &va->nodes[i];

		um_quat rot = vna->rot ? um_quat_lerp(vna->rot[f0], vna->rot[f1], t) : vna->const_rot;
		um_vec3 pos = vna->pos ? um_lerp3(vna->pos[f0], vna->pos[f1], t) : vna->const_pos;
		um_vec3 scale = vna->scale ? um_lerp3(vna->scale[f0], vna->scale[f1], t) : vna->const_scale;

		vn->node_to_parent = um_mat_trs(pos, rot, scale);
	}

	for (size_t i = 0; i < vs->num_blend_channels; i++) {
		viewer_blend_channel *vbc = &vs->blend_channels[i];
		viewer_blend_channel_anim *vbca = &va->blend_channels[i];

		vbc->weight = vbca->weight ? um_lerp(vbca->weight[f0], vbca->weight[f1], t) : vbca->const_weight;
	}
}

void read_node_anim(viewer_anim *va, viewer_node_anim *vna, ufbx_anim_stack *stack, ufbx_node *node)
{
	vna->rot = alloc(um_quat, va->num_frames);
	vna->pos = alloc(um_vec3, va->num_frames);
	vna->scale = alloc(um_vec3, va->num_frames);

	bool const_rot = true, const_pos = true, const_scale = true;

	// Sample the node's transform evenly for the whole animation stack duration
	for (size_t i = 0; i < va->num_frames; i++) {
		double time = stack->time_begin + (double)i / va->framerate;

		ufbx_transform transform = ufbx_evaluate_transform(&stack->anim, node, time);
		vna->rot[i] = ufbx_to_um_quat(transform.rotation);
		vna->pos[i] = ufbx_to_um_vec3(transform.translation);
		vna->scale[i] = ufbx_to_um_vec3(transform.scale);

		if (i > 0) {
			// Negated quaternions are equivalent, but interpolating between ones of different
			// polarity takes a the longer path, so flip the quaternion if necessary.
			if (um_quat_dot(vna->rot[i], vna->rot[i - 1]) < 0.0f) {
				vna->rot[i] = um_quat_neg(vna->rot[i]);
			}

			// Keep track of which channels are constant for the whole animation as an optimization
			if (!um_quat_equal(vna->rot[i - 1], vna->rot[i])) const_rot = false;
			if (!um_equal3(vna->pos[i - 1], vna->pos[i])) const_pos = false;
			if (!um_equal3(vna->scale[i - 1], vna->scale[i])) const_scale = false;
		}
	}

	if (const_rot) { vna->const_rot = vna->rot[0]; free(vna->rot); vna->rot = NULL; }
	if (const_pos) { vna->const_pos = vna->pos[0]; free(vna->pos); vna->pos = NULL; }
	if (const_scale) { vna->const_scale = vna->scale[0]; free(vna->scale); vna->scale = NULL; }
}
