#pragma once

#include "RL.h"

// A repository that caches all loaded resources and their file paths.
// It is implemented as a singleton with a static interface. 
// (This circumvents certain limitations regarding static members and allows the constructor to be called automatically when the first method is called.)
class Assets
{
public:
	//A RAII wrapper for a Raylib RLTexture (unloads on destruction)
	class TexHandle
	{
	public:
		inline TexHandle(RLTexture2D texture, std::filesystem::path path) { _texture = texture; _path = path; }
		inline ~TexHandle() { UnloadTexture(_texture); }
		inline RLTexture2D GetTexture() const { return _texture; }
		inline std::filesystem::path GetPath() const { return _path; }
	private:
		RLTexture2D _texture;
		std::filesystem::path _path;
	};

	//A RAII wrapper for a Raylib Model (unloads on destruction)
	class ModelHandle
	{
	public:
		// Loads an .obj model from the given path and initializes a handle for it.
		ModelHandle(const std::filesystem::path path);
		~ModelHandle();
		inline RLModel GetModel() const { return _model; }
		inline std::filesystem::path GetPath() const { return _path; }
	private:
		RLModel _model;
		std::filesystem::path _path;
	};

	static std::shared_ptr<TexHandle>   GetTexture(std::filesystem::path path); //Returns a shared pointer to the cached texture at `path`, loading it if it hasn't been loaded.
	static std::shared_ptr<ModelHandle> GetModel(std::filesystem::path path);   //Returns a shared pointer to the cached model at `path`, loading it if it hasn't been loaded.

	static const Font& GetFont(); //Returns the default application font (dejavu.fnt)
	static const RLShader& GetMapShader(bool instanced); //Returns the shader used to render tiles
	static const RLShader& GetSpriteShader(); // Returns the shader used to render billboards
	static const RLModel& GetEntSphere(); //Returns the sphere that represents entities visually
	static const RLMesh& GetSpriteQuad();
	static RLTexture GetMissingTexture();
protected:
	//Asset caches that hold weak references to all the loaded textures and models
	std::map<std::filesystem::path, std::weak_ptr<TexHandle>>   _textures;
	std::map<std::filesystem::path, std::weak_ptr<ModelHandle>> _models;

	//Assets that are alive the whole application
	Font _font; //Default application font (dejavu.fnt)
	RLTexture2D _missingTexture; //RLTexture to display when the texture file to be loaded isn't found
	RLModel _entSphere; //The sphere that represents entities visually
	RLShader _mapShader; //The non-instanced version that is used to render tiles outside of the map itself
	RLShader _mapShaderInstanced; //The instanced version is used to render the tiles
	RLShader _spriteShader;
	RLMesh _spriteQuad;
private:
	Assets();
	~Assets();
	static Assets* _Get();
};