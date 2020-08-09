#pragma once
#include <stdint.h>

namespace flash_storage
{

constexpr bool is_odd_parity(uint8_t v)
{
	v ^= v >> 4;
	v &= 0xf;
	return (0x6996 >> v) & 1;
}

constexpr bool is_even_parity(uint8_t v)
{
	return not is_odd_parity(v);
}


constexpr bool is_odd_parity(uint16_t v)
{
	return is_odd_parity(uint8_t(v ^ (v >> 8)));
}

constexpr bool is_even_parity(uint16_t v)
{
	return not is_odd_parity(v);
}


constexpr bool is_odd_parity(uint32_t v)
{
	return is_odd_parity(uint16_t(v ^ (v >> 16)));
}

constexpr bool is_even_parity(uint32_t v)
{
	return not is_odd_parity(v);
}


constexpr bool is_odd_parity(uint64_t v)
{
	return is_odd_parity(uint32_t(v ^ (v >> 32)));
}

constexpr bool is_even_parity(uint64_t v)
{
	return not is_odd_parity(v);
}

} //namespace flash_storage
