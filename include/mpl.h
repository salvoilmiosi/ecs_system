#ifndef __MPL_H__
#define __MPL_H__

#include <type_traits>

namespace mpl {
	template <template <typename...> class TNewName, typename T>
	struct RenameHelper;

	// "Renames" `TOldName<Ts...>` to `TNewName<Ts...>`.
	template <template <typename...> class TNewName,
		template <typename...> class TOldName, typename... Ts>
	struct RenameHelper<TNewName, TOldName<Ts...>>
	{
		using type = TNewName<Ts...>;
	};

	template <template <typename...> class TNewName, typename T>
	using Rename = typename RenameHelper<TNewName, T>::type;
	
	// Compile-time list of types.
	template <typename... Ts>
	struct TypeList
	{
		// Size of the list.
		static constexpr std::size_t size{sizeof...(Ts)};
	};

	// Count base case: 0.
	template <typename T, typename TTypeList>
	struct CountHelper : std::integral_constant<std::size_t, 0>
	{
	};

	template <typename, typename>
	struct IndexOf;

	// IndexOf base case: found the type we're looking for.
	template <typename T, typename... Ts>
	struct IndexOf<T, TypeList<T, Ts...>>
		: std::integral_constant<std::size_t, 0>
	{
	};

	// IndexOf recursive case: 1 + IndexOf the rest of the types.
	template <typename T, typename TOther, typename... Ts>
	struct IndexOf<T, TypeList<TOther, Ts...>>
		: std::integral_constant<std::size_t,
	      1 + IndexOf<T, TypeList<Ts...>>{}>
	{
	};

	// Interface type alias.
	template <typename T, typename TTypeList>
	using Count = CountHelper<T, TTypeList>;

	// Count recursive case.
	template <typename T, typename T0, typename... Ts>
	struct CountHelper<T, TypeList<T0, Ts...>>
		: std::integral_constant<std::size_t,
			  (std::is_same<T, T0>{} ? 1 : 0) +
				  Count<T, TypeList<Ts...>>{}>
	{
	};

	// Alias for `Count > 0`.
	template <typename T, typename TTypeList>
	using Contains =
		std::integral_constant<bool, (Count<T, TTypeList>{} > 0)>;

	template <typename TCheckTypeList, typename TTypeList>
	struct ContainsAllHelper;

	template <typename TCheckTypeList, typename TTypeList>
	using ContainsAll =
		typename ContainsAllHelper<TCheckTypeList, TTypeList>::type;

	template <typename T, typename... TRest, typename TTypeList>
	struct ContainsAllHelper<TypeList<T, TRest...>, TTypeList>
		: std::integral_constant<bool,
			  Contains<T, TTypeList>{} &&
				  ContainsAll<TypeList<TRest...>, TTypeList>{}>
	{
	};

	template <typename TTypeList>
	struct ContainsAllHelper<TypeList<>, TTypeList> : std::true_type
	{
	};
}

#endif // __MPL_H__