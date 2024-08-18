#include "GE/GEpch.h"

#include "Renderer2D.h"

#include "GE/Project/Project.h"

#include "GE/Rendering/Camera/Camera.h"
#include "GE/Rendering/RenderCommand.h"
#include "GE/Rendering/Shader/Shader.h"

namespace GE
{
	Renderer2D::Data Renderer2D::s_Data = Renderer2D::Data();

	void Renderer2D::Init()
	{
		GE_PROFILE_FUNCTION();

		//Creates Index Buffer - Can be used for both Quad & Circle
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(s_Data.MaxIndices);

		// Quad/Sprite Rendering Setup
		{ 
			GE_PROFILE_SCOPE("Renderer2D - Init() : Quad/Sprite Rendering Setup");

			// TODO: Let vertexArray control buffers?
			//Creates Vertex Array
			s_Data.quadData.VertexArray = VertexArray::Create();

			s_Data.quadData.VertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
			//Sets up Layout using Vertex Buffer
			VertexBuffer::Layout layout =
			{
				{ GE::Shader::DataType::Float3,	"a_Position"	 },
				{ GE::Shader::DataType::Float4,	"a_Color"		 },
				{ GE::Shader::DataType::Float2,	"a_TextureCoord" },
				{ GE::Shader::DataType::Int,	"a_TextureIndex" },
				{ GE::Shader::DataType::Float,	"a_TilingFactor" },
				{ GE::Shader::DataType::Int,	"a_EntityID"	 }
			};
			s_Data.quadData.VertexBuffer->SetLayout(layout);

			//Add Vertex Buffer to Vertex Array
			s_Data.quadData.VertexArray->AddVertexBuffer(s_Data.quadData.VertexBuffer);

			//Add Index Buffer to Vertex Array
			s_Data.quadData.VertexArray->AddIndexBuffer(indexBuffer);
			s_Data.quadData.VertexBufferBase = new QuadVertex[s_Data.MaxVertices];

			// Texture Creation
			uint32_t textureData = 0xFFFFFFFF;
			s_Data.spriteData.EmptyTexture = Texture2D::Create(Texture::Config{}, Buffer(&textureData, sizeof(uint32_t)));

			int32_t samplers[s_Data.spriteData.MaxTextureSlots];
			for (uint32_t i = 0; i < s_Data.spriteData.MaxTextureSlots; i++)
			{
				samplers[i] = i;
			}

			s_Data.quadData.Shader = Shader::Create("assets/shaders/Renderer2D_Sprite.glsl");
			s_Data.quadData.Shader->SetIntArray("u_Textures", samplers, s_Data.spriteData.MaxTextureSlots);

			s_Data.spriteData.TextureSlots[0] = s_Data.spriteData.EmptyTexture;

			s_Data.quadData.Vertices[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
			s_Data.quadData.Vertices[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
			s_Data.quadData.Vertices[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
			s_Data.quadData.Vertices[3] = { -0.5f, 0.5f, 0.0f, 1.0f };
		}

		// Circle Rendering Setup
		{
			GE_PROFILE_SCOPE("Renderer2D - Init() : Circle Rendering Setup");

			//Creates Vertex Array
			s_Data.circleData.VertexArray = VertexArray::Create();

			s_Data.circleData.VertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(CircleVertex));
			//Sets up Layout using Vertex Buffer
			VertexBuffer::Layout layout =
			{
				{ GE::Shader::DataType::Float3, "a_GlobalPosition"	},
				{ GE::Shader::DataType::Float3, "a_LocalPosition"	},
				{ GE::Shader::DataType::Float4, "a_Color"			},
				{ GE::Shader::DataType::Float, "a_Radius"			},
				{ GE::Shader::DataType::Float, "a_Thickness"		},
				{ GE::Shader::DataType::Float, "a_Fade"				},
				{ GE::Shader::DataType::Int,   "a_EntityID"			}
			};
			s_Data.circleData.VertexBuffer->SetLayout(layout);

			//Add Vertex Buffer to Vertex Array
			s_Data.circleData.VertexArray->AddVertexBuffer(s_Data.circleData.VertexBuffer);
			//Add Index Buffer to Vertex Array
			s_Data.circleData.VertexArray->AddIndexBuffer(indexBuffer);
			s_Data.circleData.VertexBufferBase = new CircleVertex[s_Data.MaxVertices];

			s_Data.circleData.Shader = Shader::Create("assets/shaders/Renderer2D_Circle.glsl");
		}
		
		// Line Rendering Setup
		{
			GE_PROFILE_SCOPE("Renderer2D - Init() : Line Rendering Setup");

			//Creates Vertex Array
			s_Data.lineData.VertexArray = VertexArray::Create();

			s_Data.lineData.VertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(LineVertex));
			//Sets up Layout using Vertex Buffer
			VertexBuffer::Layout layout =
			{
				{ GE::Shader::DataType::Float3, "a_Position"	},
				{ GE::Shader::DataType::Float4, "a_Color"		},
				{ GE::Shader::DataType::Int, "a_EntityID"		}
			};
			s_Data.lineData.VertexBuffer->SetLayout(layout);

			//Add Vertex Buffer to Vertex Array
			s_Data.lineData.VertexArray->AddVertexBuffer(s_Data.lineData.VertexBuffer);
			
			s_Data.lineData.VertexBufferBase = new LineVertex[s_Data.MaxVertices];

			s_Data.lineData.Shader = Shader::Create("assets/shaders/Renderer2D_Line.glsl");
		}

		// Text Rendering Setup
		{
			GE_PROFILE_SCOPE("Renderer2D - Init() : Text Rendering Setup");

			//Creates Vertex Array
			s_Data.textData.VertexArray = VertexArray::Create();

			s_Data.textData.VertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(TextVertex));
			//Sets up Layout using Vertex Buffer
			VertexBuffer::Layout layout =
			{
				{ GE::Shader::DataType::Float3,	"a_Position"	 },
				{ GE::Shader::DataType::Float4,	"a_TextColor"	 },
				{ GE::Shader::DataType::Float4,	"a_BGColor"		 },
				{ GE::Shader::DataType::Float2,	"a_TextureCoord" },
				{ GE::Shader::DataType::Int,	"a_EntityID"	 }
			};
			s_Data.textData.VertexBuffer->SetLayout(layout);

			//Add Vertex Buffer to Vertex Array
			s_Data.textData.VertexArray->AddVertexBuffer(s_Data.textData.VertexBuffer);

			//Add Index Buffer to Vertex Array
			s_Data.textData.VertexArray->AddIndexBuffer(indexBuffer);
			s_Data.textData.VertexBufferBase = new TextVertex[s_Data.MaxVertices];

			// Shader Creation
			s_Data.textData.Shader = Shader::Create("assets/shaders/Renderer2D_Text.glsl");
		}
	}

