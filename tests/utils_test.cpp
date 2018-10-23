#include "gmock/gmock.h"
#include "utils.h"
using namespace testing;

TEST(is_Numeric_Test_case, is_Numeric_True)
{
    using t1 = std::string;
    using t2 = elem_traits<std::size_t>::value_type;
    std::string tmp = "16";
    ASSERT_TRUE((is_numeric<t1, t2>(tmp)));
    tmp = "1";
    ASSERT_TRUE((is_numeric<t1, t2>(tmp)));
}

TEST(is_Numeric_Test_case, is_Numeric_False)
{
    using t1 = std::string;
    using t2 = elem_traits<std::size_t>::value_type;
    std::string tmp = "01";
    ASSERT_FALSE((is_numeric<t1, t2>(tmp)));
    tmp = "0";
    ASSERT_FALSE((is_numeric<t1, t2>(tmp)));
    tmp = "a";
    ASSERT_FALSE((is_numeric<t1, t2>(tmp)));
}

TEST(Test_Port_Case, is_Port_True)
{
    using t1 = std::string;
    std::string tmp = "1";
    ASSERT_TRUE((is_port<t1>(tmp)));

    tmp = "65535";
    ASSERT_TRUE((is_port<t1>(tmp)));
}

TEST(Test_Port_Case, is_Port_False)
{
    using t1 = std::string;
    std::string tmp = "0";
    ASSERT_FALSE((is_port<t1>(tmp)));

    tmp = "65536";
    ASSERT_FALSE((is_port<t1>(tmp)));
}

TEST(Test_Counter_Increment_case, Test_Counter_Increment)
{
    int a = TypeID<int>::value();
    ASSERT_EQ(a, 1);

    a = TypeID<int>::value();
    ASSERT_EQ(a, 2);
}
