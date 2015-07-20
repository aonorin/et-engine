/*
 * This file is part of `et engine`
 * Copyright 2009-2015 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/collision/aabb.h>
#include <et/collision/sphere.h>
#include <et/collision/obb.h>
#include <et/rendering/vertexarrayobject.h>
#include <et/scene3d/renderableelement.h>

namespace et
{
	namespace s3d
	{
		class Mesh : public RenderableElement
		{
		public:
			ET_DECLARE_POINTER(Mesh)
			
			static const std::string defaultMeshName;
			
			struct SupportData
			{
				vec3 minMaxCenter;
				vec3 averageCenter;
				vec3 dimensions;
				float boundingSphereRadius = 0.0f;
				bool valid = false;
			};

		public:
			Mesh(const std::string& = defaultMeshName, BaseElement* = nullptr);
			
			Mesh(const std::string&, const VertexArrayObject&, const Material::Pointer&,
				uint32_t, uint32_t, BaseElement* = nullptr);
			
			Mesh(const std::string&, const VertexArrayObject&, const Material::Pointer&,
				uint32_t, uint32_t, const VertexStorage::Pointer&, const IndexArray::Pointer&, 
				BaseElement* = nullptr);

			ElementType type() const 
				{ return ElementType::Mesh; }

			Mesh* duplicate();

			VertexArrayObject& vertexArrayObject();
			const VertexArrayObject& vertexArrayObject() const;

			VertexBuffer::Pointer& vertexBuffer();
			const VertexBuffer::Pointer& vertexBuffer() const;

			IndexBuffer::Pointer& indexBuffer();
			const IndexBuffer::Pointer& indexBuffer() const;

			uint32_t startIndex() const;
			void setStartIndex(uint32_t index);
			
			uint32_t numIndexes() const;
			virtual void setNumIndexes(uint32_t num);

			void setVertexBuffer(VertexBuffer::Pointer);
			void setIndexBuffer(IndexBuffer::Pointer);
			void setVertexArrayObject(VertexArrayObject);

			void serialize(Dictionary, const std::string&);
			void deserialize(Dictionary, SerializationHelper*);

			void setVertexStorage(VertexStorage::Pointer);
			void setIndexArray(IndexArray::Pointer);
			
			void cleanupLodChildren();
			void attachLod(uint32_t level, Mesh::Pointer mesh);

			void setLod(uint32_t level);
			
			const std::string& vaoName() const
				{ return _vaoName; }
			
			const std::string& vbName() const
				{ return _vbName; }
			
			const std::string& ibName() const
				{ return _ibName; }
			
			void calculateSupportData();
			
			const SupportData& supportData() const
				{ return _supportData; }

			const VertexStorage::Pointer& vertexStorage() const
				{ return _vertexStorage; }
			
			const IndexArray::Pointer& indexArray() const
				{ return _indexArray; }
			
			const Sphere& boundingSphere();
			const AABB& boundingBox();
			const OBB& orientedBoundingBox();
			
			float finalTransformScale();
			
		private:
			Mesh* currentLod();
			const Mesh* currentLod() const;
			void transformInvalidated() override;

		private:
			VertexArrayObject _vao;
			VertexStorage::Pointer _vertexStorage;
			IndexArray::Pointer _indexArray;
			
			AABB _cachedBoundingBox;
			OBB _cachedOrientedBoundingBox;
			Sphere _cachedBoundingSphere;
			
			SupportData _supportData;
			std::map<uint32_t, Mesh::Pointer> _lods;
			uint32_t _startIndex = 0;
			uint32_t _numIndexes = 0;
			uint32_t _selectedLod = 0;
			std::string _vaoName;
			std::string _vbName;
			std::string _ibName;
			
			bool _shouldUpdateBoundingBox = true;
			bool _shouldUpdateOrientedBoundingBox = true;
			bool _shouldUpdateBoundingSphere = true;
		};
	}
}
