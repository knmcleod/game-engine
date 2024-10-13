#include "GE/GEpch.h"

#include "OpenGLRenderer.h"

#include "GE/Project/Project.h"

#include "GE/Rendering/Camera/Camera.h"
#include "GE/Rendering/Shader/Shader.h"

#include <glad/glad.h>

namespace GE
{
	OpenGLRenderer::API OpenGLRenderer::Data::RenderAPI = OpenGLRenderer::API::OpenGL;
	OpenGLRenderer::Data OpenGLRenderer::s_Data = OpenGLRenderer::Data();

	OpenGLRenderer::OpenGLRenderer()
	{
		GE_PROFILE_FUNCTION();

		glEnable(GL_DEPTH);
		glEnable(GL_BLEND);

		glBlendFunc(GL_SRC0_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}

	OpenGLRenderer::~OpenGLRenderer()
	{
		ClearData();
	}

#pragma region Renderer overrides

	void OpenGLRenderer::CreateData()
	{
		//Creates Index Buffer - Can be used for both Quad & Circle
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(s_Data.MaxIndices);

		// Quad/Sprite Rendering Setup
		{
			GE_PROFILE_SCOPE("OpenGLRenderer - Init() : Quad/Sprite Rendering Setup");

			// TODO: Let vertexArray control buffers?
			//Creates Vertex Array
			s_Data.quadData.VertexArray = VertexArray::Create();

			s_Data.quadData.VertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
			//Sets up Layout using Vertex Buffer
			VertexBuffer::Layout layout =
			{
				{ GE::Math::Type::Float3,	"a_Position"	 },
				{ GE::Math::Type::Float4,	"a_Color"		 },
				{ GE::Math::Type::Float2,	"a_TextureCoord" },
				{ GE::Math::Type::Int,	"a_TextureIndex" },
				{ GE::Math::Type::Float,	"a_TilingFactor" },
				{ GE::Math::Type::Int,	"a_EntityID"	 }
			};
			s_Data.quadData.VertexBuffer->SetLayout(layout);

			//Add Vertex Buffer to Vertex Array
			s_Data.quadData.VertexArray->AddVertexBuffer(s_Data.quadData.VertexBuffer);

			//Add Index Buffer to Vertex Array
			s_Data.quadData.VertexArray->AddIndexBuffer(indexBuffer);
			s_Data.quadData.VertexBufferBase = new QuadVertex[s_Data.MaxVertices];

			// Texture Creation
			uint32_t textureData = 0xFFFFFFFF;
			s_Data.spriteData.EmptyTexture = Texture2D::Create(0, Texture::Config(), Buffer(&textureData, sizeof(uint32_t)));

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
			GE_PROFILE_SCOPE("OpenGLRenderer - Init() : Circle Rendering Setup");

			//Creates Vertex Array
			s_Data.circleData.VertexArray = VertexArray::Create();

			s_Data.circleData.VertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(CircleVertex));
			//Sets up Layout using Vertex Buffer
			VertexBuffer::Layout layout =
			{
				{ GE::Math::Type::Float3, "a_GlobalPosition"	},
				{ GE::Math::Type::Float3, "a_LocalPosition"	},
				{ GE::Math::Type::Float4, "a_Color"			},
				{ GE::Math::Type::Float, "a_Radius"			},
				{ GE::Math::Type::Float, "a_Thickness"		},
				{ GE::Math::Type::Float, "a_Fade"				},
				{ GE::Math::Type::Int,   "a_EntityID"			}
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
			GE_PROFILE_SCOPE("OpenGLRenderer - Init() : Line Rendering Setup");

			//Creates Vertex Array
			s_Data.lineData.VertexArray = VertexArray::Create();

			s_Data.lineData.VertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(LineVertex));
			//Sets up Layout using Vertex Buffer
			VertexBuffer::Layout layout =
			{
				{ GE::Math::Type::Float3, "a_Position"	},
				{ GE::Math::Type::Float4, "a_Color"		},
				{ GE::Math::Type::Int, "a_EntityID"		}
			};
			s_Data.lineData.VertexBuffer->SetLayout(layout);

			//Add Vertex Buffer to Vertex Array
			s_Data.lineData.VertexArray->AddVertexBuffer(s_Data.lineData.VertexBuffer);

			s_Data.lineData.VertexBufferBase = new LineVertex[s_Data.MaxVertices];

			s_Data.lineData.Shader = Shader::Create("assets/shaders/Renderer2D_Line.glsl");
		}

		// Text Rendering Setup
		{
			GE_PROFILE_SCOPE("OpenGLRenderer - Init() : Text Rendering Setup");

			//Creates Vertex Array
			s_Data.textData.VertexArray = VertexArray::Create();

			s_Data.textData.VertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(TextVertex));
			//Sets up Layout using Vertex Buffer
			VertexBuffer::Layout layout =
			{
				{ GE::Math::Type::Float3,	"a_Position"	 },
				{ GE::Math::Type::Float4,	"a_TextColor"	 },
				{ GE::Math::Type::Float4,	"a_BGColor"		 },
				{ GE::Math::Type::Float2,	"a_TextureCoord" },
				{ GE::Math::Type::Int,	"a_EntityID"	 }
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

	void OpenGLRenderer::ClearData()
	{
		GE_PROFILE_FUNCTION();

		ClearBuffers();

		delete[] s_Data.quadData.VertexBufferBase;

		delete[] s_Data.circleData.VertexBufferBase;

		delete[] s_Data.lineData.VertexBufferBase;

		delete[] s_Data.textData.VertexBufferBase;

	}

	void OpenGLRenderer::ClearBuffers()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void OpenGLRenderer::Start(const Camera& camera)
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

	void OpenGLRenderer::End()
	{
		GE_PROFILE_FUNCTION();

		Flush();
	}

	void OpenGLRenderer::Flush()
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
			DrawIndices(s_Data.quadData.VertexArray, s_Data.quadData.IndexCount);
			s_Data.quadData.Shader->Unbind();
			s_Data.Stats.DrawCalls++;
		}

		if (s_Data.circleData.IndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.circleData.VertexBufferPtr - (uint8_t*)s_Data.circleData.VertexBufferBase);
			s_Data.circleData.VertexBuffer->SetData(s_Data.circleData.VertexBufferBase, dataSize);

			// Draw Circle Indices
			s_Data.circleData.Shader->Bind();
			DrawIndices(s_Data.circleData.VertexArray, s_Data.circleData.IndexCount);
			s_Data.circleData.Shader->Unbind();
			s_Data.Stats.DrawCalls++;

		}

