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
	using Functions_t =
		Functions<
			SectorList,
			Flash,
			DataValues::id_count(),
			DataValues::max_byte_count()>;

	template<class Tag>
	static type_of<Tag> read_or(
		type_of<Tag> const& default_value)
	{
		static_assert(
			DataValues::template contains<Tag>(),
			"T is not element of DataValues");

		type_of<Tag> result;

		if(Functions_t::read_value(
			index_of<Tag, DataValues>(),
			&result,
			sizeof(result)) == 0)
		{
			result = default_value;
		}

		return result;
	}

	template<class Tag>
	static type_of<Tag> read()
	{
		static_assert(
			DataValues::template contains<Tag>(),
			"T is not element of DataValues");

		type_of<Tag> result;

		if(Functions_t::read_value(
			index_of<Tag, DataValues>(),
			&result,
			sizeof(result)) == 0)
		{
			result = Tag::default_value();
		}

		return result;
	}

	template<class Tag>
	static bool write(type_of<Tag> const& value)
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
