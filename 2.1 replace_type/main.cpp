#include <iostream>

static int test_case_number()
{
    static int i = 0;
    return i++;
}

#define TEST_CASE(x) {\
    std::shared_ptr<int> temp{ nullptr, [](int*) {\
        const char* bold = "\033[1m";\
        const char* reset = "\033[0m";\
        const char* red = "\033[31m";\
        const char* green = "\033[32m";\
        const char* yellow = "\033[33m";\
        const char* blue = "\033[34m"; \
        std::cout << reset << "Test Case #" << test_case_number() << ": ";\
        std::cout << bold << yellow << #x << " " << reset;\
        constexpr bool is_pass = x;\
        is_pass ? (std::cout << green << "Passed!\n", 0) : (std::cout << red << "Failed!\n", 0);\
    }};\
}

template<typename X, typename Y, typename Z>
struct replace_type
{
    using type = X;
};

template<typename X, typename Y, typename Z>
using replace_type_t = typename replace_type<X, Y, Z>::type;

template<typename X, typename Y>
using add_const_if_t = std::conditional_t<std::is_const_v<std::remove_reference_t<X>>, std::add_const_t<Y>, Y>;

template<typename X, typename Y>
using add_volatile_if_t = std::conditional_t<std::is_volatile_v<X>, std::add_volatile_t<Y>, Y>;

template<typename X, typename Y>
using add_reference_if_t = std::conditional_t<std::is_lvalue_reference_v<X>, std::add_lvalue_reference_t<Y>, std::conditional_t<std::is_rvalue_reference_v<X>, std::add_rvalue_reference_t<Y>, Y>>;


template<bool, typename X, typename Y>
struct add_pointer_if_impl
{
    using type = Y;
};

template<typename X, typename Y>
struct add_pointer_if_impl<true, X, Y>
{
   using type = typename ::add_pointer_if_impl<std::is_pointer_v<std::remove_pointer_t<X>>, std::remove_pointer_t<X>, std::add_pointer_t<Y>>::type;
};

template<typename X, typename Y>
using add_pointer_if_t = typename ::add_pointer_if_impl<std::is_pointer_v<X>, X, Y>::type;

template<typename X>
struct remove_all_pointer_impl
{
    using type = X;
};

template<typename X>
struct remove_all_pointer_impl<X*>
{
    using type = typename remove_all_pointer_impl<X>::type;
};

template<typename X>
using remove_all_pointer_t = typename remove_all_pointer_impl<X>::type;

template<typename X, typename Y, unsigned I = 0>
struct upcast_array_if_impl
{
    using type = Y;
};

template<typename X, typename Y, size_t M>
struct upcast_array_if_impl<X[M], Y, 0>
{
    using type = Y[M];
};

template<typename X, typename Y, size_t M, unsigned I>
struct upcast_array_if_impl<X[M], Y, I>
{
    using type = typename upcast_array_if_impl<X, Y, I - 1>::type[M];
};

template<typename X, typename Y>
using upcast_array_if_t = typename ::upcast_array_if_impl<X, Y, std::rank_v<X> - 1>::type;

template<typename X>
using downcast_array_t = typename std::conditional_t<std::is_array_v<remove_all_pointer_t<std::remove_reference_t<X>>>, std::remove_all_extents_t<remove_all_pointer_t<std::remove_reference_t<std::remove_const_t<std::remove_volatile_t<X>>>>>, X>;

template<typename X>
using raw_type_t = std::remove_const_t<std::remove_volatile_t<remove_all_pointer_t<std::remove_reference_t<downcast_array_t<X>>>>>;

template<typename X, typename Y>
constexpr bool is_same_raw_type_v = std::is_same_v<raw_type_t<X>, raw_type_t<Y>>;

template<typename X>
using remove_ref_attr_t = remove_all_pointer_t<std::remove_reference_t<X>>;

template<typename X, typename Y, typename Z>
requires (
    is_same_raw_type_v<X, Y> &&
    (std::is_arithmetic_v<remove_ref_attr_t<X>> || 
        std::is_class_v<remove_ref_attr_t<X>> || std::is_void_v<remove_ref_attr_t<X>> ||
        std::is_enum_v<remove_ref_attr_t<X>> || std::is_union_v<remove_ref_attr_t<X>>)
)
struct replace_type<X, Y, Z>
{
    using type = add_reference_if_t<X, add_pointer_if_t<std::remove_reference_t<X>, add_const_if_t<remove_ref_attr_t<X>, add_volatile_if_t<remove_ref_attr_t<X>, Z>>>>;
};

