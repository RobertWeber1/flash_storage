#include "catch.h"
#include <flash_storage/flash_storage.h>
#include <flash_storage/functions/min_ram_max_runtime.h>
#include "test_flash.h"
#include <chrono>

using namespace flash_storage;

using FlashLayout_t =
	Layout<
		FlashBaseAddress<0x1001>,
		SectorSizes<0x100, 0x2000, 0x5000, 0x5000 , 0x80000, 0x10000>>;

using DataValues_t =
	DataValues<
		DataValue<struct SomeTag1, int>,
		DataValue<struct SomeTag2, float>,
		DataValue<struct SomeTag3, bool>,
		DataValue<struct SomeTag4, std::array<double, 5>>,
		DataValue<struct SomeTag5, std::chrono::milliseconds>>;

using FlashImpl = TestFlash<FlashLayout_t>;

using Storage =
	FlashStorage<
		make_sector_list<
			FlashLayout_t,
			1, 2, 5>,
		DataValues_t,
		FlashImpl,
		functions::MinRamMaxRuntime>;



TEST_CASE("Read and write values")
{
	Storage::init();
	REQUIRE(Storage::write<SomeTag4>({1.2, 2.3, 3.4, 4.5, 5.6}));
	REQUIRE(
		Storage::read<SomeTag4>() ==
		(std::array<double, 5>{1.2, 2.3, 3.4, 4.5, 5.6}));
}
