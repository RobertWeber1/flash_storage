#pragma once
#include <flash_storage/version.h>
#include <flash_storage/data_value.h>
#include <flash_storage/layout.h>
#include <flash_storage/sector_list.h>

#include <type_traits>

namespace flash_storage
{

namespace detail
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
	static type_of<Tag> read(
		type_of<Tag> const& default_value = Tag::default_value())
	{
		return *reinterpret_cast<type_of<Tag>*>(
			Functions_t::read_value(
				flash_storage::index_of<Tag, DataValues>(),
				reinterpret_cast<type_of<Tag>*>(&buffer_),
				sizeof(type_of<Tag>),
				reinterpret_cast<void const*>(&default_value)));
	}

	template<class Tag>
	static bool write(
		type_of<Tag> const& value,
		type_of<Tag> const& default_value = Tag::default_value())
	{
		return Functions_t::write_value(
			flash_storage::index_of<Tag, DataValues>(),
			&value,
			sizeof(value),
			reinterpret_cast<void*>(&buffer_),
			reinterpret_cast<void const*>(&default_value));
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

	using Buffer_t =
		typename std::aligned_storage<
			DataValues::max_byte_count(),
			DataValues::max_alignment()>::type;


	static Buffer_t buffer_;
};


template<
	class S,
	class D,
	class F,
	template<class, class, uint32_t, uint32_t>class Fun>
typename FlashStorage<S, D, F, Fun>::Buffer_t
	FlashStorage<S, D, F, Fun>::buffer_;

} //namespace detail

} //namespace flash_storage
