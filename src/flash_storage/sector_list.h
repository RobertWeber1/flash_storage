#pragma once
#include <stdint.h>


namespace flash_storage
{


struct Slot
{
	uint8_t index;
	uint8_t sector_number;
	uint32_t address;
	uint32_t size;

	operator bool() const
	{
		return size != 0;
	}

	bool operator!=(Slot const& other)
	{
		return index != other.index;
	}
};


template<size_t Index, size_t SectorNr, uint32_t BaseAddress, uint32_t Size>
struct Sector
{
	static constexpr Slot slot()
	{
		return Slot{Index, SectorNr, BaseAddress, Size};
	}
};


template<class ... Sectors>
struct SectorList
{
	static const Slot slots[sizeof...(Sectors)+1];

	static constexpr size_t slot_count()
	{
		return sizeof...(Sectors);
	}

	static constexpr Slot const& first()
	{
		return slots[0];
	}

	static constexpr Slot const& next(Slot const& previous)
	{
		return slots[(previous.index+1)%slot_count()];
	}

	static constexpr Slot const& invalid()
	{
		return slots[slot_count()];
	}
};


template<class ... Sectors>
const Slot SectorList<Sectors...>::slots[] = {Sectors::slot()... , {0,0,0,0}};


////////////////////////////////////////////////////////////////////////////////


template<class Layout, class Slots, class Indices>
struct make_sector_helper_;


template<class Layout, size_t ... Slots, size_t ... Is>
struct make_sector_helper_<
	Layout,
	std::index_sequence<Slots...>,
	std::index_sequence<Is ...>>
{
	using type =
		SectorList<
			Sector<
				Slots,
				Is,
				Layout::template sector_offset<Is>(),
				Layout::template sector_size<Is>() > ...>;

};


template<class Layout, size_t ... Is>
using make_sector_list =
	typename make_sector_helper_<
		Layout,
		std::make_index_sequence<sizeof...(Is)>,
		std::index_sequence<Is...>>::type;


template<class Layout>
using make_full_sector_list =
	typename make_sector_helper_<
		Layout,
		std::make_index_sequence<Layout::sector_count()>,
		std::make_index_sequence<Layout::sector_count()>>::type;

} //namespace flash_storage
