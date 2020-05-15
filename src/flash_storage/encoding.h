#pragma once
#include <stdint.h>

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

	Encoding(type_rep_t type, size_rep_t size)
	: value_((type << SizeBits) | size)
	{}

	Encoding(rep_t value)
	: value_(value)
	{}

	rep_t value() const
	{
		return value_;
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

	SectorState(uint16_t value)
	: value_(value)
	{}

	SectorState(State state, uint8_t type_bits, uint8_t size_bits)
	: value_(state << 10 | type_bits << 5 | size_bits)
	{}

	SectorState(uint16_t previous_value, State state)
	: value_(state << 10 & previous_value)
	{}

	State state() const
	{
		switch(value_ >> 10)
		{
		case Erased:      return Erased;
		case ReceiveData: return ReceiveData;
		case Valid:       return Valid;
		default:          return Invalid;
		}
	}

	SectorState& state(State state)
	{
		value_ &= state << 10;
		return *this;
	}

	uint8_t type_bits() const
	{
		return (value_ >> 5) & mask<uint8_t>(5);
	}

	uint8_t size_bits() const
	{
		return value_ & mask<uint8_t>(5);
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