	void Renderer2D::ShutDown()
	{
		GE_PROFILE_FUNCTION();

		delete[] s_Data.quadData.VertexBufferBase;
	
		delete[] s_Data.circleData.VertexBufferBase;
	
		delete[] s_Data.lineData.VertexBufferBase;

		delete[] s_Data.textData.VertexBufferBase;
	
	}

	void Renderer2D::Start(const Camera& camera)
	{
		GE_PROFILE_FUNCTION();

		const glm::mat4& viewProjection = camera.GetViewProjection();
		
		s_Data.quadData.Shader->SetMat4("u_ViewProjection", viewProjection);
		s_Data.circleData.Shader->SetMat4("u_ViewProjection", viewProjection);
		s_Data.lineData.Shader->SetMat4("u_ViewProjection", viewProjection);
		s_Data.textData.Shader->SetMat4("u_ViewProjection", viewProjection);
		
		ResetQuadData();
		ResetCircleData();
		ResetLineData();
		ResetTextData();
	}

	void Renderer2D::End()
	{
		GE_PROFILE_FUNCTION();

		Flush();
	}

	void Renderer2D::Flush()
	{
		if (s_Data.quadData.IndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.quadData.VertexBufferPtr - (uint8_t*)s_Data.quadData.VertexBufferBase);
			s_Data.quadData.VertexBuffer->SetData(s_Data.quadData.VertexBufferBase, dataSize);

			// Bind Textures
			for (uint32_t i = 0; i < s_Data.spriteData.TextureSlotIndex; i++)
				s_Data.spriteData.TextureSlots[i]->Bind(i);

			// Draw Quad Indices
			s_Data.quadData.Shader->Bind();
			RenderCommand::DrawIndices(s_Data.quadData.VertexArray, s_Data.quadData.IndexCount);
			s_Data.quadData.Shader->Unbind();
			s_Data.Stats.DrawCalls++;
		}

