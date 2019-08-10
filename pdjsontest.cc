#include "pdjson.hpp"
#include "include/minunit.h"
#include <limits>

using namespace pd;

static void TEST_STRING(std::string expect, std::string json)
{
    std::stringstream ins(json);
    auto res = parse_json(ins);
    mu_assert_string_eq(expect.c_str(), res->get_string().c_str());
};

static auto TEST_DOUBLE = [](double expect, std::string json) {
    std::stringstream ins(json);
    double res = parse_json(ins)->get_double();
    mu_assert_double_eq(expect, res);
};

MU_TEST(test_base_null_object)
{
    JsonNode nd;
    mu_check(nd.get_type() == JsonType::kNull);

    std::stringstream ins("");
    auto res = parse_json(ins);
    mu_check(res->get_type() == JsonType::kNull);
}

MU_TEST(test_string_object)
{
    JsonString nd("Hello");
    std::string tmp;
    tmp = nd.get_string();

    mu_assert_string_eq(tmp.c_str(), "Hello");
    mu_check(nd.get_type() == JsonType::kString);
    std::ostringstream oss;
    nd.write(oss, 0);
    mu_assert_string_eq(oss.str().c_str(), "\"Hello\"");
}

MU_TEST(test_string_parse)
{
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}

MU_TEST(test_double_parse)
{
    TEST_DOUBLE(0.0, "0.0");
    TEST_DOUBLE(0.0, "0");
    TEST_DOUBLE(0.0, "-0");
    TEST_DOUBLE(0.0, "-0.0");
    TEST_DOUBLE(1.0, "1");
    TEST_DOUBLE(-1.0, "-1");
    TEST_DOUBLE(1.5, "1.5");
    TEST_DOUBLE(-1.5, "-1.5");
    TEST_DOUBLE(3.1416, "3.1416");
    TEST_DOUBLE(1E10, "1E10");
    TEST_DOUBLE(1e10, "1e10");
    TEST_DOUBLE(1E+10, "1E+10");
    TEST_DOUBLE(1E-10, "1E-10");
    TEST_DOUBLE(-1E10, "-1E10");
    TEST_DOUBLE(-1e10, "-1e10");
    TEST_DOUBLE(-1E+10, "-1E+10");
    TEST_DOUBLE(-1E-10, "-1E-10");
    TEST_DOUBLE(1.234E+10, "1.234E+10");
    TEST_DOUBLE(1.234E-10, "1.234E-10");
    TEST_DOUBLE(0.0, "1e-10000"); /* must underflow */
    TEST_DOUBLE(std::numeric_limits<double>::max(),
                std::to_string(std::numeric_limits<double>::max()));
    TEST_DOUBLE(std::numeric_limits<double>::min(),
                std::to_string(std::numeric_limits<double>::min()));
}

MU_TEST(test_bool_parse)
{
#define TEST_BOOL(expect, json) \
    do { \
        std::stringstream ins(json); \
        mu_check(expect == parse_json(ins)->get_bool()); \
    } while (0)

    TEST_BOOL(true, "true");
    //    TEST_BOOL(false, "true"); //failed
    TEST_BOOL(false, "false");
    //    TEST_BOOL(false, "fase"); //throw an exception
    //    TEST_BOOL(true, "true"); //throw an exception
}

MU_TEST(test_array_parse)
{
    {
        std::stringstream ins("[ null , false ,true, 123.0, \"hello\nworld\"]");
        auto res = parse_json(ins);

        mu_check(res->get_array().at(0)->get_type() == JsonType::kNull);
        mu_check(true == res->get_array().at(2)->get_bool());
        mu_check(false == res->get_array().at(1)->get_bool());
        mu_check(123.0 == res->get_array().at(3)->get_double());
        mu_check("hello\nworld" == res->get_array().at(4)->get_string());
    }

    {
        std::stringstream ins("[true,[true,false],true]");
        auto res = parse_json(ins);
        mu_check(true == res->get_array().at(0)->get_bool());
        mu_check(true == res->get_array().at(2)->get_bool());
        mu_check(true == res->get_array().at(1)->get_array().at(0)->get_bool());
        mu_check(false == res->get_array().at(1)->get_array().at(1)->get_bool());

        // [t,[t,t],t];
        res->get_array().at(1)->get_array().at(1)->get_bool() = true;
        res->write(std::cout, 0);
    }

    {
        std::stringstream ins("[\"Hello\",\"Wo\\trld\" ,0.9,true]");
        auto res = parse_json(ins);
        std::cout << std::endl;
        res->write(std::cout, 0);
    }
}

MU_TEST(test_object_parse)
{
    {
        std::stringstream ins("{\"a\":1.0}");
        auto res = parse_json(ins);
        mu_assert_double_eq(1.0, res->get_object().find("a")->second->get_double());
        std::cout << '\n';
        res->write(std::cout, 0);
    }
    {
        std::stringstream ins("{"
                              "\"a\":1.2,"
                              "\"null\":null ,"
                              "\"array\":[true,false],"
                              "\"str\":\"string\"}");
        auto res = parse_json(ins);
        mu_assert_double_eq(1.2, res->get_object().find("a")->second->get_double());
        mu_assert_string_eq("array", res->get_object().find("array")->first.c_str());
        mu_check(res->get_object().find("array")->second->get_array().at(0)->get_bool());
        mu_check(!res->get_object().find("array")->second->get_array().at(1)->get_bool());
        std::cout << '\n';
        res->write(std::cout, 0);
    }
}

MU_TEST_SUITE(parser_suit)
{
    MU_RUN_TEST(test_base_null_object);
    MU_RUN_TEST(test_string_object);
    MU_RUN_TEST(test_string_parse);
    MU_RUN_TEST(test_double_parse);
    MU_RUN_TEST(test_bool_parse);
    MU_RUN_TEST(test_array_parse);
    MU_RUN_TEST(test_object_parse);
}

int main()
{
    MU_RUN_SUITE(parser_suit);
    MU_REPORT();
    return minunit_status;
}
