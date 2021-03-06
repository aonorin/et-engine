/*
 * This file is part of `et engine`
 * Copyright 2009-2015 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/vertexbuffer/indexarray.h>
#include <et/vertexbuffer/vertexstorage.h>
#include <et/scene3d/material.h>

namespace et
{
	namespace s3d
	{
		class Storage
		{
		public:
			Storage();

			Dictionary serialize(const std::string&);
			
			void deserializeWithOptions(RenderContext*, Dictionary, SerializationHelper*, ObjectsCache&,
				uint32_t);

			std::vector<VertexStorage::Pointer>& vertexStorages()
				{ return _vertexStorages; }

			const std::vector<VertexStorage::Pointer>& vertexStorages() const
				{ return _vertexStorages; }
			
			IndexArray::Pointer indexArray()
				{ return _indexArray; }

			const IndexArray::Pointer indexArray() const
				{ return _indexArray; }

			Material::Map& materials()
				{ return _materials; }

			const Material::Map& materials() const
				{ return _materials; }

			std::vector<Texture::Pointer>& textures()
				{ return _textures; }

			const std::vector<Texture::Pointer>& textures() const
				{ return _textures; }

			void addTexture(Texture::Pointer t)
				{ _textures.push_back(t); }

			void addMaterial(Material::Pointer m)
				{ _materials.insert({m->name(), m}); }

			void addVertexStorage(const VertexStorage::Pointer&);

			void setIndexArray(const IndexArray::Pointer&);
			
			VertexStorage::Pointer addVertexStorageWithDeclaration(const VertexDeclaration& decl, size_t size);
			VertexStorage::Pointer vertexStorageWithDeclarationForAppendingSize(const VertexDeclaration& decl, size_t size);
			
			int indexOfVertexStorage(const VertexStorage::Pointer& va);
			
			void flush();

		private:
			Storage* duplicate()
				{ return nullptr; }

		private:
			std::vector<VertexStorage::Pointer> _vertexStorages;
			IndexArray::Pointer _indexArray;
			Material::Map _materials;
			std::vector<Texture::Pointer> _textures;
		};
	}
}