template<typename X, typename Y, typename Z>
requires (std::is_array_v<remove_all_pointer_t<std::remove_reference_t<X>>> && ::is_same_raw_type_v<X, Y>)
struct replace_type<X, Y, Z>
{
    using type = add_reference_if_t<X, add_pointer_if_t<std::remove_reference_t<X>, add_const_if_t<X, add_volatile_if_t<X, upcast_array_if_t<remove_all_pointer_t<std::remove_reference_t<X>>, replace_type_t<downcast_array_t<X>, Y, Z>>>>>>;
};

// 函数指针,引用
template<typename X, typename Y, typename Z>
requires (std::is_function_v<std::remove_pointer_t<std::remove_reference_t<X>>>)
struct replace_type<X, Y, Z>
{
    using type = add_reference_if_t<X, add_pointer_if_t<std::remove_reference_t<X>, replace_type_t<std::remove_pointer_t<std::remove_reference_t<X>>, Y, Z>>>;
};

// 函数
template<typename R, typename Y, typename Z, typename ...Args>
struct replace_type<R(Args...), Y, Z>
{
    using type = replace_type_t<R, Y, Z>(replace_type_t<Args, Y, Z>...);
};

// 成员变量指针/成员函数指针 member-object-pointer member-function-pointer
template<typename M, typename C, typename Y, typename Z>
struct replace_type<M C::*, Y, Z>
{
    using type = replace_type_t<M, Y, Z> replace_type_t<C, Y, Z>::*;
};


struct Base
{
    int i_mem;
    float f_mem;
    int i_func(int, double) = delete;
    float f_func(float, double) = delete;
};

struct Derived
{
};

int main(int argc, char** argv)
{
    // normal type
    TEST_CASE((std::is_same_v<::replace_type_t<int, int, float>, float>));
    // const normal type
    TEST_CASE((std::is_same_v<::replace_type_t<const int, int, float>, const float>));
    // volatile normal type
    TEST_CASE((std::is_same_v<::replace_type_t<volatile void, void, float>, volatile float>));
    // cv qualifier normal type
    TEST_CASE((std::is_same_v<::replace_type_t<const volatile void, void, const float>, const volatile float>));
    // lvalue reference
    TEST_CASE((std::is_same_v<::replace_type_t<const int&, int, float>, const float&>));
    // rvalue reference
    TEST_CASE((std::is_same_v<::replace_type_t<const int&&, int, float>, const float&&>));
    // pointer
    TEST_CASE((std::is_same_v<::replace_type_t<int***, int, float>, float***>));
    // top cv qualifier pointer
    TEST_CASE((std::is_same_v<::replace_type_t<const volatile int*, int, float>, volatile const float*>));
    // array
    TEST_CASE((std::is_same_v<::replace_type_t<int[2][4], int, float>, float[2][4]>));
    TEST_CASE((std::is_same_v<::replace_type_t<const int(&)[20][30][4][3], int, float>, const float(&)[20][30][4][3]>));
    TEST_CASE((std::is_same_v<::replace_type_t<int(*)[20][3][2], int, float>, float(*)[20][3][2]>));
    TEST_CASE((std::is_same_v<::replace_type_t<char(*)(double, int), int, float>, char(*)(double, float)>));
    TEST_CASE((std::is_same_v<::replace_type_t<char(&)(double, int), int, float>, char(&)(double, float)>));    
    TEST_CASE((std::is_same_v<::replace_type_t<char(*&)(int), int, double>, char(*&)(double)>));

    // normal function
    TEST_CASE((std::is_same_v<::replace_type_t<int**&(*)(const int[3][2]), int, float>, float**&(*)(const float[3][2])>));
    TEST_CASE((std::is_same_v<::replace_type_t<int***&&(const int***[3][2], double), int, float>, float***&&(const float***[3][2], double)>));
    TEST_CASE((std::is_same_v<::replace_type_t<int&(&)(int[2], char), int, float>, float&(&)(float[2], char)>));
    TEST_CASE((std::is_same_v<::replace_type_t<int&(*&)(int[2], char), int, float>, float&(*&)(float[2], char)>));

    // member-function-pointer member-object-pointer
    TEST_CASE((std::is_same_v<::replace_type_t<decltype(&Base::i_mem), int, float>, decltype(&Base::f_mem)>));
    TEST_CASE((std::is_same_v<::replace_type_t<int(Base::*)(int, double), int, float>, float(Base::*)(float, double)>));
    TEST_CASE((std::is_same_v<::replace_type_t<int Base::*, int, float>, float Base::*>));  
    TEST_CASE((std::is_same_v<::replace_type_t<int Base::*, Base, Derived>, int Derived::*>));
    TEST_CASE((std::is_same_v<::replace_type_t<const int*&(Base::*)(int[3][4], double), int, float>, const float*&(Base::*)(float[3][4], double)>));

    return 0;
}