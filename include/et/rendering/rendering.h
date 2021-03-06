/*
 * This file is part of `et engine`
 * Copyright 2009-2015 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/core/et.h>

namespace et
{
	enum class VertexAttributeUsage : uint32_t
	{
		Position,
		Normal,
		Color,
		Tangent,
		Binormal,
		
		TexCoord0,
		TexCoord1,
		TexCoord2,
		TexCoord3,
		
		Smoothing,
		BuiltIn_InstanceId,
		BuiltIn_InstanceIdExt,
		BlendWeights,
		BlendIndices,
		
		BuiltIn_VertexId,
		
		Unknown,
		max
	};
	
	enum class VertexAttributeType : uint32_t
	{
		Float,
		Vec2,
		Vec3,
		Vec4,
		
		Mat3,
		Mat4,
		
		Int,
		IntVec2,
		IntVec3,
		IntVec4,
		
		max
	};
	
	enum class BlendState : uint32_t
	{
		Disabled,
		Current,
		Default,
		AlphaPremultiplied,
		Additive,
		AlphaAdditive,
		AlphaMultiplicative,
		ColorAdditive,
		AlphaInverseMultiplicative,
		
		max
	};
	
	enum class CullState : uint32_t
	{
		Current,
		Front,
		Back,
		
		max
	};
	
	enum class DepthFunc : uint32_t
	{
		Never,
		Less,
		LessOrEqual,
		Equal,
		GreaterOrEqual,
		Greater,
		Always,
		
		max
	};
	
	enum ColorMask : uint32_t
	{
		None = 0x00,
		Red = 0x01,
		Green = 0x02,
		Blue = 0x04,
		Alpha = 0x08,
		RGB = Red | Green | Blue,
		RGBA = RGB | Alpha
	};
	
	enum class BufferDrawType : uint32_t
	{
		Static,
		Dynamic,
		Stream,
		
		max
	};
	
	enum class TextureTarget : uint32_t
	{
		Texture_2D,
		Texture_2D_Array,
		Texture_Rectangle,
		Texture_Cube,
		max,
	};
	
	enum class TextureWrap : uint32_t
	{
		Repeat,
		ClampToEdge,
		MirrorRepeat,
		
		max
	};
	
	enum class TextureFormat : uint32_t
	{
		Invalid,
		
		R,
		R8,
		R16,
		R16F,
		R32F,
		
		RG,
		RG8,
		RG16,
		RG16F,
		RG32F,
		
		RGB,
		RGB8,
		RGB16,
		RGB16F,
		RGB32F,
		
		BGR,
		
		RGBA,
		RGBA8,
		RGBA16,
		RGBA16F,
		RGBA32F,
		
		BGRA,
				
		DXT1_RGB,
		DXT1_RGBA,
		DXT3,
		DXT5,
		
		RGTC2,
		
		Depth,
		Depth16,
		Depth24,
		Depth32,
		Depth32F,
		
		PVR_2bpp_RGB,
		PVR_2bpp_sRGB,
		PVR_2bpp_RGBA,
		PVR_2bpp_sRGBA,
		PVR_4bpp_RGB,
		PVR_4bpp_sRGB,
		PVR_4bpp_RGBA,
		PVR_4bpp_sRGBA,

		R11G11B10F,
		
		max
	};
	
	enum class TextureFiltration : uint32_t
	{
		Nearest,
		Linear,
		NearestMipMapNearest,
		LinearMipMapNearest,
		NearestMipMapLinear,
		LinearMipMapLinear,
		
		max
	};
	
	enum class PrimitiveType : uint32_t
	{
		Points,
		Lines,
		Triangles,
		TriangleStrips,
		LineStrips,
		LineStripAdjacency,
		LinesAdjacency,
		max
	};
	
	enum class IndexArrayFormat : uint32_t
	{
		Format_8bit = 1,
		Format_16bit = 2,
		Format_32bit = 4,
		max
	};
	
	enum class DataType : uint32_t
	{
		Char,
		UnsignedChar,
		Short,
		UnsignedShort,
		Int,
		UnsignedInt,
		Half,
		Float,
		Double,
		
		UnsignedShort_4444,
		UnsignedShort_5551,
		UnsignedShort_565,

		UnsignedInt_8888_Rev,
		
		max,
	};
	
	enum class TextureOrigin
	{
		TopLeft,
		BottomLeft,
		
		max
	};
	
	enum class TextureDataLayout
	{
		FacesFirst,
		MipsFirst,
		
		max
	};
	
	enum MapBufferOptions : uint32_t
	{
		Read = 0x01,
		Write = 0x02,
		Unsynchronized = 0x04,
		InvalidateRange = 0x08,
		InvalidateBuffer = 0x10,
	};
	
	enum : uint32_t
	{
		VertexAttributeUsage_max = static_cast<uint32_t>(VertexAttributeUsage::max),
		VertexAttributeType_max = static_cast<uint32_t>(VertexAttributeType::max),

		IndexArrayFormat_max = static_cast<uint32_t>(IndexArrayFormat::max),
		PrimitiveType_max = static_cast<uint32_t>(PrimitiveType::max),

		DataType_max = static_cast<uint32_t>(DataType::max),
		BlendState_max = static_cast<uint32_t>(BlendState::max),

		TextureTarget_max = static_cast<uint32_t>(TextureTarget::max),
		TextureFormat_max = static_cast<uint32_t>(TextureFormat::max),
		
		InvalidIndex = static_cast<uint32_t>(-1),
		InvalidShortIndex = static_cast<uint16_t>(-1),
		InvalidSmallIndex = static_cast<uint8_t>(-1),
		
		MaxDrawBuffers = 8
	};
	
	DataType vertexAttributeTypeDataType(VertexAttributeType t);

	VertexAttributeUsage stringToVertexAttributeUsage(const std::string& s, bool& builtIn);
	VertexAttributeType stringToVertexAttributeType(const std::string& s);
	DataType stringToDataType(const std::string&);

	std::string vertexAttributeUsageToString(VertexAttributeUsage);
	std::string vertexAttributeTypeToString(VertexAttributeType);
	std::string dataTypeToString(DataType);

	std::string primitiveTypeToString(PrimitiveType);
	PrimitiveType stringToPrimitiveType(const std::string&);

	std::string indexArrayFormatToString(IndexArrayFormat);
	IndexArrayFormat stringToIndexArrayFormat(const std::string&);

	uint32_t sizeOfDataType(DataType);
	
	uint32_t vertexAttributeUsageMask(VertexAttributeUsage u);
	uint32_t vertexAttributeTypeSize(VertexAttributeType t);
	uint32_t vertexAttributeTypeComponents(VertexAttributeType t);

	uint32_t bitsPerPixelForType(DataType type);
	uint32_t bitsPerPixelForTextureFormat(TextureFormat internalFormat, DataType type);
	uint32_t channelsForTextureFormat(TextureFormat internalFormat);
}
