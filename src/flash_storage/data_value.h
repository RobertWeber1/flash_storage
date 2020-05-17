#pragma once

namespace flash_storage
{

template<class Tag, class Type>
struct DataValue
{
	using tag = Tag;
	using type = Type;
};


template<class ... Values>
struct DataValues : Values ...
{
    static constexpr size_t count()
    {
        return sizeof...(Values);
    }

    template<class Tag>
    static constexpr bool contains()
    {
        return (false | ... | std::is_same<Tag, typename Values::tag>::value);
    }
};


namespace detail
{

template<class, class>
struct type_of;


template<class Tag, class ... Values>
struct type_of<Tag, DataValues<Values...>>
{
    template<class Type>
	static typename DataValue<Tag, Type>::type helper_(DataValue<Tag, Type>);

    using type = decltype(helper_(DataValues<Values...>{}));
};

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

    static constexpr size_t value()
    {
        return ( 0 + ... +
            helper_<
                std::is_same<
                    Tag,
                    typename Values::tag>::value,
                Is>());
    }
};


} //namespace detail

template<class Tag, class Values>
using type_of = typename detail::type_of<Tag, Values>::type;

template<class Tag, class Values>
constexpr size_t index_of()
{
    return detail::index_of<
    	Tag,
    	Values,
    	std::make_index_sequence<Values::count()>>::value();
}


} //namespace flash_storage
