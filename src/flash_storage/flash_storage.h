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

	template<class Tag>
	static bool update(type_of<Tag> const& value)
	{
		static_assert(
			DataValues::template contains<Tag>(),
			"T is not element of DataValues");

		type_of<Tag> old_value;

		if(Functions_t::read_value(
			index_of<Tag, DataValues>(),
			&old_value,
			sizeof(old_value)) == 0)
		{
			if(old_value == Tag::default_value())
			{
				return true;
			}
			else
			{
				return Functions_t::write_value(
					index_of<Tag, DataValues>(),
					&value,
					sizeof(value));
			}
		}
		else
		{
			if(is_updateable_(old_value, value))
			{
				Functions_t::update_value(
					index_of<Tag, DataValues>(),
					&value,
					sizeof(value));
				return true;
			}
			else
			{
				return Functions_t::write_value(
					index_of<Tag, DataValues>(),
					&value,
					sizeof(value));
			}
		}
	}

	static void init()
	{
		Functions_t::init();
	}

	static void format()
	{
		Functions_t::format();
	}

	template<class Visitor>
	static void for_each(Visitor && visitor)
	{
		for_each_(std::forward<Visitor>(visitor), DataValues{});
	}

private:
	template<class Visitor, class ... Tags>
	static void for_each_(Visitor && visitor, make_data_values<Tags...> const&)
	{
		using dummy_ = int[];
		(void)dummy_{0, (visitor(Tags{}, read<Tags>()), 0)...};
	}

	template<class T>
	static bool is_updateable_(T const& old_val, T const& new_val)
	{
		return is_updateable_(
			reinterpret_cast<uint8_t const*>(&old_val),
			reinterpret_cast<uint8_t const*>(&new_val),
			sizeof(old_val));
	}

	static bool is_updateable_(
		uint8_t const* old_val,
		uint8_t const* new_val,
		size_t size)
	{
		for(size_t i=0; i<size; ++i)
		{
			if((old_val[i] & new_val[i]) != new_val[i])
			{
				return false;
			}
		}

		return true;
	}
};


} //namespace flash_storage
