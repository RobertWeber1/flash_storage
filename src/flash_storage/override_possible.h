#pragma once
#include <stdint.h>

namespace flash_storage
{


enum class WriteOperation
{
	RecordUpToDate,
	UpdateRecord,
	CreateNewRecord
};



static constexpr bool write_necessary(
	uint8_t const* current,
	uint8_t const* next,
	uint32_t size)
{
	for(uint32_t i = 0; i<size; ++i)
	{
		if(*current != *next)
		{
			return true;
		}
	}

	return false;
}

static constexpr bool updateable(
	uint8_t const* current,
	uint8_t const* next,
	uint32_t size)
{
	for(uint32_t i = 0; i<size; ++i)
	{
		if(*current != *next)
		{

			return false;
		}
	}

	return true;
}


} //namespace flash_storage
