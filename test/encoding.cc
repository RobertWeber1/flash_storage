#include "catch.h"
#include <flash_storage/encoding.h>

using namespace flash_storage;

TEST_CASE("required bits")
{
	REQUIRE(required_bits(0) == 0);
	REQUIRE(required_bits(1) == 1);
	REQUIRE(required_bits(2) == 2);
	REQUIRE(required_bits(7) == 3);
	REQUIRE(required_bits(8) == 4);
	REQUIRE(required_bits(15) == 4);
	REQUIRE(required_bits(16) == 5);
	REQUIRE(required_bits(31) == 5);
	REQUIRE(required_bits(32) == 6);
	REQUIRE(required_bits(63) == 6);
	REQUIRE(required_bits(64) == 7);
	REQUIRE(required_bits(127) == 7);
	REQUIRE(required_bits(128) == 8);
	REQUIRE(required_bits(255) == 8);

	REQUIRE(required_bits(511) == 9);
	REQUIRE(required_bits(1023) == 10);
}


TEST_CASE("required bytes")
{
	REQUIRE(required_bytes(0) == 0);
	REQUIRE(required_bytes(1) == 1);
	REQUIRE(required_bytes(7) == 1);
	REQUIRE(required_bytes(8) == 1);
	REQUIRE(required_bytes(9) == 2);
	REQUIRE(required_bytes(16) == 2);
	REQUIRE(required_bytes(17) == 3);
	REQUIRE(required_bytes(32) == 4);
}


TEST_CASE("representation type")
{
	REQUIRE((std::is_same<rep_type<1>, uint8_t>::value));
	REQUIRE((std::is_same<rep_type<2>, uint16_t>::value));
	REQUIRE((std::is_same<rep_type<3>, uint32_t>::value));
	REQUIRE((std::is_same<rep_type<4>, uint32_t>::value));
}

TEST_CASE("bit mask")
{
	REQUIRE(mask<uint8_t>(0) == 0);
	REQUIRE(mask<uint8_t>(1) == 0b00000001);
	REQUIRE(mask<uint8_t>(2) == 0b00000011);
	REQUIRE(mask<uint8_t>(3) == 0b00000111);
	REQUIRE(mask<uint8_t>(4) == 0b00001111);
	REQUIRE(mask<uint8_t>(5) == 0b00011111);
	REQUIRE(mask<uint8_t>(6) == 0b00111111);
	REQUIRE(mask<uint8_t>(7) == 0b01111111);
	REQUIRE(mask<uint8_t>(8) == 0b11111111);
}


TEST_CASE("encoding")
{
	using enc1 = Encoding<5,2>;

	REQUIRE((std::is_same<typename enc1::rep_t, uint8_t>::value));
	REQUIRE((std::is_same<typename enc1::type_rep_t, uint8_t>::value));
	REQUIRE((std::is_same<typename enc1::size_rep_t, uint8_t>::value));

	REQUIRE(enc1(7,2).value() == 30);
	REQUIRE(enc1(30).type() == 7);
	REQUIRE(enc1(30).size() == 2);

	using enc2 = Encoding<8,2>;

	REQUIRE((std::is_same<typename enc2::rep_t, uint16_t>::value));
	REQUIRE((std::is_same<typename enc2::type_rep_t, uint8_t>::value));
	REQUIRE((std::is_same<typename enc2::size_rep_t, uint8_t>::value));

	using enc3 = Encoding<8,16>;

	REQUIRE((std::is_same<typename enc3::rep_t, uint32_t>::value));
	REQUIRE((std::is_same<typename enc3::type_rep_t, uint8_t>::value));
	REQUIRE((std::is_same<typename enc3::size_rep_t, uint16_t>::value));
}


TEST_CASE("SectorState")
{
	// SectorState

}