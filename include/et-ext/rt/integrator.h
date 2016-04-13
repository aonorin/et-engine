/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et-ext/rt/kdtree.h>
#include <et-ext/rt/environment.h>

namespace et
{
namespace rt
{
    class Integrator : public Object
    {
    public:
        ET_DECLARE_POINTER(Integrator)
        
    public:
        virtual ~Integrator() { }
        virtual float4 gather(const Ray& inRay, size_t depth, size_t& maxDepth,
            KDTree& tree, EnvironmentSampler::Pointer&, const Material::Collection&) = 0;
    };

	class NormalsIntegrator : public Integrator
	{
	public:
		ET_DECLARE_POINTER(NormalsIntegrator)

	public:
		float4 gather(const Ray& inRay, size_t depth, size_t& maxDepth,
			KDTree& tree, EnvironmentSampler::Pointer&, const Material::Collection&) override;
	};

	class FresnelIntegrator : public Integrator
	{
	public:
		ET_DECLARE_POINTER(FresnelIntegrator)

	public:
		float4 gather(const Ray& inRay, size_t depth, size_t& maxDepth,
			KDTree& tree, EnvironmentSampler::Pointer&, const Material::Collection&) override;
	};

    class AmbientOcclusionIntegrator : public Integrator
    {
    public:
        ET_DECLARE_POINTER(AmbientOcclusionIntegrator)
        
    public:
        float4 gather(const Ray& inRay, size_t depth, size_t& maxDepth,
            KDTree& tree, EnvironmentSampler::Pointer&, const Material::Collection&) override;
    };
    
    class PathTraceIntegrator : public Integrator
    {
    public:
        ET_DECLARE_POINTER(PathTraceIntegrator)
        
        enum
        {
            MaxTraverseDepth = 32
        };
        
    public:
        float4 gather(const Ray& inRay, size_t depth, size_t& maxDepth,
            KDTree& tree, EnvironmentSampler::Pointer&, const Material::Collection&) override;

		static float4 selectNextDirection(const float4& Wi, float4 N, const Material& mat, float4& color);
		static float_type brdf(const float4& Wi, const float4& Wo, const float4& N, const Material& mat);
    };
}
}
