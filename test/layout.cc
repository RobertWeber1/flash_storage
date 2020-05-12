#include "catch.h"
#include <flash_storage/layout.h>

using namespace flash_storage;

TEST_CASE("layout")
{
	using layout = Layout<0x1001, 0x100, 0x2000, 0x5000, 0x5000 , 0x80000, 0x10000>;

	REQUIRE(layout::sector_count() == 6);

	REQUIRE(layout::sector_offset<0>() == 0x1001);
	REQUIRE(layout::sector_offset<1>() == 0x1101);
	REQUIRE(layout::sector_offset<2>() == 0x3101);
	REQUIRE(layout::sector_offset<3>() == 0x8101);
	REQUIRE(layout::sector_offset<4>() == 0xd101);
	REQUIRE(layout::sector_offset<5>() == 0x8d101);

	REQUIRE(layout::sector_size<0>() == 0x100);
	REQUIRE(layout::sector_size<1>() == 0x2000);
	REQUIRE(layout::sector_size<2>() == 0x5000);
	REQUIRE(layout::sector_size<3>() == 0x5000);
	REQUIRE(layout::sector_size<4>() == 0x80000);
	REQUIRE(layout::sector_size<5>() == 0x10000);
}
