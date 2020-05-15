#pragma once
#include <stdint.h>

namespace flash_storage
{

template<class T>
struct greater
{
	constexpr bool operator()(const T &lhs, const T &rhs) const
	{
	    return lhs > rhs;
	}
};

template<class T>
struct equal_to
{
	constexpr bool operator()(const T &lhs, const T &rhs) const
	{
	    return lhs == rhs;
	}
};

template<uint32_t Value>
struct BaseAddress
{};

template<uint32_t ... Values>
struct Sizes
{};

template<class, class>
struct Layout;

template<uint32_t Address, uint32_t ... Sectors>
struct Layout<BaseAddress<Address>, Sizes<Sectors...>>
{
	static constexpr size_t sector_count()
	{
		return sizeof...(Sectors);
	}

	template<size_t I>
	static constexpr uint32_t sector_offset()
	{
		static_assert(I < sector_count(), "");
		return
			Address +
			for_each_<greater<size_t>, I>(
				std::make_index_sequence<sector_count()>{});
	}

	template<size_t I>
	static constexpr uint32_t sector_size()
	{
		static_assert(I < sector_count(), "");
		return for_each_<equal_to<size_t>,I>(
			std::make_index_sequence<sector_count()>{});
	}

private:
	template<class Func, size_t I, size_t ... Is>
	static constexpr uint32_t for_each_(
		std::index_sequence<Is...> const&,
		uint32_t accu = 0)
	{
		using dummy_ = uint32_t[];
		return ( dummy_{accu+=(Sectors * (Func{}(I,Is) ? 1 : 0)) ... }, accu);
	}
};

} //namespace flash_storage
