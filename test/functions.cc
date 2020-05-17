#include "catch.h"
#include <flash_storage/encoding.h>
#include <flash_storage/layout.h>
#include <flash_storage/sector_list.h>
#include <flash_storage/functions/min_ram_max_runtime.h>
#include "test_flash.h"
#include <tuple>
#include <array>
#include <stdexcept>


using namespace flash_storage;

TEST_CASE("TestFlash")
{
	using layout =
		Layout<
			FlashBaseAddress<0x1001>,
			SectorSizes<0x10, 0x20, 0x50, 0x50, 0x80, 0x100>>;

	using flash = TestFlash<layout>;

	REQUIRE(flash::read<uint16_t>(0x1001) == 0xffff);

	flash::write(0x1001, 0xaa00);

	REQUIRE(flash::read<uint16_t>(0x1001) == 0xaa00);

	flash::write(0x1001, 0xf000);

	REQUIRE(flash::read<uint16_t>(0x1001) == 0xa000);

	flash::erase(0);

	REQUIRE(flash::read<uint16_t>(0x1001) == 0xffff);
}

namespace
{

using layout =
	Layout<
		FlashBaseAddress<0x1001>,
		SectorSizes<0x10, 0x20, 0x50, 0x50, 0x80, 0x100>>;

using flash = TestFlash<layout>;
using list = make_sector_list<layout, 1, 2, 5>;
using func = functions::MinRamMaxRuntime<list, flash, 4, 4, 10>;

}

TEST_CASE("get current read sector")
{
	flash::init();

	SECTION("No valid sector")
	{
		REQUIRE(func::current_read_sector() == false);
	}

	SECTION("first sector valid")
	{
		flash::write(0x1011, uint8_t(0xC0));

		REQUIRE(func::current_read_sector().address == 0x1011);
	}

	SECTION("second sector valid")
	{
		flash::write(0x1011, uint8_t(0xD5));
		flash::write(0x1031, uint8_t(0xC0));

		REQUIRE(func::current_read_sector().address == 0x1031);
	}

	SECTION("third sector valid")
	{
		flash::write(0x1031, uint8_t(0xD5));
		flash::write(0x1151, uint8_t(0xC0));

		REQUIRE(func::current_read_sector().address == 0x1151);
	}
}

TEST_CASE("get current write sector")
{
	flash::init();

	SECTION("No valid sector")
	{
		REQUIRE(func::current_write_sector() == false);
	}

	SECTION("first sector receiving")
	{
		flash::write(0x1011, uint8_t(0xD5));

		REQUIRE(func::current_write_sector().address == 0x1011);
	}

	SECTION("second sector valid")
	{
		flash::write(0x1011, uint8_t(0xC0));
		flash::write(0x1031, uint8_t(0xD5));

		REQUIRE(func::current_write_sector().address == 0x1031);
	}

	SECTION("third sector valid")
	{
		flash::write(0x1031, uint8_t(0xC0));
		flash::write(0x1151, uint8_t(0xD5));

		auto const slot = func::current_write_sector();
		REQUIRE(slot.address == 0x1151);
		REQUIRE(slot.index == 2);
		REQUIRE(slot.sector_number == 5);
	}
}


TEST_CASE("check_fully_erased")
{
	flash::init();
	auto const slot = Slot{1, 1, 0x1011, 0x20};

	SECTION("is fully erased")
	{
		REQUIRE(func::check_fully_erased(slot));

		flash::write(0x1010, uint8_t(0x00));
		REQUIRE(func::check_fully_erased(slot));

		flash::write(0x1031, uint8_t(0x00));
		REQUIRE(func::check_fully_erased(slot));
	}

	SECTION("first byte written")
	{
		flash::write(0x1011, uint8_t(0x00));
		REQUIRE_FALSE(func::check_fully_erased(slot));
	}

	SECTION("last byte written")
	{
		flash::write(0x1030, uint8_t(0x00));
		REQUIRE_FALSE(func::check_fully_erased(slot));
	}
}


TEST_CASE("find last entry")
{
	flash::init();
	auto const slot = Slot{1, 1, 0x1011, 0x20};

	SECTION("erades sector yields no valid address")
	{
		REQUIRE(func::find_last_enty(slot) == 0x1012);
	}

	SECTION("only SectorState is written")
	{
		flash::write(0x1011, uint16_t(0x0000));
		REQUIRE(func::find_last_enty(slot) == 0x1012);
	}

	SECTION("first address after SectorState is written")
	{
		flash::write(0x1013, uint8_t(0x00));
		REQUIRE(func::find_last_enty(slot) == 0x1013);
	}

	SECTION("some address in middle of sector is written")
	{
		flash::write(0x1021, uint8_t(0x00));
		REQUIRE(func::find_last_enty(slot) == 0x1021);
	}

	SECTION("first address is written")
	{
		flash::write(0x1030, uint8_t(0x00));
		REQUIRE(func::find_last_enty(slot) == 0x1030);
	}
}

