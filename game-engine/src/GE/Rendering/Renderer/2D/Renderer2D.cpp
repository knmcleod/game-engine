#include "GE/GEpch.h"

#include "Renderer2D.h"

namespace GE
{
	const glm::mat4 Renderer2D::s_IdentityMat4 = glm::mat4(1.0f);

	static Renderer2D::Renderer2DData s_Data;

	void Renderer2D::Init()
	{
		GE_PROFILE_FUNCTION();

		//Creates Index Buffer - Can be used for both Quad & Circle
		uint32_t* indices = new uint32_t[s_Data.MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
		{
			indices[i + 0] = offset + 0;
			indices[i + 1] = offset + 1;
			indices[i + 2] = offset + 2;

			indices[i + 3] = offset + 2;
			indices[i + 4] = offset + 3;
			indices[i + 5] = offset + 0;

			offset += 4;
		}
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(s_Data.MaxIndices, indices);

		// Quad/Sprite Rendering Setup
		{ 
			GE_PROFILE_SCOPE();

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

			//Add Index Buffer to Vertex Array
			s_Data.QuadVertexArray->AddIndexBuffer(indexBuffer);
			s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

			// Texture Creation
			uint32_t textureData = 0xFFFFFFFF;
			//s_Data.Texture = Texture2D::Create(1, 1, 4, &textureData, sizeof(uint32_t));
			s_Data.EmptyTexture = Texture2D::Create(1, 1, 4);
			s_Data.EmptyTexture->SetData(&textureData, sizeof(uint32_t));

			int32_t samplers[s_Data.MaxTextureSlots];
			for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
			{
				samplers[i] = i;
			}

			s_Data.QuadShader = Shader::Create("assets/shaders/Renderer2D_Sprite.glsl");
			s_Data.QuadShader->Bind();
			s_Data.QuadShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

			s_Data.TextureSlots[0] = s_Data.EmptyTexture;

			s_Data.QuadVertices[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
			s_Data.QuadVertices[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
			s_Data.QuadVertices[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
			s_Data.QuadVertices[3] = { -0.5f, 0.5f, 0.0f, 1.0f };
		}

		// Circle Rendering Setup
		{
			GE_PROFILE_SCOPE();

			//Creates Vertex Array
			s_Data.CircleVertexArray = VertexArray::Create();

			s_Data.CircleVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(CircleVertex));
			//Sets up Layout using Vertex Buffer
			BufferLayout layout =
			{
				{ GE::Shader::ShaderDataType::Float3, "a_GlobalPosition" },
				{ GE::Shader::ShaderDataType::Float3, "a_LocalPosition" },
				{ GE::Shader::ShaderDataType::Float4, "a_Color" },
				{ GE::Shader::ShaderDataType::Float, "a_Radius" },
				{ GE::Shader::ShaderDataType::Float, "a_Thickness" },
				{ GE::Shader::ShaderDataType::Float, "a_Fade" },
				{ GE::Shader::ShaderDataType::Int, "a_EntityID" }
			};
			s_Data.CircleVertexBuffer->SetLayout(layout);

			//Add Vertex Buffer to Vertex Array
			s_Data.CircleVertexArray->AddVertexBuffer(s_Data.CircleVertexBuffer);
			//Add Index Buffer to Vertex Array
			s_Data.CircleVertexArray->AddIndexBuffer(indexBuffer);
			s_Data.CircleVertexBufferBase = new CircleVertex[s_Data.MaxVertices];

			s_Data.CircleVertices[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
			s_Data.CircleVertices[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
			s_Data.CircleVertices[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
			s_Data.CircleVertices[3] = { -0.5f, 0.5f, 0.0f, 1.0f };

			s_Data.CircleShader = Shader::Create("assets/shaders/Renderer2D_Circle.glsl");
			s_Data.CircleShader->Bind();

		}

		// Line Rendering Setup
		{
			GE_PROFILE_SCOPE();

			//Creates Vertex Array
			s_Data.LineVertexArray = VertexArray::Create();

			s_Data.LineVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(LineVertex));
			//Sets up Layout using Vertex Buffer
			BufferLayout layout =
			{
				{ GE::Shader::ShaderDataType::Float3, "a_Position" },
				{ GE::Shader::ShaderDataType::Float4, "a_Color" },
				{ GE::Shader::ShaderDataType::Int, "a_EntityID" }
			};
			s_Data.LineVertexBuffer->SetLayout(layout);

			//Add Vertex Buffer to Vertex Array
			s_Data.LineVertexArray->AddVertexBuffer(s_Data.LineVertexBuffer);
			
			s_Data.LineVertexBufferBase = new LineVertex[s_Data.MaxVertices];

			s_Data.LineShader = Shader::Create("assets/shaders/Renderer2D_Line.glsl");
			s_Data.LineShader->Bind();
		}

		delete[] indices;
	}

	void Renderer2D::ShutDown()
	{
		GE_PROFILE_FUNCTION();

		delete[] s_Data.QuadVertexBufferBase;
	}

	void Renderer2D::Start(const EditorCamera& camera)
	{
		GE_PROFILE_FUNCTION();

		glm::mat4 viewProjection = camera.GetViewProjection();
		s_Data.QuadShader->SetMat4("u_ViewProjection", viewProjection);
		s_Data.CircleShader->SetMat4("u_ViewProjection", viewProjection);
		s_Data.LineShader->SetMat4("u_ViewProjection", viewProjection);

		ResetQuadData();
		ResetCircleData();
		ResetLineData();
	}

	void Renderer2D::Start(const OrthographicCamera& orthoCamera)
	{
		GE_PROFILE_FUNCTION();

		s_Data.QuadShader->SetMat4("u_ViewProjection", orthoCamera.GetViewProjectionMatrix());
		s_Data.CircleShader->SetMat4("u_ViewProjection", orthoCamera.GetViewProjectionMatrix());
		s_Data.LineShader->SetMat4("u_ViewProjection", orthoCamera.GetViewProjectionMatrix());

		ResetQuadData();
		ResetCircleData();
		ResetLineData();
	}

	void Renderer2D::Start(const Camera& camera, const glm::mat4& transform)
	{
		GE_PROFILE_FUNCTION();

		glm::mat4 viewProjection = camera.GetProjection() * glm::inverse(transform);
		s_Data.QuadShader->SetMat4("u_ViewProjection", viewProjection);
		s_Data.CircleShader->SetMat4("u_ViewProjection", viewProjection);
		s_Data.LineShader->SetMat4("u_ViewProjection", viewProjection);

		ResetQuadData();
		ResetCircleData();
		ResetLineData();
	}

	void Renderer2D::End()
	{
		GE_PROFILE_FUNCTION();

		Flush();
	}

	void Renderer2D::FlushLines()
	{
		if (s_Data.LineVertexCount == 0)
			return;

		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.LineVertexBufferPtr - (uint8_t*)s_Data.LineVertexBufferBase);
		s_Data.LineVertexBuffer->SetData(s_Data.LineVertexBufferBase, dataSize);

		// Draw Line Indices
		s_Data.LineShader->Bind();
		RenderCommand::SetLineWidth(s_Data.LineWidth);
		RenderCommand::DrawLines(s_Data.LineVertexArray, s_Data.LineVertexCount);
		s_Data.Stats.DrawCalls++;

		ResetLineData();
	}

	void Renderer2D::FlushCircles()
	{
		if (s_Data.CircleIndexCount == 0)
			return;

		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CircleVertexBufferPtr - (uint8_t*)s_Data.CircleVertexBufferBase);
		s_Data.CircleVertexBuffer->SetData(s_Data.CircleVertexBufferBase, dataSize);

		// Draw Circle Indices
		s_Data.CircleShader->Bind();
		RenderCommand::DrawIndices(s_Data.CircleVertexArray, s_Data.CircleIndexCount);
		s_Data.Stats.DrawCalls++;

		ResetCircleData();
	}

	void Renderer2D::FlushQuads()
	{
		if (s_Data.QuadIndexCount == 0)
			return;

		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

		// Bind Textures
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			s_Data.TextureSlots[i]->Bind(i);

		// Draw Quad Indices
		s_Data.QuadShader->Bind();
		RenderCommand::DrawIndices(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
		s_Data.Stats.DrawCalls++;

		ResetQuadData();
	}
	
	void Renderer2D::Flush()
	{
		FlushQuads();
		FlushCircles();
		FlushLines();
	}

#pragma region Sprite/Quad
	void Renderer2D::SetQuadData(const glm::mat4& transform, const float& textureIndex,
		const glm::vec2 textureCoords[4], const float& tilingFactor, const glm::vec4& color, const int entityID)
	{
		constexpr size_t quadVertexCount = 4;

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->GlobalPosition = transform * s_Data.QuadVertices[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TextureCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TextureIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr->EntityID = entityID;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;
		s_Data.Stats.SpawnCount++;
	}

	void Renderer2D::ResetQuadData()
	{
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer2D::FillQuad(const glm::mat4& transform, const glm::vec4& color, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushQuads();

		const glm::vec2 textureCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		const float textureIndex = 0.0f; // White Texture
		const float tilingFactor = 1.0f;

		SetQuadData(transform, textureIndex, textureCoords, tilingFactor, color, entityID);

	}

	void Renderer2D::FillQuad(const glm::mat4& transform, const Ref<Texture2D>& texture,
		const float& tilingFactor, const glm::vec4& color, const int entityID)
	{
		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushQuads();

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
				Flush();

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

		glm::mat4 transform = glm::translate(s_IdentityMat4, position)
			* glm::rotate(s_IdentityMat4, glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(s_IdentityMat4, { size.x, size.y, 1.0f });

		FillQuad(transform, color, entityID);

	}

	void Renderer2D::FillQuadTexture(const glm::vec3& position, const glm::vec2& size,
		const float rotation, const Ref<Texture2D>& texture, const float& tilingFactor, const glm::vec4& tintColor, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(s_IdentityMat4, position)
			* glm::rotate(s_IdentityMat4, glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(s_IdentityMat4, { size.x, size.y, 1.0f });

		FillQuad(transform, texture, tilingFactor, tintColor, entityID);
	}

	void Renderer2D::FillQuadSubTexture(const glm::vec3& position, const glm::vec2& size, const float rotation,
		const Ref<SubTexture2D>& subTexture, const float& tilingFactor, const glm::vec4& tintColor, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushQuads();

		glm::mat4 transform = glm::translate(s_IdentityMat4, position)
			* glm::rotate(s_IdentityMat4, glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(s_IdentityMat4, { size.x, size.y, 1.0f });

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
				Flush();

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

	void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& component, int entityID)
	{
		GE_PROFILE_FUNCTION();

		if (component.Texture)
			FillQuad(transform, component.Texture, component.TilingFactor, component.Color, entityID);
		else
			FillQuad(transform, component.Color, entityID);
	}

#pragma endregion

#pragma region Circle
	void Renderer2D::SetCircleData(const glm::mat4& transform, const glm::vec4& color, const float& radius,
		const float& thickness, const float& fade, const int entityID)
	{
		constexpr size_t circleVertexCount = 4;

		for (size_t i = 0; i < circleVertexCount; i++)
		{
			s_Data.CircleVertexBufferPtr->GlobalPosition = transform * s_Data.CircleVertices[i];
			s_Data.CircleVertexBufferPtr->LocalPosition = s_Data.CircleVertices[i] * 2.0f;
			s_Data.CircleVertexBufferPtr->Color = color;
			s_Data.CircleVertexBufferPtr->Radius = radius;
			s_Data.CircleVertexBufferPtr->Thickness = thickness;
			s_Data.CircleVertexBufferPtr->Fade = fade;
			s_Data.CircleVertexBufferPtr->EntityID = entityID;
			s_Data.CircleVertexBufferPtr++;
		}

		s_Data.CircleIndexCount += 6;
		s_Data.Stats.SpawnCount++;
	}

	void Renderer2D::ResetCircleData()
	{
		s_Data.CircleIndexCount = 0;
		s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;

	}

	void Renderer2D::FillCircle(const glm::mat4& transform, const glm::vec4& color, float radius, float thickness, float fade, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.CircleIndexCount >= s_Data.MaxIndices)
			FlushCircles();
			
		SetCircleData(transform, color, radius, thickness, fade, entityID);
	}

#pragma endregion

#pragma region Line

	void Renderer2D::SetLineData(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color, const int entityID)
	{
		s_Data.LineVertexBufferPtr->Position = initialPosition;
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr->EntityID = entityID;
		s_Data.LineVertexBufferPtr++;

		s_Data.LineVertexBufferPtr->Position = finalPosition;
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr->EntityID = entityID;
		s_Data.LineVertexBufferPtr++;

		s_Data.LineVertexCount += 2;
		s_Data.Stats.SpawnCount++;
	}

	void Renderer2D::ResetLineData()
	{
		s_Data.LineVertexCount = 0;
		s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;
	}

	void Renderer2D::SetLineWidth(float thickness)
	{
		s_Data.LineWidth = thickness;
	}

	float Renderer2D::GetLineWidth()
	{
		return s_Data.LineWidth;
	}

	void Renderer2D::FillLine(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.LineVertexCount >= s_Data.MaxIndices)
			FlushLines();

		SetLineData(initialPosition, finalPosition, color, entityID);
	}

	void Renderer2D::FillRectangle(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, const int entityID)
	{
		glm::vec3 p0 = glm::vec3(position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.y);
		glm::vec3 p1 = glm::vec3(position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.y);
		glm::vec3 p2 = glm::vec3(position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.y);
		glm::vec3 p3 = glm::vec3(position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.y);

		FillLine(p0, p1, color, entityID);
		FillLine(p1, p2, color, entityID);
		FillLine(p2, p3, color, entityID);
		FillLine(p3, p0, color, entityID);
	}
	
	void Renderer2D::DrawRectangle(const glm::mat4& transform, const glm::vec4& color, const int entityID)
	{
		glm::vec3 lineVertices[4];
		for (size_t i = 0; i < 4; i++)
		{
			lineVertices[i] = transform * s_Data.QuadVertices[i];
		}

		FillLine(lineVertices[0], lineVertices[1], color, entityID);
		FillLine(lineVertices[1], lineVertices[2], color, entityID);
		FillLine(lineVertices[2], lineVertices[3], color, entityID);
		FillLine(lineVertices[3], lineVertices[0], color, entityID);
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
