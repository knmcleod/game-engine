#include "GE/GEpch.h"

#include "OpenGLRenderer.h"

#include "GE/Asset/Assets/Scene/Entity.h"
#include "GE/Asset/Assets/Scene/Scene.h"

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

			// TODO: Let vertexArray initialize buffers(?)
			//Creates Vertex Array
			s_Data.quadData.VertexArray = VertexArray::Create();

			s_Data.quadData.VertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
			//Sets up Layout using Vertex Buffer
			VertexBuffer::Layout layout =
			{
				{ GE::Math::Type::Float3,	"a_Position"	 },
				{ GE::Math::Type::Float4,	"a_Color"		 },
				{ GE::Math::Type::Float2,	"a_TextureCoord" },
				{ GE::Math::Type::Int,		"a_TextureIndex" },
				{ GE::Math::Type::Float,	"a_TilingFactor" },
				{ GE::Math::Type::Int,		"a_EntityID"	 }
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

			s_Data.quadData.PivotPoints[Pivot::Center][0] = { -0.5f, -0.5f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::Center][1] = { 0.5f, -0.5f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::Center][2] = { 0.5f, 0.5f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::Center][3] = { -0.5f, 0.5f, 0.0f, 1.0f };

			s_Data.quadData.PivotPoints[Pivot::LowerLeft][0] = { 0.0f, 0.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::LowerLeft][1] = { 1.0f, 0.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::LowerLeft][2] = { 1.0f, 1.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::LowerLeft][3] = { 0.0f, 1.0f, 0.0f, 1.0f };

			s_Data.quadData.PivotPoints[Pivot::TopLeft][0] = { 0.0f, 0.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::TopLeft][1] = { 1.0f, 0.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::TopLeft][2] = { 1.0f, -1.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::TopLeft][3] = { 0.0f, -1.0f, 0.0f, 1.0f };

			s_Data.quadData.PivotPoints[Pivot::TopRight][0] = { 0.0f, 0.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::TopRight][1] = { 0.0f, -1.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::TopRight][2] = { -1.0f, -1.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::TopRight][3] = { -1.0f, 0.0f, 0.0f, 1.0f };

			s_Data.quadData.PivotPoints[Pivot::LowerRight][0] = { 0.0f, 0.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::LowerRight][1] = { 0.0f, 1.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::LowerRight][2] = { -1.0f, 1.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::LowerRight][3] = { -1.0f, 0.0f, 0.0f, 1.0f };

			s_Data.quadData.PivotPoints[Pivot::MiddleRight][0] = { 0.0f, 0.5f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::MiddleRight][1] = { -1.0f, 0.5f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::MiddleRight][2] = { -1.0f, -0.5f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::MiddleRight][3] = { 0.0f, -0.5f, 0.0f, 1.0f };

			s_Data.quadData.PivotPoints[Pivot::TopMiddle][0] = { -0.5f, 0.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::TopMiddle][1] = { -0.5f, -1.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::TopMiddle][2] = { 0.5f, -1.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::TopMiddle][3] = { 0.5f, 0.0f, 0.0f, 1.0f };

			s_Data.quadData.PivotPoints[Pivot::MiddleLeft][0] = { 0.0f, -0.5f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::MiddleLeft][1] = { 1.0f, -0.5f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::MiddleLeft][2] = { 1.0f, 0.5f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::MiddleLeft][3] = { 0.0f, 0.5f, 0.0f, 1.0f };

			s_Data.quadData.PivotPoints[Pivot::BottomMiddle][0] = { 0.5f, 0.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::BottomMiddle][1] = { 0.5f, 1.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::BottomMiddle][2] = { -0.5f, 1.0f, 0.0f, 1.0f };
			s_Data.quadData.PivotPoints[Pivot::BottomMiddle][3] = { -0.5f, 0.0f, 0.0f, 1.0f };
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

	void OpenGLRenderer::Start(const Camera*& camera)
	{
		GE_PROFILE_FUNCTION();

		const glm::mat4 viewProjection = camera->GetViewProjection();
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

	const Renderer::QuadData::Vertices& OpenGLRenderer::GetVertices(const Pivot& pivot)
	{	
		if (s_Data.quadData.PivotPoints.find(pivot) != s_Data.quadData.PivotPoints.end())
			return s_Data.quadData.PivotPoints.at(Pivot::Center);
		return s_Data.quadData.PivotPoints.at(pivot);
	}

	glm::vec2 OpenGLRenderer::GetTextSize(UUID fontHandle, const glm::vec2& offset, 
		const float& scalar, const float& kerningOffset, const float& lineHeightOffset, const std::string& text, const glm::vec2& padding)
	{
		glm::vec2 ret = glm::vec2(1.0f);

		Ref<Font> font = Project::GetAsset<Font>(fontHandle);
		if (!font || !font->GetAtlasTexture() || !font->GetMSDFData())
			return ret;

		Ref<Texture2D> atlasTexture = font->GetAtlasTexture();
		Ref<Font::MSDFData> msdfData = font->GetMSDFData();

		const auto& metrics = msdfData->FontGeometry.getMetrics();
		double fsScale = scalar / (metrics.ascenderY - metrics.descenderY);
		double x = 0.0 + offset.x;
		double y = 0.0 + offset.y;

		float texelWidth = 1.0f / (float)atlasTexture->GetWidth();
		float texelHeight = 1.0f / (float)atlasTexture->GetHeight();

		for (size_t i = 0; i < text.size(); i++)
		{
			char character = text[i];
			if (character == '\r')
				continue;

			if (character == '\n' || character == '\0')
			{
				x = 0.0;
				y += fsScale * metrics.lineHeight + lineHeightOffset;
				continue;
			}
			if (auto glyph = msdfData->FontGeometry.getGlyph(character))
			{
				if (!glyph)
				{
					if (character == '\t')
						glyph = msdfData->FontGeometry.getGlyph(' ');
					if (!glyph)
						glyph = msdfData->FontGeometry.getGlyph('?');
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

				texCoordMin *= glm::vec2(texelWidth, texelHeight);
				texCoordMax *= glm::vec2(texelWidth, texelHeight);

				if (i < text.size() - 1)
				{
					double advance = glyph->getAdvance();
					char nextCharacter = text[i + 1];
					msdfData->FontGeometry.getAdvance(advance, character, nextCharacter);

					x += fsScale * advance + kerningOffset;
				}
			}
		}

		if (!text.empty())
		{
			if (x >= ret.x)
				ret.x = (float)x + padding.x;
			ret.y = y <= 0.0 ? 1.0f : (float)y + padding.y;
		}
		return ret;
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

	void OpenGLRenderer::SetQuadData(const glm::mat4& transform, const Pivot& pivot, const uint32_t& textureIndex,
		const glm::vec2 textureCoords[4], const float& tilingFactor, const glm::vec4& color, const uint32_t& entityID)
	{
		if (s_Data.quadData.PivotPoints.find(pivot) == s_Data.quadData.PivotPoints.end())
		{
			GE_CORE_ERROR("OpenGLRenderer::SetQuadData() Failed - Unknown Pivot");
			return;
		}
		QuadData::Vertices& vertices = s_Data.quadData.PivotPoints.at(pivot);
		for (int i = 0; i < 4; i++)
		{
			s_Data.quadData.VertexBufferPtr->Position = transform * vertices[i];
			s_Data.quadData.VertexBufferPtr->Color = color;
			s_Data.quadData.VertexBufferPtr->TextureCoord = textureCoords[i];
			s_Data.quadData.VertexBufferPtr->TextureIndex = textureIndex;
			s_Data.quadData.VertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.quadData.VertexBufferPtr->EntityID = (uint64_t)entityID;
			s_Data.quadData.VertexBufferPtr++;
		}
	
		// Update Index & Spawn Count
		s_Data.quadData.IndexCount += 6;
		s_Data.Stats.SpawnCount++;
	}

	void OpenGLRenderer::ResetQuadData()
	{
		s_Data.quadData.IndexCount = 0;
		s_Data.quadData.VertexBufferPtr = s_Data.quadData.VertexBufferBase;

		s_Data.spriteData.TextureSlotIndex = 1;
	}

	void OpenGLRenderer::DrawQuadColor(const glm::mat4& transform, const Pivot& pivot, const glm::vec4& color, const uint32_t& entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.quadData.IndexCount >= s_Data.MaxIndices)
			Flush();

		const glm::vec2 textureCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		const uint32_t textureIndex = 0; // White Texture
		const float tilingFactor = 1.0f;

		SetQuadData(transform, pivot, textureIndex, textureCoords, tilingFactor, color, entityID);
	}

	void OpenGLRenderer::DrawQuadTexture(const glm::mat4& transform, const Pivot& pivot, Ref<Texture2D> texture, const float& tilingFactor,
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

		if (textureIndex == 0 && texture)
		{
			if (s_Data.spriteData.TextureSlotIndex >= s_Data.spriteData.MaxTextureSlots)
				Flush();

			textureIndex = s_Data.spriteData.TextureSlotIndex;
			s_Data.spriteData.TextureSlots.at(textureIndex) = texture;
			s_Data.spriteData.TextureSlotIndex++;
		}

		SetQuadData(transform, pivot, textureIndex, textureCoords, tilingFactor, color, entityID);

	}

	void OpenGLRenderer::DrawQuadSubTexture(const glm::mat4& transform, const Pivot& pivot, Ref<SubTexture2D> subTexture,
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

		SetQuadData(transform, pivot, textureIndex, textureCoords, tilingFactor, tintColor, entityID);
	}

	void OpenGLRenderer::DrawSprite(const glm::mat4& transform, const Pivot& pivot, const SpriteRendererComponent& src, const uint32_t& entityID)
	{
		GE_PROFILE_FUNCTION();

		if (src.AssetHandle)
		{
			Ref<Texture2D> texture = Project::GetAsset<Texture2D>(src.AssetHandle);
			DrawQuadTexture(transform, pivot, texture, src.TilingFactor, src.Color, entityID);
		}
		else
			DrawQuadColor(transform, pivot, src.Color, entityID);

	}

#pragma endregion

#pragma region Circle
	void OpenGLRenderer::SetCircleData(const glm::mat4& transform, const Pivot& pivot, const float& radius, const float& thickness, const float& fade,
		const glm::vec4& color, const uint32_t& entityID) 
	{
		if (s_Data.quadData.PivotPoints.find(pivot) == s_Data.quadData.PivotPoints.end())
		{
			GE_CORE_ERROR("OpenGLRenderer::SetCircleData() Failed - Unknown Pivot");
			return;
		}
		QuadData::Vertices& vertices = s_Data.quadData.PivotPoints.at(pivot);
		for (int i = 0; i < 4; i++)
		{
			glm::vec3 pivotOffset = vertices[i];
			switch (pivot)
			{
			case Pivot::Center: // Default
				break;
			case Pivot::LowerLeft:
				pivotOffset -= glm::vec3(0.5f, 0.5f, 0.0f);
				break;
			case Pivot::TopLeft:
				pivotOffset += glm::vec3(-0.5f, 0.5f, 0.0f);
				break;
			case Pivot::TopRight:
				pivotOffset += glm::vec3(0.5f, 0.5f, 0.0f);
				break;
			case Pivot::LowerRight:
				pivotOffset += glm::vec3(0.5f, -0.5f, 0.0f);
				break;
			case Pivot::MiddleRight:
				pivotOffset += glm::vec3(0.5f, 0.0f, 0.0f);
				break;
			case Pivot::TopMiddle:
				pivotOffset += glm::vec3(0.0f, 0.5f, 0.0f);
				break;
			case Pivot::MiddleLeft:
				pivotOffset -= glm::vec3(0.5f, 0.0f, 0.0f);
				break;
			case Pivot::BottomMiddle:
				pivotOffset -= glm::vec3(0.0f, 0.5f, 0.0f);
				break;
			}
			s_Data.circleData.VertexBufferPtr->Position = transform * vertices[i];
			s_Data.circleData.VertexBufferPtr->LocalPosition = pivotOffset * 2.0f;
			s_Data.circleData.VertexBufferPtr->Color = color;
			s_Data.circleData.VertexBufferPtr->Radius = radius;
			s_Data.circleData.VertexBufferPtr->Thickness = thickness;
			s_Data.circleData.VertexBufferPtr->Fade = fade;
			s_Data.circleData.VertexBufferPtr->EntityID = (uint64_t)entityID;
			s_Data.circleData.VertexBufferPtr++;
		}

		// Update Index & Spawn Count
		s_Data.circleData.IndexCount += 6;
		s_Data.Stats.SpawnCount++;
	}

	void OpenGLRenderer::ResetCircleData()
	{
		s_Data.circleData.IndexCount = 0;
		s_Data.circleData.VertexBufferPtr = s_Data.circleData.VertexBufferBase;

	}

	void OpenGLRenderer::DrawCircle(const glm::mat4& transform, const Pivot& pivot, const float& radius, const float& thickness, const float& fade,
		const glm::vec4& color, const uint32_t& entityID)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.circleData.IndexCount >= s_Data.MaxIndices)
			Flush();

		SetCircleData(transform, pivot, radius, thickness, fade, color, entityID);
	}

	void OpenGLRenderer::DrawCircle(const glm::mat4& transform, const Pivot& pivot, const CircleRendererComponent& crc, const uint32_t& entityID)
	{
		DrawCircle(transform, pivot, crc.Radius, crc.Thickness, crc.Fade, crc.Color, entityID);
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

	void OpenGLRenderer::DrawRectangle(const glm::mat4& transform, const Pivot& pivot, const glm::vec4& color, const uint32_t& entityID)
	{
		if (s_Data.quadData.PivotPoints.find(pivot) == s_Data.quadData.PivotPoints.end())
		{
			GE_CORE_ERROR("OpenGLRenderer::DrawRectangle() Failed - Unknown Pivot");
			return;
		}
		QuadData::Vertices& vertices = s_Data.quadData.PivotPoints.at(pivot);
		glm::vec3 lineVertices[4] = { transform * vertices[0], transform * vertices[1], 
			transform * vertices[2], transform * vertices[3] };

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
		const glm::vec4& textColor, const glm::vec4& bgColor, const float& kerningOffset, const float& lineHeightOffset,
		const float& scalar, const glm::vec2& offset, const uint32_t& entityID)
	{
		if (!font || !font->GetMSDFData() || !font->GetAtlasTexture())
			return;

		if (s_Data.textData.IndexCount >= s_Data.MaxIndices)
			Flush();

		Ref<Texture2D> atlas = font->GetAtlasTexture();
		s_Data.textData.AtlasTexture = atlas;
		float texelWidth = 1.0f / (float)s_Data.textData.AtlasTexture->GetWidth();
		float texelHeight = 1.0f / (float)s_Data.textData.AtlasTexture->GetHeight();

		Ref<Font::MSDFData> msdfData = font->GetMSDFData();
		const auto& metrics = msdfData->FontGeometry.getMetrics();
		double fsScale = scalar / (metrics.ascenderY - metrics.descenderY);
		double x = 0.0 + offset.x;
		double y = 0.0 + offset.y;

		for (size_t i = 0; i < text.size(); i++)
		{
			char character = text[i];
			if (character == '\r')
				continue;

			if (character == '\n' || character == '\0')
			{
				x = 0;
				y -= fsScale * metrics.lineHeight + lineHeightOffset;
				continue;
			}
			auto glyph = msdfData->FontGeometry.getGlyph(character);
			if (!glyph)
			{
				if (character == '\t')
					glyph = msdfData->FontGeometry.getGlyph(' ');
				if (!glyph)
					glyph = msdfData->FontGeometry.getGlyph('?');
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

			texCoordMin *= glm::vec2(texelWidth, texelHeight);
			texCoordMax *= glm::vec2(texelWidth, texelHeight);

			SetTextData(transform, texCoordMin, texCoordMax, quadMin, quadMax, font, textColor, bgColor, entityID);

			if (i < text.size() - 1)
			{
				double advance = glyph->getAdvance();
				char nextCharacter = text[i + 1];
				msdfData->FontGeometry.getAdvance(advance, character, nextCharacter);

				x += fsScale * advance + kerningOffset;
			}
		}
	}

	void OpenGLRenderer::DrawString(const glm::mat4& transform, const TextRendererComponent& trc, const uint32_t& entityID)
	{
		Ref<Font> font = Project::GetAsset<Font>(trc.AssetHandle);
		if (font)
		{
			DrawString(transform, trc.Text, font, trc.TextColor, trc.BGColor,
				trc.KerningOffset, trc.LineHeightOffset, trc.TextScalar, trc.TextOffset,
				entityID);
		}
	}
	
#pragma endregion

#pragma region GUI

	/*
	* Used by GUIComponents with Fonts(Button, InputField)
	*/
	static glm::vec3 GetFontOffset(const Pivot& pivot, const glm::vec2& scalar)
	{
		glm::vec3 ret = glm::vec3(0.0f);
		switch (pivot)
		{
		case Pivot::Center:
			ret = glm::vec3(-0.5f * scalar.x, 0.5f * scalar.y, 0.0f);
			break;
		case Pivot::LowerLeft:
			ret = glm::vec3(0.0f, scalar.y, 0.0f);
			break;
		case Pivot::TopLeft:
			break;
		case Pivot::TopRight:
			ret = glm::vec3(-scalar.x, 0.0f, 0.0f);
			break;
		case Pivot::LowerRight:
			ret = glm::vec3(-scalar.x, scalar.y, 0.0f);
			break;
		case Pivot::MiddleRight:
			ret = glm::vec3(-scalar.x, 0.5f * scalar.y, 0.0f);
			break;
		case Pivot::TopMiddle:
			ret = glm::vec3(-0.5f * scalar.x, 0.0f, 0.0f);
			break;
		case Pivot::MiddleLeft:
			ret = glm::vec3(0.0f, 0.5f * scalar.y, 0.0f);
			break;
		case Pivot::BottomMiddle:
			ret = glm::vec3(-0.5f * scalar.x, scalar.y, 0.0f);
			break;
		default:
			GE_CORE_WARN("OpenGLRenderer::GetFontOffset(const Pivot&, const glm::vec2&) - Unknown Pivot. Returning offset(0, 0).");
			break;
		}
		return ret;
	}
	void OpenGLRenderer::DrawButton(const glm::mat4& transform, const Pivot& pivot, const GUIButtonComponent& guiBC, const GUIState& state, const uint32_t& entityID)
	{
		if (Ref<Texture2D> bgTexture = Project::GetAsset<Texture2D>(guiBC.BackgroundTextureHandle))
			DrawQuadTexture(transform, pivot, bgTexture, 1.0f, guiBC.BackgroundColor, entityID);
		else
			DrawQuadColor(transform, pivot, guiBC.BackgroundColor, entityID);

		Ref<Texture2D> currentTexture = nullptr;
		glm::vec4 currentColor = glm::vec4(1.0f);
		switch (state)
		{
		case GUIState::Disabled:
		{
			currentTexture = Project::GetAsset<Texture2D>(guiBC.DisabledTextureHandle);
			currentColor = guiBC.DisabledColor;
		}
		break;
		case GUIState::Enabled:
		{
			currentTexture = Project::GetAsset<Texture2D>(guiBC.EnabledTextureHandle);
			currentColor = guiBC.EnabledColor;
		}
		break;
		case GUIState::Hovered:
		case GUIState::Focused:
		{
			currentTexture = Project::GetAsset<Texture2D>(guiBC.HoveredTextureHandle);
			currentColor = guiBC.HoveredColor;
		}
		break;
		case GUIState::Active:
		case GUIState::Selected:
		{
			currentTexture = Project::GetAsset<Texture2D>(guiBC.SelectedTextureHandle);
			currentColor = guiBC.SelectedColor;
		}
		break;
		default:
			GE_CORE_WARN("OpenGLRenderer::DrawButton() - Unknown GUIState.");
			break;
		}

		if (currentTexture)
			DrawQuadTexture(transform, pivot, currentTexture, 1.0f, currentColor, entityID);
		else
			DrawQuadColor(transform, pivot, currentColor, entityID);

		if (Ref<Font> font = Project::GetAsset<Font>(guiBC.FontAssetHandle))
		{
			glm::vec3 offset = GetFontOffset(pivot, guiBC.TextSize);
			glm::mat4 fontTransform = glm::translate(transform, offset);

			DrawString(fontTransform, guiBC.Text, font, guiBC.TextColor, guiBC.BGColor,
				guiBC.KerningOffset, guiBC.LineHeightOffset, guiBC.TextScalar, guiBC.TextStartingOffset, entityID);
		}
		if (Ref<Texture2D> fgTexture = Project::GetAsset<Texture2D>(guiBC.ForegroundTextureHandle))
			DrawQuadTexture(transform, pivot, fgTexture, 1.0f, guiBC.ForegroundColor, entityID);
		else
			DrawQuadColor(transform, pivot, guiBC.ForegroundColor, entityID);

	}

	void OpenGLRenderer::DrawInputField(const glm::mat4& transform, const Pivot& pivot, const GUIInputFieldComponent& guiIFC, const GUIState& state, const uint32_t& entityID)
	{
		glm::mat4 bgTransform = glm::scale(transform, glm::vec3(glm::vec2(guiIFC.TextSize), 1.0f));
		if (Ref<Texture2D> texture2D = Project::GetAsset<Texture2D>(guiIFC.BackgroundTextureHandle))
			DrawQuadTexture(bgTransform, pivot, texture2D, 1.0f, guiIFC.BackgroundColor, entityID);
		else
			DrawQuadColor(bgTransform, pivot, guiIFC.BackgroundColor, entityID);

		if (Ref<Font> font = Project::GetAsset<Font>(guiIFC.FontAssetHandle))
		{
			glm::vec3 offset = GetFontOffset(pivot, guiIFC.TextSize);
			glm::mat4 fontTransform = glm::translate(transform, offset);
			DrawString(fontTransform, guiIFC.Text, font, guiIFC.TextColor, guiIFC.BGColor,
				guiIFC.KerningOffset, guiIFC.LineHeightOffset, guiIFC.TextScalar, guiIFC.TextStartingOffset, entityID);
		}
	}

	static Pivot PivotFromSliderDirection(const SliderDirection& sliderDirection)
	{
		switch (sliderDirection)
		{
		case SliderDirection::Left:
			return Pivot::MiddleRight;
		case SliderDirection::Center:
			return Pivot::Center;
		case SliderDirection::Right:
			return Pivot::MiddleLeft;
		case SliderDirection::Top:
			return Pivot::BottomMiddle;
		case SliderDirection::Middle:
			return Pivot::Center;
		case SliderDirection::Bottom:
			return Pivot::TopMiddle;
		}
		GE_CORE_WARN("OpenGLRenderer::PivotFromSliderDirection() - Unknown SliderDirection.\n\tReturning Center Pivot.");
		return Pivot::Center;
	}

	/*
	* Returns the offset of GUISliderComponent::Middleground based on the entitys pivot & the components pivot
	* 
	* @param pPivot : entity pivot
	* @param cPivot : GUISliderComponent Pivot from PivotFromSliderDirection(const SliderDirection&, const HorizontalDirection& horizontalDirection, const VerticalDirection&)
	*/
	static const glm::vec3 GetSliderOffset(const Pivot& pPivot, const Pivot& cPivot)
	{
		glm::vec3 ret = glm::vec3(0.0f);
		switch (pPivot)
		{
		case Pivot::Center:
		{
			switch (cPivot)
			{
			case Pivot::Center:
				break;
			case Pivot::LowerLeft:
				ret = glm::vec3(0.5f, 0.5f, 0.0f);
				break;
			case Pivot::TopLeft:
				ret = glm::vec3(0.5f, -0.5f, 0.0f);
				break;
			case Pivot::TopRight:
				ret = glm::vec3(-0.5f, -0.5f, 0.0f);
				break;
			case Pivot::LowerRight:
				ret = glm::vec3(-0.5f, 0.5f, 0.0f);
				break;
			case Pivot::MiddleRight:
				ret = glm::vec3(-0.5f, 0.0f, 0.0f);
				break;
			case Pivot::TopMiddle:
				ret = glm::vec3(0.0f, -0.5f, 0.0f);
				break;
			case Pivot::MiddleLeft:
				ret = glm::vec3(0.5f, 0.0f, 0.0f);
				break;
			case Pivot::BottomMiddle:
				ret = glm::vec3(0.0f, 0.5f, 0.0f);
				break;
			default:
				break;
			}
		}
		break;
		case Pivot::LowerLeft:
		{
			switch (cPivot)
			{
			case Pivot::Center:
				ret = glm::vec3(-0.5f, -0.5f, 0.0f);
				break;
			case Pivot::LowerLeft:
				break;
			case Pivot::TopLeft:
				ret = glm::vec3(0.0f, -1.0f, 0.0f);
				break;
			case Pivot::TopRight:
				ret = glm::vec3(-1.0f, -1.0f, 0.0f);
				break;
			case Pivot::LowerRight:
				ret = glm::vec3(-1.0f, 0.0f, 0.0f);
				break;
			case Pivot::MiddleRight:
				ret = glm::vec3(-1.0f, -0.5f, 0.0f);
				break;
			case Pivot::TopMiddle:
				ret = glm::vec3(-0.5f, -1.0f, 0.0f);
				break;
			case Pivot::MiddleLeft:
				ret = glm::vec3(0.0f, -0.5f, 0.0f);
				break;
			case Pivot::BottomMiddle:
				ret = glm::vec3(-0.5f, 0.0f, 0.0f);
				break;
			default:
				break;
			}
		}
		break;
		case Pivot::TopLeft:
		{
			switch (cPivot)
			{
			case Pivot::Center:
				ret = glm::vec3(-0.5f, 0.5f, 0.0f);
				break;
			case Pivot::LowerLeft:
				ret = glm::vec3(0.0f, 1.0f, 0.0f);
				break;
			case Pivot::TopLeft:
				break;
			case Pivot::TopRight:
				ret = glm::vec3(-1.0f, 0.0f, 0.0f);
				break;
			case Pivot::LowerRight:
				ret = glm::vec3(-1.0f, 1.0f, 0.0f);
				break;
			case Pivot::MiddleRight:
				ret = glm::vec3(-1.0f, 0.5f, 0.0f);
				break;
			case Pivot::TopMiddle:
				ret = glm::vec3(-0.5f, 0.0f, 0.0f);
				break;
			case Pivot::MiddleLeft:
				ret = glm::vec3(0.0f, 0.5f, 0.0f);
				break;
			case Pivot::BottomMiddle:
				ret = glm::vec3(-0.5f, 1.0f, 0.0f);
				break;
			default:
				break;
			}
		}
		break;
		case Pivot::TopRight:
		{
			switch (cPivot)
			{
			case Pivot::Center:
				ret = glm::vec3(0.5f, 0.5f, 0.0f);
				break;
			case Pivot::LowerLeft:
				ret = glm::vec3(1.0f, 1.0f, 0.0f);
				break;
			case Pivot::TopLeft:
				ret = glm::vec3(1.0f, 0.0f, 0.0f);
				break;
			case Pivot::TopRight:
				break;
			case Pivot::LowerRight:
				ret = glm::vec3(0.0f, 1.0f, 0.0f);
				break;
			case Pivot::MiddleRight:
				ret = glm::vec3(0.0f, 0.5f, 0.0f);
				break;
			case Pivot::TopMiddle:
				ret = glm::vec3(0.5f, 0.0f, 0.0f);
				break;
			case Pivot::MiddleLeft:
				ret = glm::vec3(1.0f, 0.5f, 0.0f);
				break;
			case Pivot::BottomMiddle:
				ret = glm::vec3(0.5f, 1.0f, 0.0f);
				break;
			default:
				break;
			}
		}
		break;
		case Pivot::LowerRight:
		{
			switch (cPivot)
			{
			case Pivot::Center:
				ret = glm::vec3(0.5f, -0.5f, 0.0f);
				break;
			case Pivot::LowerLeft:
				ret = glm::vec3(1.0f, 0.0f, 0.0f);
				break;
			case Pivot::TopLeft:
				ret = glm::vec3(1.0f, -1.0f, 0.0f);
				break;
			case Pivot::TopRight:
				ret = glm::vec3(0.0f, -1.0f, 0.0f);
				break;
			case Pivot::LowerRight:
				break;
			case Pivot::MiddleRight:
				ret = glm::vec3(0.5f, 0.0f, 0.0f);
				break;
			case Pivot::TopMiddle:
				ret = glm::vec3(0.5f, -1.0f, 0.0f);
				break;
			case Pivot::MiddleLeft:
				ret = glm::vec3(1.0f, -0.5f, 0.0f);
				break;
			case Pivot::BottomMiddle:
				ret = glm::vec3(0.5f, 0.0f, 0.0f);
				break;

			default:
				break;
			}
		}
		break;
		case Pivot::MiddleRight:
		{
			switch (cPivot)
			{
			case Pivot::Center:
				ret = glm::vec3(0.5f, 0.0f, 0.0f);
				break;
			case Pivot::LowerLeft:
				ret = glm::vec3(1.0f, 0.5f, 0.0f);
				break;
			case Pivot::TopLeft:
				ret = glm::vec3(1.0f, -0.5f, 0.0f);
				break;
			case Pivot::TopRight:
				ret = glm::vec3(0.0f, -0.5f, 0.0f);
				break;
			case Pivot::LowerRight:
				ret = glm::vec3(0.0f, 0.5f, 0.0f);
				break;
			case Pivot::MiddleRight:
				break;
			case Pivot::TopMiddle:
				ret = glm::vec3(0.5f, -0.5f, 0.0f);
				break;
			case Pivot::MiddleLeft:
				ret = glm::vec3(1.0f, 0.0f, 0.0f);
				break;
			case Pivot::BottomMiddle:
				ret = glm::vec3(0.5f, 0.5f, 0.0f);
				break;
			default:
				break;
			}
		}
		break;
		case Pivot::TopMiddle:
		{
			switch (cPivot)
			{
			case Pivot::Center:
				ret = glm::vec3(0.0f, 0.5f, 0.0f);
				break;
			case Pivot::LowerLeft:
				ret = glm::vec3(0.5f, 1.0f, 0.0f);
				break;
			case Pivot::TopLeft:
				ret = glm::vec3(0.5f, 0.0f, 0.0f);
				break;
			case Pivot::TopRight:
				ret = glm::vec3(-0.5f, 0.0f, 0.0f);
				break;
			case Pivot::LowerRight:
				ret = glm::vec3(-0.5f, 1.0f, 0.0f);
				break;
			case Pivot::MiddleRight:
				ret = glm::vec3(-0.5f, 0.5f, 0.0f);
				break;
			case Pivot::TopMiddle:
				break;
			case Pivot::MiddleLeft:
				ret = glm::vec3(0.5f, 0.5f, 0.0f);
				break;
			case Pivot::BottomMiddle:
				ret = glm::vec3(0.5f, 1.0f, 0.0f);
				break;
			default:
				break;
			}
		}
		break;
		case Pivot::MiddleLeft:
		{
			switch (cPivot)
			{
			case Pivot::Center:
				ret = glm::vec3(-0.5f, 0.0f, 0.0f);
				break;
			case Pivot::LowerLeft:
				ret = glm::vec3(0.0f, 0.5f, 0.0f);
				break;
			case Pivot::TopLeft:
				ret = glm::vec3(0.0f, -0.5f, 0.0f);
				break;
			case Pivot::TopRight:
				ret = glm::vec3(-1.0f, -0.5f, 0.0f);
				break;
			case Pivot::LowerRight:
				ret = glm::vec3(-1.0f, 0.5f, 0.0f);
				break;
			case Pivot::MiddleRight:
				ret = glm::vec3(-1.0f, 0.5f, 0.0f);
				break;
			case Pivot::TopMiddle:
				ret = glm::vec3(-0.5f, -0.5f, 0.0f);
				break;
			case Pivot::MiddleLeft:
				break;
			case Pivot::BottomMiddle:
				ret = glm::vec3(0.5f, 0.5f, 0.0f);
				break;
			default:
				break;
			}
		}
		break;
		case Pivot::BottomMiddle:
		{
			switch (cPivot)
			{
			case Pivot::Center:
				ret = glm::vec3(0.0f, -0.5f, 0.0f);
				break;
			case Pivot::LowerLeft:
				ret = glm::vec3(0.5f, 0.0f, 0.0f);
				break;
			case Pivot::TopLeft:
				ret = glm::vec3(0.5f, -1.0f, 0.0f);
				break;
			case Pivot::TopRight:
				ret = glm::vec3(-0.5f, -0.5f, 0.0f);
				break;
			case Pivot::LowerRight:
				ret = glm::vec3(-0.5f, 0.0f, 0.0f);
				break;
			case Pivot::MiddleRight:
				ret = glm::vec3(-0.5f, -0.5f, 0.0f);
				break;
			case Pivot::TopMiddle:
				ret = glm::vec3(0.0f, -1.0f, 0.0f);
				break;
			case Pivot::MiddleLeft:
				ret = glm::vec3(0.5f, -0.5f, 0.0f);
				break;
			case Pivot::BottomMiddle:
				break;
			default:
				break;
			}
		}
		break;
		default:
			break;
		}
		return ret;
	}
	static const glm::vec3 GetSliderScalar(const SliderDirection& direction, const float& current)
	{
		glm::vec3 ret = glm::vec3(0.0f);
		float tempCurrent = current;
		if (tempCurrent > 1.0f)
			tempCurrent = 1.0f;
		else if (tempCurrent < 0.0f)
			tempCurrent = 0.0f;

		switch (direction)
		{
		case SliderDirection::Left:
		case SliderDirection::Center:
		case SliderDirection::Right:
			ret = glm::vec3(1.0f - tempCurrent, 0.0f, 0.0f);
			break;
		case SliderDirection::Top:
		case SliderDirection::Middle:
		case SliderDirection::Bottom:
			ret = glm::vec3(0.0f, 1.0f - tempCurrent, 0.0f);
			break;
		default:
			break;
		}
		return ret;
	}
	void OpenGLRenderer::DrawSlider(const glm::mat4& transform, const Pivot& pivot, const GUISliderComponent& guiSC, const GUIState& state, const uint32_t& entityID)
	{
		// Background
		if (Ref<Texture2D> texture2D = Project::GetAsset<Texture2D>(guiSC.BackgroundTextureHandle))
			DrawQuadTexture(transform, pivot, texture2D, 1.0f, guiSC.BackgroundColor, entityID);
		else
			DrawQuadColor(transform, pivot, guiSC.BackgroundColor, entityID);

		// Middleground
		if (guiSC.Fill > 0.0f)
		{
			Pivot mgPivot = PivotFromSliderDirection(guiSC.Direction);
			glm::vec3 pivotOffset = GetSliderOffset(pivot, mgPivot);
			glm::vec3 mgScalar = GetSliderScalar(guiSC.Direction, guiSC.Fill);
			glm::mat4 mgTransformOffset = glm::translate(s_Data.IdentityMat4, pivotOffset) * glm::scale(s_Data.IdentityMat4, mgScalar);
			
			Ref<Texture2D> currentTexture = nullptr;
			glm::vec4 currentColor = glm::vec4(1.0f);
			switch (state)
			{
			case GUIState::Disabled:
			{
				currentTexture = Project::GetAsset<Texture2D>(guiSC.DisabledTextureHandle);
				currentColor = guiSC.DisabledColor;
			}
			break;
			case GUIState::Enabled:
			{
				currentTexture = Project::GetAsset<Texture2D>(guiSC.EnabledTextureHandle);
				currentColor = guiSC.EnabledColor;
			}
			break;
			case GUIState::Hovered:
			case GUIState::Focused:
			{
				currentTexture = Project::GetAsset<Texture2D>(guiSC.HoveredTextureHandle);
				currentColor = guiSC.HoveredColor;
			}
			break;
			case GUIState::Active:
			case GUIState::Selected:
			{
				currentTexture = Project::GetAsset<Texture2D>(guiSC.SelectedTextureHandle);
				currentColor = guiSC.SelectedColor;
			}
			break;
			default:
				GE_CORE_WARN("OpenGLRenderer::DrawCheckbox() - Unknown GUIState.");
				break;
			}

			if (currentTexture)
				DrawQuadTexture(transform - mgTransformOffset, mgPivot, currentTexture, 1.0f, currentColor, entityID);
			else
				DrawQuadColor(transform - mgTransformOffset, mgPivot, currentColor, entityID);
		}

		// Foreground
		if (Ref<Texture2D> texture2D = Project::GetAsset<Texture2D>(guiSC.ForegroundTextureHandle))
			DrawQuadTexture(transform, pivot, texture2D, 1.0f, guiSC.ForegroundColor, entityID);
		else
			DrawQuadColor(transform, pivot, guiSC.ForegroundColor, entityID);

	}

	void OpenGLRenderer::DrawCheckbox(const glm::mat4& transform, const Pivot& pivot, const GUICheckboxComponent& guiCB, const GUIState& state, const uint32_t& entityID)
	{
		// Background
		if (Ref<Texture2D> bgTexture = Project::GetAsset<Texture2D>(guiCB.BackgroundTextureHandle))
			DrawQuadTexture(transform, pivot, bgTexture, 1.0f, guiCB.BackgroundColor, entityID);
		else
			DrawQuadColor(transform, pivot, guiCB.BackgroundColor, entityID);
		
		Ref<Texture2D> currentTexture = nullptr;
		glm::vec4 currentColor = glm::vec4(1.0f);
		switch (state)
		{
		case GUIState::Disabled:
		{
			currentTexture = Project::GetAsset<Texture2D>(guiCB.DisabledTextureHandle);
			currentColor = guiCB.DisabledColor;
		}
		break;
		case GUIState::Enabled:
		{
			currentTexture = Project::GetAsset<Texture2D>(guiCB.EnabledTextureHandle);
			currentColor = guiCB.EnabledColor;
		}
		break;
		case GUIState::Hovered:
		case GUIState::Focused:
		{
			currentTexture = Project::GetAsset<Texture2D>(guiCB.HoveredTextureHandle);
			currentColor = guiCB.HoveredColor;
		}
		break;
		case GUIState::Active:
		case GUIState::Selected:
		{
			currentTexture = Project::GetAsset<Texture2D>(guiCB.SelectedTextureHandle);
			currentColor = guiCB.SelectedColor;
		}
		break;
		default:
			GE_CORE_WARN("OpenGLRenderer::DrawCheckbox() - Unknown GUIState.");
			break;
		}

		if (currentTexture)
			DrawQuadTexture(transform, pivot, currentTexture, 1.0f, currentColor, entityID);
		else
			DrawQuadColor(transform, pivot, currentColor, entityID);

		// Foreground
		if (Ref<Texture2D> fgTexture = Project::GetAsset<Texture2D>(guiCB.ForegroundTextureHandle))
			DrawQuadTexture(transform, pivot, fgTexture, 1.0f, guiCB.ForegroundColor, entityID);
		else
			DrawQuadColor(transform, pivot, guiCB.ForegroundColor, entityID);
	
	}
#pragma endregion

#pragma endregion

}