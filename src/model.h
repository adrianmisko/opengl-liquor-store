#pragma once

#include "mesh.h"

#include "stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model {
public:
	std::vector<Texture> textures_loaded;
	std::vector<Mesh> meshes;
	std::string directory;
	bool gammaCorrection;

	Model(const char* path, bool gamma=false);
	void draw(Shader* shader);

private:
	void loadModel(std::string path);
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	unsigned int TextureFromFile(const char* path, const std::string &directory, bool gamma=false);
	std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
		std::string typeName);
};