		if (s_Data.circleData.IndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.circleData.VertexBufferPtr - (uint8_t*)s_Data.circleData.VertexBufferBase);
			s_Data.circleData.VertexBuffer->SetData(s_Data.circleData.VertexBufferBase, dataSize);
			
			// Draw Circle Indices
			s_Data.circleData.Shader->Bind();
			RenderCommand::DrawIndices(s_Data.circleData.VertexArray, s_Data.circleData.IndexCount);
			s_Data.circleData.Shader->Unbind();
			s_Data.Stats.DrawCalls++;

		}
		
		if(s_Data.lineData.IndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.lineData.VertexBufferPtr - (uint8_t*)s_Data.lineData.VertexBufferBase);
			s_Data.lineData.VertexBuffer->SetData(s_Data.lineData.VertexBufferBase, dataSize);

			// Draw Line Indices
			s_Data.lineData.Shader->Bind();
			RenderCommand::SetLineWidth(s_Data.lineData.Width);
			RenderCommand::DrawLines(s_Data.lineData.VertexArray, s_Data.lineData.IndexCount);
			s_Data.lineData.Shader->Unbind();
			s_Data.Stats.DrawCalls++;

		}
		
		if (s_Data.textData.IndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.textData.VertexBufferPtr - (uint8_t*)s_Data.textData.VertexBufferBase);
			s_Data.textData.VertexBuffer->SetData(s_Data.textData.VertexBufferBase, dataSize);

			// Bind Text Texture
			s_Data.textData.AtlasTexture->Bind();

			// Draw Text Indices
			s_Data.textData.Shader->Bind();
			RenderCommand::DrawIndices(s_Data.textData.VertexArray, s_Data.textData.IndexCount);
			s_Data.textData.Shader->Unbind();
			s_Data.Stats.DrawCalls++;

		}

		ResetQuadData();
		ResetCircleData();
		ResetLineData();
		ResetTextData();
	}

