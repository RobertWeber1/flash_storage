#pragma once
#include <flash_storage/version.h>
#include <flash_storage/detail/flash_storage.h>
#include <flash_storage/detail/cached_access.h>

namespace flash_storage
{

template<class SectorList, class DataValues, class Flash>
using FlashStorage =
	detail::FlashStorage<
		SectorList,
		DataValues,
		Flash,
		detail::CachedAccess>;

} //namespace flash_storage
