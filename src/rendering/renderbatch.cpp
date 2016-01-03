/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#include <et/rendering/renderbatch.h>

using namespace et;

RenderBatch::RenderBatch(const Material::Pointer& m, const VertexArrayObject& v) :
	_material(m), _data(v), _firstIndex(0), _numIndexes(v->indexBuffer()->size())
{
	
}

RenderBatch::RenderBatch(const Material::Pointer& m, const VertexArrayObject& v, uint32_t i, uint32_t ni) :
	_material(m), _data(v), _firstIndex(i), _numIndexes(ni)
{
	
}