#include "catch.h"
#include <flash_storage/flash_storage.h>
#include <flash_storage/functions/min_ram_max_runtime.h>
#include <flash_storage/functions/cached_access.h>
#include "test_flash.h"
#include <chrono>

#include <iostream>
#include <iomanip>

namespace std
{

template<class T, size_t N>
ostream & operator<<(ostream & out, array<T, N> const& val)
{
	out << '{';

	for(size_t i=0; i<N; ++i)
	{
		if(i!=0)
		{
			out << ", ";
		}
		out << val[i];
	}

	return out << '}';
}


template<size_t N>
ostream & operator<<(ostream & out, array<uint8_t, N> const& val)
{
	out << '{';

	for(size_t i=0; i<N; ++i)
	{
		if(i!=0)
		{
			out << ", ";
		}
		out << "0x" << hex << setw(2) << setfill('0') << uint16_t(val[i]);
	}

	return out << '}';
}


} //namespace std



using namespace flash_storage;
using namespace std::chrono_literals;

using FlashLayout_t =
	Layout<
		FlashBaseAddress<0x1001>,
		SectorSizes<0x100, 0x2000, 0x5000, 0x5000 , 0x80000, 0x10000>>;

struct SomeTag1
{
	static constexpr int default_value()
	{
		return 42;
	}
};

struct SomeTag2
{
	static constexpr float default_value()
	{
		return 42.42;
		 }
};

struct SomeTag3
{
	static constexpr bool default_value()
	{
		return false;
	}
};

struct SomeTag4
{
	static constexpr std::array<double, 5> default_value()
	{
		return {5.6, 4.5, 3.4, 2.3, 1.2};
	}
};

struct SomeTag5
{
	static constexpr std::chrono::milliseconds default_value()
	{
		return 42s;
	}
};

struct SomeTag6
{
	static constexpr std::array<uint8_t, 5> default_value()
	{
		return {0,0,0,0,0};
	}
};

using FlashImpl = TestFlash<FlashLayout_t>;

using Storage =
	FlashStorage<
		make_sector_list<
			FlashLayout_t,
			1, 2, 5>,
		make_data_values<
			SomeTag1,
			SomeTag2,
			SomeTag3,
			SomeTag4,
			SomeTag5>,
		FlashImpl,
		functions::MinRamMaxRuntime>;



TEST_CASE("Read and write values (local default)")
{
	FlashImpl::init();

	REQUIRE(
		Storage::read_or<SomeTag4>({0.1, 0.2, 0.3, 0.4, 0.5}) ==
		(std::array<double, 5>{0.1, 0.2, 0.3, 0.4, 0.5}));

	Storage::init();
	REQUIRE(Storage::write<SomeTag4>({1.2, 2.3, 3.4, 4.5, 5.6}));

	REQUIRE(
		Storage::read_or<SomeTag4>({0.1, 0.2, 0.3, 0.4, 0.5}) ==
		(std::array<double, 5>{1.2, 2.3, 3.4, 4.5, 5.6}));
}


TEST_CASE("Read and write values (global default)")
{
	FlashImpl::init();

	REQUIRE(
		Storage::read<SomeTag4>() ==
		(std::array<double, 5>{5.6, 4.5, 3.4, 2.3, 1.2}));

	Storage::init();
	REQUIRE(Storage::write<SomeTag4>({1.2, 2.3, 3.4, 4.5, 5.6}));

	REQUIRE(
		Storage::read<SomeTag4>() ==
		(std::array<double, 5>{1.2, 2.3, 3.4, 4.5, 5.6}));
}


using CachedStorage =
	FlashStorage<
		make_sector_list<
			FlashLayout_t,
			1, 2, 5>,
		make_data_values<
			SomeTag1,
			SomeTag2,
			SomeTag3,
			SomeTag4,
			SomeTag5,
			SomeTag6>,
		FlashImpl,
		functions::CachedAccess>;

TEST_CASE("Read with cached start of data")
{
	FlashImpl::init();

	REQUIRE(
		CachedStorage::read<SomeTag4>() ==
		(std::array<double, 5>{5.6, 4.5, 3.4, 2.3, 1.2}));

	CachedStorage::init();
	REQUIRE(CachedStorage::write<SomeTag4>({1.2, 2.3, 3.4, 4.5, 5.6}));

	REQUIRE(
		CachedStorage::read<SomeTag4>() ==
		(std::array<double, 5>{1.2, 2.3, 3.4, 4.5, 5.6}));
}


TEST_CASE("update value")
{
	FlashImpl::init();

	REQUIRE(
		CachedStorage::read<SomeTag6>() ==
		(std::array<uint8_t, 5>{0,0,0,0,0}));

	CachedStorage::init();
	REQUIRE(CachedStorage::update<SomeTag6>({0xff, 0xff, 0xff, 0xff, 0xfe}));

	REQUIRE(
		CachedStorage::read<SomeTag6>() ==
		(std::array<uint8_t, 5>{0xff, 0xff, 0xff, 0xff, 0xfe}));


	REQUIRE(CachedStorage::update<SomeTag6>({0xff, 0xff, 0xff, 0xff, 0xf0}));

	// REQUIRE(
	// 	CachedStorage::read<SomeTag6>() ==
	// 	(std::array<uint8_t, 5>{0xff, 0xff, 0xff, 0xff, 0xf0}));
}

