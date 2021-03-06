/*
 * This file is part of `et engine`
 * Copyright 2009-2015 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#include <et/rt/environment.h>

using namespace et;
using namespace et::rt;

EnvironmentEquirectangularMapSampler::EnvironmentEquirectangularMapSampler(
	TextureDescription::Pointer data, const float4& scale) : _data(data), _scale(scale)
{
	if (_data->bitsPerPixel != 128)
	{
		ET_FAIL("Only RGBA32F textures are supported at this time")
	}
}

float4 EnvironmentEquirectangularMapSampler::sampleTexture(vec2i texCoord)
{
	{
		while (texCoord.x >= _data->size.x) texCoord.x -= _data->size.x;
		while (texCoord.y >= _data->size.y) texCoord.y -= _data->size.y;
		while (texCoord.x < 0) texCoord.x += _data->size.x;
		while (texCoord.y < 0) texCoord.y += _data->size.y;
	}
	
	const vec4* rawData = reinterpret_cast<const vec4*>(_data->data.binary());
	return float4(rawData[texCoord.x + texCoord.y * _data->size.x]);
}

float4 EnvironmentEquirectangularMapSampler::sampleInDirection(const float4& r)
{
	float phi = 0.5f + std::atan2(r.cZ(), r.cX()) / DOUBLE_PI;
	float theta = 0.5f + std::asin(r.cY()) / PI;
	
	vec2 tc(phi * _data->size.x, theta * _data->size.y);
	vec2i baseTexCoord(static_cast<int>(tc.x), static_cast<int>(tc.y));
	
	float4 c00 = sampleTexture(baseTexCoord); ++baseTexCoord.x;
	float4 c10 = sampleTexture(baseTexCoord); ++baseTexCoord.y;
	float4 c11 = sampleTexture(baseTexCoord); --baseTexCoord.x;
	float4 c01 = sampleTexture(baseTexCoord);
	
	vec2 dudv(tc.x - std::floor(tc.x), tc.y - std::floor(tc.y));
	float4 cx1 = c00 * (1.0f - dudv.x) + c01 * dudv.x;
	float4 cx2 = c10 * (1.0f - dudv.x) + c11 * dudv.x;
	
	return _scale * (cx1 * (1.0f - dudv.y) + cx2 * dudv.y);
}
