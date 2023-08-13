#include <iostream>

//#define PLAIN_DEF
//#define ENABLE_IF_DEF
#define REQUIRE_DEF
#define ENABLE_CONCEPT 0
#ifdef PLAIN_DEF
template<typename T>
struct add_const_ref
{
    typedef const T& type;
};

template<typename T>
struct add_const_ref<T&>
{
    typedef T& type;
};
#endif

#ifdef ENABLE_IF_DEF

template<typename T, bool = false>
struct add_const_ref_impl
{
    using type = const T&;
};

template<typename T>
struct add_const_ref_impl<T, true>
{
    using type = T&;
};

template<typename T>
struct add_const_ref
{
    using type = typename add_const_ref_impl<T, std::is_reference_v<T>>::type;
};
#endif

#ifdef REQUIRE_DEF
template<typename T>
struct add_const_ref
{
    using type = const T&;
};

#if ENABLE_CONCEPT
template<typename T>
concept IS_REFERENCE = std::is_reference_v<T>;
template<IS_REFERENCE T>
struct add_const_ref<T>
{
    using type = T&;
};
#else
template<typename T> requires (std::is_reference_v<T>)
struct add_const_ref<T>
{
    using type = T&;
};
#endif

#endif


int main(int argc, char** argv)
{
    std::cout << std::boolalpha << std::is_same_v<int&, ::add_const_ref<int&>::type> << std::endl;
    std::cout << std::boolalpha << std::is_same_v<const int&, ::add_const_ref<int>::type> << std::endl;
    
    return 0;
}