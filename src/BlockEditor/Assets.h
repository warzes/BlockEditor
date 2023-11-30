#pragma once

#include "Temp.h"

//A repository that caches all loaded resources and their file paths.
class Assets
{
public:
	//A RAII wrapper for a Raylib Texture (unloads on destruction)
	class TexHandle
	{
	public:
		inline TexHandle(Texture2DRef texture, std::filesystem::path path) { _texture = texture; _path = path; }
		inline Texture2DRef GetTexture() const { return _texture; }
		inline std::filesystem::path GetPath() const { return _path; }
	private:
		Texture2DRef _texture;
		std::filesystem::path _path;
	};

	//A RAII wrapper for a Raylib Model (unloads on destruction)
	class ModelHandle
	{
	public:
		// Loads an .obj model from the given path and initializes a handle for it.
		ModelHandle(const std::filesystem::path path);
		~ModelHandle();
		inline NewModel GetModel() const { return _model; }
		inline std::filesystem::path GetPath() const { return _path; }
	private:
		NewModel _model;
		std::filesystem::path _path;
	};

	static std::shared_ptr<TexHandle>   GetTexture(std::filesystem::path path); //Returns a shared pointer to the cached texture at `path`, loading it if it hasn't been loaded.
	static std::shared_ptr<ModelHandle> GetModel(std::filesystem::path path);   //Returns a shared pointer to the cached model at `path`, loading it if it hasn't been loaded.

	static const Font& GetFont(); //Returns the default application font (dejavu.fnt)
	static const ShaderProgramRef GetMapShader(bool instanced); //Returns the shader used to render tiles
	static const ShaderProgramRef GetSpriteShader(); // Returns the shader used to render billboards
	static const NewModel& GetEntSphere(); //Returns the sphere that represents entities visually
	static const NewMesh& GetSpriteQuad();
	static Texture2DRef GetMissingTexture();
protected:
	//Asset caches that hold weak references to all the loaded textures and models
	std::map<std::filesystem::path, std::weak_ptr<TexHandle>>   _textures;
	std::map<std::filesystem::path, std::weak_ptr<ModelHandle>> _models;

	//Assets that are alive the whole application
	Font _font; //Default application font (dejavu.fnt)
	Texture2DRef _missingTexture; //Texture to display when the texture file to be loaded isn't found
	NewModel _entSphere; //The sphere that represents entities visually
	ShaderProgramRef _mapShader; //The non-instanced version that is used to render tiles outside of the map itself
	ShaderProgramRef _mapShaderInstanced; //The instanced version is used to render the tiles
	ShaderProgramRef _spriteShader;
	NewMesh _spriteQuad;
private:
	Assets();
	~Assets();
	static Assets* _Get();
};