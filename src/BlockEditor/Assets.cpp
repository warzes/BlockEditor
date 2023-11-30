#include "stdafx.h"
#include "Assets.h"
#include "font_dejavu.h"

static Assets* _instance = nullptr;



Assets* Assets::_Get()
{
	if (!_instance)
	{
		_instance = new Assets();
	}
	return _instance;
};

Assets::ModelHandle::ModelHandle(std::filesystem::path path)
{
	_path = path;

	// We are loading the .OBJ file manually because Raylib's loader doesn't take indices into account.
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec2 uv;
		glm::vec3 norm;
	};

	std::vector<Vertex> meshVerts;
	std::vector<unsigned short> meshInds;
	// This takes a triplet string from the .obj face and points it to a vertex in meshVertices
	std::map<std::string, int> vertMapper;

	// These track the vertex data as laid out in the .obj file
	// (Since attributes are reused between vertices, they don't map directly onto the mesh's attributes)
	std::vector<glm::vec3> objPositions;
	objPositions.reserve(128);
	std::vector<glm::vec2> objUVs;
	objUVs.reserve(128);
	std::vector<glm::vec3> objNormals;
	objNormals.reserve(128);

	// Parse the .obj file
	std::ifstream objFile(path);
	std::string line;
	std::string objectName;

	while (std::getline(objFile, line), !line.empty())
	{
		// Get the items between spaces
		std::vector<std::string> tokens = SplitString(line, " ");
		if (tokens.empty()) continue;

		if (tokens[0].compare("v") == 0) // Vertex positions
		{
			objPositions.push_back(
				{
				(float)atof(tokens[1].c_str()),
				(float)atof(tokens[2].c_str()),
				(float)atof(tokens[3].c_str())
				});
		}
		else if (tokens[0].compare("vt") == 0) // Vertex texture coordinates
		{
			objUVs.push_back(
				{
				(float)atof(tokens[1].c_str()),
				1.0f - (float)atof(tokens[2].c_str())
				});
		}
		else if (tokens[0].compare("vn") == 0) // Vertex normals
		{
			objNormals.push_back(
				{
				(float)atof(tokens[1].c_str()),
				(float)atof(tokens[2].c_str()),
				(float)atof(tokens[3].c_str())
				});
		}
		else if (tokens[0].compare("f") == 0) // Faces
		{
			if (tokens.size() > 4)
				std::cerr << "Error: Shape loaded from " << path << " should be triangulated!" << std::endl;

			for (int i = 1; i < tokens.size(); ++i)
			{
				std::vector<std::string> indices = SplitString(tokens[i], "/");
				if (indices.size() != 3 || indices[0].empty() || indices[1].empty() || indices[2].empty())
				{
					std::cerr << "Error: Face in .OBJ file " << path << "does not have the right number of indices.";
					break;
				}

				int posIdx = atoi(indices[0].c_str()) - 1,
					uvIdx = atoi(indices[1].c_str()) - 1,
					normIdx = atoi(indices[2].c_str()) - 1;
				if (posIdx < 0 || uvIdx < 0 || normIdx < 0)
				{
					std::cerr << "Error: Invalid indices in .OBJ file " << path << "." << std::endl;
					break;
				}

				// Add indices to mesh
				if (vertMapper.find(tokens[i]) != vertMapper.end())
				{
					meshInds.push_back(vertMapper[tokens[i]]);
				}
				else
				{
					vertMapper[tokens[i]] = meshVerts.size();
					meshInds.push_back(meshVerts.size());
					meshVerts.push_back(
						Vertex{
						objPositions[posIdx],
						objUVs[uvIdx],
						objNormals[normIdx]
						});
				}
			}
		}
		else if (tokens[0].compare("o") == 0) // Objects
		{
			if (objectName.empty())
				objectName = tokens[1];
			else
				break; // Only parse the first object in the file.
		}
	}

	// Initialize the model with the meshes
	_model = NewModel{};
	_model.bindPose = nullptr;
	_model.boneCount = 0;
	_model.bones = nullptr;
	_model.materialCount = 1;
	_model.materials = (NewMaterial*)malloc(sizeof(NewMaterial) * _model.materialCount);
	_model.meshes.resize(_model.materialCount);
	_model.meshMaterial = (int*)malloc(sizeof(NewMesh) * _model.materialCount);
	_model.transform = glm::mat4(1.0f);

	int m = 0;
	//_model.materials[m] = LoadMaterialDefault();
	_model.meshMaterial[m] = m;

	// Encode the dynamic vertex arrays into a Raylib mesh
	NewMesh mesh = { 0 };
	mesh.vertices = (float*)malloc(meshVerts.size() * 3 * sizeof(float));
	mesh.vertexCount = meshVerts.size();
	mesh.texcoords = (float*)malloc(meshVerts.size() * 2 * sizeof(float));
	mesh.texcoords2 = mesh.animNormals = mesh.animVertices = mesh.boneWeights = mesh.tangents = nullptr;
	mesh.boneIds = mesh.colors = nullptr;
	mesh.normals = (float*)malloc(meshVerts.size() * 3 * sizeof(float));
	mesh.indices.resize(meshInds.size());
	mesh.triangleCount = meshInds.size() / 3;

	for (int v = 0, p = 0, n = 0, u = 0; v < meshVerts.size(); ++v)
	{
		mesh.vertices[p++] = meshVerts[v].pos.x;
		mesh.vertices[p++] = meshVerts[v].pos.y;
		mesh.vertices[p++] = meshVerts[v].pos.z;
		mesh.normals[n++] = meshVerts[v].norm.x;
		mesh.normals[n++] = meshVerts[v].norm.y;
		mesh.normals[n++] = meshVerts[v].norm.z;
		mesh.texcoords[u++] = meshVerts[v].uv.x;
		mesh.texcoords[u++] = meshVerts[v].uv.y;
	}
	for (int i = 0; i < meshInds.size(); ++i)
	{
		mesh.indices[i] = meshInds[i];
	}
	UploadMesh(mesh, false);

	_model.meshes[m] = mesh;
}

