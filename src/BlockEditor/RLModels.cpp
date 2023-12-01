#include "stdafx.h"
#include "RL.h"
#include "RLMath.h"
#define PAR_SHAPES_IMPLEMENTATION
#include "par_shapes.h"

// Load vertex array object (VAO)
unsigned int rlLoadVertexArray()
{
	unsigned int vaoId = 0;
	glGenVertexArrays(1, &vaoId);
	return vaoId;
}

// Enable vertex array object (VAO)
bool rlEnableVertexArray(unsigned int vaoId)
{
	bool result = false;
	glBindVertexArray(vaoId);
	result = true;
	return result;
}

// Load a new attributes buffer
unsigned int rlLoadVertexBuffer(const void* buffer, int size, bool dynamic)
{
	unsigned int id = 0;

	glGenBuffers(1, &id);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, size, buffer, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

	return id;
}

// Set vertex attribute
void rlSetVertexAttribute(unsigned int index, int compSize, int type, bool normalized, int stride, const void* pointer)
{
	// NOTE: Data type could be: GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT
	// Additional types (depends on OpenGL version or extensions): 
	//  - GL_HALF_FLOAT, GL_FLOAT, GL_DOUBLE, GL_FIXED, 
	//  - GL_INT_2_10_10_10_REV, GL_UNSIGNED_INT_2_10_10_10_REV, GL_UNSIGNED_INT_10F_11F_11F_REV
	glVertexAttribPointer(index, compSize, type, normalized, stride, pointer);
}

// Enable vertex attribute index
void rlEnableVertexAttribute(unsigned int index)
{
	glEnableVertexAttribArray(index);
}

// Set shader value attribute
void rlSetVertexAttributeDefault(int locIndex, const void* value, int attribType, int count)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
	switch (attribType)
	{
	case RL_SHADER_ATTRIB_FLOAT: if (count == 1) glVertexAttrib1fv(locIndex, (float*)value); break;
	case RL_SHADER_ATTRIB_VEC2: if (count == 2) glVertexAttrib2fv(locIndex, (float*)value); break;
	case RL_SHADER_ATTRIB_VEC3: if (count == 3) glVertexAttrib3fv(locIndex, (float*)value); break;
	case RL_SHADER_ATTRIB_VEC4: if (count == 4) glVertexAttrib4fv(locIndex, (float*)value); break;
	default: TRACELOG(RL_LOG_WARNING, "SHADER: Failed to set attrib default value, data type not recognized");
	}
#endif
}

// Disable vertex attribute index
void rlDisableVertexAttribute(unsigned int index)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
	glDisableVertexAttribArray(index);
#endif
}

