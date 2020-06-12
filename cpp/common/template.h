#pragma once
#include <type_traits>

template <typename T, typename... Args>
struct contains : public std::false_type {};

template <typename T, typename U, typename... Args>
struct contains<T, U, Args...> :
    public std::conditional_t<std::is_same_v<T, U>, std::true_type, contains<T, Args...>> {};


template <typename T>
constexpr bool always_false = false;

// template <typename TypeSet, typename... Args>
// type_in
