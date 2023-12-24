#include "GE/GEpch.h"

#include "Renderer2D.h"

namespace GE
{
	static Renderer2D::Renderer2DData s_Data;

	static const glm::mat4 m_identityMat4(1.0f);

	void Renderer2D::Init()
	{
		GE_PROFILE_FUNCTION();

		//Creates Vertex Array
		s_Data.QuadVertexArray = VertexArray::Create();
		
		s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
		//Sets up Layout using Vertex Buffer
		BufferLayout layout =
		{
			{ GE::Shader::ShaderDataType::Float3, "a_Position" },
			{ GE::Shader::ShaderDataType::Float4, "a_Color" },
			{ GE::Shader::ShaderDataType::Float2, "a_TextureCoord" },
			{ GE::Shader::ShaderDataType::Float, "a_TextureIndex" },
			{ GE::Shader::ShaderDataType::Float, "a_TilingFactor" },
			{ GE::Shader::ShaderDataType::Int, "a_EntityID" }
		};
		s_Data.QuadVertexBuffer->SetLayout(layout);

		//Add Vertex Buffer to Vertex Array
		s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

		//Creates Index Buffer
		uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices]; // PROBLEM

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(s_Data.MaxIndices, quadIndices);

		//Add Index Buffer to Vertex Array
		s_Data.QuadVertexArray->AddIndexBuffer(indexBuffer);
		
		delete[] quadIndices;

		// Texture Creation
		uint32_t textureData = 0xFFFFFFFF;
		s_Data.EmptyTexture = Texture2D::Create(1, 1, 4);
		s_Data.EmptyTexture->SetData(&textureData, sizeof(uint32_t));
		
		int32_t samplers[s_Data.MaxTextureSlots];
		for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
		{
			samplers[i] = i;
		}

		s_Data.Shader = Shader::Create("assets/shaders/Texture.glsl");
		//s_Data.Shader->Bind();
		//s_Data.Shader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

		s_Data.TextureSlots[0] = s_Data.EmptyTexture;

		s_Data.QuadVertices[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
		s_Data.QuadVertices[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertices[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
		s_Data.QuadVertices[3] = { -0.5f, 0.5f, 0.0f, 1.0f };

		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(UniformBuffer), 0);
	}

	void Renderer2D::ShutDown()
	{
		GE_PROFILE_FUNCTION();

		delete[] s_Data.QuadVertexBufferBase;
	}

	void Renderer2D::Start(const Camera& camera, const glm::mat4& transform)
	{
		GE_PROFILE_FUNCTION();

		s_Data.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Camera::CameraData));

		//s_Data.Shader->SetMat4("u_ViewProjection", viewProjection);

		ResetQuadData();
	}

	void Renderer2D::Start(const EditorCamera& camera)
	{
		GE_PROFILE_FUNCTION();

		s_Data.CameraBuffer.ViewProjection = camera.GetProjection();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Camera::CameraData));

		//s_Data.Shader->SetMat4("u_ViewProjection", viewProjection);

		ResetQuadData();
	}

	void Renderer2D::Start(const OrthographicCamera& orthoCamera)
	{
		GE_PROFILE_FUNCTION();

		s_Data.Shader->SetMat4("u_ViewProjection", orthoCamera.GetViewProjectionMatrix());
	
		ResetQuadData();
	}

	void Renderer2D::End()
	{
		GE_PROFILE_FUNCTION();

		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

		Flush();

	}

	void Renderer2D::Flush()
	{
		if (s_Data.QuadIndexCount == 0)
			return;

		// Bind Textures
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
		{
			s_Data.TextureSlots[i]->Bind(i);
		}

		// Draw
		RenderCommand::DrawIndices(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
		s_Data.Stats.DrawCalls++;
	}
	
	void Renderer2D::FlushAndReset()
	{
		End();
		
		ResetQuadData();
	}
#pragma region Quad

	void Renderer2D::FillQuad(const glm::mat4& transform, const glm::vec4& color, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
		{
			FlushAndReset();
		}

		const glm::vec2 textureCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		const float textureIndex = 0.0f; // White Texture
		const float tilingFactor = 1.0f;

		SetQuadData(transform, textureIndex, textureCoords, tilingFactor, color, entityID);

	}

	void Renderer2D::FillQuad(const glm::mat4& transform, const Ref<Texture2D>& texture,
		const float& tilingFactor, const glm::vec4& color, const int entityID)
	{
		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
		{
			FlushAndReset();
		}

		const glm::vec2 textureCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		float textureIndex = 0.0f;

		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (s_Data.TextureSlotIndex >= s_Data.MaxTextureSlots)
				FlushAndReset();

			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[(int)textureIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		SetQuadData(transform, textureIndex, textureCoords, tilingFactor, color, entityID);

	}

	void Renderer2D::FillQuadColor(const glm::vec3& position, const glm::vec2& size,
		const float rotation, const glm::vec4& color, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(m_identityMat4, position)
			* glm::rotate(m_identityMat4, glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(m_identityMat4, { size.x, size.y, 1.0f });

		FillQuad(transform, color, entityID);

	}

	void Renderer2D::FillQuadTexture(const glm::vec3& position, const glm::vec2& size,
		const float rotation, const Ref<Texture2D>& texture, const float& tilingFactor, const glm::vec4& tintColor, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(m_identityMat4, position)
			* glm::rotate(m_identityMat4, glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(m_identityMat4, { size.x, size.y, 1.0f });

		FillQuad(transform, texture, tilingFactor, tintColor, entityID);
	}

	void Renderer2D::FillQuadSubTexture(const glm::vec3& position, const glm::vec2& size, const float rotation,
		const Ref<SubTexture2D>& subTexture, const float& tilingFactor, const glm::vec4& tintColor, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
		{
			FlushAndReset();
		}

		glm::mat4 transform = glm::translate(m_identityMat4, position)
			* glm::rotate(m_identityMat4, glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(m_identityMat4, { size.x, size.y, 1.0f });

		const glm::vec2* textureCoords = subTexture->GetTextureCoords();

		float textureIndex = 0.0f;

		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *subTexture->GetTexture().get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (s_Data.TextureSlotIndex >= s_Data.MaxTextureSlots)
				FlushAndReset();

			if (s_Data.TextureSlots[s_Data.TextureSlotIndex] != NULL)
			{
				s_Data.TextureSlotIndex++;
			}
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[(int)textureIndex] = subTexture->GetTexture();
			s_Data.TextureSlotIndex++;
		}

		SetQuadData(transform, textureIndex, textureCoords, tilingFactor, tintColor, entityID);
	}

	void Renderer2D::SetQuadData(const glm::mat4& transform, const float& textureIndex,
		const glm::vec2 textureCoords[4], const float& tilingFactor, const glm::vec4& color, const int entityID)
	{
		constexpr size_t quadVertexCount = 4;

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertices[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TextureCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr->EntityID = entityID;
			s_Data.QuadVertexBufferPtr++;
		}
		
		s_Data.QuadIndexCount += 6;
		s_Data.Stats.QuadCount++;
	}
	
	void Renderer2D::ResetQuadData()
	{
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.TextureSlotIndex = 1;
	}

#pragma endregion

#pragma region Sprite
	void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& component, int entityID)
	{
		GE_PROFILE_FUNCTION();

		FillQuad(transform, component.Color, entityID);
	}
#pragma endregion

#pragma region Statistics
	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_Data.Stats;
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Renderer2D::Statistics));
	}
#pragma endregion

}