Assets::ModelHandle::~ModelHandle()
{
	UnloadModel(_model);
}

Assets::Assets()
{
	// Generate missing texture image (a black-and-magenta checkerboard)
	uint8_t* pixels = (uint8_t*)malloc(3 * 64 * 64);
	const int BLOCK_SIZE = 32;
	for (int x = 0; x < 64; ++x)
	{
		for (int y = 0; y < 64; ++y)
		{
			int base = 3 * (x + y * 64);
			if ((((x / BLOCK_SIZE) % 2) == 0 && ((y / BLOCK_SIZE) % 2) == 0) ||
				(((x / BLOCK_SIZE) % 2) == 1 && ((y / BLOCK_SIZE) % 2) == 1))
			{
				pixels[base] = char(0xFF);
				pixels[base + 1] = 0x00;
				pixels[base + 2] = char(0xFF);
			}
			else
			{
				pixels[base] = pixels[base + 1] = pixels[base + 2] = 0x00;
			}
		}
	}
	ImageRef texImg(new Image);
	texImg->Create(Image::PixelFormat::RGB_U8, 64, 64, pixels);		
	_missingTexture = GetRenderSystem().CreateTexture2D(texImg, {});
	free(pixels);
	texImg.reset();

	// Initialize instanced shader for map geometry
	// TODO: шейдеры
	//_mapShaderInstanced = LoadShaderFromMemory(MAP_SHADER_INSTANCED_V_SRC, MAP_SHADER_F_SRC);
	//_mapShaderInstanced.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(_mapShaderInstanced, "mvp");
	//_mapShaderInstanced.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(_mapShaderInstanced, "viewPos");
	//_mapShaderInstanced.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(_mapShaderInstanced, "instanceTransform");

	//_mapShader = LoadShaderFromMemory(MAP_SHADER_V_SRC, MAP_SHADER_F_SRC);
	//_mapShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(_mapShader, "mvp");
	//_mapShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(_mapShader, "viewPos");

	//// Sprite shader
	//_spriteShader = LoadShaderFromMemory(SPRITE_SHADER_V_SRC, SPRITE_SHADER_F_SRC);
	//_mapShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(_mapShader, "mvp");
	//_mapShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(_mapShader, "viewPos");
	//_spriteShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(_spriteShader, "matModel");
	//_spriteShader.locs[SHADER_LOC_MATRIX_VIEW] = GetShaderLocation(_spriteShader, "matView");
	//_spriteShader.locs[SHADER_LOC_MATRIX_PROJECTION] = GetShaderLocation(_spriteShader, "matProj");

	// Initialize the sprite's quad model
	{
		NewMesh m = {};
		m.vertexCount = 4;
		m.triangleCount = 2;
		m.vertices = SAFE_MALLOC(float, 4 * 3);
		m.texcoords = SAFE_MALLOC(float, 4 * 2);
		m.indices.resize(2 * 3);
		// Vertex positions (x, y, z)
		m.vertices[0] = -1.0f; m.vertices[1] = +1.0f; m.vertices[2] = 0.0f;
		m.vertices[3] = +1.0f; m.vertices[4] = +1.0f; m.vertices[5] = 0.0f;
		m.vertices[6] = +1.0f; m.vertices[7] = -1.0f; m.vertices[8] = 0.0f;
		m.vertices[9] = -1.0f; m.vertices[10] = -1.0f; m.vertices[11] = 0.0f;
		// UV
		m.texcoords[0] = 0.0f; m.texcoords[1] = 0.0f;
		m.texcoords[2] = 1.0f; m.texcoords[3] = 0.0f;
		m.texcoords[4] = 1.0f; m.texcoords[5] = 1.0f;
		m.texcoords[6] = 0.0f; m.texcoords[7] = 1.0f;
		// Indices
		m.indices[0] = 2; m.indices[1] = 1; m.indices[2] = 0;
		m.indices[3] = 3; m.indices[4] = 2; m.indices[5] = 0;
		UploadMesh(m, false);
		_spriteQuad = m;
	}

	// Generate entity sphere
	//_entSphere = LoadModelFromMesh(GenMeshSphere(1.0f, 8, 8)); // TODO: генерация сферы
	// TODO: возможно ненужно
	//for (int m = 0; m < _entSphere.materialCount; ++m)
	//	_entSphere.materials[m].shader = _mapShader;

	_font = LoadFont_Dejavu();
}

