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

struct SomeTag1 {};
struct SomeTag2 {};
struct SomeTag3 {};
struct SomeTag4 {};
struct SomeTag5 {};

using DataValues_t =
	DataValues<
		DataValue<SomeTag1, int>,
		DataValue<SomeTag2, float>,
		DataValue<SomeTag3, bool>,
		DataValue<SomeTag4, std::array<double, 5>>,
		DataValue<SomeTag5, std::chrono::milliseconds>>;

using namespace std::chrono_literals;

int default_value(SomeTag1) { return 42; }
float default_value(SomeTag2) { return 42.42; }
bool default_value(SomeTag3) { return false; }
std::array<double, 5> default_value(SomeTag4) { return {5.6, 4.5, 3.4, 2.3, 1.2}; }
std::chrono::milliseconds default_value(SomeTag5) { return 42s; }

using FlashImpl = TestFlash<FlashLayout_t>;

using Storage =
	FlashStorage<
		make_sector_list<
			FlashLayout_t,
			1, 2, 5>,
		DataValues_t,
		FlashImpl,
		functions::MinRamMaxRuntime>;



TEST_CASE("Read and write values (local default)")
{
	FlashImpl::init();

	REQUIRE(
		Storage::read<SomeTag4>({0.1, 0.2, 0.3, 0.4, 0.5}) ==
		(std::array<double, 5>{0.1, 0.2, 0.3, 0.4, 0.5}));

	Storage::init();
	REQUIRE(Storage::write<SomeTag4>({1.2, 2.3, 3.4, 4.5, 5.6}));

	REQUIRE(
		Storage::read<SomeTag4>({0.1, 0.2, 0.3, 0.4, 0.5}) ==
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
