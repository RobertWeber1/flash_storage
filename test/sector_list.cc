#include "catch.h"
#include <flash_storage/sector_list.h>
#include <flash_storage/layout.h>

using namespace flash_storage;

using layout =
	Layout<
		FlashBaseAddress<0x1001>,
		SectorSizes<0x100, 0x2000, 0x5000, 0x5000 , 0x80000, 0x10000>>;

using list = make_sector_list<layout, 1, 2, 5>;

TEST_CASE("make sector list")
{
	static_assert(list::slot_count() == 3, "");

	Slot first   = list::first();
	Slot second  = list::next(first);
	Slot third   = list::next(second);
	Slot wrapped = list::next(third);

	REQUIRE(first.address == 0x1101);
	REQUIRE(first.size == 0x2000);

	REQUIRE(second.address == 0x3101);
	REQUIRE(second.size == 0x5000);

	REQUIRE(third.address == 0x8d101);
	REQUIRE(third.size == 0x10000);

	REQUIRE(wrapped.address == 0x1101);
	REQUIRE(wrapped.size == 0x2000);
}


TEST_CASE("for each")
{
	int count = 0;
	for_each<list>(
		[&count](flash_storage::Slot const& /*slot*/)
		{
			count++;
		});

	REQUIRE(count == 3);
}
