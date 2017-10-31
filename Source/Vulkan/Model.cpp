#include "Model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Model::Model(const std::string &filename, const std::shared_ptr<Buffers> buffers)
{
	Assimp::Importer importer;
	auto scene = importer.ReadFile(filename, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipUVs);

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
		aiMesh *mesh = scene->mMeshes[i];

		if (mesh->HasFaces())
		{
			for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
			{
				aiFace face = mesh->mFaces[j];

				if (face.mNumIndices != 3)
				{
					continue;
				}

				for (unsigned int k = 0; k < face.mNumIndices; ++k)
				{
					// TODO: we need to add mesh->mNumVertices + face.mIndices[k] here probably to have multiple meshes working!
					buffers->getIndices()->push_back(face.mIndices[k]);
				}
			}
		}

		if (mesh->HasPositions())
		{
			for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
			{
				const aiVector3D position = mesh->mVertices[j];
				const aiVector3D uv = mesh->mTextureCoords[0][j];

				buffers->getVertices()->push_back({ { position.x, position.y, position.z },{ 1.0f, 1.0f, 1.0f },{ uv.x, uv.y } });
			}
		}
	}
}