// Load a new attributes element buffer
unsigned int rlLoadVertexBufferElement(const void* buffer, int size, bool dynamic)
{
	unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
	glGenBuffers(1, &id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, buffer, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
#endif

	return id;
}

// Disable vertex array object (VAO)
void rlDisableVertexArray(void)
{
	glBindVertexArray(0);
}

// Load default material (Supports: DIFFUSE, SPECULAR, NORMAL maps)
RLMaterial LoadMaterialDefault()
{
	RLMaterial material = { 0 };
	material.maps = (RLMaterialMap*)RL_CALLOC(MAX_MATERIAL_MAPS, sizeof(RLMaterialMap));

	// Using rlgl default shader
	material.shader.id = rlGetShaderIdDefault();
	material.shader.locs = rlGetShaderLocsDefault();

	// Using rlgl default texture (1x1 pixel, UNCOMPRESSED_R8G8B8A8, 1 mipmap)
	material.maps[MATERIAL_MAP_DIFFUSE].texture = { rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
	//material.maps[MATERIAL_MAP_NORMAL].texture;         // NOTE: By default, not set
	//material.maps[MATERIAL_MAP_SPECULAR].texture;       // NOTE: By default, not set

	material.maps[MATERIAL_MAP_DIFFUSE].color = RLWHITE;    // Diffuse color
	material.maps[MATERIAL_MAP_SPECULAR].color = RLWHITE;   // Specular color

	return material;
}

// Upload vertex data into a VAO (if supported) and VBO
void UploadMesh(RLMesh* mesh, bool dynamic)
{
	if (mesh->vaoId > 0)
	{
		// Check if mesh has already been loaded in GPU
		TRACELOG(LOG_WARNING, "VAO: [ID %i] Trying to re-load an already loaded mesh", mesh->vaoId);
		return;
	}

	mesh->vboId = (unsigned int*)RL_CALLOC(MAX_MESH_VERTEX_BUFFERS, sizeof(unsigned int));

	mesh->vaoId = 0;        // Vertex Array Object
	mesh->vboId[0] = 0;     // Vertex buffer: positions
	mesh->vboId[1] = 0;     // Vertex buffer: texcoords
	mesh->vboId[2] = 0;     // Vertex buffer: normals
	mesh->vboId[3] = 0;     // Vertex buffer: colors
	mesh->vboId[4] = 0;     // Vertex buffer: tangents
	mesh->vboId[5] = 0;     // Vertex buffer: texcoords2
	mesh->vboId[6] = 0;     // Vertex buffer: indices

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
	mesh->vaoId = rlLoadVertexArray();
	rlEnableVertexArray(mesh->vaoId);

	// NOTE: Vertex attributes must be uploaded considering default locations points and available vertex data

	// Enable vertex attributes: position (shader-location = 0)
	void* vertices = (mesh->animVertices != NULL) ? mesh->animVertices : mesh->vertices;
	mesh->vboId[0] = rlLoadVertexBuffer(vertices, mesh->vertexCount * 3 * sizeof(float), dynamic);
	rlSetVertexAttribute(0, 3, RL_FLOAT, 0, 0, 0);
	rlEnableVertexAttribute(0);

	// Enable vertex attributes: texcoords (shader-location = 1)
	mesh->vboId[1] = rlLoadVertexBuffer(mesh->texcoords, mesh->vertexCount * 2 * sizeof(float), dynamic);
	rlSetVertexAttribute(1, 2, RL_FLOAT, 0, 0, 0);
	rlEnableVertexAttribute(1);

	// WARNING: When setting default vertex attribute values, the values for each generic vertex attribute
	// is part of current state, and it is maintained even if a different program object is used

	if (mesh->normals != NULL)
	{
		// Enable vertex attributes: normals (shader-location = 2)
		void* normals = (mesh->animNormals != NULL) ? mesh->animNormals : mesh->normals;
		mesh->vboId[2] = rlLoadVertexBuffer(normals, mesh->vertexCount * 3 * sizeof(float), dynamic);
		rlSetVertexAttribute(2, 3, RL_FLOAT, 0, 0, 0);
		rlEnableVertexAttribute(2);
	}
	else
	{
		// Default vertex attribute: normal
		// WARNING: Default value provided to shader if location available
		float value[3] = { 1.0f, 1.0f, 1.0f };
		rlSetVertexAttributeDefault(2, value, SHADER_ATTRIB_VEC3, 3);
		rlDisableVertexAttribute(2);
	}

	if (mesh->colors != NULL)
	{
		// Enable vertex attribute: color (shader-location = 3)
		mesh->vboId[3] = rlLoadVertexBuffer(mesh->colors, mesh->vertexCount * 4 * sizeof(unsigned char), dynamic);
		rlSetVertexAttribute(3, 4, RL_UNSIGNED_BYTE, 1, 0, 0);
		rlEnableVertexAttribute(3);
	}
	else
	{
		// Default vertex attribute: color
		// WARNING: Default value provided to shader if location available
		float value[4] = { 1.0f, 1.0f, 1.0f, 1.0f };    // WHITE
		rlSetVertexAttributeDefault(3, value, SHADER_ATTRIB_VEC4, 4);
		rlDisableVertexAttribute(3);
	}

	if (mesh->tangents != NULL)
	{
		// Enable vertex attribute: tangent (shader-location = 4)
		mesh->vboId[4] = rlLoadVertexBuffer(mesh->tangents, mesh->vertexCount * 4 * sizeof(float), dynamic);
		rlSetVertexAttribute(4, 4, RL_FLOAT, 0, 0, 0);
		rlEnableVertexAttribute(4);
	}
	else
	{
		// Default vertex attribute: tangent
		// WARNING: Default value provided to shader if location available
		float value[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		rlSetVertexAttributeDefault(4, value, SHADER_ATTRIB_VEC4, 4);
		rlDisableVertexAttribute(4);
	}

	if (mesh->texcoords2 != NULL)
	{
		// Enable vertex attribute: texcoord2 (shader-location = 5)
		mesh->vboId[5] = rlLoadVertexBuffer(mesh->texcoords2, mesh->vertexCount * 2 * sizeof(float), dynamic);
		rlSetVertexAttribute(5, 2, RL_FLOAT, 0, 0, 0);
		rlEnableVertexAttribute(5);
	}
	else
	{
		// Default vertex attribute: texcoord2
		// WARNING: Default value provided to shader if location available
		float value[2] = { 0.0f, 0.0f };
		rlSetVertexAttributeDefault(5, value, SHADER_ATTRIB_VEC2, 2);
		rlDisableVertexAttribute(5);
	}

	if (mesh->indices != NULL)
	{
		mesh->vboId[6] = rlLoadVertexBufferElement(mesh->indices, mesh->triangleCount * 3 * sizeof(unsigned short), dynamic);
	}

	if (mesh->vaoId > 0) TRACELOG(LOG_INFO, "VAO: [ID %i] Mesh uploaded successfully to VRAM (GPU)", mesh->vaoId);
	else TRACELOG(LOG_INFO, "VBO: Mesh uploaded successfully to VRAM (GPU)");

	rlDisableVertexArray();
#endif
}

// Unload vertex array object (VAO)
void rlUnloadVertexArray(unsigned int vaoId)
{
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vaoId);
	TRACELOG(RL_LOG_INFO, "VAO: [ID %i] Unloaded vertex array data from VRAM (GPU)", vaoId);
}

// Unload vertex buffer (VBO)
void rlUnloadVertexBuffer(unsigned int vboId)
{
	glDeleteBuffers(1, &vboId);
}

// Unload model (meshes/materials) from memory (RAM and/or VRAM)
// NOTE: This function takes care of all model elements, for a detailed control
// over them, use UnloadMesh() and UnloadMaterial()
void UnloadModel(RLModel model)
{
	// Unload meshes
	for (int i = 0; i < model.meshCount; i++) UnloadMesh(model.meshes[i]);

	// Unload materials maps
	// NOTE: As the user could be sharing shaders and textures between models,
	// we don't unload the material but just free its maps,
	// the user is responsible for freeing models shaders and textures
	for (int i = 0; i < model.materialCount; i++) RL_FREE(model.materials[i].maps);

	// Unload arrays
	RL_FREE(model.meshes);
	RL_FREE(model.materials);
	RL_FREE(model.meshMaterial);

	// Unload animation data
	RL_FREE(model.bones);
	RL_FREE(model.bindPose);

	TRACELOG(LOG_INFO, "MODEL: Unloaded model (and meshes) from RAM and VRAM");
}

// Unload mesh from memory (RAM and VRAM)
void UnloadMesh(RLMesh mesh)
{
	// Unload rlgl mesh vboId data
	rlUnloadVertexArray(mesh.vaoId);

	if (mesh.vboId != NULL) for (int i = 0; i < MAX_MESH_VERTEX_BUFFERS; i++) rlUnloadVertexBuffer(mesh.vboId[i]);
	RL_FREE(mesh.vboId);

	RL_FREE(mesh.vertices);
	RL_FREE(mesh.texcoords);
	RL_FREE(mesh.normals);
	RL_FREE(mesh.colors);
	RL_FREE(mesh.tangents);
	RL_FREE(mesh.texcoords2);
	RL_FREE(mesh.indices);

	RL_FREE(mesh.animVertices);
	RL_FREE(mesh.animNormals);
	RL_FREE(mesh.boneWeights);
	RL_FREE(mesh.boneIds);
}

// Load model from generated mesh
// WARNING: A shallow copy of mesh is generated, passed by value,
// as long as struct contains pointers to data and some values, we get a copy
// of mesh pointing to same data as original version... be careful!
RLModel LoadModelFromMesh(RLMesh mesh)
{
	RLModel model = { 0 };

	model.transform = MatrixIdentity();

	model.meshCount = 1;
	model.meshes = (RLMesh*)RL_CALLOC(model.meshCount, sizeof(RLMesh));
	model.meshes[0] = mesh;

	model.materialCount = 1;
	model.materials = (RLMaterial*)RL_CALLOC(model.materialCount, sizeof(RLMaterial));
	model.materials[0] = LoadMaterialDefault();

	model.meshMaterial = (int*)RL_CALLOC(model.meshCount, sizeof(int));
	model.meshMaterial[0] = 0;  // First material index

	return model;
}

// Generate sphere mesh (standard sphere)
RLMesh GenMeshSphere(float radius, int rings, int slices)
{
	RLMesh mesh = { 0 };

	if ((rings >= 3) && (slices >= 3))
	{
		par_shapes_mesh* sphere = par_shapes_create_parametric_sphere(slices, rings);
		par_shapes_scale(sphere, radius, radius, radius);
		// NOTE: Soft normals are computed internally

		mesh.vertices = (float*)RL_MALLOC(sphere->ntriangles * 3 * 3 * sizeof(float));
		mesh.texcoords = (float*)RL_MALLOC(sphere->ntriangles * 3 * 2 * sizeof(float));
		mesh.normals = (float*)RL_MALLOC(sphere->ntriangles * 3 * 3 * sizeof(float));

		mesh.vertexCount = sphere->ntriangles * 3;
		mesh.triangleCount = sphere->ntriangles;

		for (int k = 0; k < mesh.vertexCount; k++)
		{
			mesh.vertices[k * 3] = sphere->points[sphere->triangles[k] * 3];
			mesh.vertices[k * 3 + 1] = sphere->points[sphere->triangles[k] * 3 + 1];
			mesh.vertices[k * 3 + 2] = sphere->points[sphere->triangles[k] * 3 + 2];

			mesh.normals[k * 3] = sphere->normals[sphere->triangles[k] * 3];
			mesh.normals[k * 3 + 1] = sphere->normals[sphere->triangles[k] * 3 + 1];
			mesh.normals[k * 3 + 2] = sphere->normals[sphere->triangles[k] * 3 + 2];

			mesh.texcoords[k * 2] = sphere->tcoords[sphere->triangles[k] * 2];
			mesh.texcoords[k * 2 + 1] = sphere->tcoords[sphere->triangles[k] * 2 + 1];
		}

		par_shapes_free_mesh(sphere);

		// Upload vertex data to GPU (static mesh)
		UploadMesh(&mesh, false);
	}
	else TRACELOG(LOG_WARNING, "MESH: Failed to generate mesh: sphere");

	return mesh;
}

// Draw vertex array
void rlDrawVertexArray(int offset, int count)
{
	glDrawArrays(GL_TRIANGLES, offset, count);
}

// Draw vertex array elements
void rlDrawVertexArrayElements(int offset, int count, const void* buffer)
{
	// NOTE: Added pointer math separately from function to avoid UBSAN complaining
	unsigned short* bufferPtr = (unsigned short*)buffer;
	if (offset > 0) bufferPtr += offset;

	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (const unsigned short*)bufferPtr);
}

// Disable vertex buffer (VBO)
void rlDisableVertexBuffer(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
}

// Enable vertex buffer (VBO)
void rlEnableVertexBuffer(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
	glBindBuffer(GL_ARRAY_BUFFER, id);
#endif
}

// Draw vertex array elements instanced
void rlDrawVertexArrayElementsInstanced(int offset, int count, const void* buffer, int instances)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
	// NOTE: Added pointer math separately from function to avoid UBSAN complaining
	unsigned short* bufferPtr = (unsigned short*)buffer;
	if (offset > 0) bufferPtr += offset;

	glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (const unsigned short*)bufferPtr, instances);
