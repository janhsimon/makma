#include "Model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

std::shared_ptr<Mesh> Model::loadMeshData(const std::shared_ptr<Context> context, const aiMesh *mesh, const aiMaterial *material, const std::string &path, const std::string &filename, const uint32_t numIndices)
{
	auto meshData = std::make_shared<Mesh>();

	meshData->firstIndex = static_cast<uint32_t>(numIndices);
	meshData->indexCount = mesh->mNumFaces * 3;

	aiString materialName;
	if (material->Get(AI_MATKEY_NAME, materialName) != aiReturn::aiReturn_SUCCESS)
	{
		throw std::runtime_error("Model \"" + path + filename + "\" has invalid material with no name.");
	}

	meshData->material = Material::getMaterialFromCache(materialName.C_Str());
	
	if (meshData->material)
	{
		return meshData;
	}

	meshData->material = std::make_shared<Material>(context, materialName.C_Str());

	aiString diffuseTextureFilename, normalTextureFilename, occlusionTextureFilename, metallicTextureFilename, roughnessTextureFilename;

	if (material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTextureFilename) == aiReturn::aiReturn_SUCCESS)
	{
		meshData->material->setDiffuseTexture(path + std::string(diffuseTextureFilename.C_Str()));
	}

	if (material->GetTexture(aiTextureType_HEIGHT, 0, &normalTextureFilename) == aiReturn::aiReturn_SUCCESS)
	{
		meshData->material->setNormalTexture(path + std::string(normalTextureFilename.C_Str()));
	}
	
	if (material->GetTexture(aiTextureType_AMBIENT, 0, &metallicTextureFilename) == aiReturn::aiReturn_SUCCESS)
	{
		meshData->material->setMetallicTexture(path + std::string(metallicTextureFilename.C_Str()));
	}

	if (material->GetTexture(aiTextureType_SPECULAR, 0, &roughnessTextureFilename) == aiReturn::aiReturn_SUCCESS)
	{
		meshData->material->setRoughnessTexture(path + std::string(roughnessTextureFilename.C_Str()));
	}

	Material::addMaterialToCache(meshData->material);

	return meshData;
}

void Model::appendDataToIndexBuffer(const aiMesh *mesh, const std::shared_ptr<IndexBuffer> indexBuffer, const uint32_t numVertices)
{
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		const auto face = mesh->mFaces[i];

		if (face.mNumIndices != 3)
		{
			continue;
		}

		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			indexBuffer->getIndices()->push_back(static_cast<uint32_t>(numVertices) + face.mIndices[j]);
		}
	}
}

void Model::appendDataToVertexBuffer(const aiMesh *mesh, const std::shared_ptr<VertexBuffer> vertexBuffer)
{
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		const auto position = mesh->mVertices[i];
		const auto uv = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : aiVector3D(0.0f);
		const auto normal = mesh->HasNormals() ? mesh->mNormals[i] : aiVector3D(0.0f);
		const auto tangent = mesh->HasTangentsAndBitangents() ? mesh->mTangents[i] : aiVector3D(0.0f);
		const auto bitangent = mesh->HasTangentsAndBitangents() ? mesh->mBitangents[i] : aiVector3D(0.0f);

		vertexBuffer->getVertices()->push_back({ { position.x, position.y, position.z }, { uv.x, uv.y }, { normal.x, normal.y, normal.z }, {tangent.x, tangent.y, tangent.z }, {bitangent.x, bitangent.y, bitangent.z } });
	}
}

Model::Model(const std::shared_ptr<Context> context, const std::shared_ptr<VertexBuffer> vertexBuffer, const std::shared_ptr<IndexBuffer> indexBuffer, const std::string &path, const std::string &filename)
{
	Assimp::Importer importer;
	const auto scene = importer.ReadFile(path + filename, aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);
	
	if (!scene)
	{
		throw std::runtime_error("Failed to load model \"" + path + filename + "\".");
	}

	if (!scene->HasMeshes())
	{
		throw std::runtime_error("Model \"" + path + filename + "\" has no meshes.");
	}

	if (!scene->HasMaterials())
	{
		throw std::runtime_error("Model \"" + path + filename + "\" has no materials.");
	}

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		const auto mesh = scene->mMeshes[i];

		if (!mesh->HasPositions())
		{
			continue;
		}

		const auto material = scene->mMaterials[mesh->mMaterialIndex];
		meshes.push_back(loadMeshData(context, mesh, material, path, filename, static_cast<uint32_t>(indexBuffer->getIndices()->size())));
		appendDataToIndexBuffer(mesh, indexBuffer, static_cast<uint32_t>(vertexBuffer->getVertices()->size()));
		appendDataToVertexBuffer(mesh, vertexBuffer);
	}
}

void Model::finalizeMaterials(const std::shared_ptr<DescriptorPool> descriptorPool)
{
	for (auto &mesh : meshes)
	{
		mesh->material->finalize(descriptorPool);
	}
}