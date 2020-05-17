#pragma once
#include <stdint.h>
#include <limits>

namespace flash_storage
{


constexpr uint8_t required_bits(size_t value, uint8_t accu = 0)
{
	return (value == 0) ? accu : required_bits(value>>1, accu+1);
}


constexpr uint8_t required_bytes(uint8_t bits)
{
	return (bits + 7) / 8;
}

template<uint8_t>
struct rep_type_;

template<>
struct rep_type_<1>
{
	using type = uint8_t;
};

template<>
struct rep_type_<2>
{
	using type = uint16_t;
};

template<>
struct rep_type_<3>
{
	using type = uint32_t;
};

template<>
struct rep_type_<4>
{
	using type = uint32_t;
};


template<class Type>
constexpr Type mask(uint8_t bits, Type accu = 0)
{
	return (bits == 0) ? accu : mask<Type>(bits - 1, (accu<<1)|1);
}


template<uint8_t Bytes>
using rep_type = typename rep_type_<Bytes>::type;


template<uint8_t TypeBits, uint8_t SizeBits>
struct Encoding
{
	using rep_t = rep_type<required_bytes(TypeBits+SizeBits)>;
	using type_rep_t = rep_type<required_bytes(TypeBits)>;
	using size_rep_t = rep_type<required_bytes(SizeBits)>;
	using size_limit = std::numeric_limits<type_rep_t>;

	Encoding()
	: value_(0)
	{}

	Encoding(
		size_rep_t size,
		type_rep_t type = std::numeric_limits<size_rep_t>::max())
	: value_((rep_t(type) << SizeBits) | size)
	{}

	rep_t value() const
	{
		return value_;
	}

	bool is_erased() const
	{
		return value_ == std::numeric_limits<rep_t>::max();
	}

	type_rep_t type() const
	{
		return value_ >> SizeBits;
	}

	size_rep_t size() const
	{
		return value_ & mask<size_rep_t>(SizeBits);
	}
private:
	rep_t value_;
};



struct SectorState
{
	enum State
	{
		Erased      = 0b111111,
		ReceiveData = 0b010101,
		Valid       = 0b000000,
		Invalid
	};

	SectorState(uint16_t value = 0xabcd)
	: value_(value)
	{}

	SectorState(State state, uint8_t type_bits, uint8_t size_bits)
	: value_(type_bits << 10 | size_bits << 5 | state)
	{}

	SectorState(uint16_t previous_value, State state)
	: value_(previous_value & state)
	{}

	operator State() const
	{
		return state();
	}

	State state() const
	{
		switch(value_ & 0x003f)
		{
		case Erased:      return Erased;
		case ReceiveData: return ReceiveData;
		case Valid:       return Valid;
		default:          return Invalid;
		}
	}

	SectorState& state(State state)
	{
		value_ &= state;
		return *this;
	}

	uint8_t type_bits() const
	{
		return (value_ >> 10) & mask<uint8_t>(5);
	}

	uint8_t size_bits() const
	{
		return (value_ >> 5) & mask<uint8_t>(5);
	}

	uint16_t value() const
	{
		return value_;
	}

	uint8_t encoding_size() const
	{
		return required_bytes(type_bits() + size_bits());
	}

private:
	uint16_t value_;
};


} //namespace flash_storage