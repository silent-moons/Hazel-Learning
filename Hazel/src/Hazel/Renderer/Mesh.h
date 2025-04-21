#pragma once

#include <glm/glm.hpp>

#include "Hazel/Renderer/Texture.h"
#include "Hazel/Renderer/VertexArray.h"

namespace Hazel
{
    enum class MeshType  // 未来支持复杂模型和骨骼动画
    {
        StaticBatchable,  // 可批处理的
        StaticUnique,     // 不可批处理的
        SkinnedMesh       // 有骨骼动画的
    };

    struct Vertex 
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
    };

    class Mesh 
    {
    public:
        virtual ~Mesh() = default;

        virtual void Bind() const {}
        virtual void Unbind() const {}

        virtual const std::vector<Vertex>& GetVertices() const { return m_EmptyVertices; }
        virtual const std::vector<glm::vec3>& GetVerticesPositions() const = 0;
        virtual const std::vector<uint32_t>& GetIndices() const = 0;
        virtual const std::vector<glm::vec2>& GetTextureCoords() const = 0;
        virtual size_t GetVertexCount() const = 0;
        virtual size_t GetIndexCount() const = 0;
        virtual MeshType GetMeshType() const = 0;
        virtual void SetMeshType(MeshType meshType) = 0;
        virtual Ref<VertexArray> GetVAO() const { return nullptr; }
        virtual const std::vector<Ref<Texture2D>>& GetTextures() const { return m_EmptyTextures; }
    private:
        std::vector<Vertex> m_EmptyVertices;
        std::vector<Ref<Texture2D>> m_EmptyTextures;
    };

    class BatchMesh : public Mesh 
    {
    public:
        const std::vector<glm::vec3>& GetVerticesPositions() const override { return m_VerticesPositions; }
        const std::vector<uint32_t>& GetIndices() const override { return m_Indices; }
        const std::vector<glm::vec2>& GetTextureCoords() const override { return m_TextureCoords; }
        size_t GetVertexCount() const override { return m_VerticesPositions.size(); }
        size_t GetIndexCount() const override { return m_Indices.size(); }
        MeshType GetMeshType() const override { return m_MeshType; }
        void SetMeshType(MeshType meshType) override { m_MeshType = meshType; }
    private:
        void LoadBaseGeometry(
            const std::vector<glm::vec3>& vertices,
            const std::vector<uint32_t>& indices,
            const std::vector<glm::vec2>& textureCoords);
    private:
        // 顶点属性，批处理Mesh只存储顶点位置和纹理坐标，其它属性在渲染器批处理缓冲区中处理
        std::vector<glm::vec3> m_VerticesPositions;
        std::vector<uint32_t> m_Indices;
        std::vector<glm::vec2> m_TextureCoords;
        MeshType m_MeshType;

        friend class BatchMeshLibrary;
    };

    class UniqueMesh : public Mesh
    {
    public:
        UniqueMesh(std::vector<Vertex>& vertices,
            std::vector<uint32_t>& indices,
            std::vector<Ref<Texture2D>>& textures);
        void Bind() const override;
        void Unbind() const override;

        const std::vector<Vertex>& GetVertices() const override { return m_Vertices; }
        const std::vector<glm::vec3>& GetVerticesPositions() const override { return m_VerticesPositions; }
        const std::vector<uint32_t>& GetIndices() const override { return m_Indices; }
        const std::vector<glm::vec2>& GetTextureCoords() const override { return m_TextureCoords; }
        size_t GetVertexCount() const override { return m_Vertices.size(); }
        size_t GetIndexCount() const override { return m_Indices.size(); }

        MeshType GetMeshType() const override { return MeshType::StaticUnique; }
        void SetMeshType(MeshType meshType) override { m_MeshType = meshType; }
        Ref<VertexArray> GetVAO() const override { return m_VAO; }
        const std::vector<Ref<Texture2D>>& GetTextures() const override { return m_Textures; }

    private:
        std::vector<glm::vec3> m_VerticesPositions;
        std::vector<glm::vec2> m_TextureCoords;

        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;
        std::vector<Ref<Texture2D>> m_Textures;
        Ref<VertexArray> m_VAO;
        Ref<VertexBuffer> m_VBO;
        Ref<IndexBuffer> m_IBO;

        MeshType m_MeshType;
    };

    class BatchMeshLibrary // 批处理Mesh数据只需加载一次
    {
    public:
        BatchMeshLibrary() = delete;
        BatchMeshLibrary(const BatchMeshLibrary&) = delete;
        BatchMeshLibrary& operator=(const BatchMeshLibrary&) = delete;
    public:
        static Ref<BatchMesh> GetCubeMesh();
        static Ref<BatchMesh> GetSphereMesh();
        static Ref<Mesh> GetTempMesh();
    private:
        static Ref<BatchMesh> s_CubeMesh;
        static Ref<BatchMesh> s_SphereMesh;
        static Ref<Mesh> s_TempMesh;
    };
}