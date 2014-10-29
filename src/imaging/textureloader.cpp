/*
 * This file is part of `et engine`
 * Copyright 2009-2014 by Sergey Reznik
 * Please, do not modify content without approval.
 *
 */

#include <et/opengl/opengl.h>
#include <et/imaging/textureloader.h>
#include <et/imaging/pngloader.h>
#include <et/imaging/ddsloader.h>
#include <et/imaging/pvrloader.h>
#include <et/imaging/hdrloader.h>
#include <et/imaging/jpegloader.h>

using namespace et;

TextureDescription::Pointer et::loadTextureDescription(const std::string& fileName, bool initWithZero)
{
	if (!fileExists(fileName))
		return TextureDescription::Pointer();

	TextureDescription::Pointer desc;
	
	std::string ext = getFileExt(fileName);
	lowercase(ext);
	
	if (ext == "png")
	{
		desc = TextureDescription::Pointer::create();
		desc->target = GL_TEXTURE_2D;
		desc->setOrigin(fileName);
		png::loadInfoFromFile(fileName, desc.reference());
	}
	else if (ext == "dds")
	{
		desc = TextureDescription::Pointer::create();
		desc->target = GL_TEXTURE_2D;
		desc->setOrigin(fileName);
		dds::loadInfoFromFile(fileName, desc.reference());
	}
	else if (ext == "pvr")
	{
		desc = TextureDescription::Pointer::create();
		desc->target = GL_TEXTURE_2D;
		desc->setOrigin(fileName);
		pvr::loadInfoFromFile(fileName, desc.reference());
	}
	else if (ext == "hdr")
	{
		desc = TextureDescription::Pointer::create();
		desc->target = GL_TEXTURE_2D;
		desc->setOrigin(fileName);
		hdr::loadInfoFromFile(fileName, desc.reference());
	}
	else if ((ext == "jpg") || (ext == "jpeg"))
	{
		desc = TextureDescription::Pointer::create();
		desc->target = GL_TEXTURE_2D;
		desc->setOrigin(fileName);
		jpeg::loadInfoFromFile(fileName, desc.reference());
	}
	
	if (desc.valid() && initWithZero)
	{
		desc->data = BinaryDataStorage(desc->dataSizeForAllMipLevels());
		desc->data.fill(0);
	}
	
	return desc;
}

TextureDescription::Pointer et::loadTexture(const std::string& fileName)
{
	if (!fileExists(fileName))
		return TextureDescription::Pointer();
	
	TextureDescription::Pointer desc;
	
	std::string ext = getFileExt(fileName);
	lowercase(ext);
	
	if (ext == "png")
	{
		desc = TextureDescription::Pointer::create();
		desc->target = GL_TEXTURE_2D;
		desc->setOrigin(fileName);
		png::loadFromFile(fileName, desc.reference(), true);
	}
	else if (ext == "dds")
	{
		desc = TextureDescription::Pointer::create();
		desc->target = GL_TEXTURE_2D;
		desc->setOrigin(fileName);
		dds::loadFromFile(fileName, desc.reference());
	}
	else if (ext == "pvr")
	{
		desc = TextureDescription::Pointer::create();
		desc->target = GL_TEXTURE_2D;
		desc->setOrigin(fileName);
		pvr::loadFromFile(fileName, desc.reference());
	}
	else if (ext == "hdr")
	{
		desc = TextureDescription::Pointer::create();
		desc->target = GL_TEXTURE_2D;
		desc->setOrigin(fileName);
		hdr::loadFromFile(fileName, desc.reference());
	}
	else if ((ext == "jpg") || (ext == "jpeg"))
	{
		desc = TextureDescription::Pointer::create();
		desc->target = GL_TEXTURE_2D;
		desc->setOrigin(fileName);
		jpeg::loadFromFile(fileName, desc.reference());
	}
	
	return desc;
}
