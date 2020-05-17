#pragma once
#include <flash_storage/sector_list.h>
#include <flash_storage/encoding.h>

namespace flash_storage
{

namespace functions
{


template<
	class SectorList,
	class Flash,
	uint8_t TypeBits,
	uint8_t SizeBits,
	uint32_t IdCount,
	uint32_t MaxDataSize = 8>
struct MinRamMaxRuntime
{
	using Self_t =
		MinRamMaxRuntime<
			SectorList,
			Flash,
			TypeBits,
			SizeBits,
			IdCount,
			MaxDataSize>;
	using Id_t = rep_type<required_bytes(TypeBits)>;
	using Encoding_t = Encoding<TypeBits, SizeBits>;

	static uint32_t read_value(Id_t id, void* dst, uint32_t byte_count)
	{
		const auto slot = current_read_sector();

		if(not slot)
		{
			return 0;
		}

		auto address = find_last_enty(slot);
		auto smallest_possible_addess = slot.address + sizeof(SectorState);

		if(address == smallest_possible_addess)
		{
			return 0;
		}

		do
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
				address = begin - 1;
			}
		}
		while(address > smallest_possible_addess);

		return 0;
	}

	static bool write_value(Id_t id, void* src, uint32_t byte_count)
	{
		const auto slot = current_write_sector();

		if(not slot)
		{
			return false;
		}

		auto address = find_last_enty(slot) + sizeof(Encoding_t);

		auto const occupied = (address - slot.address);

		if(slot.size - occupied < byte_count + sizeof(Encoding_t))
		{
			address = transfer_data(slot, SectorList::next(slot), id);
		}

		write_value_(address, byte_count, src, id);

		return true;
	}

	static uint32_t transfer_data(Slot const& from, Slot const& to, Id_t spare)
	{
		if(not check_fully_erased(to))
		{
			Flash::erase(to.sector_number);
		}

		write_flash(
			to.address,
			SectorState(SectorState::ReceiveData, TypeBits, SizeBits));

		auto address = to.address + sizeof(SectorState);

		for(Id_t id = 0; id < IdCount; ++id)
		{
			if(id == spare)
			{
				continue;
			}

			uint8_t buffer[MaxDataSize];
			uint32_t size = read_value(id, buffer, from.size);

			if(size == 0)
			{
				continue;
			}

			write_value_(address, size, buffer, id);

			address += size + sizeof(Encoding_t);
		}

		Flash::erase(from.sector_number);

		write_flash(
			to.address,
			SectorState(SectorState::Valid, TypeBits, SizeBits));

		return address;
	}

	static uint32_t find_last_enty(Slot const& slot)
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

		write_flash(
			first.address,
			SectorState(SectorState::Valid, TypeBits, SizeBits));

		auto current = first;
		while((current = SectorList::next(current)) != first)
		{
			if(not check_fully_erased(current))
			{
				Flash::erase(current.sector_number);
			}
		}
	}

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

private:
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
	}

	template<class T>
	static T read_flash(uint32_t address)
	{
		return Flash::template read<T>(address);
	}

	template<class T>
	static void write_flash(uint32_t address, T const& data)
	{
		Flash::template write<T>(address, data);
	}
};


} //namespace functions

} //namespace flash_storage
