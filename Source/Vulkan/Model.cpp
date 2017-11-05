#include "Model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Model::Model(const std::string &filename, const std::shared_ptr<Buffers> buffers)
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

		if (mesh->HasPositions() && mesh->HasTextureCoords(0) && mesh->HasFaces())
		{
			for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
			{
				const auto face = mesh->mFaces[j];

				if (face.mNumIndices != 3)
				{
					continue;
				}

				for (unsigned int k = 0; k < face.mNumIndices; ++k)
				{
					buffers->getIndices()->push_back(buffers->getVertices()->size() + face.mIndices[k]);
				}
			}

			for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
			{
				const auto position = mesh->mVertices[i];
				const auto uv = mesh->mTextureCoords[0][i];

				buffers->getVertices()->push_back({ { position.x, position.y, position.z },{ 1.0f, 1.0f, 1.0f },{ uv.x, uv.y } });
			}
		}
	}

	if (scene->HasMaterials())
	{
		for (unsigned i = 0; i < scene->mNumMaterials; ++i)
		{
			const auto material = scene->mMaterials[i];

			for (unsigned int j = 0; j < material->GetTextureCount(aiTextureType_DIFFUSE); ++j)
			{
				aiString path;
				auto texture = material->GetTexture(aiTextureType_DIFFUSE, j, &path);
				//Window::showMessageBox("Ding", path.C_Str());
			}
		}
	}
}