		if (s_Data.lineData.IndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.lineData.VertexBufferPtr - (uint8_t*)s_Data.lineData.VertexBufferBase);
			s_Data.lineData.VertexBuffer->SetData(s_Data.lineData.VertexBufferBase, dataSize);

			// Draw Line Indices
			s_Data.lineData.Shader->Bind();
			SetLineWidth(s_Data.lineData.Width);
			DrawLines(s_Data.lineData.VertexArray, s_Data.lineData.IndexCount);
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
			DrawIndices(s_Data.textData.VertexArray, s_Data.textData.IndexCount);
			s_Data.textData.Shader->Unbind();
			s_Data.Stats.DrawCalls++;

		}

		ResetQuadData();
		ResetCircleData();
		ResetLineData();
		ResetTextData();
	}

	void OpenGLRenderer::SetViewport(uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height)
	{
		glViewport(xOffset, yOffset, width, height);
	}

	void OpenGLRenderer::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRenderer::SetLineWidth(float thickness)
	{
		s_Data.lineData.Width = thickness;
		glLineWidth(thickness);
	}

	const float& OpenGLRenderer::GetLineWidth()
	{
		return s_Data.lineData.Width;
	}

	void OpenGLRenderer::DrawIndices(Ref<VertexArray> vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
		vertexArray->Unbind();

	}

	void OpenGLRenderer::DrawLines(Ref<VertexArray> vertexArray, uint32_t vertexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, vertexCount);
		vertexArray->Unbind();
	}

#pragma region Statistics
	const OpenGLRenderer::Statistics& OpenGLRenderer::GetStats()
	{
		return s_Data.Stats;
	}

	void OpenGLRenderer::ClearStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Renderer::Statistics));
	}
#pragma endregion