TEST_CASE("format sectors")
{
	flash::init();

	flash::write(0x1001, SectorState(SectorState::Invalid, 0, 0));
	flash::write(0x1011, SectorState(SectorState::Invalid, 0, 0));
	flash::write(0x1031, SectorState(SectorState::Invalid, 0, 0));
	flash::write(0x10D1, SectorState(SectorState::Invalid, 0, 0));
	flash::write(0x1121, SectorState(SectorState::Invalid, 0, 0));
	flash::write(0x11A1, SectorState(SectorState::Invalid, 0, 0));

	REQUIRE(flash::read<SectorState>(0x1001).state() == SectorState::Invalid);
	REQUIRE(flash::read<SectorState>(0x1011).state() == SectorState::Invalid);
	REQUIRE(flash::read<SectorState>(0x1031).state() == SectorState::Invalid);
	REQUIRE(flash::read<SectorState>(0x10D1).state() == SectorState::Invalid);
	REQUIRE(flash::read<SectorState>(0x1121).state() == SectorState::Invalid);
	REQUIRE(flash::read<SectorState>(0x11A1).state() == SectorState::Invalid);

	func::format();

	REQUIRE(flash::read<SectorState>(0x1001).state() == SectorState::Invalid);
	REQUIRE(flash::read<SectorState>(0x1011).state() == SectorState::Valid);
	REQUIRE(flash::read<SectorState>(0x1031).state() == SectorState::Erased);
	REQUIRE(flash::read<SectorState>(0x10D1).state() == SectorState::Invalid);
	REQUIRE(flash::read<SectorState>(0x1121).state() == SectorState::Invalid);
	REQUIRE(flash::read<SectorState>(0x11A1).state() == SectorState::Erased);
}


TEST_CASE("read value")
{
	flash::init();
	uint8_t buf[9] = {};
	using Entry_t = Encoding<4,4>;

	SECTION("no valid sector")
	{
		REQUIRE_FALSE(func::read_value(0x05, buf, 8));
	}

	SECTION("only SectorState is written")
	{
		flash::write(0x1011, uint16_t(0x0000));
		REQUIRE_FALSE(func::read_value(0x05, buf, 8));
	}

	SECTION("value not in flash")
	{
		uint32_t base = 0x1011;
		flash::write(base, uint16_t(0x0000));
		base +=2;

		flash::write(base, uint64_t(0x0123456789abcdef));
		base += 8;

		flash::write(base, Entry_t(8, 1));
		base += 1;

		flash::write(base, uint64_t(0x0123456789abcdef));
		base += 8;

		flash::write(base, Entry_t(8, 3));
		base += 1;

		REQUIRE_FALSE(func::read_value(0x05, buf, 8));
	}

	SECTION("value in flash")
	{
		uint32_t base = 0x1011;
		flash::write(base, uint16_t(0x0000));
		base +=2;

		flash::write(base, uint64_t(0x0123456789abcdef));
		base += 8;

		flash::write(base, Entry_t(8, 5));
		base += 1;

		flash::write(base, uint64_t(0x0123456789abcdef));
		base += 8;

		flash::write(base, Entry_t(8, 3));
		base += 1;

		REQUIRE(func::read_value(5, buf, 8));
		REQUIRE(*reinterpret_cast<uint64_t const*>(buf) == 0x0123456789abcdef);


		flash::write(base, uint64_t(0xfedcba9876543210));
		base += 8;

		flash::write(base, Entry_t(8, 5));
		base += 1;

		REQUIRE(func::read_value(5, buf, 8));
		REQUIRE(*reinterpret_cast<uint64_t const*>(buf) == 0xfedcba9876543210);
	}
}


TEST_CASE("transfer data")
{


}


TEST_CASE("write value")
{
	flash::init();
	uint8_t buf[8] = {0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe};
	// using Entry_t = Encoding<4,4>;

	SECTION("no valid sector")
	{
		REQUIRE_FALSE(func::write_value(5, buf, 8));
	}

	SECTION("sector has enough space")
	{
		flash::write(0x1011, uint16_t(0x0000));

		REQUIRE(func::write_value(5, buf, 8));
		REQUIRE(flash::read<uint64_t>(0x1013) == 0xfedcba9876543210);
		REQUIRE(flash::read<uint8_t>(0x1013+8) == 0x58);

		REQUIRE(func::write_value(5, buf, 8));
		REQUIRE(flash::read<uint64_t>(0x1013+9) == 0xfedcba9876543210);
		REQUIRE(flash::read<uint8_t>(0x1013+17) == 0x58);
	}

	SECTION("sector full")
	{
		flash::write(0x1011, uint16_t(0x0000));
		REQUIRE(func::write_value(1, buf, 8));
		REQUIRE(func::write_value(2, buf, 8));
		REQUIRE(func::write_value(3, buf, 8));

		REQUIRE(flash::read<SectorState>(0x1011).state() == SectorState::Valid);
		REQUIRE(flash::read<SectorState>(0x1031).state() == SectorState::Erased);

		REQUIRE(func::write_value(4, buf, 8));

		REQUIRE(flash::read<SectorState>(0x1011).state() == SectorState::Erased);
		REQUIRE(flash::read<SectorState>(0x1031).state() == SectorState::Valid);

	}
}
