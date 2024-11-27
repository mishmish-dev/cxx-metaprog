// https://www.slamecka.cz/posts/2021-03-17-cpp-metaprogramming-exercises-1/
//
// This is a set of exercises for type level programming in C++, suitable for
// beginners or as a refresher for intermediate programmers.
//
// Scope:
// - Most exercises in this set are confined to vectors of integers for simplicity.
// - Exercises are centered around basic functional techniques since they are easy to express with template metaprogramming.
//
// How to solve it:
// - You should be able to compile with gcc version 10.2, g++ --std=c++20 typelevel_set1.cpp
// - You should understand the requested behaviours from static_assert-s,
//   except for print, which comes with a test in main().
// - Tests are commented-out and you should uncomment them as you go.
// - You might want to read some introductory material before attempting these:
//
//   https://en.cppreference.com/w/cpp/language/templates
//
//   https://en.cppreference.com/w/cpp/language/parameter_pack
//
//   https://en.cppreference.com/w/cpp/language/template_parameters
//      in this set we'll only see non-type template parameters and type template parameters
//
//   https://en.cppreference.com/w/cpp/header/type_traits
//
//   https://en.cppreference.com/w/cpp/language/partial_specialization
//      also see https://stackoverflow.com/questions/17005985/what-is-the-partial-ordering-procedure-in-template-deduction
//

#include <iostream>
#include <type_traits>