#pragma region Sprite/Quad
	void OpenGLRenderer::SetQuadData(const glm::mat4& transform, const uint32_t& textureIndex,
		const glm::vec2 textureCoords[4], const float& tilingFactor, const glm::vec4& color, const uint32_t& entityID)
	{
		if (transform == glm::mat4())
			GE_CORE_WARN("");
		constexpr size_t quadVertexCount = 4;

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.quadData.VertexBufferPtr->Position = transform * s_Data.quadData.Vertices[i];
			s_Data.quadData.VertexBufferPtr->Color = color;
			s_Data.quadData.VertexBufferPtr->TextureCoord = textureCoords[i];
			s_Data.quadData.VertexBufferPtr->TextureIndex = textureIndex;
			s_Data.quadData.VertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.quadData.VertexBufferPtr->EntityID = (uint64_t)entityID;
			s_Data.quadData.VertexBufferPtr++;
		}

		s_Data.quadData.IndexCount += 6;
		s_Data.Stats.SpawnCount++;
	}

	void OpenGLRenderer::ResetQuadData()
	{
		s_Data.quadData.IndexCount = 0;
		s_Data.quadData.VertexBufferPtr = s_Data.quadData.VertexBufferBase;

		s_Data.spriteData.TextureSlotIndex = 1;
	}

	void OpenGLRenderer::DrawQuadColor(const glm::mat4& transform, const glm::vec4& color, const uint32_t& entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.quadData.IndexCount >= s_Data.MaxIndices)
			Flush();

		const glm::vec2 textureCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		const uint32_t textureIndex = 0; // White Texture
		const float tilingFactor = 1.0f;

		SetQuadData(transform, textureIndex, textureCoords, tilingFactor, color, entityID);
	}

	void OpenGLRenderer::DrawQuadTexture(const glm::mat4& transform, Ref<Texture2D> texture, const float& tilingFactor, 
		const glm::vec4& color, const uint32_t& entityID)
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

	void OpenGLRenderer::DrawQuadSubTexture(const glm::mat4& transform, Ref<SubTexture2D> subTexture, 
		const float& tilingFactor, const glm::vec4& tintColor, const uint32_t& entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.quadData.IndexCount >= s_Data.MaxIndices)
			Flush();

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

	void OpenGLRenderer::DrawSprite(const glm::mat4& transform, const SpriteRendererComponent& component, const uint32_t& entityID)
	{
		GE_PROFILE_FUNCTION();

		if (component.AssetHandle)
		{
			Ref<Texture2D> texture = Project::GetAsset<Texture2D>(component.AssetHandle);
			DrawQuadTexture(transform, texture, component.TilingFactor, component.Color, entityID);
		}
		else
			DrawQuadColor(transform, component.Color, entityID);

	}

#pragma endregion

#pragma region Circle
	void OpenGLRenderer::SetCircleData(const glm::mat4& transform, const glm::vec4& color, const float& radius, 
		const float& thickness, const float& fade, const uint32_t& entityID)
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
			s_Data.circleData.VertexBufferPtr->EntityID = (uint64_t)entityID;
			s_Data.circleData.VertexBufferPtr++;
		}

		s_Data.circleData.IndexCount += 6;
		s_Data.Stats.SpawnCount++;
	}

	void OpenGLRenderer::ResetCircleData()
	{
		s_Data.circleData.IndexCount = 0;
		s_Data.circleData.VertexBufferPtr = s_Data.circleData.VertexBufferBase;

	}

	void OpenGLRenderer::DrawCircle(const glm::mat4& transform, const glm::vec4& color, const float& radius, 
		const float& thickness, const float& fade, const uint32_t& entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.circleData.IndexCount >= s_Data.MaxIndices)
			Flush();

		SetCircleData(transform, color, radius, thickness, fade, entityID);
	}

	void OpenGLRenderer::DrawCircle(const glm::mat4& transform, const CircleRendererComponent& component, const uint32_t& entityID)
	{
		DrawCircle(transform, component.Color, component.Radius, component.Thickness, component.Fade, entityID);
	}

#pragma endregion

#pragma region Line

	void OpenGLRenderer::SetLineData(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color, 
		const uint32_t& entityID)
	{
		s_Data.lineData.VertexBufferPtr->Position = initialPosition;
		s_Data.lineData.VertexBufferPtr->Color = color;
		s_Data.lineData.VertexBufferPtr->EntityID = (uint64_t)entityID;
		s_Data.lineData.VertexBufferPtr++;

		s_Data.lineData.VertexBufferPtr->Position = finalPosition;
		s_Data.lineData.VertexBufferPtr->Color = color;
		s_Data.lineData.VertexBufferPtr->EntityID = (uint64_t)entityID;
		s_Data.lineData.VertexBufferPtr++;

		s_Data.lineData.IndexCount += 2;
		s_Data.Stats.SpawnCount++;
	}

	void OpenGLRenderer::ResetLineData()
	{
		s_Data.lineData.IndexCount = 0;
		s_Data.lineData.VertexBufferPtr = s_Data.lineData.VertexBufferBase;
	}

	void OpenGLRenderer::DrawLine(const glm::vec3& initialPosition, const glm::vec3& finalPosition, const glm::vec4& color, const uint32_t& entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.lineData.IndexCount >= s_Data.MaxIndices)
			Flush();

		SetLineData(initialPosition, finalPosition, color, entityID);
	}

	void OpenGLRenderer::DrawRectangle(const glm::mat4& transform, const glm::vec4& color, const uint32_t& entityID)
	{
		glm::vec3 lineVertices[4];
		for (size_t i = 0; i < 4; i++)
		{
			lineVertices[i] = transform * s_Data.quadData.Vertices[i];
		}

		DrawLine(lineVertices[0], lineVertices[1], color, entityID);
		DrawLine(lineVertices[1], lineVertices[2], color, entityID);
		DrawLine(lineVertices[2], lineVertices[3], color, entityID);
		DrawLine(lineVertices[3], lineVertices[0], color, entityID);
	}

