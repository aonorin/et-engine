/*
 * This file is part of `et engine`
 * Copyright 2009-2015 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/rt/kdtree.h>

namespace et
{
	class RaytracePrivate;
	class Raytrace
	{
	public:
		typedef std::function<void(const vec2i&, const vec4&)> OutputMethod;
		
		struct Options
		{
			size_t raysPerPixel = 32;
			size_t maxRecursionDepth = 8;
		};

	public:
		Raytrace();
		~Raytrace();

		template <typename F>
		void setOutputMethod(F func)
			{ _outputMethod = func; }

		void perform(s3d::Scene::Pointer, const Camera&, const vec2i&);
		vec4 performAtPoint(s3d::Scene::Pointer, const Camera&, const vec2i&, const vec2i&);

		void stop();
		void setOptions(const Options&);
		
		void renderSpacePartitioning();

	private:
		friend class RaytracePrivate;
		ET_DECLARE_PIMPL(Raytrace, 2048);
		OutputMethod _outputMethod;
	};
}
