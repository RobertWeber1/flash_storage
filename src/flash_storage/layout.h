#pragma once
#include <stdint.h>

namespace flash_storage
{

template<uint32_t BaseAddress, size_t ... Sectors>
struct Layout
{
	static constexpr size_t sector_count()
	{
		return sizeof...(Sectors);
	}

	template<size_t I>
	static constexpr uint32_t sector_offset()
	{
		static_assert(I < sector_count(), "");
		return offset_<I>(std::make_index_sequence<sector_count()>{});
	}

	template<size_t I>
	static constexpr size_t sector_size()
	{
		static_assert(I < sector_count(), "");
		return size_<I>(std::make_index_sequence<sector_count()>{});
	}

private:
	template<size_t I, size_t ... Is>
	static constexpr uint32_t offset_(std::index_sequence<Is...> const&)
	{
		return BaseAddress + ( (Sectors*((I>Is)?1:0)) + ... );
	}

	template<size_t I, size_t ... Is>
	static constexpr size_t size_(std::index_sequence<Is...> const&)
	{
		return ( (Sectors*((I==Is)?1:0)) + ... );
	}
};

} //namespace flash_storage
