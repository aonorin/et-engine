/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#include <et/app/application.h>
#include <et/rendering/rendercontext.h>
#include <et/rendering/base/helpers.h>

namespace et
{
namespace renderhelper
{

namespace rh_local
{
	VertexArrayObject::Pointer fullscreenMesh;
	Material::Pointer plainMaterial;
	DepthState disabledDepthState;
	extern const std::string vertexShader;
	extern const std::string fragmentShader;
}

void init(RenderContext* rc)
{
	auto vd = VertexDeclaration(false, VertexAttributeUsage::Position, DataType::Vec2);
	IndexArray::Pointer ia = IndexArray::Pointer::create(IndexArrayFormat::Format_8bit, 4, PrimitiveType::TriangleStrips);
	VertexStorage::Pointer vs = VertexStorage::Pointer::create(vd, 4);
	auto pos = vs->accessData<DataType::Vec2>(VertexAttributeUsage::Position, 0);
	pos[0] = vec2(-1.0f, -1.0f);
	pos[1] = vec2( 1.0f, -1.0f);
	pos[2] = vec2(-1.0f,  1.0f);
	pos[3] = vec2( 1.0f,  1.0f);
	ia->linearize(4);

	rh_local::fullscreenMesh = rc->renderer()->createVertexArrayObject("rh_local::mesh");
	auto vb = rc->renderer()->createVertexBuffer("rh_local::vb", vs, BufferDrawType::Static);
	auto ib = rc->renderer()->createIndexBuffer("rh_local::ib", ia, BufferDrawType::Static);
	rh_local::fullscreenMesh->setBuffers(vb, ib);

	rh_local::disabledDepthState.compareFunction = CompareFunction::Always;
	rh_local::disabledDepthState.depthTestEnabled = false;
	rh_local::disabledDepthState.depthWriteEnabled = false;
    
    Program::Pointer fullScreenProgram = rc->renderer()->createProgram(rh_local::vertexShader, rh_local::fragmentShader);
    
    rh_local::plainMaterial = Material::Pointer::create(nullptr);
    rh_local::plainMaterial->setDepthState(rh_local::disabledDepthState);
    rh_local::plainMaterial->setProgram(fullScreenProgram);
}

void release()
{
	rh_local::plainMaterial.reset(nullptr);
	rh_local::fullscreenMesh.reset(nullptr);
}
	
RenderBatch::Pointer createFullscreenRenderBatch(Texture::Pointer texture)
{
	ET_ASSERT(rh_local::plainMaterial.valid());
	ET_ASSERT(rh_local::fullscreenMesh.valid());

	PipelineState::ConstructInfo psInfo;
	PipelineState::Pointer ps = PipelineState::Pointer::create(psInfo);

	rh_local::plainMaterial->setTexutre("color_texture", texture);
	return RenderBatch::Pointer::create(rh_local::plainMaterial, rh_local::fullscreenMesh);
}

}
}

/*
 * Shaders
 */
const std::string et::renderhelper::rh_local::vertexShader = R"(
etVertexIn vec2 Vertex;
etVertexOut vec2 TexCoord;
void main()
{
	TexCoord = 0.5 * Vertex + vec2(0.5);
	gl_Position = vec4(Vertex, 0.0, 1.0);
})";

const std::string et::renderhelper::rh_local::fragmentShader = R"(
#if defined(TEXTURE_CUBE)
uniform etLowp samplerCube color_texture;
#elif defined(TEXTURE_RECTANGLE)
uniform etLowp sampler2DRect color_texture;
#elif defined(TEXTURE_2D_ARRAY)
uniform etLowp sampler2DArray color_texture;
#else
uniform etLowp sampler2D color_texture;
#endif

uniform etHighp vec2 color_texture_size;
etFragmentIn etHighp vec2 TexCoord;

void main()
{
#if defined(TEXTURE_CUBE)
	etFragmentOut = etTextureCube(color_texture, vec3(TexCoord, 0.0));
#elif defined(TEXTURE_RECTANGLE)
	etFragmentOut = etTextureRect(color_texture, TexCoord * color_texture_size);
#elif defined(TEXTURE_2D_ARRAY)
	etFragmentOut = etTexture2DArray(color_texture, vec3(TexCoord, 0.0));
#else
	etFragmentOut = etTexture2D(color_texture, TexCoord);
#endif
})";
