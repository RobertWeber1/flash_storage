#pragma once
#include <flash_storage/static_vector.h>
#include <flash_storage/sector_list.h>
#include <flash_storage/encoding.h>
#include <string.h>


namespace flash_storage
{

namespace detail
{


template<
	class SectorList,
	class Flash,
	uint32_t IdCount,
	uint32_t MaxDataSize>
struct CachedAccess
{
	using Self_t =
		CachedAccess<
			SectorList,
			Flash,
			IdCount,
			MaxDataSize>;

	static constexpr uint8_t type_bits() { return required_bits(IdCount); }
	static constexpr uint8_t size_bits() { return required_bits(MaxDataSize); }

	using Id_t = rep_type<required_bytes(type_bits())>;
	using Encoding_t = Encoding<type_bits(), size_bits()>;
	using Sectors_t = StaticVector<Slot, SectorList::slot_count()>;

	static bool is_erased(Slot const& slot)
	{
		return read_flash<SectorState>(slot.address) == SectorState::Erased;
	}

	static bool is_receiving(Slot const& slot)
	{
		return read_flash<SectorState>(slot.address) == SectorState::ReceiveData;
	}

	static bool is_valid(Slot const& slot)
	{
		return read_flash<SectorState>(slot.address) == SectorState::Valid;
	}

	static bool is_invalid(Slot const& slot)
	{
		auto const state = read_flash<SectorState>(slot.address);

		return
			state != SectorState::Erased and
			state != SectorState::ReceiveData and
			state != SectorState::Valid;
	}

	template<class Pred>
	static Sectors_t get_sectors(Pred pred)
	{
		Sectors_t result;

		for_each<SectorList>(
			[&result, &pred](Slot const& slot)
			{
				if(pred(slot))
				{
					result.push_back(slot);
				}
			});

		return result;
	}


	static void init()
	{
		Flash::unlock();

		auto const valid_sectors = get_sectors(&Self_t::is_valid);
		auto const transfering_sectors = get_sectors(&Self_t::is_receiving);

		if(valid_sectors.empty() and transfering_sectors.size() == 1)
		{
			set(transfering_sectors[0], SectorState::Valid);
		}
		else if(valid_sectors.size() == 1 and transfering_sectors.size() == 1)
		{
			transfer_data(valid_sectors[0], transfering_sectors[0]);
		}
		else if(valid_sectors.size() == 1 and transfering_sectors.empty())
		{
			auto state = read_flash<SectorState>(valid_sectors[0].address);

			if(state.type_bits() != type_bits() or
			   state.size_bits() != size_bits())
			{
				format();
			}
		}
		else
		{
			format();
		}

		for_each<SectorList>(
			[](Slot const& slot)
			{
				if(is_invalid(slot) or is_erased(slot))
				{
					if(not check_fully_erased(slot))
					{
						Flash::erase(slot.sector_number);
					}
				}
			});

		set_current_slot(current_read_sector());
	}

	static void* read_value(
		Id_t id,
		void* dst,
		uint32_t byte_count,
		void const* default_value)
	{
		if(read_value(id, dst, byte_count, start_of_data_) == 0)
		{
			::memcpy(dst, default_value, byte_count);
		}

		return dst;
	}

	static uint32_t read_value(Id_t id, void* dst, uint32_t byte_count)
	{
		return read_value(id, dst, byte_count, start_of_data_);
	}

	static uint32_t read_value(
		Id_t id,
		void* dst,
		uint32_t byte_count,
		uint32_t address)
	{
		while(address > smallest_possible_addess_)
		{
			auto const entry = read_flash<Encoding_t>(address);
			auto begin = address - entry.size();

			if(entry.type() == id)
			{
				auto const bytes_to_read =
					std::min<uint32_t>(byte_count, entry.size());

				Flash::read(begin, dst, bytes_to_read);

				return bytes_to_read;
			}
			else
			{
				address = begin - sizeof(Encoding_t);
			}
		}

		return 0;
	}

	static bool write_value(
		Id_t id,
		void const* src,
		uint32_t byte_count)
	{
		auto address = start_of_data_ + sizeof(Encoding_t);

		auto const occupied = (address - current_slot_.address);

		if(current_slot_.size - occupied < byte_count + sizeof(Encoding_t))
		{
			address = transfer_data(current_slot_, SectorList::next(current_slot_), id);
		}

		write_value_(address, byte_count, src, id);

		return true;
	}

	static bool write_value(
		Id_t id,
		void const* src,
		uint32_t byte_count,
		void * buffer,
		void const* default_value)
	{
		if(read_value(id, buffer, byte_count, start_of_data_) == 0)
		{
			// puts("write_value: value not in storage");
			if(::memcmp(default_value, src, byte_count) == 0)
			{
				// puts("    is equal to default value -> nothing to do");
				return true;
			}
			else
			{
				// puts("    is NOT equal to default value -> write new entry");
				return write_value(id, src, byte_count);
			}
		}
		else
		{
			// puts("write_value: value already in storage");
			if(is_updateable_(
				reinterpret_cast<uint8_t const*>(src),
				reinterpret_cast<uint8_t const*>(buffer),
				byte_count))
			{
				// puts("    update possible -> update old value");
				update_value(id, src, byte_count, start_of_data_);
				return true;
			}
			else
			{
				// puts("    update NOT possible -> write new entry");
				return write_value(id, src, byte_count);
			}
		}
	}

	static void update_value(Id_t id, void const* src, uint32_t byte_count)
	{
		update_value(id, src, byte_count, start_of_data_);
	}

