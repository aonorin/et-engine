/*
 * This file is part of `et engine`
 * Copyright 2009-2015 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/geometry/vector4-simd.h>
#include <et/scene3d/scene3d.h>

namespace et
{
	namespace rt
	{
		struct Material
		{
			std::string name;
			vec4simd diffuse;
			vec4simd specular;
			vec4simd emissive;
			float roughness = 0.0f;
		};

		struct Triangle
		{
			vec4simd v[3];
			vec4simd n[3];
			vec4simd edge1to0;
			vec4simd edge2to0;
			size_t materialIndex = 0;
			float square = 0.0f;

			float _dot00 = 0.0f;
			float _dot10 = 0.0f;
			float _dot01 = 0.0f;
			float _dot11 = 0.0f;
			float _invDenom = 0.0f;

			void computeSupportData()
			{
				edge1to0 = v[1] - v[0];
				edge2to0 = v[2] - v[0];
				square = edge1to0.crossXYZ(edge2to0).length() * 0.5f;
				
				_dot00 = edge1to0.dotSelf();
				_dot11 = edge2to0.dotSelf();
				_dot01 = edge1to0.dot(edge2to0);
				_invDenom = 1.0f / (_dot00 * _dot11 - _dot01 * _dot01);
			}

			vec4simd barycentric(vec4simd p) const
			{
				p -= v[0];
				float dot20 = p.dot(edge1to0);
				float dot21 = p.dot(edge2to0);
				float y = (_dot11 * dot20 - _dot01 * dot21) * _invDenom;
				float z = (_dot00 * dot21 - _dot01 * dot20) * _invDenom;
				float x = 1.0f - y - z;
				return vec4simd(x, y, z, 0.0f);
			}

			vec4simd interpolatedNormal(const vec4simd& b) const
			{
				return n[0] * b.shuffle<0, 0, 0, 0>() + 
					n[1] * b.shuffle<1, 1, 1, 1>() + n[2] * b.shuffle<2, 2, 2, 2>();
			}
			
			vec4simd averageNormal() const
			{
				vec4simd c = edge2to0.crossXYZ(edge1to0);
				c.normalize();
				return c;
			}
		};

		struct Ray
		{
			vec4simd origin;
			vec4simd direction;

			Ray() { }

			Ray(const vec4simd& o, const vec4simd& d) : 
				origin(o), direction(d) { }

			Ray(const ray3d& r) : 
				origin(r.origin, 1.0f), direction(r.direction, 0.0f) { } 
		};
		
		struct BoundingBox
		{
			vec4simd center = vec4simd(0.0f);
			vec4simd halfSize = vec4simd(0.0f);
			
			BoundingBox()
				{ }
			
			BoundingBox(const vec4simd& c, const vec4simd& hs) :
				center(c), halfSize(hs) { }
			
			BoundingBox(const vec4simd& minVertex, const vec4simd& maxVertex, int)
			{
				center = (minVertex + maxVertex) * 0.5f;
				halfSize = (maxVertex - minVertex) * 0.5f;
			}
			
			vec4simd minVertex() const
				{ return center - halfSize; }
			
			vec4simd maxVertex() const
				{ return center + halfSize; }
			
			float square() const
			{
				return 4.0f *
				(
					halfSize.x() * halfSize.y() +
					halfSize.y() * halfSize.z() +
					halfSize.x() * halfSize.z()
				);
			}
			
			float volume() const
			{
				return 8.0f * halfSize.x() * halfSize.y() * halfSize.z();
			};
		};

		struct Region
		{
			vec2i origin = vec2i(0);
			vec2i size = vec2i(0);
			size_t estimatedBounces = 0;
			bool sampled = false;
		};

		inline bool rayTriangle(const Ray& ray, const Triangle& tri, vec4simd& intersection_pt, vec4simd& barycentric)
		{
			const float epsilon = 0.0005f;
			const float minusEpsilon = -0.5f * epsilon;
			const float onePlusEpsilon = 1.0f + 0.5f * epsilon;
			const float epsilonSquared = epsilon * epsilon;

			vec4simd pvec = ray.direction.crossXYZ(tri.edge2to0);
			float det = tri.edge1to0.dot(pvec);
			if (det * det < epsilonSquared)
				return false;

			vec4simd tvec = ray.origin - tri.v[0];
			float u = tvec.dot(pvec) / det;
			if ((u < minusEpsilon) || (u > onePlusEpsilon))
				return false;

			vec4simd qvec = tvec.crossXYZ(tri.edge1to0);
			float v = ray.direction.dot(qvec) / det;
			if ((v < minusEpsilon) || (u + v > onePlusEpsilon))
				return false;

			float t = tri.edge2to0.dot(qvec) / det;
			intersection_pt = ray.origin + ray.direction * t;
			barycentric = vec4simd(1.0f - u - v, u, v, 0.0f);
			return (t > epsilon);
		}

		inline vec4simd perpendicularVector(const vec4simd& normal)
		{
			vec3 componentsLength = (normal * normal).xyz();

			if (componentsLength.x > 0.5f)
			{
				float scaleFactor = std::sqrt(componentsLength.z + componentsLength.x);
				return normal.shuffle<2, 3, 0, 1>() * vec4simd(1.0f / scaleFactor, 0.0f, -1.0f / scaleFactor, 0.0f);
			}
			else if (componentsLength.y > 0.5f)
			{
				float scaleFactor = std::sqrt(componentsLength.y + componentsLength.x);
				return normal.shuffle<1, 0, 3, 3>() * vec4simd(-1.0f / scaleFactor, 1.0f / scaleFactor, 0.0f, 0.0f);
			}
			float scaleFactor = std::sqrt(componentsLength.z + componentsLength.y);
			return normal.shuffle<3, 2, 1, 3>() * vec4simd(0.0f, -1.0f / scaleFactor, 1.0f / scaleFactor, 0.0f);
		}

		inline vec4simd randomVectorOnHemisphere(const vec4simd& normal, float distributionAngle)
		{
			float phi = randomFloat() * DOUBLE_PI;
			float theta = std::sin(randomFloat() * clamp(distributionAngle, 0.0f, HALF_PI));
			vec4simd u = perpendicularVector(normal);
			vec4simd result = (u * std::cos(phi) + u.crossXYZ(normal) * std::sin(phi)) * std::sqrt(theta) +
				normal * std::sqrt(1.0f - theta);
			result.normalize();
			return result;
		}

		inline vec4simd reflect(const vec4simd& v, const vec4simd& n)
		{
			const vec4simd two(2.0f);
			return v - two * n * v.dotVector(n);
		}
		
		inline bool pointInsideBoundingBox(const vec4simd& p, const BoundingBox& box)
		{
			vec4simd lower = box.center - box.halfSize;
			if ((p.x() < lower.x()) || (p.y() < lower.y()) || (p.z() < lower.z())) return false;
			
			vec4simd upper = box.center + box.halfSize;
			if ((p.x() > upper.x()) || (p.y() > upper.y()) || (p.z() > upper.z())) return false;
			
			return true;
		}
		
		inline bool rayHitsBoundingBox(const Ray& r, const BoundingBox& box)
		{
			vec4 origin;
			vec4 invDirection;
			r.origin.loadToVec4(origin);
			r.direction.reciprocal().loadToVec4(invDirection);
			
			int r_sign_x = (invDirection.x < 0.0f ? 1 : 0);
			int r_sign_y = (invDirection.y < 0.0f ? 1 : 0);
			
			vec4simd parameters[2] = { box.minVertex(), box.maxVertex() };
			
			float txmin = (parameters[r_sign_x].x() - origin.x) * invDirection.x;
			float tymin = (parameters[r_sign_y].y() - origin.y) * invDirection.y;
			float txmax = (parameters[1 - r_sign_x].x() - origin.x) * invDirection.x;
			float tymax = (parameters[1 - r_sign_y].y() - origin.y) * invDirection.y;
			
			if ((txmin < tymax) && (tymin < txmax))
			{
				if (tymin > txmin)
					txmin = tymin;
				
				if (tymax < txmax)
					txmax = tymax;
				
				int r_sign_z = (invDirection.z < 0.0f ? 1 : 0);
				float tzmin = (parameters[r_sign_z].z() - origin.z) * invDirection.z;
				float tzmax = (parameters[1 - r_sign_z].z() - origin.z) * invDirection.z;
				
				if ((txmin < tzmax) && (tzmin < txmax))
					return true;
			}
			
			return false;
		}
		
		inline bool triangleIntersectsBoundingBox(const Triangle& t, const BoundingBox& b)
		{
			auto projectToTriangle = [&t](const vec4simd& axis, float& minValue, float& maxValue)
			{
				minValue = std::numeric_limits<float>::max();
				maxValue = -std::numeric_limits<float>::max();
				for (int i = 0; i < 3; ++i)
				{
					double val = axis.dot(t.v[i]);
					if (val < minValue) minValue = val;
					if (val > maxValue) maxValue = val;
				}
			};
			auto projectToBox = [&b](const vec4simd& axis, float& minValue, float& maxValue)
			{
				const vec4simd scales[8] =
				{
					vec4simd(-1.0f, -1.0f, -1.0f, 1.0f),
					vec4simd( 1.0f, -1.0f, -1.0f, 1.0f),
					vec4simd(-1.0f,  1.0f, -1.0f, 1.0f),
					vec4simd( 1.0f,  1.0f, -1.0f, 1.0f),
					vec4simd(-1.0f, -1.0f,  1.0f, 1.0f),
					vec4simd( 1.0f, -1.0f,  1.0f, 1.0f),
					vec4simd(-1.0f,  1.0f,  1.0f, 1.0f),
					vec4simd( 1.0f,  1.0f,  1.0f, 1.0f),
				};
				minValue = std::numeric_limits<float>::max();
				maxValue = -std::numeric_limits<float>::max();
				for (int i = 0; i < 8; ++i)
				{
					double val = axis.dot(b.center + b.halfSize * scales[i]);
					if (val < minValue) minValue = val;
					if (val > maxValue) maxValue = val;
				}
			};
			
			vec4simd boxNormals[3] =
			{
				vec4simd(1.0f, 0.0f, 0.0f, 0.0f),
				vec4simd(0.0f, 1.0f, 0.0f, 0.0f),
				vec4simd(0.0f, 0.0f, 1.0f, 0.0f)
			};
			
			vec3 boxMin = (b.center - b.halfSize).xyz();
			vec3 boxMax = (b.center + b.halfSize).xyz();
			
			for (int i = 0; i < 3; i++)
			{
				float minValue = 0.0f;
				float maxValue = 0.0f;
				projectToTriangle(boxNormals[i], minValue, maxValue);
				if ((minValue < boxMin[i]) || (maxValue > boxMax[i]))
					return false;
			}
			
			// Test the triangle normal
			float triangleOffset = t.averageNormal().dot(t.v[0]);
			float minValue = 0.0f;
			float maxValue = 0.0f;
			projectToBox(t.averageNormal(), minValue, maxValue);
			if ((maxValue < triangleOffset) || (minValue > triangleOffset))
				return false; // No intersection possible.
			
			vec4simd triangleEdges[] =
			{
				t.v[0] - t.v[1],
				t.v[1] - t.v[2],
				t.v[2] - t.v[0]
			};
			
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					// The box normals are the same as it's edge tangents
					float boxMinProj;
					float boxMaxProj;
					vec4simd axis = triangleEdges[i].crossXYZ(boxNormals[j]);
					projectToBox(axis, boxMinProj, boxMaxProj);
					projectToTriangle(axis, minValue, maxValue);
					if ((boxMaxProj < minValue)|| (boxMinProj > maxValue))
						return false; // No intersection possible
				}
			}
		
			return true;
		}
	}
}