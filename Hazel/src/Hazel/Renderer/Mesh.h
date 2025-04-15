#pragma once

#include <glm/glm.hpp>

namespace Hazel
{
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
    private:
        std::vector<glm::vec4> m_Vertices;
        std::vector<uint32_t> m_Indices;
        std::vector<glm::vec2> m_TextureCoords;
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