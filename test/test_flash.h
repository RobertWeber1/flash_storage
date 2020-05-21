#pragma once
#include <array>
#include <cstring>

namespace flash_storage
{


template<uint32_t Size>
struct TestMemory
{
	using storage_t = std::array<uint8_t, Size>;

	storage_t storage;

	static constexpr size_t size()
	{
		return Size;
	}

	TestMemory()
	{
		init();
	}

	void init()
	{
		std::fill(storage.begin(), storage.end(), 0xff);
	}

	uint8_t* operator[](int addr)
	{
		return &storage[addr];
	}
};


template<class Layout>
struct TestFlash
{
////////////////////////////////////////////////////////////////////////////////
//public interface used by flash_storage/functions/xyz.h
//implement this functions target flash
	static void read(uint32_t address, void* dst, uint32_t byte_count)
	{
		check(address, byte_count);
		auto const src = memory[address-base_address];
		std::memcpy(dst, src, byte_count);
	}


	static void write(uint32_t address, void const* data, uint32_t byte_count)
	{
		check(address, byte_count);

		auto addr = memory[address-base_address];
		uint8_t const* src = reinterpret_cast<uint8_t const*>(data);

		for(uint8_t* dst = addr; dst<(addr+byte_count); ++dst, ++src)
		{
			*dst = *dst & *src;
		}
	}

	static void erase(uint32_t slot_index)
	{
		check_slot(slot_index);

		auto start = memory[sectors::slots[slot_index].address - base_address];
		auto end = start + sectors::slots[slot_index].size;

		while(start < end)
		{
			*start = 0xff;
			++start;
		}

		++erase_counter_[slot_index];
	}

////////////////////////////////////////////////////////////////////////////////
//stuff used by unit tests
	using sectors = make_full_sector_list<Layout>;

	static constexpr uint32_t base_address = Layout::base_address();
	static uint32_t erase_counter_[sectors::slot_count()];
	static TestMemory<Layout::size()> memory;


	template<class T>
	static T read(uint32_t address)
	{
		typename std::aligned_storage<sizeof(T), alignof(T)>::type data;
		read(address, &data, sizeof(T));
		return reinterpret_cast<T&>(data);
	}

	template<class T>
	static void write(uint32_t address, T const& data)
	{
		write(address, &data, sizeof(T));
	}

	static void init()
	{
		memory.init();
		std::memset(erase_counter_, 0, sizeof(erase_counter_));
	}

	static uint32_t erase_counter(size_t index)
	{
		check_slot(index);
		return erase_counter_[index];
	}

private:
	static void check(uint32_t address, size_t bytes)
	{
		if(address < base_address)
		{
			throw std::out_of_range(
				"Address[" + std::to_string(address) + "] "
				"before Storage[" + std::to_string(base_address) + "]");
		}

		if(address + bytes > (base_address + memory.size()))
		{
			throw std::out_of_range(
				"address[" + std::to_string(address) + "] + "
				"byte_count[" + std::to_string(bytes) + "] overruns "
				"Storage[" + std::to_string(base_address + memory.size()) + "]");
		}
	}

	static void check_slot(uint32_t index)
	{
		if(index >= Layout::sector_count())
		{
			throw std::out_of_range(
				"Slot " + std::to_string(index) + " does not exist");
		}
	}
};

template<class Layout>
TestMemory<Layout::size()> TestFlash<Layout>::memory =
	TestMemory<Layout::size()>();

template<class Layout>
uint32_t TestFlash<Layout>::erase_counter_[];

} //namespace flash_storage
