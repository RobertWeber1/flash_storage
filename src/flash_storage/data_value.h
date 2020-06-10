#pragma once
#include <algorithm>


namespace flash_storage
{


template<class Tag>
struct DataValue
{
	using tag = Tag;
	using type = decltype(Tag::default_value());
};


template<class ... Values>
struct DataValues : Values ...
{
	static constexpr size_t count()
	{
		return sizeof...(Values);
	}

	template<class Tag>
	static constexpr bool contains(bool accu = false)
	{
		using dummy = bool[];
		(void)dummy{(accu |= std::is_same<Tag, typename Values::tag>::value) ...};
		return accu;
	}

	static constexpr size_t id_count()
	{
		return count() + 1;
	}

	static constexpr size_t max_byte_count()
	{
		return std::max<size_t>({sizeof(typename Values::type)...});
	}
};


template<class ... Tags>
using make_data_values = DataValues<DataValue<Tags>...>;

namespace detail
{


// template<class, class>
// struct type_of;


// template<class Tag, class ... Values>
// struct type_of<Tag, DataValues<Values...>>
// {
// 	template<class Type>
// 	static typename DataValue<Tag, Type>::type helper_(DataValue<Tag, Type>);

// 	using type = decltype(helper_(DataValues<Values...>{}));
// };


template<bool IsSame, size_t Index>
constexpr size_t helper_()
{
	return (IsSame ? Index : 0);
}


template<class, class, class>
struct index_of;


template<class Tag, class ... Values, size_t ... Is>
struct index_of<Tag, DataValues<Values...>, std::index_sequence<Is...>>
{
	static_assert(
		DataValues<Values...>::template contains<Tag>(),
		"Tag not found in DataValues<...>");

	static constexpr size_t value(size_t accu = 0)
	{
		using dummy = size_t[];
		(void)dummy{
			(accu += helper_<
				std::is_same<
					Tag,
					typename Values::tag>::value,
				Is>())...};

		return accu;
	}
};


} //namespace detail


template<class Tag>
using type_of = decltype(Tag::default_value());


template<class Tag, class Values>
constexpr size_t index_of()
{
	return detail::index_of<
		Tag,
		Values,
		std::make_index_sequence<Values::count()>>::value();
}


} //namespace flash_storage
