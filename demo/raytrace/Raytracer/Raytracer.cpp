//
//  Raytracer.cpp
//  Raytracer
//
//  Created by Sergey Reznik on 27/6/2014.
//  Copyright (c) 2014 Cheetek. All rights reserved.
//

#include <et/collision/collision.h>
#include "Raytracer.h"

using namespace rt;
using namespace et;

vec3 randomVectorOnHemisphere(const vec3& base, float distribution);

vec3 randomDiffuseVector(const vec3& normal);
vec3 randomReflectedVector(const vec3& incidence, const vec3& normal, float roughness);
vec3 randomRefractedVector(const vec3& incidence, const vec3& normal, float eta, float k, float roughness);
vec3 refract(const vec3& incidence, const vec3& normal, float eta, float k);

float calculateRefractiveCoefficient(const vec3& incidence, const vec3& normal, float eta);
float computeFresnelTerm(const vec3& incidence, const vec3& normal, float indexOfRefraction);
float phong(const vec3& incidence, const vec3& reflected, const vec3& normal, float exponent);

vec4 gatherBouncesRecursive(const RaytraceScene& scene, const ray3d& ray, int depth, int currentObject);
vec4 performRaytracing(const RaytraceScene& scene, const ray3d& ray);
vec4 sampleEnvironmentColor(const RaytraceScene& scene, const ray3d& r);

const float defaultRefractiveIndex = 1.0f;

Intersection findNearestIntersection(const RaytraceScene& scene, const ray3d& ray)
{
	Intersection result;
	
	int index = 0;
	vec3 hitPoint;
	vec3 hitNormal;
	
	float distance = std::numeric_limits<float>::max();
	for (const auto& obj : scene.objects)
	{
		if (obj.intersectsRay(ray, hitPoint, hitNormal))
		{
			float d = (hitPoint - ray.origin).dotSelf();
			if (d < distance)
			{
				distance = d;
				result = index;
				result.hitPoint = hitPoint;
				result.hitNormal = hitNormal;
			}
		}
		++index;
	}
	
	return result;
}

vec2 anglesToTexCoord(const vec2& angles)
{
	return vec2(angles.x / DOUBLE_PI + 0.5f, angles.y / PI + 0.5f);
}

vec4 sampleTexture(const TextureDescription::Pointer& tex, vec2i texCoord)
{
	if (texCoord.x >= tex->size.x) texCoord.x -= tex->size.x;
	if (texCoord.x < 0) texCoord.x += tex->size.x;
	
	if (texCoord.y >= tex->size.y) texCoord.y -= tex->size.y;
	if (texCoord.y < 0) texCoord.y += tex->size.y;
	
	const vec4* colors = reinterpret_cast<const vec4*>(tex->data.binary());
	return colors[texCoord.x + texCoord.y * tex->size.x];
}

vec4 sampleEnvironmentColor(const RaytraceScene& scene, const ray3d& r)
{
	if (scene.environmentMap.invalid())
		return scene.ambientColor;
	
	if (scene.environmentMap->bitsPerPixel != 128)
		exit(3);
	
	vec3 sp = toSpherical(r.direction);
	vec2 tc = anglesToTexCoord(sp.xy()) * vector2ToFloat(scene.environmentMap->size);
	vec2 dudv = tc - floorv(tc);
	
	vec2i baseTexCoord(static_cast<int>(tc.x), static_cast<int>(tc.y));
	
	vec4 c00 = sampleTexture(scene.environmentMap, baseTexCoord);
	vec4 c10 = sampleTexture(scene.environmentMap, baseTexCoord + vec2i(1, 0));
	vec4 c01 = sampleTexture(scene.environmentMap, baseTexCoord + vec2i(0, 1));
	vec4 c11 = sampleTexture(scene.environmentMap, baseTexCoord + vec2i(1, 1));
	
	vec4 topRow = mix(c00, c10, dudv.x);
	vec4 bottomRow = mix(c01, c11, dudv.x);
	
	return scene.ambientColor * mix(topRow, bottomRow, dudv.y);
}

