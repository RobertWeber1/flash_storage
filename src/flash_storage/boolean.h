#pragma once
#include <flash_storage/parity.h>
#include <stdint.h>
#include <limits>

namespace flash_storage
{

template<class Rep>
struct Boolean
{
	Boolean(bool value = false)
	: value_(
		value
		?(std::numeric_limits<Rep>::max()<<1)
		:std::numeric_limits<Rep>::max())
	{}

	operator bool() const
	{
		return is_odd_parity(value_);
	}

	bool operator!()
	{
		if(value_ != 0)
		{
			value_ = value_ << 1;
		}
		else
		{
			value_ = std::numeric_limits<Rep>::max();
		}

		return is_odd_parity(value_);
	}

private:
	Rep value_;
};


using Bool = Boolean<uint8_t>;


} //namespace flash_storage