namespace {

/**
 * 1. Define Vector, a template level list of integers.
 * Hint: Use non-type template parameter pack.
 */

template <int... Xs>
struct Vector {};
// using Vector = std::integer_sequence<int, Xs...>;

static_assert(std::is_same_v<Vector<1,2>, Vector<1,2>>);


/**
 * 2. Define function print() that prints Vector-s.
 * Example: print(Vector<1,2,3>{}); // prints "1 2 3" (newline)
 * See main() below.
 */


void print(Vector<>) {
    std::cout << std::endl;
}

template <int... Xs>
void print(Vector<Xs...>) {
    std::cout << ((std::to_string(Xs) + " ") + ...) << std::endl;
}


/**
 * 3. Define Prepend.
 * Hint: Use `using type = ...` inside a struct that has both non-type and type template parameters.
 */

template <int X, class V>
struct Prepend;

template <int X, int... Ys>
struct Prepend<X, Vector<Ys...>> {
    using type = Vector<X, Ys...>;
};

static_assert(std::is_same_v<Prepend<1, Vector<2,3>>::type, Vector<1,2,3>>);

/**
 * 4. Define PrependT that can be used without ::type.
 * Hint: See how enable_if_t is defined in terms of enable_if.
 *
 * This technique is not used further to reduce boilerplate.
 */

template <int X, class V>
using PrependT = Prepend<X, V>::type;

static_assert(std::is_same_v<PrependT<1, Vector<2,3>>, Vector<1,2,3>>);


/**
 * 5. Define Append.
 */

template <int X, class V>
struct Append;

template <int X, class V>
using AppendT = Append<X, V>::type;

template <int X>
struct Append<X, Vector<>> {
    using type = Vector<X>;
};

template <int X, int Y, int... Zs>
struct Append<X, Vector<Y, Zs...>> {
    using type = PrependT< Y, AppendT< X, Vector<Zs...> > >;
};

static_assert(std::is_same_v< Append<4, Vector<1,2,3>>::type , Vector<1,2,3,4> >);


/**
 * 6. Define PopBack.
 */

template <class V>
struct PopBack;

template <class V>
using PopBackT = PopBack<V>::type;

template <int X>
struct PopBack<Vector<X>> {
    using type = Vector<>;
};

template <int X, int... Ys>
struct PopBack<Vector<X, Ys...>> {
    using type = PrependT< X, PopBackT< Vector<Ys...> > >;
};

static_assert(std::is_same_v< PopBack<Vector<1,2,3,4>>::type , Vector<1,2,3> >);


/**
 * 7. Define RemoveFirst, that removes the first occurence of element R from vector V.
 */

template <int X, class V>
struct RemoveFirst;

template <int X, class V>
using RemoveFirstT = RemoveFirst<X, V>::type;

template <int X>
struct RemoveFirst<X, Vector<>> {
    using type = Vector<>;
};

template <int X, int Y, int... Zs>
struct RemoveFirst<X, Vector<Y, Zs...>> {
    using type = std::conditional_t<
        X == Y,
        Vector<Zs...>,
        PrependT< Y, RemoveFirstT<X, Vector<Zs...>> >
    >;
};

static_assert(std::is_same_v<RemoveFirst<1, Vector<1,1,2>>::type, Vector<1,2>>);
static_assert(std::is_same_v<RemoveFirst<1, Vector<0,1,0,1,2>>::type, Vector<0,0,1,2>>);


/**
 * 8. Define RemoveAll, that removes all occurences of element R from vector V.
 */

template <int X, class V>
struct RemoveAll;

template <int X, class V>
using RemoveAllT = RemoveAll<X, V>::type;

template <int X>
struct RemoveAll<X, Vector<>> {
    using type = Vector<>;
};

template <int X, int Y, int... Zs>
struct RemoveAll<X, Vector<Y, Zs...>> {
    using type = std::conditional_t<
        X == Y,
        RemoveAllT< X, Vector<Zs...> >,
        PrependT< Y, RemoveAllT<X, Vector<Zs...>> >
    >;
};

static_assert(std::is_same_v<RemoveAll<9, Vector<1,9,2,9,3,9>>::type, Vector<1,2,3>>);


/**
 * 9. Define Length.
 * Hint: Use `static constexpr int value = ...` inside the struct.
 */

template <class V>
struct Length;

template <int... Xs>
struct Length<Vector<Xs...>> {
    static constexpr int value = sizeof...(Xs);
};

static_assert(Length<Vector<1,2,3>>::value == 3);


/**
 * 10. Define length, which works like Length<V>::value.
 * Hint: See how is_same_v is defined in terms of is_same.
 */

template <class V>
inline constexpr int length = Length<V>::value;

static_assert(length<Vector<>> == 0);
static_assert(length<Vector<1,2,3>> == 3);


/**
 * 11. Define Min, that stores the minimum of a vector in its property `value`.
 */

template <int X>
struct MinHelper {
    static constexpr int value = X;
};

template <int X>
consteval int minHelperValue(MinHelper<X>) { return X; }

template <int X, int Y>
consteval auto operator+(MinHelper<X>, MinHelper<Y>) {
    if constexpr (X <= Y) {
        return MinHelper<X>{};
    } else {
        return MinHelper<Y>{};
    }
}

template <class V>
struct Min;

template <class V>
inline constexpr int MinV = Min<V>::value;

template <int... Xs>
struct Min<Vector<Xs...>> {
    static constexpr int value = minHelperValue((MinHelper<Xs>{} + ...));
};

static_assert(Min<Vector<3,1,2>>::value == 1);
static_assert(Min<Vector<1,2,3>>::value == 1);
static_assert(Min<Vector<3,2,1>>::value == 1);


/**
 * 12. Define Sort.
 */

template <class V>
struct Sort;

template <class V>
using SortT = Sort<V>::type;

template <>
struct Sort<Vector<>> {
    using type = Vector<>;
};

template <int Y>
struct Sort<Vector<Y>> {
    using type = Vector<Y>;
};

template <int... Xs>
struct Sort<Vector<Xs...>> {
    using type =
        PrependT< MinV<Vector<Xs...>>, SortT<
            RemoveFirstT< MinV<Vector<Xs...>>, Vector<Xs...> >
        > >;
};

static_assert(std::is_same_v<Sort<Vector<4,1,2,5,6,3>>::type, Vector<1,2,3,4,5,6>>);
static_assert(std::is_same_v<Sort<Vector<3,3,1,1,2,2>>::type, Vector<1,1,2,2,3,3>>);
static_assert(std::is_same_v<Sort<Vector<2,2,1,1,3,3>>::type, Vector<1,1,2,2,3,3>>);
static_assert(std::is_same_v<Sort<Vector<2,1,3,1,2,3>>::type, Vector<1,1,2,2,3,3>>);


/**
 * 13. Define Uniq.
 */

template <class V>
struct Uniq;

template <class V>
using UniqT = typename Uniq<V>::type;

template <>
struct Uniq<Vector<>> {
    using type = Vector<>;
};

template <int X>
struct Uniq<Vector<X>> {
    using type = Vector<X>;
};

template <int X, int Y, int... Zs>
struct Uniq<Vector<X, Y, Zs...>> {
    using type = std::conditional_t<
        X == Y,
        UniqT<Vector<X, Zs...>>,
        PrependT<X, UniqT<Vector<Y, Zs...>>>    
    >;
};

static_assert(std::is_same_v<Uniq<Vector<1,1,2,2,1,1>>::type, Vector<1,2,1>>);


/**
 * 14. Define type Set.
 */

template <int... Xs>
struct Set {
    using type = UniqT< SortT< Vector<Xs...> > >;
};

template <int... Xs>
using SetT = Set<Xs...>::type;

static_assert(std::is_same_v<Set<2,1,3,1,2,3>::type, Set<1,2,3>::type>);


/**
 * 15. Define SetFrom.
 */


template <class V>
struct SetFrom;

template <class V>
using SetFromT = SetFrom<V>::type;

template <int... Xs>
struct SetFrom<Vector<Xs...>> {
    using type = SetT<Xs...>;
};

static_assert(std::is_same_v<SetFrom<Vector<2,1,3,1,2,3>>::type, Set<1,2,3>::type>);


/**
 * 16. Define Get for Vector.
 * Provide an improved error message when accessing outside of Vector bounds.
 */

template <size_t Countdown>
struct GetInitiator {};

template <int X>
struct GetTerminator {};

template <int X>
consteval int getTerminatorValue(GetTerminator<X>) { return X; }

template <size_t Index, size_t Length>
consteval int checkBoundaries() {
    static_assert(Index <= Length, "Get: index out of range");
    return 0;
}

template <size_t Countdown, int X>
consteval auto operator+(GetInitiator<Countdown>, std::integral_constant<int, X>) {
    if constexpr (Countdown == 0) {
        return GetTerminator<X>{};
    } else {
        return GetInitiator<Countdown-1>{};
    }
}

template <int X, int Y>
consteval auto operator+(GetTerminator<X>, std::integral_constant<int, Y>) {
    return GetTerminator<X>{};
}

template <size_t Index, class V>
struct Get;

template <size_t Index, class V>
inline constexpr int GetV = Get<Index, V>::value;

template <size_t Index, int... Xs>
struct Get<Index, Vector<Xs...>> {
    static constexpr int value = (
        checkBoundaries<Index, sizeof...(Xs)>(),
        getTerminatorValue((GetInitiator<Index>{} + ... + std::integral_constant<int, Xs>{}))
    );
};

static_assert(Get<0, Vector<0,1,2>>::value == 0);
static_assert(Get<1, Vector<0,1,2>>::value == 1);
static_assert(Get<2, Vector<0,1,2>>::value == 2);
// static_assert(Get<9, Vector<0,1,2>>::value == 2); // How good is your error message?


/**
 * 17. Define BisectLeft for Vector.
 * Given n and arr, return the first index i such that arr[i] >= n.
 * If it doesn't exist, return the length of the array.
 *
 * Don't worry about complexity but aim for the binary search pattern.
 *
 * Hint: You might find it convenient to define a constexpr helper function.
 */

template <size_t Index, int LowerBound>
struct BisectInitiator {};

template <size_t Index>
struct BisectTerminator {};

template <size_t Index>
consteval int getTerminatorValue(BisectTerminator<Index>) { return Index; }

template <size_t Index, int LowerBound>
consteval int getTerminatorValue(BisectInitiator<Index, LowerBound>) { return Index; }

template <size_t Index, int LowerBound, int X>
consteval auto operator+(BisectInitiator<Index, LowerBound>, std::integral_constant<int, X>) {
    if constexpr (X >= LowerBound) {
        return BisectTerminator<Index>{};
    } else {
        return BisectInitiator<Index+1, LowerBound>{};
    }
}

template <size_t Index, int X>
consteval auto operator+(BisectTerminator<Index>, std::integral_constant<int, X>) {
    return BisectTerminator<Index>{};
}

template <int LowerBound, class V>
struct BisectLeft;

template <int LowerBound, class V>
inline constexpr int BisectLeftV = BisectLeft<LowerBound, V>::value;

template <int LowerBound, int... Xs>
struct BisectLeft<LowerBound, Vector<Xs...>> {
    static constexpr int value = 
        getTerminatorValue((BisectInitiator<0, LowerBound>{} + ... + std::integral_constant<int, Xs>{}));
};

static_assert(BisectLeft<  3, Vector<0,1,2,3,4>>::value == 3);
static_assert(BisectLeft<  3, Vector<0,1,2,4,5>>::value == 3);
static_assert(BisectLeft<  9, Vector<0,1,2,4,5>>::value == 5);
static_assert(BisectLeft< -1, Vector<0,1,2,4,5>>::value == 0);
static_assert(BisectLeft<  2, Vector<0,2,2,2,2,2>>::value == 1);


/**
 * 18. Define Insert for Vector, it should take position, value and vector.
 * Don't worry about bounds.
 * Hint: use the enable_if trick, e.g.
 *   template<typename X, typename Enable = void> struct Foo;
 *   template<typename X> struct<std::enable_if_t<..something      about X..>> Foo {...};
 *   template<typename X> struct<std::enable_if_t<..something else about X..>> Foo {...};
 */

template <size_t Index, int X, class V, typename Enable = void>
struct Insert;

template <size_t Index, int X, class V>
using InsertT = Insert<Index, X, V>::type;

template <int X, class V>
struct Insert<0, X, V> {
    using type = PrependT<X, V>;
};

template <size_t Index, int X, int Y, int... Zs>
struct Insert< Index, X, Vector<Y, Zs...>, std::enable_if_t<Index != 0> > {
    using type = PrependT< Y, InsertT< Index-1, X, Vector<Zs...> > >;
};

static_assert(std::is_same_v<Insert<0, 3, Vector<4,5,6>>::type, Vector<3,4,5,6>>);
static_assert(std::is_same_v<Insert<1, 3, Vector<4,5,6>>::type, Vector<4,3,5,6>>);
static_assert(std::is_same_v<Insert<2, 3, Vector<4,5,6>>::type, Vector<4,5,3,6>>);
static_assert(std::is_same_v<Insert<3, 3, Vector<4,5,6>>::type, Vector<4,5,6,3>>);

} // namespace


int main()
{
    print(Vector<>{});
    print(Vector<1>{});
    print(Vector<1,2,3,4,5,6>{});

    std::cout << typeid(Vector<1,2,3,4,5,6>{}).name() << '\n';
}