vec4 gatherBouncesRecursive(const RaytraceScene& scene, const ray3d& ray, int depth, int currentObject)
{
	if (depth >= scene.options.bounces)
		return vec4(0.0f, 0.0f, 0.0f, 0.0f);
	
	auto i = findNearestIntersection(scene, ray);
	if (i.objectIndex == Intersection::missingObject)
		return sampleEnvironmentColor(scene, ray);
	
	const auto& obj = scene.objectAtIndex(i.objectIndex);
	const SceneMaterial& mat = scene.materialAtIndex(obj.materialId);
	
	if (mat.refractiveIndex > 0.0f)
	{
		bool enteringObject = currentObject == Intersection::missingObject;
		vec3 targetNormal = enteringObject ? i.hitNormal : -i.hitNormal;
		float eta = enteringObject ? (defaultRefractiveIndex / mat.refractiveIndex) : (mat.refractiveIndex / defaultRefractiveIndex);
		float k = calculateRefractiveCoefficient(ray.direction, targetNormal, eta);
		if (k < 0.0f) // total internal reflection
		{
			vec3 reflectedRay = randomReflectedVector(ray.direction, targetNormal, mat.roughness);
			return mat.emissiveColor + dot(reflectedRay, reflect(ray.direction, targetNormal)) *
			(mat.reflectiveColor * gatherBouncesRecursive(scene, ray3d(i.hitPoint, reflectedRay), depth + 1, currentObject));
		}
		else
		{
			float fresnel = computeFresnelTerm(ray.direction, targetNormal, eta);
			if (randomFloat(0.0f, 1.0f) <= fresnel)
			{
				vec3 reflectedRay = randomReflectedVector(ray.direction, targetNormal, mat.roughness);
				return mat.emissiveColor + dot(reflectedRay, reflect(ray.direction, targetNormal)) *
					(mat.reflectiveColor * gatherBouncesRecursive(scene, ray3d(i.hitPoint, reflectedRay), depth + 1, currentObject));
			}
			else
			{
				vec3 refractedRay = randomRefractedVector(ray.direction, targetNormal, eta, k, mat.roughness);
				return mat.emissiveColor + dot(refractedRay, refract(ray.direction, targetNormal, eta, k)) *
					(mat.diffuseColor * gatherBouncesRecursive(scene, ray3d(i.hitPoint, refractedRay), depth + 1, enteringObject ? i.objectIndex : Intersection::missingObject));
			}
		}
	}
	else if (randomFloat(0.0f, 1.0f) > mat.roughness)
	{
		vec3 reflectedRay = randomReflectedVector(ray.direction, i.hitNormal, mat.roughness);
		float scale = dot(reflectedRay, reflect(ray.direction, i.hitNormal));
		
		return mat.emissiveColor +
			scale * (mat.reflectiveColor * gatherBouncesRecursive(scene, ray3d(i.hitPoint, reflectedRay), depth + 1, currentObject));
	}
	else
	{
		vec3 newDirection = randomDiffuseVector(i.hitNormal);
		float scale = dot(newDirection, i.hitNormal);
		return mat.emissiveColor +
			scale * (mat.diffuseColor * gatherBouncesRecursive(scene, ray3d(i.hitPoint, newDirection), depth + 1, currentObject));
	}
	
	return vec4(0.0f);
}