#endif
}

// Draw vertex array instanced
void rlDrawVertexArrayInstanced(int offset, int count, int instances)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
	glDrawArraysInstanced(GL_TRIANGLES, 0, count, instances);
#endif
}

// Disable vertex buffer element (VBO element)
void rlDisableVertexBufferElement(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
}

// Set texture for a material map type (MATERIAL_MAP_DIFFUSE, MATERIAL_MAP_SPECULAR...)
// NOTE: Previous texture should be manually unloaded
void SetMaterialTexture(RLMaterial* material, int mapType, RLTexture2D texture)
{
	material->maps[mapType].texture = texture;
}

// Enable vertex buffer element (VBO element)
void rlEnableVertexBufferElement(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
#endif
}

// Draw multiple mesh instances with material and different transforms
void DrawMeshInstanced(RLMesh mesh, RLMaterial material, const Matrix* transforms, int instances)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
	// Instancing required variables
	float16* instanceTransforms = NULL;
	unsigned int instancesVboId = 0;

	// Bind shader program
	rlEnableShader(material.shader.id);

	// Send required data to shader (matrices, values)
	//-----------------------------------------------------
	// Upload to shader material.colDiffuse
	if (material.shader.locs[SHADER_LOC_COLOR_DIFFUSE] != -1)
	{
		float values[4] = {
			(float)material.maps[MATERIAL_MAP_DIFFUSE].color.r / 255.0f,
			(float)material.maps[MATERIAL_MAP_DIFFUSE].color.g / 255.0f,
			(float)material.maps[MATERIAL_MAP_DIFFUSE].color.b / 255.0f,
			(float)material.maps[MATERIAL_MAP_DIFFUSE].color.a / 255.0f
		};

		rlSetUniform(material.shader.locs[SHADER_LOC_COLOR_DIFFUSE], values, SHADER_UNIFORM_VEC4, 1);
	}

	// Upload to shader material.colSpecular (if location available)
	if (material.shader.locs[SHADER_LOC_COLOR_SPECULAR] != -1)
	{
		float values[4] = {
			(float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.r / 255.0f,
			(float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.g / 255.0f,
			(float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.b / 255.0f,
			(float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.a / 255.0f
		};

		rlSetUniform(material.shader.locs[SHADER_LOC_COLOR_SPECULAR], values, SHADER_UNIFORM_VEC4, 1);
	}

	// Get a copy of current matrices to work with,
	// just in case stereo render is required, and we need to modify them
	// NOTE: At this point the modelview matrix just contains the view matrix (camera)
	// That's because BeginMode3D() sets it and there is no model-drawing function
	// that modifies it, all use rlPushMatrix() and rlPopMatrix()
	Matrix matModel = MatrixIdentity();
	Matrix matView = rlGetMatrixModelview(); // TODO: здесь передалть
	Matrix matModelView = MatrixIdentity();
	Matrix matProjection = rlGetMatrixProjection();

	// Upload view and projection matrices (if locations available)
	if (material.shader.locs[SHADER_LOC_MATRIX_VIEW] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_VIEW], matView);
	if (material.shader.locs[SHADER_LOC_MATRIX_PROJECTION] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_PROJECTION], matProjection);

	// Create instances buffer
	instanceTransforms = (float16*)RL_MALLOC(instances * sizeof(float16));

	// Fill buffer with instances transformations as float16 arrays
	for (int i = 0; i < instances; i++) instanceTransforms[i] = MatrixToFloatV(transforms[i]);

	// Enable mesh VAO to attach new buffer
	rlEnableVertexArray(mesh.vaoId);

	// This could alternatively use a static VBO and either glMapBuffer() or glBufferSubData().
	// It isn't clear which would be reliably faster in all cases and on all platforms,
	// anecdotally glMapBuffer() seems very slow (syncs) while glBufferSubData() seems
	// no faster, since we're transferring all the transform matrices anyway
	instancesVboId = rlLoadVertexBuffer(instanceTransforms, instances * sizeof(float16), false);

	// Instances transformation matrices are send to shader attribute location: SHADER_LOC_MATRIX_MODEL
	for (unsigned int i = 0; i < 4; i++)
	{
		rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i);
		rlSetVertexAttribute(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i, 4, RL_FLOAT, 0, sizeof(Matrix), (void*)(i * sizeof(Vector4)));
		rlSetVertexAttributeDivisor(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i, 1);
	}

	rlDisableVertexBuffer();
	rlDisableVertexArray();

	// Accumulate internal matrix transform (push/pop) and view matrix
	// NOTE: In this case, model instance transformation must be computed in the shader
	matModelView = MatrixMultiply(rlGetMatrixTransform(), matView);

	// Upload model normal matrix (if locations available)
	if (material.shader.locs[SHADER_LOC_MATRIX_NORMAL] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_NORMAL], MatrixTranspose(MatrixInvert(matModel)));
	//-----------------------------------------------------

	// Bind active texture maps (if available)
	for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
	{
		if (material.maps[i].texture.id > 0)
		{
			// Select current shader texture slot
			rlActiveTextureSlot(i);

			// Enable texture for active slot
			if ((i == MATERIAL_MAP_IRRADIANCE) ||
				(i == MATERIAL_MAP_PREFILTER) ||
				(i == MATERIAL_MAP_CUBEMAP)) rlEnableTextureCubemap(material.maps[i].texture.id);
			else rlEnableTexture(material.maps[i].texture.id);

			rlSetUniform(material.shader.locs[SHADER_LOC_MAP_DIFFUSE + i], &i, SHADER_UNIFORM_INT, 1);
		}
	}

	// Try binding vertex array objects (VAO)
	// or use VBOs if not possible
	if (!rlEnableVertexArray(mesh.vaoId))
	{
		// Bind mesh VBO data: vertex position (shader-location = 0)
		rlEnableVertexBuffer(mesh.vboId[0]);
		rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_POSITION], 3, RL_FLOAT, 0, 0, 0);
		rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_POSITION]);

		// Bind mesh VBO data: vertex texcoords (shader-location = 1)
		rlEnableVertexBuffer(mesh.vboId[1]);
		rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01], 2, RL_FLOAT, 0, 0, 0);
		rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01]);

		if (material.shader.locs[SHADER_LOC_VERTEX_NORMAL] != -1)
		{
			// Bind mesh VBO data: vertex normals (shader-location = 2)
			rlEnableVertexBuffer(mesh.vboId[2]);
			rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_NORMAL], 3, RL_FLOAT, 0, 0, 0);
			rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_NORMAL]);
		}

		// Bind mesh VBO data: vertex colors (shader-location = 3, if available)
		if (material.shader.locs[SHADER_LOC_VERTEX_COLOR] != -1)
		{
			if (mesh.vboId[3] != 0)
			{
				rlEnableVertexBuffer(mesh.vboId[3]);
				rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR], 4, RL_UNSIGNED_BYTE, 1, 0, 0);
				rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR]);
			}
			else
			{
				// Set default value for unused attribute
				// NOTE: Required when using default shader and no VAO support
				float value[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
				rlSetVertexAttributeDefault(material.shader.locs[SHADER_LOC_VERTEX_COLOR], value, SHADER_ATTRIB_VEC4, 4);
				rlDisableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR]);
			}
		}

		// Bind mesh VBO data: vertex tangents (shader-location = 4, if available)
		if (material.shader.locs[SHADER_LOC_VERTEX_TANGENT] != -1)
		{
			rlEnableVertexBuffer(mesh.vboId[4]);
			rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TANGENT], 4, RL_FLOAT, 0, 0, 0);
			rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TANGENT]);
		}

		// Bind mesh VBO data: vertex texcoords2 (shader-location = 5, if available)
		if (material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02] != -1)
		{
			rlEnableVertexBuffer(mesh.vboId[5]);
			rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02], 2, RL_FLOAT, 0, 0, 0);
			rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02]);
		}

		if (mesh.indices != NULL) rlEnableVertexBufferElement(mesh.vboId[6]);
	}

	// WARNING: Disable vertex attribute color input if mesh can not provide that data (despite location being enabled in shader)
	if (mesh.vboId[3] == 0) rlDisableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR]);

	int eyeCount = 1;
	//if (rlIsStereoRenderEnabled()) eyeCount = 2;

	for (int eye = 0; eye < eyeCount; eye++)
	{
		// Calculate model-view-projection matrix (MVP)
		Matrix matModelViewProjection = MatrixIdentity();
		if (eyeCount == 1) matModelViewProjection = MatrixMultiply(matModelView, matProjection);
		else
		{
			// Setup current eye viewport (half screen width)
			rlViewport(eye * rlGetFramebufferWidth() / 2, 0, rlGetFramebufferWidth() / 2, rlGetFramebufferHeight());
			matModelViewProjection = MatrixMultiply(MatrixMultiply(matModelView, rlGetMatrixViewOffsetStereo(eye)), rlGetMatrixProjectionStereo(eye));
		}

		// Send combined model-view-projection matrix to shader
		rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_MVP], matModelViewProjection);

		// Draw mesh instanced
		if (mesh.indices != NULL) rlDrawVertexArrayElementsInstanced(0, mesh.triangleCount * 3, 0, instances);
		else rlDrawVertexArrayInstanced(0, mesh.vertexCount, instances);
	}

	// Unbind all bound texture maps
	for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
	{
		if (material.maps[i].texture.id > 0)
		{
			// Select current shader texture slot
			rlActiveTextureSlot(i);

			// Disable texture for active slot
			if ((i == MATERIAL_MAP_IRRADIANCE) ||
				(i == MATERIAL_MAP_PREFILTER) ||
				(i == MATERIAL_MAP_CUBEMAP)) rlDisableTextureCubemap();
			else rlDisableTexture();
		}
	}

	// Disable all possible vertex array objects (or VBOs)
	rlDisableVertexArray();
	rlDisableVertexBuffer();
	rlDisableVertexBufferElement();

	// Disable shader program
	rlDisableShader();

	// Remove instance transforms buffer
	rlUnloadVertexBuffer(instancesVboId);
	RL_FREE(instanceTransforms);
