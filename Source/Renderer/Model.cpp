#include "Model.hpp"

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>

std::shared_ptr<Mesh> Model::loadMeshData(const aiMesh *mesh, const aiMaterial *material, const std::string &path, const std::string &filename)
{
	auto meshData = std::make_shared<Mesh>();

	meshData->firstIndex = static_cast<uint32_t>(buffers->getIndices()->size());
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
	
	if (material->GetTexture(aiTextureType_SHININESS, 0, &occlusionTextureFilename) == aiReturn::aiReturn_SUCCESS)
	{
		meshData->material->setOcclusionTexture(path + std::string(occlusionTextureFilename.C_Str()));
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

void Model::appendDataToIndexBuffer(const aiMesh *mesh)
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
			buffers->getIndices()->push_back(static_cast<uint32_t>(buffers->getVertices()->size()) + face.mIndices[j]);
		}
	}
}

void Model::appendDataToVertexBuffer(const aiMesh *mesh)
{
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		const auto position = mesh->mVertices[i];
		const auto uv = mesh->mTextureCoords[0][i];
		const auto normal = mesh->mNormals[i];
		const auto tangent = mesh->mTangents[i];
		const auto bitangent = mesh->mBitangents[i];

		buffers->getVertices()->push_back({ { position.x, position.y, position.z }, { uv.x, uv.y }, { normal.x, normal.y, normal.z }, {tangent.x, tangent.y, tangent.z }, {bitangent.x, bitangent.y, bitangent.z } });
	}
}

Model::Model(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::string &path, const std::string &filename)
{
	this->context = context;
	this->buffers = buffers;
	
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

		if (!mesh->HasPositions() || !mesh->HasTextureCoords(0) || !mesh->HasFaces() || !mesh->HasTangentsAndBitangents())
		{
			continue;
		}

		const auto material = scene->mMaterials[mesh->mMaterialIndex];
		meshes.push_back(loadMeshData(mesh, material, path, filename));
		appendDataToIndexBuffer(mesh);
		appendDataToVertexBuffer(mesh);
	}
}

void Model::finalizeMaterials(const std::shared_ptr<Descriptor> descriptor)
{
	for (auto &mesh : meshes)
	{
		mesh->material->finalize(descriptor);
	}
}