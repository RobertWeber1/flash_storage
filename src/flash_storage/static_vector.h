#pragma once
#include <cstddef>

namespace flash_storage
{


template<class Type, size_t Size>
struct StaticVector
{
	size_t size() const
	{
		return size_;
	}

	bool empty() const
	{
		return size_ == 0;
	}

	Type operator[](size_t index) const
	{
		if(index >= size_)
		{
			return {};
		}

		return data_[index];
	}

	void push_back(Type const& value)
	{
		data_[size_++] = value;
	}

private:
	size_t size_ = 0;
	Type data_[Size];
};


} //namespace flash_storage