#endif
}

// Draw a model (with texture if set)
void DrawModel(RLModel model, Vector3 position, float scale, RLColor tint)
{
	Vector3 vScale = { scale, scale, scale };
	Vector3 rotationAxis = { 0.0f, 1.0f, 0.0f };

	DrawModelEx(model, position, rotationAxis, 0.0f, vScale, tint);
}

// Draw a model with extended parameters
void DrawModelEx(RLModel model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, RLColor tint)
{
	// Calculate transformation matrix from function parameters
	// Get transform matrix (rotation -> scale -> translation)
	Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
	Matrix matRotation = MatrixRotate(rotationAxis, rotationAngle * DEG2RAD);
	Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);

	Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

	// Combine model transformation matrix (model.transform) with matrix generated by function parameters (matTransform)
	model.transform = MatrixMultiply(model.transform, matTransform);

	for (int i = 0; i < model.meshCount; i++)
	{
		RLColor color = model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color;

		RLColor colorTint = RLWHITE;
		colorTint.r = (unsigned char)((((float)color.r / 255.0f) * ((float)tint.r / 255.0f)) * 255.0f);
		colorTint.g = (unsigned char)((((float)color.g / 255.0f) * ((float)tint.g / 255.0f)) * 255.0f);
		colorTint.b = (unsigned char)((((float)color.b / 255.0f) * ((float)tint.b / 255.0f)) * 255.0f);
		colorTint.a = (unsigned char)((((float)color.a / 255.0f) * ((float)tint.a / 255.0f)) * 255.0f);

		model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = colorTint;
		DrawMesh(model.meshes[i], model.materials[model.meshMaterial[i]], model.transform);
		model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = color;
	}
}