#pragma endregion

#pragma region Text
	void OpenGLRenderer::SetTextData(const glm::mat4& transform, const glm::vec2& minTextureCoord, const glm::vec2& maxTextureCoord,
		const glm::vec2& minQuadPlane, const glm::vec2& maxQuadPlane, Ref<Font> font,
		const glm::vec4& textColor, const glm::vec4& bgColor, const uint32_t& entityID)
	{
		s_Data.textData.VertexBufferPtr->Position = transform * glm::vec4(minQuadPlane, 0.0f, 1.0f);
		s_Data.textData.VertexBufferPtr->TextColor = textColor;
		s_Data.textData.VertexBufferPtr->BGColor = bgColor;
		s_Data.textData.VertexBufferPtr->TextureCoord = minTextureCoord;
		s_Data.textData.VertexBufferPtr->EntityID = (uint64_t)entityID;
		s_Data.textData.VertexBufferPtr++;

		s_Data.textData.VertexBufferPtr->Position = transform * glm::vec4(minQuadPlane.x, maxQuadPlane.y, 0.0f, 1.0f);
		s_Data.textData.VertexBufferPtr->TextColor = textColor;
		s_Data.textData.VertexBufferPtr->BGColor = bgColor;
		s_Data.textData.VertexBufferPtr->TextureCoord = { minTextureCoord.x, maxTextureCoord.y };
		s_Data.textData.VertexBufferPtr->EntityID = (uint64_t)entityID;
		s_Data.textData.VertexBufferPtr++;

		s_Data.textData.VertexBufferPtr->Position = transform * glm::vec4(maxQuadPlane, 0.0f, 1.0f);
		s_Data.textData.VertexBufferPtr->TextColor = textColor;
		s_Data.textData.VertexBufferPtr->BGColor = bgColor;
		s_Data.textData.VertexBufferPtr->TextureCoord = maxTextureCoord;
		s_Data.textData.VertexBufferPtr->EntityID = (uint64_t)entityID;
		s_Data.textData.VertexBufferPtr++;

		s_Data.textData.VertexBufferPtr->Position = transform * glm::vec4(maxQuadPlane.x, minQuadPlane.y, 0.0f, 1.0f);
		s_Data.textData.VertexBufferPtr->TextColor = textColor;
		s_Data.textData.VertexBufferPtr->BGColor = bgColor;
		s_Data.textData.VertexBufferPtr->TextureCoord = { maxTextureCoord.x, minTextureCoord.y };
		s_Data.textData.VertexBufferPtr->EntityID = (uint64_t)entityID;
		s_Data.textData.VertexBufferPtr++;

		s_Data.textData.IndexCount += 6;
		s_Data.Stats.SpawnCount++;
	}

	void OpenGLRenderer::ResetTextData()
	{
		s_Data.textData.IndexCount = 0;
		s_Data.textData.VertexBufferPtr = s_Data.textData.VertexBufferBase;
	}

	void OpenGLRenderer::DrawString(const glm::mat4& transform, const std::string& text, Ref<Font> font, 
		const glm::vec4& textColor, const glm::vec4& bgColor, const float& kerningOffset, const float& lineHeightOffset, const uint32_t& entityID)
	{
		if (s_Data.textData.IndexCount >= s_Data.MaxIndices)
			Flush();

		const auto& fontGeometry = font->GetMSDFData()->FontGeometry;
		if (!&fontGeometry)
			return;

		Ref<Texture2D> atlas = font->GetAtlasTexture();
		if (!atlas)
			return;
		
		s_Data.textData.AtlasTexture = atlas;

		const auto& metrics = fontGeometry.getMetrics();
		double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
		double x = 0.0;
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
			if (!glyph)
			{
				if (character == '\t')
					glyph = fontGeometry.getGlyph(' ');
				if (!glyph)
					glyph = fontGeometry.getGlyph('?');
				if (!glyph)
				{
					GE_CORE_ERROR("Could not get Text Glyph.");
					return;
				}
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

	void OpenGLRenderer::DrawString(const glm::mat4& transform, const TextRendererComponent& component, const uint32_t& entityID)
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

#pragma endregion

}