	static void update_value(
		Id_t id,
		void const* src,
		uint32_t byte_count,
		uint32_t address)
	{
		while(address > smallest_possible_addess_)
		{
			auto const entry = read_flash<Encoding_t>(address);
			auto begin = address - entry.size();

			if(entry.type() == id)
			{
				write_value_(begin, byte_count, src, id);
				return;
			}
			else
			{
				address = begin - sizeof(Encoding_t);
			}
		}
	}

	static bool equal(
		uint8_t const* lhs,
		size_t lhs_size,
		uint8_t const* rhs,
		size_t rhs_size)
	{
		if(lhs_size != rhs_size)
		{
			return false;
		}

		for(size_t i=0; i<lhs_size; ++i)
		{
			if(lhs[i] != rhs[i])
			{
				return false;
			}
		}

		return true;
	}

	static uint32_t transfer_data(
		Slot const& src,
		Slot const& dst,
		Id_t spare = IdCount)
	{
		if(not check_fully_erased(dst))
		{
			Flash::erase(dst.sector_number);
		}

		set(dst, SectorState::ReceiveData);

		auto address = dst.address + sizeof(SectorState);

		for(Id_t id = 0; id < IdCount; ++id)
		{
			if(id == spare)
			{
				continue;
			}
			uint8_t buffer[MaxDataSize];
			uint32_t size = read_value(id, buffer, MaxDataSize, src);

			if(size == 0)
			{
				continue;
			}

			uint8_t buffer_dst[MaxDataSize];
			uint32_t size_dst = read_value(id, buffer_dst, MaxDataSize, dst);

			if(equal(buffer, size, buffer_dst, size_dst))
			{
				continue;
			}

			write_value_(address, size, buffer, id);

			address += size + sizeof(Encoding_t);
		}

		Flash::erase(src.sector_number);

		set(dst, SectorState::Valid);

		set_current_slot(dst);

		return address;
	}

	static void set(Slot const& slot, SectorState::State state)
	{
		write_flash(
			slot.address,
			SectorState(state, type_bits(), size_bits()));
	}

	static uint32_t find_last_entry(Slot const& slot)
	{
		uint32_t address = slot.address + slot.size - sizeof(Encoding_t);
		auto const first_addr = slot.address + sizeof(SectorState);
		do
		{
			if(not read_flash<Encoding_t>(address).is_erased())
			{
				return address;
			}

			address -= sizeof(Encoding_t);
		}
		while(address >= first_addr);

		return address;
	}

	static Slot current_read_sector()
	{
		const auto first = SectorList::first();
		auto current = first;

		do
		{
			if(is_valid(current))
			{
				return current;
			}
		}
		while((current = SectorList::next(current)) != first);

		return SectorList::invalid();
	}

	static Slot current_write_sector()
	{
		const auto first = SectorList::first();
		auto current = first;
		do
		{
			if(is_receiving(current))
			{
				return current;
			}
		}
		while((current = SectorList::next(current)) != first);

		return current_read_sector();
	}

	static bool check_fully_erased(Slot const& slot)
	{
		auto const end = slot.address + slot.size;

		for(uint32_t addr = slot.address; addr < end; addr += 4)
		{
			if(read_flash<uint32_t>(addr) != 0xffffffff)
			{
				return false;
			}
		}

		return true;
	}

	static void format()
	{
		const auto first = SectorList::first();

		if(not check_fully_erased(first))
		{
			Flash::erase(first.sector_number);
		}

		set(first, SectorState::Valid);

		auto current = first;
		while((current = SectorList::next(current)) != first)
		{
			if(not check_fully_erased(current))
			{
				Flash::erase(current.sector_number);
			}
		}
	}

private:
	static void set_current_slot(Slot slot)
	{
		current_slot_ = slot;
		start_of_data_ = find_last_entry(slot);
		smallest_possible_addess_ = slot.address + sizeof(SectorState);
	}

	static void write_value_(
		uint32_t address,
		uint32_t byte_count,
		void const* src,
		Id_t id)
	{
		auto const entry_addr = address + byte_count;

		write_flash(entry_addr, Encoding_t(byte_count));

		Flash::write(address, src, byte_count);

		write_flash(entry_addr, Encoding_t(byte_count, id));

		start_of_data_ = entry_addr;
	}

	template<class T>
	static T read_flash(uint32_t address)
	{
		typename std::aligned_storage<sizeof(T), alignof(T)>::type data;
		Flash::read(address, &data, sizeof(T));
		return reinterpret_cast<T&>(data);
	}

	template<class T>
	static void write_flash(uint32_t address, T const& data)
	{
		Flash::write(address, &data, sizeof(T));
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
private:
	static Slot current_slot_;
	static uint32_t start_of_data_;
	static uint32_t smallest_possible_addess_;
};


template<class SectorList, class Flash, uint32_t IdCount, uint32_t MaxDataSize>
Slot CachedAccess<SectorList, Flash, IdCount, MaxDataSize>::current_slot_;

template<class SectorList, class Flash, uint32_t IdCount, uint32_t MaxDataSize>
uint32_t CachedAccess<SectorList, Flash, IdCount, MaxDataSize>::start_of_data_;

template<class SectorList, class Flash, uint32_t IdCount, uint32_t MaxDataSize>
uint32_t CachedAccess<SectorList, Flash, IdCount, MaxDataSize>::smallest_possible_addess_;

} //namespace detail

} //namespace flash_storage
