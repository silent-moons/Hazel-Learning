#pragma once

#include <glm/glm.hpp>

namespace Hazel
{
    enum class MeshType  // 未来支持复杂模型和骨骼动画
    {
        StaticBatchable,  // 可批处理的
        StaticUnique,     // 不可批处理的
        SkinnedMesh       // 有骨骼动画的
    };

    class Mesh 
    {
    public:
        void LoadBaseGeometry(
            const std::vector<glm::vec4>& vertices,
            const std::vector<uint32_t>& indices,
            const std::vector<glm::vec2>& textureCoords);
        const std::vector<glm::vec4>& GetVertices() const { return m_Vertices; }
        const std::vector<uint32_t>& GetIndices() const { return m_Indices; }
        const std::vector<glm::vec2>& GetTextureCoords() const { return m_TextureCoords; }
        size_t GetVertexCount() const { return m_Vertices.size(); }
        size_t GetIndexCount() const { return m_Indices.size(); }
        MeshType GetMeshType() const { return m_MeshType; }
        void SetMeshType(MeshType meshType) { m_MeshType = meshType; }
    private:
        std::vector<glm::vec4> m_Vertices;
        std::vector<uint32_t> m_Indices;
        std::vector<glm::vec2> m_TextureCoords;
        MeshType m_MeshType;
    };

    class MeshLibrary 
    {
    public:
        static Ref<Mesh> GetCubeMesh();
        static Ref<Mesh> GetSphereMesh();
    private:
        static Ref<Mesh> s_CubeMesh;
        static Ref<Mesh> s_SphereMesh;
    };
}