#include "stdafx.h"
#include "RL.h"
#include "RLMath.h"

#if defined(_WIN32)
#include <direct.h>             // Required for: _getch(), _chdir()
#define GETCWD _getcwd          // NOTE: MSDN recommends not to use getcwd(), chdir()
#define CHDIR _chdir
#include <io.h>                 // Required for: _access() [Used in FileExists()]
#else
#include <unistd.h>             // Required for: getch(), chdir() (POSIX), access()
#define GETCWD getcwd
#define CHDIR chdir
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include <tinyobj_loader_c.h>
#ifdef __cplusplus
}
#endif

// Process obj materials
static void ProcessMaterialsOBJ(RLMaterial* materials, tinyobj_material_t* mats, int materialCount)
{
    // Init model mats
    for (int m = 0; m < materialCount; m++)
    {
        // Init material to default
        // NOTE: Uses default shader, which only supports MATERIAL_MAP_DIFFUSE
        materials[m] = LoadMaterialDefault();

        // Get default texture, in case no texture is defined
        // NOTE: rlgl default texture is a 1x1 pixel UNCOMPRESSED_R8G8B8A8
        materials[m].maps[MATERIAL_MAP_DIFFUSE].texture = { rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };

        if (mats[m].diffuse_texname != NULL) materials[m].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(mats[m].diffuse_texname);  //char *diffuse_texname; // map_Kd
        else materials[m].maps[MATERIAL_MAP_DIFFUSE].color = { (unsigned char)(mats[m].diffuse[0] * 255.0f), (unsigned char)(mats[m].diffuse[1] * 255.0f), (unsigned char)(mats[m].diffuse[2] * 255.0f), 255 }; //float diffuse[3];
        materials[m].maps[MATERIAL_MAP_DIFFUSE].value = 0.0f;

        if (mats[m].specular_texname != NULL) materials[m].maps[MATERIAL_MAP_SPECULAR].texture = LoadTexture(mats[m].specular_texname);  //char *specular_texname; // map_Ks
        materials[m].maps[MATERIAL_MAP_SPECULAR].color = { (unsigned char)(mats[m].specular[0] * 255.0f), (unsigned char)(mats[m].specular[1] * 255.0f), (unsigned char)(mats[m].specular[2] * 255.0f), 255 }; //float specular[3];
        materials[m].maps[MATERIAL_MAP_SPECULAR].value = 0.0f;

        if (mats[m].bump_texname != NULL) materials[m].maps[MATERIAL_MAP_NORMAL].texture = LoadTexture(mats[m].bump_texname);  //char *bump_texname; // map_bump, bump
        materials[m].maps[MATERIAL_MAP_NORMAL].color = RLWHITE;
        materials[m].maps[MATERIAL_MAP_NORMAL].value = mats[m].shininess;

        materials[m].maps[MATERIAL_MAP_EMISSION].color = { (unsigned char)(mats[m].emission[0] * 255.0f), (unsigned char)(mats[m].emission[1] * 255.0f), (unsigned char)(mats[m].emission[2] * 255.0f), 255 }; //float emission[3];

        if (mats[m].displacement_texname != NULL) materials[m].maps[MATERIAL_MAP_HEIGHT].texture = LoadTexture(mats[m].displacement_texname);  //char *displacement_texname; // disp
    }
}


