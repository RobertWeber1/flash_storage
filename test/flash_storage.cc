#include "catch.h"
#include <flash_storage/flash_storage.h>
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
		SectorSizes<0x10, 0x2000, 0x5000, 0x5000 , 0x80000, 0x10000>>;

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
			0, 2, 5>,
		make_data_values<
			SomeTag1,
			SomeTag2,
			SomeTag3,
			SomeTag4,
			SomeTag5,
			SomeTag6>,
		FlashImpl>;


TEST_CASE("Read and write values (local default)")
{
	Storage::init();

	REQUIRE(
		Storage::read<SomeTag4>({0.1, 0.2, 0.3, 0.4, 0.5}) ==
		(std::array<double, 5>{0.1, 0.2, 0.3, 0.4, 0.5}));

	REQUIRE(Storage::write<SomeTag4>({1.2, 2.3, 3.4, 4.5, 5.6}));

	REQUIRE(
		Storage::read<SomeTag4>({0.1, 0.2, 0.3, 0.4, 0.5}) ==
		(std::array<double, 5>{1.2, 2.3, 3.4, 4.5, 5.6}));
}

TEST_CASE("Read and write values (global default)")
{
	Storage::init();

	REQUIRE(
		Storage::read<SomeTag4>() ==
		(std::array<double, 5>{5.6, 4.5, 3.4, 2.3, 1.2}));

	REQUIRE(Storage::write<SomeTag4>({1.2, 2.3, 3.4, 4.5, 5.6}));

	REQUIRE(
		Storage::read<SomeTag4>() ==
		(std::array<double, 5>{1.2, 2.3, 3.4, 4.5, 5.6}));
}

TEST_CASE("write/update value")
{
	Storage::init();

	SECTION("value not present")
	{
		REQUIRE(Storage::write<SomeTag6>({0x00, 0xff, 0xff, 0xff, 0xfe}));

		REQUIRE(
			Storage::read<SomeTag6>() ==
			(std::array<uint8_t, 5>{0x00, 0xff, 0xff, 0xff, 0xfe}));

		SECTION("update possible")
		{
			REQUIRE(Storage::write<SomeTag6>({0xff, 0xff, 0xff, 0xff, 0xf0}));
			REQUIRE(
				Storage::read<SomeTag6>() ==
				(std::array<uint8_t, 5>{0xff, 0xff, 0xff, 0xff, 0xf0}));
		}

		SECTION("update not possible")
		{
			REQUIRE(Storage::write<SomeTag6>({0xff, 0xff, 0xff, 0xff, 0x0f}));
			REQUIRE(
				Storage::read<SomeTag6>() ==
				(std::array<uint8_t, 5>{0xff, 0xff, 0xff, 0xff, 0x0f}));
		}
	}

	SECTION("update causes transfer")
	{
		REQUIRE(Storage::write<SomeTag6>({0xff, 0xff, 0xff, 0xff, 0xfe}));
		REQUIRE(Storage::write<SomeTag6>({0xff, 0xff, 0xff, 0xff, 0x0f}));
		REQUIRE(Storage::write<SomeTag6>({0xff, 0xff, 0xff, 0xff, 0xfe}));

		REQUIRE(
			Storage::read<SomeTag6>() ==
			(std::array<uint8_t, 5>{0xff, 0xff, 0xff, 0xff, 0xfe}));
	}
}