#pragma region Sprite/Quad
	void Renderer2D::SetQuadData(const glm::mat4& transform, const uint32_t& textureIndex,
		const glm::vec2 textureCoords[4], const float& tilingFactor, const glm::vec4& color, const int entityID)
	{
		constexpr size_t quadVertexCount = 4;

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.quadData.VertexBufferPtr->Position = transform * s_Data.quadData.Vertices[i];
			s_Data.quadData.VertexBufferPtr->Color = color;
			s_Data.quadData.VertexBufferPtr->TextureCoord = textureCoords[i];
			s_Data.quadData.VertexBufferPtr->TextureIndex = textureIndex;
			s_Data.quadData.VertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.quadData.VertexBufferPtr->EntityID = entityID;
			s_Data.quadData.VertexBufferPtr++;
		}

		s_Data.quadData.IndexCount += 6;
		s_Data.Stats.SpawnCount++;
	}

	void Renderer2D::ResetQuadData()
	{
		s_Data.quadData.IndexCount = 0;
		s_Data.quadData.VertexBufferPtr = s_Data.quadData.VertexBufferBase;

		s_Data.spriteData.TextureSlotIndex = 1;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.quadData.IndexCount >= s_Data.MaxIndices)
			Flush();

		const glm::vec2 textureCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		const uint32_t textureIndex = 0; // White Texture
		const float tilingFactor = 1.0f;

		SetQuadData(transform, textureIndex, textureCoords, tilingFactor, color, entityID);

	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, Ref<Texture2D> texture,
		const float& tilingFactor, const glm::vec4& color, const int entityID)
	{
		if (s_Data.quadData.IndexCount >= s_Data.MaxIndices)
			Flush();

		const glm::vec2 textureCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		uint32_t textureIndex = 0;

		for (uint32_t i = 1; i < s_Data.spriteData.TextureSlotIndex; i++)
		{
			if (*s_Data.spriteData.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = i;
				break;
			}
		}

		if (textureIndex == 0)
		{
			if (s_Data.spriteData.TextureSlotIndex >= s_Data.spriteData.MaxTextureSlots)
				Flush();

			textureIndex = s_Data.spriteData.TextureSlotIndex;
			s_Data.spriteData.TextureSlots.at(textureIndex) = texture;
			s_Data.spriteData.TextureSlotIndex++;
		}

		SetQuadData(transform, textureIndex, textureCoords, tilingFactor, color, entityID);

	}

	void Renderer2D::FillQuadColor(const glm::vec3& position, const glm::vec2& size,
		const float rotation, const glm::vec4& color, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(s_Data.IdentityMat4, position)
			* glm::rotate(s_Data.IdentityMat4, glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(s_Data.IdentityMat4, { size.x, size.y, 1.0f });

		DrawQuad(transform, color, entityID);

	}

	void Renderer2D::FillQuadTexture(const glm::vec3& position, const glm::vec2& size,
		const float rotation, Ref<Texture2D> texture, const float& tilingFactor, const glm::vec4& tintColor, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(s_Data.IdentityMat4, position)
			* glm::rotate(s_Data.IdentityMat4, glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(s_Data.IdentityMat4, { size.x, size.y, 1.0f });

		DrawQuad(transform, texture, tilingFactor, tintColor, entityID);
	}

	void Renderer2D::FillQuadSubTexture(const glm::vec3& position, const glm::vec2& size, const float rotation,
		Ref<SubTexture2D> subTexture, const float& tilingFactor, const glm::vec4& tintColor, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.quadData.IndexCount >= s_Data.MaxIndices)
			Flush();

		glm::mat4 transform = glm::translate(s_Data.IdentityMat4, position)
			* glm::rotate(s_Data.IdentityMat4, glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(s_Data.IdentityMat4, { size.x, size.y, 1.0f });

		const glm::vec2* textureCoords = subTexture->GetTextureCoords();

		uint32_t textureIndex = 0;

		for (uint32_t i = 1; i < s_Data.spriteData.TextureSlotIndex; i++)
		{
			if (*s_Data.spriteData.TextureSlots[i].get() == *subTexture->GetTexture().get())
			{
				textureIndex = i;
				break;
			}
		}

		if (textureIndex == 0)
		{
			if (s_Data.spriteData.TextureSlotIndex >= s_Data.spriteData.MaxTextureSlots)
				Flush();

			if (s_Data.spriteData.TextureSlots.at(s_Data.spriteData.TextureSlotIndex) != nullptr)
			{
				s_Data.spriteData.TextureSlotIndex++;
			}
			textureIndex = s_Data.spriteData.TextureSlotIndex;
			s_Data.spriteData.TextureSlots.at(textureIndex) = subTexture->GetTexture();
			s_Data.spriteData.TextureSlotIndex++;
		}

		SetQuadData(transform, textureIndex, textureCoords, tilingFactor, tintColor, entityID);
	}

	void Renderer2D::DrawSprite(const glm::mat4& transform, const SpriteRendererComponent& component, int entityID)
	{
		GE_PROFILE_FUNCTION();

		if (component.AssetHandle)
		{
			Ref<Texture2D> texture = Project::GetAsset<Texture2D>(component.AssetHandle);
			DrawQuad(transform, texture, component.TilingFactor, component.Color, entityID);
		}
		else
			DrawQuad(transform, component.Color, entityID);
	
	}

#pragma endregion

#pragma region Circle
	void Renderer2D::SetCircleData(const glm::mat4& transform, const glm::vec4& color, const float& radius, const float& thickness, const float& fade, const int entityID)
	{
		constexpr size_t circleVertexCount = 4;

		for (size_t i = 0; i < circleVertexCount; i++)
		{
			s_Data.circleData.VertexBufferPtr->Position = transform * s_Data.quadData.Vertices[i];
			s_Data.circleData.VertexBufferPtr->LocalPosition = s_Data.quadData.Vertices[i] * 2.0f;
			s_Data.circleData.VertexBufferPtr->Color = color;
			s_Data.circleData.VertexBufferPtr->Radius = radius;
			s_Data.circleData.VertexBufferPtr->Thickness = thickness;
			s_Data.circleData.VertexBufferPtr->Fade = fade;
			s_Data.circleData.VertexBufferPtr->EntityID = entityID;
			s_Data.circleData.VertexBufferPtr++;
		}

		s_Data.circleData.IndexCount += 6;
		s_Data.Stats.SpawnCount++;
	}

	void Renderer2D::ResetCircleData()
	{
		s_Data.circleData.IndexCount = 0;
		s_Data.circleData.VertexBufferPtr = s_Data.circleData.VertexBufferBase;

	}

	void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, const float& radius, const float& thickness, const float& fade, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.circleData.IndexCount >= s_Data.MaxIndices)
			Flush();
			
		SetCircleData(transform, color, radius, thickness, fade, entityID);
	}

	void Renderer2D::DrawCircle(const glm::mat4& transform, const CircleRendererComponent& component, const int entityID)
	{
		DrawCircle(transform, component.Color, component.Radius, component.Thickness, component.Fade, entityID);
	}