// Load OBJ mesh data
//
// Keep the following information in mind when reading this
//  - A mesh is created for every material present in the obj file
//  - the model.meshCount is therefore the materialCount returned from tinyobj
//  - the mesh is automatically triangulated by tinyobj
static RLModel LoadOBJ(const char* fileName)
{
    RLModel model = { 0 };

    tinyobj_attrib_t attrib = { 0 };
    tinyobj_shape_t* meshes = NULL;
    unsigned int meshCount = 0;

    tinyobj_material_t* materials = NULL;
    unsigned int materialCount = 0;

    char* fileText = LoadFileText(fileName);

    if (fileText != NULL)
    {
        unsigned int dataSize = (unsigned int)strlen(fileText);

        char currentDir[1024] = { 0 };
        strcpy(currentDir, GetWorkingDirectory()); // Save current working directory
        const char* workingDir = GetDirectoryPath(fileName); // Switch to OBJ directory for material path correctness
        if (CHDIR(workingDir) != 0)
        {
            TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to change working directory", workingDir);
        }

        unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
        int ret = tinyobj_parse_obj(&attrib, &meshes, &meshCount, &materials, &materialCount, fileText, dataSize, flags);

        if (ret != TINYOBJ_SUCCESS) TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to load OBJ data", fileName);
        else TRACELOG(LOG_INFO, "MODEL: [%s] OBJ data loaded successfully: %i meshes/%i materials", fileName, meshCount, materialCount);

        // WARNING: We are not splitting meshes by materials (previous implementation)
        // Depending on the provided OBJ that was not the best option and it just crashed
        // so, implementation was simplified to prioritize parsed meshes
        model.meshCount = meshCount;

        // Set number of materials available
        // NOTE: There could be more materials available than meshes but it will be resolved at 
        // model.meshMaterial, just assigning the right material to corresponding mesh
        model.materialCount = materialCount;
        if (model.materialCount == 0)
        {
            model.materialCount = 1;
            TRACELOG(LOG_INFO, "MODEL: No materials provided, setting one default material for all meshes");
        }

        // Init model meshes and materials
        model.meshes = (RLMesh*)RL_CALLOC(model.meshCount, sizeof(RLMesh));
        model.meshMaterial = (int*)RL_CALLOC(model.meshCount, sizeof(int)); // Material index assigned to each mesh
        model.materials = (RLMaterial*)RL_CALLOC(model.materialCount, sizeof(RLMaterial));

        // Process each provided mesh
        for (int i = 0; i < model.meshCount; i++)
        {
            // WARNING: We need to calculate the mesh triangles manually using meshes[i].face_offset
            // because in case of triangulated quads, meshes[i].length actually report quads, 
            // despite the triangulation that is efectively considered on attrib.num_faces
            unsigned int tris = 0;
            if (i == model.meshCount - 1) tris = attrib.num_faces - meshes[i].face_offset;
            else tris = meshes[i + 1].face_offset;

            model.meshes[i].vertexCount = tris * 3;
            model.meshes[i].triangleCount = tris;   // Face count (triangulated)
            model.meshes[i].vertices = (float*)RL_CALLOC(model.meshes[i].vertexCount * 3, sizeof(float));
            model.meshes[i].texcoords = (float*)RL_CALLOC(model.meshes[i].vertexCount * 2, sizeof(float));
            model.meshes[i].normals = (float*)RL_CALLOC(model.meshes[i].vertexCount * 3, sizeof(float));
            model.meshMaterial[i] = 0;  // By default, assign material 0 to each mesh

            // Process all mesh faces
            for (unsigned int face = 0, f = meshes[i].face_offset, v = 0, vt = 0, vn = 0; face < tris; face++, f++, v += 3, vt += 3, vn += 3)
            {
                // Get indices for the face
                tinyobj_vertex_index_t idx0 = attrib.faces[f * 3 + 0];
                tinyobj_vertex_index_t idx1 = attrib.faces[f * 3 + 1];
                tinyobj_vertex_index_t idx2 = attrib.faces[f * 3 + 2];

                // Fill vertices buffer (float) using vertex index of the face
                for (int n = 0; n < 3; n++) { model.meshes[i].vertices[v * 3 + n] = attrib.vertices[idx0.v_idx * 3 + n]; }
                for (int n = 0; n < 3; n++) { model.meshes[i].vertices[(v + 1) * 3 + n] = attrib.vertices[idx1.v_idx * 3 + n]; }
                for (int n = 0; n < 3; n++) { model.meshes[i].vertices[(v + 2) * 3 + n] = attrib.vertices[idx2.v_idx * 3 + n]; }

                if (attrib.num_texcoords > 0)
                {
                    // Fill texcoords buffer (float) using vertex index of the face
                    // NOTE: Y-coordinate must be flipped upside-down
                    model.meshes[i].texcoords[vt * 2 + 0] = attrib.texcoords[idx0.vt_idx * 2 + 0];
                    model.meshes[i].texcoords[vt * 2 + 1] = 1.0f - attrib.texcoords[idx0.vt_idx * 2 + 1];

                    model.meshes[i].texcoords[(vt + 1) * 2 + 0] = attrib.texcoords[idx1.vt_idx * 2 + 0];
                    model.meshes[i].texcoords[(vt + 1) * 2 + 1] = 1.0f - attrib.texcoords[idx1.vt_idx * 2 + 1];

                    model.meshes[i].texcoords[(vt + 2) * 2 + 0] = attrib.texcoords[idx2.vt_idx * 2 + 0];
                    model.meshes[i].texcoords[(vt + 2) * 2 + 1] = 1.0f - attrib.texcoords[idx2.vt_idx * 2 + 1];
                }

                if (attrib.num_normals > 0)
                {
                    // Fill normals buffer (float) using vertex index of the face
                    for (int n = 0; n < 3; n++) { model.meshes[i].normals[vn * 3 + n] = attrib.normals[idx0.vn_idx * 3 + n]; }
                    for (int n = 0; n < 3; n++) { model.meshes[i].normals[(vn + 1) * 3 + n] = attrib.normals[idx1.vn_idx * 3 + n]; }
                    for (int n = 0; n < 3; n++) { model.meshes[i].normals[(vn + 2) * 3 + n] = attrib.normals[idx2.vn_idx * 3 + n]; }
                }
            }
        }

        // Init model materials
        if (materialCount > 0) ProcessMaterialsOBJ(model.materials, materials, materialCount);
        else model.materials[0] = LoadMaterialDefault(); // Set default material for the mesh

        tinyobj_attrib_free(&attrib);
        tinyobj_shapes_free(meshes, model.meshCount);
        tinyobj_materials_free(materials, materialCount);

        UnloadFileText(fileText);

        // Restore current working directory
        if (CHDIR(currentDir) != 0)
        {
            TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to change working directory", currentDir);
        }
    }

    return model;
}

