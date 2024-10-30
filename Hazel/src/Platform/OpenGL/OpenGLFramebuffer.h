#pragma once

#include "Hazel/Renderer/Framebuffer.h"

namespace Hazel 
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer();

		void Invalidate();

		void Bind() override;
		void Unbind() override;

		void Resize(uint32_t width, uint32_t height) override;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override 
		{ 
			HZ_CORE_ASSERT(index < m_ColorAttachments.size(), "");
			return m_ColorAttachments[index]; 
		}
		const FramebufferSpecification& GetSpecification() const override { return m_Specification; }
	
	private:
		uint32_t m_RendererID = 0;
		//uint32_t m_ColorAttachment = 0, m_DepthAttachment = 0;
		FramebufferSpecification m_Specification;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
		FramebufferTextureSpecification m_DepthAttachmentSpecification = FramebufferTextureFormat::None;
		std::vector<uint32_t> m_ColorAttachments; // 保存纹理缓冲附件的id
		uint32_t m_DepthAttachment = 0;
	};

}