const Font& Assets::GetFont()
{
	return _Get()->_font;
}

const ShaderProgramRef Assets::GetMapShader(bool instanced)
{
	return instanced ? _Get()->_mapShaderInstanced : _Get()->_mapShader;
}

const ShaderProgramRef Assets::GetSpriteShader()
{
	return _Get()->_spriteShader;
}

const NewModel& Assets::GetEntSphere()
{
	return _Get()->_entSphere;
}

const NewMesh& Assets::GetSpriteQuad()
{
	return _Get()->_spriteQuad;
}

Texture2DRef Assets::GetMissingTexture()
{
	return _Get()->_missingTexture;
}

std::shared_ptr<Assets::TexHandle> Assets::GetTexture(std::filesystem::path texturePath)
{
	Assets* a = _Get();
	//Attempt to find the texture in the cache
	if (a->_textures.find(texturePath) != a->_textures.end())
	{
		std::weak_ptr<TexHandle> weakHandle = a->_textures[texturePath];
		if (std::shared_ptr<TexHandle> handle = weakHandle.lock())
		{
			return handle;
		}
	}

	//Load the texture if it is no longer stored in the cache
	Texture2DRef texture = GetRenderSystem().CreateTexture2D(texturePath.string().c_str());
	//Replace with the checkerboard texture if the file didn't load
	if (!texture) texture = a->_missingTexture;

	auto sharedPtr = std::make_shared<TexHandle>(texture, texturePath);
	//Cache the texture
	a->_textures[texturePath] = std::weak_ptr<TexHandle>(sharedPtr);
	return sharedPtr;
}

std::shared_ptr<Assets::ModelHandle> Assets::GetModel(std::filesystem::path path)
{
	Assets* a = _Get();
	//Attempt to find the model in the cache
	if (a->_models.find(path) != a->_models.end())
	{
		std::weak_ptr<ModelHandle> weakHandle = a->_models[path];
		if (std::shared_ptr<ModelHandle> handle = weakHandle.lock())
		{
			return handle;
		}
	}

	auto sharedPtr = std::make_shared<ModelHandle>(path);
	//Cache the model
	a->_models[path] = std::weak_ptr<ModelHandle>(sharedPtr);
	return sharedPtr;
}