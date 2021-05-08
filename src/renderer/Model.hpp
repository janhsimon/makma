#pragma once

#include "Material.hpp"
#include "buffers/IndexBuffer.hpp"
#include "buffers/VertexBuffer.hpp"
#include "core/Transform.hpp"

#include <assimp/scene.h>

struct Mesh
{
  uint32_t firstIndex, indexCount;
  std::shared_ptr<Material> material;
};

class Model : public Transform
{
private:
  std::vector<std::shared_ptr<Mesh>> meshes;

  std::shared_ptr<Mesh> loadMeshData(const std::shared_ptr<Context> context,
                                     const aiMesh* mesh,
                                     const aiMaterial* material,
                                     const std::string& path,
                                     const std::string& filename,
                                     const uint32_t numIndices);
  void appendDataToIndexBuffer(const aiMesh* mesh,
                               const std::shared_ptr<IndexBuffer> indexBuffer,
                               const uint32_t numVertices);
  void appendDataToVertexBuffer(const aiMesh* mesh, const std::shared_ptr<VertexBuffer> vertexBuffer);

public:
  Model(const std::shared_ptr<Context> context,
        const std::shared_ptr<VertexBuffer> vertexBuffer,
        const std::shared_ptr<IndexBuffer> indexBuffer,
        const std::string& path,
        const std::string& filename);

  void finalizeMaterials(const std::shared_ptr<DescriptorPool> descriptorPool);

  std::vector<std::shared_ptr<Mesh>>* getMeshes()
  {
    return &meshes;
  }
};