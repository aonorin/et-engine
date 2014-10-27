/*
 * This file is part of `et engine`
 * Copyright 2009-2014 by Sergey Reznik
 * Please, do not modify content without approval.
 *
 */

#pragma once

namespace et
{
	enum StreamMode
	{
		StreamMode_Text,
		StreamMode_Binary
	};

	class InputStreamPrivate;
	class InputStream : public Shared
	{
	public:
		ET_DECLARE_POINTER(InputStream)
		
	public:
		InputStream();
		InputStream(const std::string& file, StreamMode mode);

		~InputStream();

		bool valid();
		bool invalid();

		std::istream& stream();

	private:
		InputStreamPrivate* _private = nullptr;
		char _privateData[16];
	};
}