void rt::raytrace(const RaytraceScene& scene, const et::vec2i& imageSize, const et::vec2i& origin,
	const et::vec2i& size, bool aa, OutputFunction output)
{
	vec2i pixel = origin;
	vec2 dudv = vec2(2.0f) / vector2ToFloat(imageSize);
	vec2 subPixel = 0.5f * dudv;
	
	ray3d centerRay = scene.camera.castRay(vec2(0.0f));
	vec3 ce1 = cross(centerRay.direction, centerRay.direction.x > 0.1f ? unitY : unitX).normalized();
	vec3 ce2 = cross(ce1, centerRay.direction).normalized();
	
	auto it = findNearestIntersection(scene, centerRay);
	
	float deltaAngleForAppertureBlades = DOUBLE_PI / static_cast<float>(scene.apertureBlades);
	float initialAngleForAppertureBlades = 0.5f * deltaAngleForAppertureBlades;
	float focalDistance = length(it.hitPoint - centerRay.origin);
	plane focalPlane(scene.camera.direction(), (scene.camera.position() - scene.camera.direction() * focalDistance).length());
 
	for (pixel.y = origin.y; pixel.y < origin.y + size.y; ++pixel.y)
	{
		pixel.x = origin.x;
		output(pixel, vec4(1.0f, 0.0f, 0.0f, 1.0f));
		pixel.x = origin.x + size.x - 1;
		output(pixel, vec4(1.0f, 0.0f, 0.0f, 1.0f));
	}

	for (pixel.x = origin.x; pixel.x < origin.x + size.x; ++pixel.x)
	{
		pixel.y = origin.y;
		output(pixel, vec4(1.0f, 0.0f, 0.0f, 1.0f));
		pixel.y = origin.y + size.y - 1;
		output(pixel, vec4(1.0f, 0.0f, 0.0f, 1.0f));
	}
	
	for (pixel.y = origin.y; pixel.y < origin.y + size.y; ++pixel.y)
	{
		for (pixel.x = origin.x; pixel.x < origin.x + size.x; ++pixel.x)
		{
			vec4 result;
			
			for (int sample = 0; sample < scene.options.samples; ++sample)
			{
				vec2 fpixel = (vector2ToFloat(pixel) + vec2(0.5f)) * dudv - vec2(1.0f);
				ray3d r = scene.camera.castRay(fpixel + subPixel * vec2(randomFloat(-1.0f, 1.0f), randomFloat(-1.0f, 1.0f)));
				
				if (scene.apertureSize > 0.0f)
				{
					vec3 focal;
					intersect::rayPlane(r, focalPlane, &focal);

					float ra1 = initialAngleForAppertureBlades + static_cast<float>(randomInteger(scene.apertureBlades)) * deltaAngleForAppertureBlades;
					float ra2 = ra1 + deltaAngleForAppertureBlades;
					float rd = scene.apertureSize * std::sqrt(randomFloat(0.0f, 1.0f));
					
					vec3 o1 = rd * (ce1 * std::sin(ra1) + ce2 * std::cos(ra1));
					vec3 o2 = rd * (ce1 * std::sin(ra2) + ce2 * std::cos(ra2));
					vec3 cameraJitter = r.origin + mix(o1, o2, randomFloat(0.0f, 1.0f));
					result += gatherBouncesRecursive(scene, ray3d(cameraJitter, normalize(focal - cameraJitter)), 0, Intersection::missingObject);
				}
				else
				{
					result += gatherBouncesRecursive(scene, r, 0, Intersection::missingObject);
				}
			}
			
			result *= -scene.options.exposure / static_cast<float>(scene.options.samples);
			result = vec4(1.0f) - vec4(std::exp(result.x), std::exp(result.y), std::exp(result.z), 0.0f);
			
			output(pixel, result);
		}
	}
}

/*
 * Service
 */

vec3 randomVectorOnHemisphere(const vec3& w, float distribution)
{
	float cr1 = 0.0f;
	float sr1 = 0.0f;

	float r2 = randomFloat(0.0f, distribution);
	float ra = randomFloat(-PI, PI);

#if (ET_PLATFORM_WIN)
	cr1 = std::cos(ra);
	sr1 = std::sin(ra);
#else
	__sincosf(ra, &sr1, &cr1);
#endif

	vec3 u = cross((std::abs(w.x) > 0.1f) ? unitY : unitX, w).normalized();
	return (u * cr1 + cross(w, u) * sr1) * std::sqrt(r2) + w * std::sqrt(1.0f - r2);
}

vec3 randomDiffuseVector(const vec3& normal)
{
	return randomVectorOnHemisphere(normal, 1.0f);
}

vec3 randomReflectedVector(const vec3& incidence, const vec3& normal, float roughness)
{
	return randomVectorOnHemisphere(reflect(incidence, normal), std::sin(HALF_PI * roughness));
}

vec3 refract(const vec3& incidence, const vec3& normal, float eta, float k)
{
	return eta * incidence - (eta * dot(normal, incidence) + std::sqrt(k)) * normal;
}

vec3 randomRefractedVector(const vec3& incidence, const vec3& normal, float eta, float k, float roughness)
{
	if (k < 0.0f)
		exit(1);
	
	return randomVectorOnHemisphere(refract(incidence, normal, eta, k), std::sin(HALF_PI * roughness));
}

float calculateRefractiveCoefficient(const vec3& incidence, const vec3& normal, float eta)
{
	return 1.0f - sqr(eta) * (1.0f - sqr(dot(normal, incidence)));
}

float phong(const vec3& incidence, const vec3& reflected, const vec3& normal, float exponent)
{
	float s = dot(reflect(incidence, normal), reflected);
	return (s <= 0.0f) ? 0.0f : std::pow(s, exponent);
}

float computeFresnelTerm(const vec3& incidence, const vec3& normal, float indexOfRefraction)
{
	float eta = indexOfRefraction * dot(incidence, normal);
	float eta2 = eta * eta;
	float beta = 1.0f - indexOfRefraction * indexOfRefraction;
	float result = 1.0f + 2.0f * (eta2 + eta * sqrt(beta + eta2)) / beta;
	return clamp(result * result, 0.0f, 1.0f);
}