#pragma endregion

#pragma region Line

	void Renderer2D::SetLineData(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color, const int entityID)
	{
		s_Data.lineData.VertexBufferPtr->Position = initialPosition;
		s_Data.lineData.VertexBufferPtr->Color = color;
		s_Data.lineData.VertexBufferPtr->EntityID = entityID;
		s_Data.lineData.VertexBufferPtr++;

		s_Data.lineData.VertexBufferPtr->Position = finalPosition;
		s_Data.lineData.VertexBufferPtr->Color = color;
		s_Data.lineData.VertexBufferPtr->EntityID = entityID;
		s_Data.lineData.VertexBufferPtr++;

		s_Data.lineData.IndexCount += 2;
		s_Data.Stats.SpawnCount++;
	}

	void Renderer2D::ResetLineData()
	{
		s_Data.lineData.IndexCount = 0;
		s_Data.lineData.VertexBufferPtr = s_Data.lineData.VertexBufferBase;
	}

	void Renderer2D::FillLine(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color, const int entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.lineData.IndexCount >= s_Data.MaxIndices)
			Flush();

		SetLineData(initialPosition, finalPosition, color, entityID);
	}

	void Renderer2D::DrawLine(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color, const int entityID)
	{
		FillLine(initialPosition, finalPosition, color, entityID);
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
			lineVertices[i] = transform * s_Data.quadData.Vertices[i];
		}

		FillLine(lineVertices[0], lineVertices[1], color, entityID);
		FillLine(lineVertices[1], lineVertices[2], color, entityID);
		FillLine(lineVertices[2], lineVertices[3], color, entityID);
		FillLine(lineVertices[3], lineVertices[0], color, entityID);
	}

#pragma endregion

