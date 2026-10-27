/**
 ******************************************************************************
 * @file    gtest.h
 * @brief   Minimal self-contained GoogleTest-compatible test harness for host tests
 *          when ThirdParty/googletest is not present locally.
 ******************************************************************************
 */

#ifndef GTEST_LITE_H
#define GTEST_LITE_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <vector>

namespace testing
{

class Test
{
public:
    virtual ~Test() = default;
    virtual void SetUp() {}
    virtual void TearDown() {}
};

struct TestCaseEntry {
    const char*           suite;
    const char*           name;
    std::function<void()> fn;
};

inline std::vector<TestCaseEntry>& registry()
{
    static std::vector<TestCaseEntry> sTests;
    return sTests;
}

inline int& failures()
{
    static int sFailures = 0;
    return sFailures;
}

struct Registrar {
    Registrar(const char* suite, const char* name, std::function<void()> fn)
    {
        registry().push_back({ suite, name, std::move(fn) });
    }
};

} // namespace testing

#define TEST_F(Fixture, Name)                                                       \
    class Fixture##_##Name##_Test : public Fixture                                  \
    {                                                                               \
    public:                                                                         \
        void TestBody();                                                            \
    };                                                                              \
    static ::testing::Registrar sReg_##Fixture##_##Name(                            \
        #Fixture, #Name, []() {                                                     \
            Fixture##_##Name##_Test instance;                                       \
            instance.SetUp();                                                       \
            instance.TestBody();                                                    \
            instance.TearDown();                                                    \
        });                                                                         \
    void Fixture##_##Name##_Test::TestBody()

#define EXPECT_TRUE(cond)                                                           \
    do {                                                                            \
        if (!(cond)) {                                                              \
            std::cerr << __FILE__ << ":" << __LINE__ << ": EXPECT_TRUE(" #cond ") failed\n"; \
            ++::testing::failures();                                                \
        }                                                                           \
    } while (0)

#define EXPECT_FALSE(cond)                                                          \
    do {                                                                            \
        if (cond) {                                                                 \
            std::cerr << __FILE__ << ":" << __LINE__ << ": EXPECT_FALSE(" #cond ") failed\n"; \
            ++::testing::failures();                                                \
        }                                                                           \
    } while (0)

#define EXPECT_EQ(a, b)                                                             \
    do {                                                                            \
        if (!((a) == (b))) {                                                        \
            std::cerr << __FILE__ << ":" << __LINE__ << ": EXPECT_EQ(" #a ", " #b ") failed\n"; \
            ++::testing::failures();                                                \
        }                                                                           \
    } while (0)

#define ASSERT_EQ(a, b)                                                             \
    do {                                                                            \
        if (!((a) == (b))) {                                                        \
            std::cerr << __FILE__ << ":" << __LINE__ << ": ASSERT_EQ(" #a ", " #b ") failed\n"; \
            ++::testing::failures();                                                \
            return;                                                                 \
        }                                                                           \
    } while (0)

#define ASSERT_NE(a, b)                                                             \
    do {                                                                            \
        if (!((a) != (b))) {                                                        \
            std::cerr << __FILE__ << ":" << __LINE__ << ": ASSERT_NE(" #a ", " #b ") failed\n"; \
            ++::testing::failures();                                                \
            return;                                                                 \
        }                                                                           \
    } while (0)

#define EXPECT_STREQ(a, b)                                                          \
    do {                                                                            \
        const char* _sa = (a);                                                      \
        const char* _sb = (b);                                                      \
        if (!_sa || !_sb || std::strcmp(_sa, _sb) != 0) {                           \
            std::cerr << __FILE__ << ":" << __LINE__ << ": EXPECT_STREQ(" #a ", " #b ") failed: '" \
                      << (_sa ? _sa : "null") << "' vs '" << (_sb ? _sb : "null") << "'\n"; \
            ++::testing::failures();                                                \
        }                                                                           \
    } while (0)

#ifdef GTEST_LITE_MAIN
int main()
{
    std::cout << "[==========] Running " << ::testing::registry().size() << " tests.\n";
    for (const auto& tc : ::testing::registry()) {
        std::cout << "[ RUN      ] " << tc.suite << "." << tc.name << "\n";
        const int before = ::testing::failures();
        tc.fn();
        if (::testing::failures() == before) {
            std::cout << "[       OK ] " << tc.suite << "." << tc.name << "\n";
        } else {
            std::cout << "[  FAILED  ] " << tc.suite << "." << tc.name << "\n";
        }
    }
    std::cout << "[==========] " << ::testing::registry().size() << " tests ran.\n";
    return (::testing::failures() == 0) ? 0 : 1;
}
#endif

#endif // GTEST_LITE_H

