#pragma once

struct Settings final
{
	std::string texturesDir;
	std::string shapesDir;
	size_t undoMax;
	float mouseSensitivity;
	bool exportSeparateGeometry, cullFaces; //For GLTF export
	std::string exportFilePath; //For GLTF export
	std::string defaultTexturePath;
	std::string defaultShapePath;
	uint8_t backgroundColor[3];
};