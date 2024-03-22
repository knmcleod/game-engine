#include "GE/GEpch.h"

#include "Renderer2D.h"

#include "GE/Rendering/Camera/Camera.h"
#include "GE/Rendering/Camera/Editor/EditorCamera.h"
#include "GE/Rendering/RenderCommand.h"
#include "GE/Rendering/Shader/Shader.h"

namespace GE
{
	const glm::mat4 Renderer2D::s_IdentityMat4 = glm::mat4(1.0f);

	static Renderer2D::Renderer2DData s_RendererData;

	void Renderer2D::Init()
	{
		GE_PROFILE_FUNCTION();

		//Creates Index Buffer - Can be used for both Quad & Circle
		uint32_t* indices = new uint32_t[s_RendererData.MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_RendererData.MaxIndices; i += 6)
		{
			indices[i + 0] = offset + 0;
			indices[i + 1] = offset + 1;
			indices[i + 2] = offset + 2;

			indices[i + 3] = offset + 2;
			indices[i + 4] = offset + 3;
			indices[i + 5] = offset + 0;

			offset += 4;
		}
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(s_RendererData.MaxIndices, indices);

		// Quad/Sprite Rendering Setup
		{ 
			GE_PROFILE_SCOPE("Renderer2D - Init() : Quad/Sprite Rendering Setup");

			//Creates Vertex Array
			s_RendererData.QuadVertexArray = VertexArray::Create();

			s_RendererData.QuadVertexBuffer = VertexBuffer::Create(s_RendererData.MaxVertices * sizeof(QuadVertex));
			//Sets up Layout using Vertex Buffer
			BufferLayout layout =
			{
				{ GE::Shader::ShaderDataType::Float3,	"a_Position"	 },
				{ GE::Shader::ShaderDataType::Float4,	"a_Color"		 },
				{ GE::Shader::ShaderDataType::Float2,	"a_TextureCoord" },
				{ GE::Shader::ShaderDataType::Float,	"a_TextureIndex" },
				{ GE::Shader::ShaderDataType::Float,	"a_TilingFactor" },
				{ GE::Shader::ShaderDataType::Int,		"a_EntityID"	 }
			};
			s_RendererData.QuadVertexBuffer->SetLayout(layout);

			//Add Vertex Buffer to Vertex Array
			s_RendererData.QuadVertexArray->AddVertexBuffer(s_RendererData.QuadVertexBuffer);

			//Add Index Buffer to Vertex Array
			s_RendererData.QuadVertexArray->AddIndexBuffer(indexBuffer);
			s_RendererData.QuadVertexBufferBase = new QuadVertex[s_RendererData.MaxVertices];

			// Texture Creation
			uint32_t textureData = 0xFFFFFFFF;
			//s_RendererData.Texture = Texture2D::Create(1, 1, 4, &textureData, sizeof(uint32_t));
			s_RendererData.EmptyTexture = Texture2D::Create(TextureConfiguration{});
			s_RendererData.EmptyTexture->SetData(&textureData, sizeof(uint32_t));

			int32_t samplers[s_RendererData.MaxTextureSlots];
			for (uint32_t i = 0; i < s_RendererData.MaxTextureSlots; i++)
			{
				samplers[i] = i;
			}

			s_RendererData.QuadShader = Shader::Create("assets/shaders/Renderer2D_Sprite.glsl");
			s_RendererData.QuadShader->Bind();
			s_RendererData.QuadShader->SetIntArray("u_Textures", samplers, s_RendererData.MaxTextureSlots);

			s_RendererData.TextureSlots[0] = s_RendererData.EmptyTexture;

			s_RendererData.QuadVertices[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
			s_RendererData.QuadVertices[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
			s_RendererData.QuadVertices[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
			s_RendererData.QuadVertices[3] = { -0.5f, 0.5f, 0.0f, 1.0f };
		}

		// Circle Rendering Setup
		{
			GE_PROFILE_SCOPE("Renderer2D - Init() : Circle Rendering Setup");

			//Creates Vertex Array
			s_RendererData.CircleVertexArray = VertexArray::Create();

			s_RendererData.CircleVertexBuffer = VertexBuffer::Create(s_RendererData.MaxVertices * sizeof(CircleVertex));
			//Sets up Layout using Vertex Buffer
			BufferLayout layout =
			{
				{ GE::Shader::ShaderDataType::Float3, "a_GlobalPosition"	},
				{ GE::Shader::ShaderDataType::Float3, "a_LocalPosition"		},
				{ GE::Shader::ShaderDataType::Float4, "a_Color"				},
				{ GE::Shader::ShaderDataType::Float, "a_Radius"				},
				{ GE::Shader::ShaderDataType::Float, "a_Thickness"			},
				{ GE::Shader::ShaderDataType::Float, "a_Fade"				},
				{ GE::Shader::ShaderDataType::Int, "a_EntityID"				}
			};
			s_RendererData.CircleVertexBuffer->SetLayout(layout);

			//Add Vertex Buffer to Vertex Array
			s_RendererData.CircleVertexArray->AddVertexBuffer(s_RendererData.CircleVertexBuffer);
			//Add Index Buffer to Vertex Array
			s_RendererData.CircleVertexArray->AddIndexBuffer(indexBuffer);
			s_RendererData.CircleVertexBufferBase = new CircleVertex[s_RendererData.MaxVertices];

			s_RendererData.CircleVertices[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
			s_RendererData.CircleVertices[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
			s_RendererData.CircleVertices[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
			s_RendererData.CircleVertices[3] = { -0.5f, 0.5f, 0.0f, 1.0f };

			s_RendererData.CircleShader = Shader::Create("assets/shaders/Renderer2D_Circle.glsl");
		}
		
		// Line Rendering Setup
		{
			GE_PROFILE_SCOPE("Renderer2D - Init() : Line Rendering Setup");

			//Creates Vertex Array
			s_RendererData.LineVertexArray = VertexArray::Create();

			s_RendererData.LineVertexBuffer = VertexBuffer::Create(s_RendererData.MaxVertices * sizeof(LineVertex));
			//Sets up Layout using Vertex Buffer
			BufferLayout layout =
			{
				{ GE::Shader::ShaderDataType::Float3, "a_Position"	},
				{ GE::Shader::ShaderDataType::Float4, "a_Color"		},
				{ GE::Shader::ShaderDataType::Int, "a_EntityID"		}
			};
			s_RendererData.LineVertexBuffer->SetLayout(layout);

			//Add Vertex Buffer to Vertex Array
			s_RendererData.LineVertexArray->AddVertexBuffer(s_RendererData.LineVertexBuffer);
			
			s_RendererData.LineVertexBufferBase = new LineVertex[s_RendererData.MaxVertices];

			s_RendererData.LineShader = Shader::Create("assets/shaders/Renderer2D_Line.glsl");
		}

		delete[] indices;
	}

	void Renderer2D::ShutDown()
	{
		GE_PROFILE_FUNCTION();

		delete[] s_RendererData.QuadVertexBufferBase;
	
		delete[] s_RendererData.CircleVertexBufferBase;
	
		delete[] s_RendererData.LineVertexBufferBase;
	
	}

	void Renderer2D::Start(const EditorCamera& camera)
	{
		GE_PROFILE_FUNCTION();

		glm::mat4 viewProjection = camera.GetViewProjection();
		s_RendererData.QuadShader->SetMat4("u_ViewProjection", viewProjection);
		s_RendererData.CircleShader->SetMat4("u_ViewProjection", viewProjection);
		s_RendererData.LineShader->SetMat4("u_ViewProjection", viewProjection);

		ResetQuadData();
		ResetCircleData();
		ResetLineData();
	}

	void Renderer2D::Start(const Camera& camera, const glm::mat4& transform)
	{
		GE_PROFILE_FUNCTION();

		glm::mat4 viewProjection = camera.GetProjection() * glm::inverse(transform);
		s_RendererData.QuadShader->SetMat4("u_ViewProjection", viewProjection);
		s_RendererData.CircleShader->SetMat4("u_ViewProjection", viewProjection);
		s_RendererData.LineShader->SetMat4("u_ViewProjection", viewProjection);

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
		if (s_RendererData.LineVertexCount == 0)
			return;

		s_RendererData.LineShader->Bind();
		uint32_t dataSize = (uint32_t)((uint8_t*)s_RendererData.LineVertexBufferPtr - (uint8_t*)s_RendererData.LineVertexBufferBase);
		s_RendererData.LineVertexBuffer->SetData(s_RendererData.LineVertexBufferBase, dataSize);

		// Draw Line Indices
		s_RendererData.LineVertexArray->Bind();
		RenderCommand::SetLineWidth(s_RendererData.LineWidth);
		RenderCommand::DrawLines(s_RendererData.LineVertexArray, s_RendererData.LineVertexCount);
		s_RendererData.Stats.DrawCalls++;

		ResetLineData();
	}

	void Renderer2D::FlushCircles()
	{
		if (s_RendererData.CircleIndexCount == 0)
			return;

		s_RendererData.CircleShader->Bind();
		uint32_t dataSize = (uint32_t)((uint8_t*)s_RendererData.CircleVertexBufferPtr - (uint8_t*)s_RendererData.CircleVertexBufferBase);
		s_RendererData.CircleVertexBuffer->SetData(s_RendererData.CircleVertexBufferBase, dataSize);

		// Draw Circle Indices
		s_RendererData.CircleVertexArray->Bind();
		RenderCommand::DrawIndices(s_RendererData.CircleVertexArray, s_RendererData.CircleIndexCount);
		s_RendererData.Stats.DrawCalls++;

		ResetCircleData();
	}

	void Renderer2D::FlushQuads()
	{
		if (s_RendererData.QuadIndexCount == 0)
			return;

		s_RendererData.QuadShader->Bind();
		uint32_t dataSize = (uint32_t)((uint8_t*)s_RendererData.QuadVertexBufferPtr - (uint8_t*)s_RendererData.QuadVertexBufferBase);
		s_RendererData.QuadVertexBuffer->SetData(s_RendererData.QuadVertexBufferBase, dataSize);

		// Bind Textures
		for (uint32_t i = 0; i < s_RendererData.TextureSlotIndex; i++)
			s_RendererData.TextureSlots[i]->Bind(i);

		// Draw Quad Indices
		s_RendererData.QuadVertexArray->Bind();
		RenderCommand::DrawIndices(s_RendererData.QuadVertexArray, s_RendererData.QuadIndexCount);
		s_RendererData.Stats.DrawCalls++;

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
			s_RendererData.QuadVertexBufferPtr->Position = transform * s_RendererData.QuadVertices[i];
			s_RendererData.QuadVertexBufferPtr->Color = color;
			s_RendererData.QuadVertexBufferPtr->TextureCoord = textureCoords[i];
			s_RendererData.QuadVertexBufferPtr->TextureIndex = textureIndex;
			s_RendererData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_RendererData.QuadVertexBufferPtr->EntityID = entityID;
			s_RendererData.QuadVertexBufferPtr++;
		}

		s_RendererData.QuadIndexCount += 6;
		s_RendererData.Stats.SpawnCount++;
	}

	void Renderer2D::ResetQuadData()
	{
		s_RendererData.QuadIndexCount = 0;
		s_RendererData.QuadVertexBufferPtr = s_RendererData.QuadVertexBufferBase;

		s_RendererData.TextureSlotIndex = 1;
	}

	void Renderer2D::FillQuad(const glm::mat4& transform, const glm::vec4& color, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_RendererData.QuadIndexCount >= s_RendererData.MaxIndices)
			FlushQuads();

		const glm::vec2 textureCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		const float textureIndex = 0.0f; // White Texture
		const float tilingFactor = 1.0f;

		SetQuadData(transform, textureIndex, textureCoords, tilingFactor, color, entityID);

	}

	void Renderer2D::FillQuad(const glm::mat4& transform, const Ref<Texture2D>& texture,
		const float& tilingFactor, const glm::vec4& color, const int entityID)
	{
		if (s_RendererData.QuadIndexCount >= s_RendererData.MaxIndices)
			FlushQuads();

		const glm::vec2 textureCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		float textureIndex = 0.0f;

		for (uint32_t i = 1; i < s_RendererData.TextureSlotIndex; i++)
		{
			if (*s_RendererData.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (s_RendererData.TextureSlotIndex >= s_RendererData.MaxTextureSlots)
				Flush();

			textureIndex = (float)s_RendererData.TextureSlotIndex;
			s_RendererData.TextureSlots[(int)textureIndex] = texture;
			s_RendererData.TextureSlotIndex++;
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

		if (s_RendererData.QuadIndexCount >= s_RendererData.MaxIndices)
			FlushQuads();

		glm::mat4 transform = glm::translate(s_IdentityMat4, position)
			* glm::rotate(s_IdentityMat4, glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(s_IdentityMat4, { size.x, size.y, 1.0f });

		const glm::vec2* textureCoords = subTexture->GetTextureCoords();

		float textureIndex = 0.0f;

		for (uint32_t i = 1; i < s_RendererData.TextureSlotIndex; i++)
		{
			if (*s_RendererData.TextureSlots[i].get() == *subTexture->GetTexture().get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (s_RendererData.TextureSlotIndex >= s_RendererData.MaxTextureSlots)
				Flush();

			if (s_RendererData.TextureSlots[s_RendererData.TextureSlotIndex] != NULL)
			{
				s_RendererData.TextureSlotIndex++;
			}
			textureIndex = (float)s_RendererData.TextureSlotIndex;
			s_RendererData.TextureSlots[(int)textureIndex] = subTexture->GetTexture();
			s_RendererData.TextureSlotIndex++;
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
	void Renderer2D::SetCircleData(const glm::mat4& transform, const glm::vec4& color, const float& radius, const float& thickness, const float& fade, const int entityID)
	{
		constexpr size_t circleVertexCount = 4;

		for (size_t i = 0; i < circleVertexCount; i++)
		{
			s_RendererData.CircleVertexBufferPtr->GlobalPosition = transform * s_RendererData.CircleVertices[i];
			s_RendererData.CircleVertexBufferPtr->LocalPosition = s_RendererData.CircleVertices[i] * 2.0f;
			s_RendererData.CircleVertexBufferPtr->Color = color;
			s_RendererData.CircleVertexBufferPtr->Radius = radius;
			s_RendererData.CircleVertexBufferPtr->Thickness = thickness;
			s_RendererData.CircleVertexBufferPtr->Fade = fade;
			s_RendererData.CircleVertexBufferPtr->EntityID = entityID;
			s_RendererData.CircleVertexBufferPtr++;
		}

		s_RendererData.CircleIndexCount += 6;
		s_RendererData.Stats.SpawnCount++;
	}

	void Renderer2D::ResetCircleData()
	{
		s_RendererData.CircleIndexCount = 0;
		s_RendererData.CircleVertexBufferPtr = s_RendererData.CircleVertexBufferBase;

	}

	void Renderer2D::FillCircle(const glm::mat4& transform, const glm::vec4& color, const float& radius, const float& thickness, const float& fade, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_RendererData.CircleIndexCount >= s_RendererData.MaxIndices)
			FlushCircles();
			
		SetCircleData(transform, color, radius, thickness, fade, entityID);
	}

#pragma endregion

#pragma region Line

	void Renderer2D::SetLineData(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color, const int entityID)
	{
		s_RendererData.LineVertexBufferPtr->Position = initialPosition;
		s_RendererData.LineVertexBufferPtr->Color = color;
		s_RendererData.LineVertexBufferPtr->EntityID = entityID;
		s_RendererData.LineVertexBufferPtr++;

		s_RendererData.LineVertexBufferPtr->Position = finalPosition;
		s_RendererData.LineVertexBufferPtr->Color = color;
		s_RendererData.LineVertexBufferPtr->EntityID = entityID;
		s_RendererData.LineVertexBufferPtr++;

		s_RendererData.LineVertexCount += 2;
		s_RendererData.Stats.SpawnCount++;
	}

	void Renderer2D::ResetLineData()
	{
		s_RendererData.LineVertexCount = 0;
		s_RendererData.LineVertexBufferPtr = s_RendererData.LineVertexBufferBase;
	}

	void Renderer2D::SetLineWidth(float thickness)
	{
		s_RendererData.LineWidth = thickness;
	}

	float Renderer2D::GetLineWidth()
	{
		return s_RendererData.LineWidth;
	}

	void Renderer2D::FillLine(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_RendererData.LineVertexCount >= s_RendererData.MaxIndices)
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
			lineVertices[i] = transform * s_RendererData.QuadVertices[i];
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
		return s_RendererData.Stats;
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_RendererData.Stats, 0, sizeof(Renderer2D::Statistics));
	}
#pragma endregion

}
