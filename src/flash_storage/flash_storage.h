#pragma once

namespace flash_storage
{


template<class SectorList, class DataValues, class Functions>
struct FlashStorage
{
	template<class Tag>
	static type_of<Tag, DataValues> const& read() const
	{
		static_assert(
			DataValues::template contains<T>(),
			"T is not element of DataValues");

		type_of<Tag, DataValues> result;


		return result;
	}

	template<class T>
	static void write(type_of<Tag, DataValues> const& value)
	{
		static_assert(
			DataValues::template contains<T>(),
			"T is not element of DataValues");
	}
};


} //namespace flash_storage