#pragma region Text
	void Renderer2D::SetTextData(const glm::mat4& transform, const glm::vec2& minTextureCoord, const glm::vec2& maxTextureCoord,
		const glm::vec2& minQuadPlane, const glm::vec2& maxQuadPlane, Ref<Font> font,
		const glm::vec4& textColor, const glm::vec4& bgColor, const int entityID)
	{
		s_Data.textData.VertexBufferPtr->Position = transform * glm::vec4(minQuadPlane, 0.0f, 1.0f);
		s_Data.textData.VertexBufferPtr->TextColor = textColor;
		s_Data.textData.VertexBufferPtr->BGColor = bgColor;
		s_Data.textData.VertexBufferPtr->TextureCoord = minTextureCoord;
		s_Data.textData.VertexBufferPtr->EntityID = entityID;
		s_Data.textData.VertexBufferPtr++;
			   
		s_Data.textData.VertexBufferPtr->Position = transform * glm::vec4(minQuadPlane.x, maxQuadPlane.y, 0.0f, 1.0f);
		s_Data.textData.VertexBufferPtr->TextColor = textColor;
		s_Data.textData.VertexBufferPtr->BGColor = bgColor;
		s_Data.textData.VertexBufferPtr->TextureCoord = { minTextureCoord.x, maxTextureCoord.y };
		s_Data.textData.VertexBufferPtr->EntityID = entityID;
		s_Data.textData.VertexBufferPtr++;
			   
		s_Data.textData.VertexBufferPtr->Position = transform * glm::vec4(maxQuadPlane, 0.0f, 1.0f);
		s_Data.textData.VertexBufferPtr->TextColor = textColor;
		s_Data.textData.VertexBufferPtr->BGColor = bgColor;
		s_Data.textData.VertexBufferPtr->TextureCoord = maxTextureCoord;
		s_Data.textData.VertexBufferPtr->EntityID = entityID;
		s_Data.textData.VertexBufferPtr++;
			   
		s_Data.textData.VertexBufferPtr->Position = transform * glm::vec4(maxQuadPlane.x, minQuadPlane.y, 0.0f, 1.0f);
		s_Data.textData.VertexBufferPtr->TextColor = textColor;
		s_Data.textData.VertexBufferPtr->BGColor = bgColor;
		s_Data.textData.VertexBufferPtr->TextureCoord = { maxTextureCoord.x, minTextureCoord.y };
		s_Data.textData.VertexBufferPtr->EntityID = entityID;
		s_Data.textData.VertexBufferPtr++;
			  
		s_Data.textData.IndexCount += 6;
		s_Data.Stats.SpawnCount++;
	}

	void Renderer2D::ResetTextData()
	{
		s_Data.textData.IndexCount = 0;
		s_Data.textData.VertexBufferPtr = s_Data.textData.VertexBufferBase;
	}

	void Renderer2D::DrawString(const glm::mat4& transform, const std::string& text, Ref<Font> font, const glm::vec4& textColor, const glm::vec4& bgColor, const float& kerningOffset, const float& lineHeightOffset, const int entityID)
	{
		if (s_Data.textData.IndexCount >= s_Data.MaxIndices)
			Flush();

		const auto& fontGeometry = font->GetMSDFData()->FontGeometry;
		const auto& metrics = fontGeometry.getMetrics();

		s_Data.textData.AtlasTexture = font->GetAtlasTexture();

		double x = 0.0;
		double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
		double y = 0.0;

		for (size_t i = 0; i < text.size(); i++)
		{
			char character = text[i];
			if (character == '\r')
				continue;

			if (character == '\n')
			{
				x = 0;
				y -= fsScale * metrics.lineHeight + lineHeightOffset;
				continue;
			}
			auto glyph = fontGeometry.getGlyph(character);
			if (character == '\t')
				glyph = fontGeometry.getGlyph(' ');
			if (!glyph)
				glyph = fontGeometry.getGlyph('?');
			if (!glyph)
			{
				GE_CORE_ERROR("Could not get Text Glyph.");
				return;
			}

			double al, ab, ar, at;
			glyph->getQuadAtlasBounds(al, ab, ar, at);
			glm::vec2 texCoordMin((float)al, (float)ab);
			glm::vec2 texCoordMax((float)ar, (float)at);

			double pl, pb, pr, pt;
			glyph->getQuadPlaneBounds(pl, pb, pr, pt);
			glm::vec2 quadMin((float)pl, (float)pb);
			glm::vec2 quadMax((float)pr, (float)pt);

			quadMin *= fsScale;
			quadMax *= fsScale;
			quadMin += glm::vec2(x, y);
			quadMax += glm::vec2(x, y);

			float texelWidth = 1.0f / s_Data.textData.AtlasTexture->GetWidth();
			float texelHeight = 1.0f / s_Data.textData.AtlasTexture->GetHeight();
			texCoordMin *= glm::vec2(texelWidth, texelHeight);
			texCoordMax *= glm::vec2(texelWidth, texelHeight);

			SetTextData(transform, texCoordMin, texCoordMax, quadMin, quadMax, font, textColor, bgColor, entityID);

			if (i < text.size() - 1)
			{
				double advance = glyph->getAdvance();
				char nextCharacter = text[i + 1];
				fontGeometry.getAdvance(advance, character, nextCharacter);

				x += fsScale * advance + kerningOffset;
			}
		}
		
	}

	void Renderer2D::DrawString(const glm::mat4& transform, const TextRendererComponent& component, int entityID)
	{
		Ref<Font> font = Project::GetAsset<Font>(component.AssetHandle);
		if (font)
		{
			DrawString(transform, component.Text, font,
				component.TextColor, component.BGColor,
				component.KerningOffset, component.LineHeightOffset,
				entityID);
		}
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
