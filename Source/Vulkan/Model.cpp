#include "Model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Model::Model(const std::string &filename, const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<std::vector<Texture*>> diffuseTextures, const std::shared_ptr<std::vector<Texture*>> normalTextures)
{
	Assimp::Importer importer;
	const auto scene = importer.ReadFile(filename, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipUVs);
	
	if (!scene)
	{
		throw std::runtime_error("Failed to load model \"" + filename + "\".");
	}

	if (!scene->HasMeshes())
	{
		throw std::runtime_error("Model \"" + filename + "\" has no meshes.");
	}

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		const auto mesh = scene->mMeshes[i];

		if (!mesh->HasPositions() || !mesh->HasTextureCoords(0) || !mesh->HasFaces())
		{
			continue;
		}

		auto meshData = new Mesh();
		meshData->firstIndex = static_cast<uint32_t>(buffers->getIndices()->size());
		meshData->indexCount = mesh->mNumFaces * 3;
		meshData->textureIndex = mesh->mMaterialIndex;
		meshes.push_back(meshData);

		for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
		{
			const auto face = mesh->mFaces[j];

			if (face.mNumIndices != 3)
			{
				continue;
			}

			for (unsigned int k = 0; k < face.mNumIndices; ++k)
			{
				buffers->getIndices()->push_back(static_cast<uint32_t>(buffers->getVertices()->size()) + face.mIndices[k]);
			}
		}

		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			const auto position = mesh->mVertices[i];
			const auto uv = mesh->mTextureCoords[0][i];

			buffers->getVertices()->push_back({ { position.x, position.y, position.z },{ 1.0f, 1.0f, 1.0f },{ uv.x, uv.y } });
		}
	}

	if (scene->HasMaterials())
	{
		for (unsigned i = 1; i < scene->mNumMaterials; ++i)
		// we start at 1 here to ignore the default material added by assimp
		{
			const auto material = scene->mMaterials[i];

			auto diffuseTextureMapCount = material->GetTextureCount(aiTextureType_DIFFUSE);
			auto normalTextureMapCount = material->GetTextureCount(aiTextureType_HEIGHT);

			if (diffuseTextureMapCount < 1)
			{
				aiString materialName;
				material->Get(AI_MATKEY_NAME, materialName);
				throw std::runtime_error("Material \"" + std::string(materialName.C_Str()) + "\", required by model \"" + filename + "\", is invalid: it has no diffuse texture assigned.");
			}

			if (normalTextureMapCount < 1)
			{
				aiString materialName;
				material->Get(AI_MATKEY_NAME, materialName);
				throw std::runtime_error("Material \"" + std::string(materialName.C_Str()) + "\", required by model \"" + filename + "\", is invalid: it has no normal texture assigned.");
			}

			for (unsigned int j = 0; j < material->GetTextureCount(aiTextureType_DIFFUSE); ++j)
			{
				aiString path;
				
				if (material->GetTexture(aiTextureType_DIFFUSE, j, &path) != aiReturn::aiReturn_SUCCESS)
				{
					aiString materialName;
					material->Get(AI_MATKEY_NAME, materialName);
					throw std::runtime_error("Failed to load diffuse texture \"" + std::string(path.C_Str()) + "\" for material \"" + std::string(materialName.C_Str()) + "\", required by model \"" + filename + "\".");
				}
				
				diffuseTextures->push_back(new Texture("Textures\\Sponza\\" + std::string(path.C_Str()), context));
			}

			for (unsigned int j = 0; j < material->GetTextureCount(aiTextureType_HEIGHT); ++j)
			{
				aiString path;

				if (material->GetTexture(aiTextureType_HEIGHT, j, &path) != aiReturn::aiReturn_SUCCESS)
				{
					aiString materialName;
					material->Get(AI_MATKEY_NAME, materialName);
					throw std::runtime_error("Failed to load normal texture \"" + std::string(path.C_Str()) + "\" for material \"" + std::string(materialName.C_Str()) + "\", required by model \"" + filename + "\".");
				}

				normalTextures->push_back(new Texture("Textures\\Sponza\\" + std::string(path.C_Str()), context));
			}
		}
	}
}