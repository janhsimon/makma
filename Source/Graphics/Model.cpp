#include "Model.hpp"

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>

Mesh *Model::loadMeshData(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const aiMesh *mesh, const aiMaterial *material)
{
	auto meshData = new Mesh();

	meshData->firstIndex = static_cast<uint32_t>(buffers->getIndices()->size());
	meshData->indexCount = mesh->mNumFaces * 3;

	aiString materialName, diffuseTextureFilename, normalTextureFilename;
	material->Get(AI_MATKEY_NAME, materialName);

	if (material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTextureFilename) != aiReturn::aiReturn_SUCCESS)
	{
		throw std::runtime_error("Failed to load diffuse texture \"" + std::string(diffuseTextureFilename.C_Str()) + "\" for material \"" + std::string(materialName.C_Str()) + "\", required by model \"" + filename + "\".");
	}

	if (material->GetTexture(aiTextureType_HEIGHT, 0, &normalTextureFilename) != aiReturn::aiReturn_SUCCESS)
	{
		throw std::runtime_error("Failed to load normal texture \"" + std::string(normalTextureFilename.C_Str()) + "\" for material \"" + std::string(materialName.C_Str()) + "\", required by model \"" + filename + "\".");
	}

	meshData->material = Material::cacheMaterial(context, filename + "\\" + std::string(materialName.C_Str()), "Textures\\" + std::string(diffuseTextureFilename.C_Str()), "Textures\\" + std::string(normalTextureFilename.C_Str()));

	return meshData;
}

void Model::appendDataToIndexBuffer(const std::shared_ptr<Buffers> buffers, const aiMesh *mesh)
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
			buffers->getIndices()->push_back(static_cast<uint32_t>(buffers->getVertices()->size()) + face.mIndices[k]);
		}
	}
}

void Model::appendDataToVertexBuffer(const std::shared_ptr<Buffers> buffers, const aiMesh *mesh)
{
	for (unsigned int k = 0; k < mesh->mNumVertices; ++k)
	{
		const auto position = mesh->mVertices[k];
		const auto uv = mesh->mTextureCoords[0][k];
		buffers->getVertices()->push_back({ { position.x, position.y, position.z },{ uv.x, uv.y } });
	}
}

Model::Model(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::string &filename)
{
	this->context = context;
	this->filename = filename;

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

	if (!scene->HasMaterials())
	{
		throw std::runtime_error("Model \"" + filename + "\" has no materials.");
	}

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		const auto mesh = scene->mMeshes[i];

		if (!mesh->HasPositions() || !mesh->HasTextureCoords(0) || !mesh->HasFaces())
		{
			continue;
		}

		const auto material = scene->mMaterials[mesh->mMaterialIndex];

		if (material->GetTextureCount(aiTextureType_DIFFUSE) < 1 || material->GetTextureCount(aiTextureType_HEIGHT) < 1)
		{
			continue;
		}

		meshes.push_back(loadMeshData(context, buffers, mesh, material));

		appendDataToIndexBuffer(buffers, mesh);
		appendDataToVertexBuffer(buffers, mesh);
	}
}

void Model::finalizeMaterials(const std::shared_ptr<Descriptor> descriptor)
{
	for (auto &mesh : meshes)
	{
		mesh->material->finalize(descriptor);
	}
}