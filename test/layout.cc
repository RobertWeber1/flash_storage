#include "catch.h"
#include <flash_storage/layout.h>

using namespace flash_storage;

TEST_CASE("layout")
{
	using layout =
		Layout<
			BaseAddress<0x1001>,
			Sizes<0x100, 0x2000, 0x5000, 0x5000 , 0x80000, 0x10000>>;

	REQUIRE(layout::sector_count() == 6);

	static_assert(layout::sector_offset<0>() == 0x1001, "");
	static_assert(layout::sector_offset<1>() == 0x1101, "");
	static_assert(layout::sector_offset<2>() == 0x3101, "");
	static_assert(layout::sector_offset<3>() == 0x8101, "");
	static_assert(layout::sector_offset<4>() == 0xd101, "");
	static_assert(layout::sector_offset<5>() == 0x8d101, "");

	static_assert(layout::sector_size<0>() == 0x100, "");
	static_assert(layout::sector_size<1>() == 0x2000, "");
	static_assert(layout::sector_size<2>() == 0x5000, "");
	static_assert(layout::sector_size<3>() == 0x5000, "");
	static_assert(layout::sector_size<4>() == 0x80000, "");
	static_assert(layout::sector_size<5>() == 0x10000, "");
}
