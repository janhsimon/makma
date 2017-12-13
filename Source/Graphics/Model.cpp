#include "Model.hpp"

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>

Mesh *Model::loadMeshData(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const aiMesh *mesh, const aiMaterial *material, const std::string &filename)
{
	auto meshData = new Mesh();

	meshData->firstIndex = static_cast<uint32_t>(buffers->getIndices()->size());
	meshData->indexCount = mesh->mNumFaces * 3;

	aiString materialName, diffuseTextureFilename, normalTextureFilename, opacityTextureFilename, occlusionTextureFilename;
	material->Get(AI_MATKEY_NAME, materialName);

	if (material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTextureFilename) == aiReturn::aiReturn_SUCCESS)
	{
		std::string strippedFilename(diffuseTextureFilename.C_Str());
		size_t i = strippedFilename.find_last_of('\\');
		if (i != std::string::npos)
		{
			strippedFilename = strippedFilename.substr(i);
		}
		diffuseTextureFilename = strippedFilename;
	}

	if (material->GetTexture(aiTextureType_HEIGHT, 0, &normalTextureFilename) == aiReturn::aiReturn_SUCCESS)
	{
		std::string strippedFilename(normalTextureFilename.C_Str());
		size_t i = strippedFilename.find_last_of('\\');
		if (i != std::string::npos)
		{
			strippedFilename = strippedFilename.substr(i);
		}
		normalTextureFilename = strippedFilename;
	}

	if (material->GetTexture(aiTextureType_OPACITY, 0, &opacityTextureFilename) == aiReturn::aiReturn_SUCCESS)
	{
		std::string strippedFilename(opacityTextureFilename.C_Str());
		size_t i = strippedFilename.find_last_of('\\');
		if (i != std::string::npos)
		{
			strippedFilename = strippedFilename.substr(i);
		}
		opacityTextureFilename = strippedFilename;
	}

	if (material->GetTexture(aiTextureType_SHININESS, 0, &occlusionTextureFilename) == aiReturn::aiReturn_SUCCESS)
	{
		std::string strippedFilename(occlusionTextureFilename.C_Str());
		size_t i = strippedFilename.find_last_of('\\');
		if (i != std::string::npos)
		{
			strippedFilename = strippedFilename.substr(i);
		}
		occlusionTextureFilename = strippedFilename;
	}

	meshData->material = Material::cacheMaterial(context, filename + "\\" + std::string(materialName.C_Str()), std::string(diffuseTextureFilename.C_Str()), std::string(normalTextureFilename.C_Str()), std::string(opacityTextureFilename.C_Str()), std::string(occlusionTextureFilename.C_Str()));

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
		const auto normal = mesh->mNormals[k];
		const auto tangent = mesh->mTangents[k];

		buffers->getVertices()->push_back({ { position.x, position.y, position.z }, { uv.x, uv.y }, { normal.x, normal.y, normal.z }, {tangent.x, tangent.y, tangent.z } });
	}
}

Model::Model(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::string &filename)
{
	this->context = context;

	Assimp::Importer importer;
	const auto scene = importer.ReadFile(filename, aiProcess_CalcTangentSpace | aiProcess_FlipUVs);
	
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

		meshes.push_back(loadMeshData(context, buffers, mesh, material, filename));

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