// Draw a 3d mesh with material and transform
void DrawMesh(RLMesh mesh, RLMaterial material, Matrix transform)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
	// Bind shader program
	rlEnableShader(material.shader.id);

	// Send required data to shader (matrices, values)
	//-----------------------------------------------------
	// Upload to shader material.colDiffuse
	if (material.shader.locs[SHADER_LOC_COLOR_DIFFUSE] != -1)
	{
		float values[4] = {
			(float)material.maps[MATERIAL_MAP_DIFFUSE].color.r / 255.0f,
			(float)material.maps[MATERIAL_MAP_DIFFUSE].color.g / 255.0f,
			(float)material.maps[MATERIAL_MAP_DIFFUSE].color.b / 255.0f,
			(float)material.maps[MATERIAL_MAP_DIFFUSE].color.a / 255.0f
		};

		rlSetUniform(material.shader.locs[SHADER_LOC_COLOR_DIFFUSE], values, SHADER_UNIFORM_VEC4, 1);
	}

	// Upload to shader material.colSpecular (if location available)
	if (material.shader.locs[SHADER_LOC_COLOR_SPECULAR] != -1)
	{
		float values[4] = {
			(float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.r / 255.0f,
			(float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.g / 255.0f,
			(float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.b / 255.0f,
			(float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.a / 255.0f
		};

		rlSetUniform(material.shader.locs[SHADER_LOC_COLOR_SPECULAR], values, SHADER_UNIFORM_VEC4, 1);
	}

	// Get a copy of current matrices to work with,
	// just in case stereo render is required, and we need to modify them
	// NOTE: At this point the modelview matrix just contains the view matrix (camera)
	// That's because BeginMode3D() sets it and there is no model-drawing function
	// that modifies it, all use rlPushMatrix() and rlPopMatrix()
	Matrix matModel = MatrixIdentity();
	Matrix matView = rlGetMatrixModelview();
	Matrix matModelView = MatrixIdentity();
	Matrix matProjection = rlGetMatrixProjection();

	// Upload view and projection matrices (if locations available)
	if (material.shader.locs[SHADER_LOC_MATRIX_VIEW] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_VIEW], matView);
	if (material.shader.locs[SHADER_LOC_MATRIX_PROJECTION] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_PROJECTION], matProjection);

	// Model transformation matrix is sent to shader uniform location: SHADER_LOC_MATRIX_MODEL
	if (material.shader.locs[SHADER_LOC_MATRIX_MODEL] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_MODEL], transform);

	// Accumulate several model transformations:
	//    transform: model transformation provided (includes DrawModel() params combined with model.transform)
	//    rlGetMatrixTransform(): rlgl internal transform matrix due to push/pop matrix stack
	matModel = MatrixMultiply(transform, rlGetMatrixTransform());

	// Get model-view matrix
	matModelView = MatrixMultiply(matModel, matView);

	// Upload model normal matrix (if locations available)
	if (material.shader.locs[SHADER_LOC_MATRIX_NORMAL] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_NORMAL], MatrixTranspose(MatrixInvert(matModel)));
	//-----------------------------------------------------

	// Bind active texture maps (if available)
	for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
	{
		if (material.maps[i].texture.id > 0)
		{
			// Select current shader texture slot
			rlActiveTextureSlot(i);

			// Enable texture for active slot
			if ((i == MATERIAL_MAP_IRRADIANCE) ||
				(i == MATERIAL_MAP_PREFILTER) ||
				(i == MATERIAL_MAP_CUBEMAP)) rlEnableTextureCubemap(material.maps[i].texture.id);
			else rlEnableTexture(material.maps[i].texture.id);

			rlSetUniform(material.shader.locs[SHADER_LOC_MAP_DIFFUSE + i], &i, SHADER_UNIFORM_INT, 1);
		}
	}

	// Try binding vertex array objects (VAO) or use VBOs if not possible
	// WARNING: UploadMesh() enables all vertex attributes available in mesh and sets default attribute values
	// for shader expected vertex attributes that are not provided by the mesh (i.e. colors)
	// This could be a dangerous approach because different meshes with different shaders can enable/disable some attributes
	if (!rlEnableVertexArray(mesh.vaoId))
	{
		// Bind mesh VBO data: vertex position (shader-location = 0)
		rlEnableVertexBuffer(mesh.vboId[0]);
		rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_POSITION], 3, RL_FLOAT, 0, 0, 0);
		rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_POSITION]);

		// Bind mesh VBO data: vertex texcoords (shader-location = 1)
		rlEnableVertexBuffer(mesh.vboId[1]);
		rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01], 2, RL_FLOAT, 0, 0, 0);
		rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01]);

		if (material.shader.locs[SHADER_LOC_VERTEX_NORMAL] != -1)
		{
			// Bind mesh VBO data: vertex normals (shader-location = 2)
			rlEnableVertexBuffer(mesh.vboId[2]);
			rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_NORMAL], 3, RL_FLOAT, 0, 0, 0);
			rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_NORMAL]);
		}

		// Bind mesh VBO data: vertex colors (shader-location = 3, if available)
		if (material.shader.locs[SHADER_LOC_VERTEX_COLOR] != -1)
		{
			if (mesh.vboId[3] != 0)
			{
				rlEnableVertexBuffer(mesh.vboId[3]);
				rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR], 4, RL_UNSIGNED_BYTE, 1, 0, 0);
				rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR]);
			}
			else
			{
				// Set default value for defined vertex attribute in shader but not provided by mesh
				// WARNING: It could result in GPU undefined behaviour
				float value[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
				rlSetVertexAttributeDefault(material.shader.locs[SHADER_LOC_VERTEX_COLOR], value, SHADER_ATTRIB_VEC4, 4);
				rlDisableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR]);
			}
		}

		// Bind mesh VBO data: vertex tangents (shader-location = 4, if available)
		if (material.shader.locs[SHADER_LOC_VERTEX_TANGENT] != -1)
		{
			rlEnableVertexBuffer(mesh.vboId[4]);
			rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TANGENT], 4, RL_FLOAT, 0, 0, 0);
			rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TANGENT]);
		}

		// Bind mesh VBO data: vertex texcoords2 (shader-location = 5, if available)
		if (material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02] != -1)
		{
			rlEnableVertexBuffer(mesh.vboId[5]);
			rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02], 2, RL_FLOAT, 0, 0, 0);
			rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02]);
		}

		if (mesh.indices != NULL) rlEnableVertexBufferElement(mesh.vboId[6]);
	}

	// WARNING: Disable vertex attribute color input if mesh can not provide that data (despite location being enabled in shader)
	if (mesh.vboId[3] == 0) rlDisableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR]);

	int eyeCount = 1;
	//if (rlIsStereoRenderEnabled()) eyeCount = 2;

	for (int eye = 0; eye < eyeCount; eye++)
	{
		// Calculate model-view-projection matrix (MVP)
		Matrix matModelViewProjection = MatrixIdentity();
		if (eyeCount == 1) matModelViewProjection = MatrixMultiply(matModelView, matProjection);
		else
		{
			// Setup current eye viewport (half screen width)
			rlViewport(eye * rlGetFramebufferWidth() / 2, 0, rlGetFramebufferWidth() / 2, rlGetFramebufferHeight());
			matModelViewProjection = MatrixMultiply(MatrixMultiply(matModelView, rlGetMatrixViewOffsetStereo(eye)), rlGetMatrixProjectionStereo(eye));
		}

		// Send combined model-view-projection matrix to shader
		rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_MVP], matModelViewProjection);

		// Draw mesh
		if (mesh.indices != NULL) rlDrawVertexArrayElements(0, mesh.triangleCount * 3, 0);
		else rlDrawVertexArray(0, mesh.vertexCount);
	}

	// Unbind all bound texture maps
	for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
	{
		if (material.maps[i].texture.id > 0)
		{
			// Select current shader texture slot
			rlActiveTextureSlot(i);

			// Disable texture for active slot
			if ((i == MATERIAL_MAP_IRRADIANCE) ||
				(i == MATERIAL_MAP_PREFILTER) ||
				(i == MATERIAL_MAP_CUBEMAP)) rlDisableTextureCubemap();
			else rlDisableTexture();
		}
	}

	// Disable all possible vertex array objects (or VBOs)
	rlDisableVertexArray();
	rlDisableVertexBuffer();
	rlDisableVertexBufferElement();

	// Disable shader program
	rlDisableShader();

	// Restore rlgl internal modelview and projection matrices
	rlSetMatrixModelview(matView);
	rlSetMatrixProjection(matProjection);
#endif
}