#pragma once
#include <flash_storage/version.h>
#include <flash_storage/data_value.h>
#include <flash_storage/layout.h>
#include <flash_storage/sector_list.h>


namespace flash_storage
{


template<
	class SectorList,
	class DataValues,
	class Flash,
	template<class, class, uint32_t, uint32_t>class Functions>
struct FlashStorage
{
	//TODOs:
	//[ ] check sum of data values + encoding overhead fits in smalles sector

	using Functions_t =
		Functions<
			SectorList,
			Flash,
			DataValues::id_count(),
			DataValues::max_byte_count()>;

	template<class Tag>
	static type_of<Tag, DataValues> read()
	{
		static_assert(
			DataValues::template contains<Tag>(),
			"T is not element of DataValues");

		type_of<Tag, DataValues> result;

		Functions_t::read_value(
			index_of<Tag, DataValues>(),
			&result,
			sizeof(result));

		return result;
	}

	template<class Tag>
	static bool write(type_of<Tag, DataValues> const& value)
	{
		static_assert(
			DataValues::template contains<Tag>(),
			"T is not element of DataValues");

		return Functions_t::write_value(
			index_of<Tag, DataValues>(),
			&value,
			sizeof(value));
	}

	static void init()
	{
		Functions_t::init();
	}
};


} //namespace flash_storage
