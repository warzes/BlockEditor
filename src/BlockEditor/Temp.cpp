#include "stdafx.h"
#include "Temp.h"

// Unload mesh from memory (RAM and VRAM)
void UnloadMesh(NewMesh& mesh)
{
	// Unload rlgl mesh vboId data
	mesh.geometry.reset();;

	free(mesh.vertices);
	free(mesh.texcoords);
	free(mesh.normals);
	free(mesh.colors);
	free(mesh.tangents);
	free(mesh.texcoords2);
	
	free(mesh.animVertices);
	free(mesh.animNormals);
	free(mesh.boneWeights);
	free(mesh.boneIds);
}

void UnloadModel(NewModel& model)
{
	// Unload meshes
	for (int i = 0; i < model.meshes.size(); i++) UnloadMesh(model.meshes[i]);

	// Unload materials maps
	// NOTE: As the user could be sharing shaders and textures between models,
	// we don't unload the material but just free its maps,
	// the user is responsible for freeing models shaders and textures
	for (int i = 0; i < model.materialCount; i++) free(model.materials[i].maps);

	// Unload arrays
	free(model.materials);
	free(model.meshMaterial);

	// Unload animation data
	free(model.bones);
	free(model.bindPose);
}