// Load model from files (mesh and material)
RLModel RLLoadModel(const char* fileName)
{
    RLModel model = { 0 };

#if defined(SUPPORT_FILEFORMAT_OBJ)
    if (IsFileExtension(fileName, ".obj")) model = LoadOBJ(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_IQM)
    if (IsFileExtension(fileName, ".iqm")) model = LoadIQM(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_GLTF)
    if (IsFileExtension(fileName, ".gltf") || IsFileExtension(fileName, ".glb")) model = LoadGLTF(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_VOX)
    if (IsFileExtension(fileName, ".vox")) model = LoadVOX(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_M3D)
    if (IsFileExtension(fileName, ".m3d")) model = LoadM3D(fileName);
#endif

    // Make sure model transform is set to identity matrix!
    model.transform = MatrixIdentity();

    if ((model.meshCount != 0) && (model.meshes != NULL))
    {
        // Upload vertex data to GPU (static meshes)
        for (int i = 0; i < model.meshCount; i++) UploadMesh(&model.meshes[i], false);
    }
    else TRACELOG(LOG_WARNING, "MESH: [%s] Failed to load model mesh(es) data", fileName);

    if (model.materialCount == 0)
    {
        TRACELOG(LOG_WARNING, "MATERIAL: [%s] Failed to load model material data, default to white material", fileName);

        model.materialCount = 1;
        model.materials = (RLMaterial*)RL_CALLOC(model.materialCount, sizeof(RLMaterial));
        model.materials[0] = LoadMaterialDefault();

        if (model.meshMaterial == NULL) model.meshMaterial = (int*)RL_CALLOC(model.meshCount, sizeof(int));
    }

    return model;
}