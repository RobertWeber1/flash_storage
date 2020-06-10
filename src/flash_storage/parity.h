#pragma once
#include <stdint.h>

namespace flash_storage
{

constexpr bool is_odd_parity(uint32_t v)
{
	v ^= v >> 16;
	v ^= v >> 8;
	v ^= v >> 4;
	v &= 0xf;
	return (0x6996 >> v) & 1;
}

constexpr bool is_even_parity(uint32_t v)
{
	return not is_odd_parity(v);
}

} //namespace flash_storage
