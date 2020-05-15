#pragma once
#include <stdint.h>

namespace flash_storage
{

struct Slot
{
	uint8_t index;
	uint32_t address;
	uint32_t size;
};

template<size_t Index, uint32_t BaseAddress, uint32_t Size>
struct Sector
{
	static constexpr Slot slot()
	{
		return Slot{Index, BaseAddress, Size};
	}
};


template<class ... Sectors>
struct SectorList
{
private:
	// struct Helper : Sectors ...
	// {};

	// template<size_t I, uint32_t Address, uint32_t Size>
	// static constexpr Sector<I, Address, Size> slot_(Sector<I, Address, Size>);

	static const Slot slots[sizeof...(Sectors)];

public:
	static constexpr size_t slot_count()
	{
		return sizeof...(Sectors);
	}

	// template<size_t I>
	// using slot = decltype(slot_<I>(Helper{}));

	static constexpr Slot first()
	{
		return slots[0];
	}

	static constexpr Slot next(Slot const& previous)
	{
		return slots[(previous.index+1)%slot_count()];
	}
};

template<class ... Sectors>
const Slot SectorList<Sectors...>::slots[] = {Sectors::slot() ... };

template<class Layout, class Slots, size_t ... Is>
struct make_sector_helper_;


template<class Layout, size_t ... Slots, size_t ... Is>
struct make_sector_helper_<Layout, std::index_sequence<Slots...>, Is ...>
{
	using type =
		SectorList<
			Sector<
				Slots,
				Layout::template sector_offset<Is>(),
				Layout::template sector_size<Is>() > ...>;

};


template<class Layout, size_t ... Is>
using make_sector_list =
	typename make_sector_helper_<
		Layout,
		std::make_index_sequence<sizeof...(Is)>,
		Is...>::type;


} //namespace flash_storage
