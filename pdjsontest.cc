#include "pdjson.hpp"
#include "include/minunit.h"

using namespace pd;

static void TEST_STRING(std::string expect, std::string json)
{
    std::stringstream ins(json);
    auto res = parse_json(ins);
    mu_assert_string_eq(expect.c_str(), res->get_string().c_str());
};

MU_TEST(test_base_null_object)
{
    JsonNode nd;
    mu_check(nd.get_type() == JsonType::kNull);
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
    auto TEST_DOUBLE = [](double expect, std::string json) {
        std::stringstream ins(json);
        double res = parse_json(ins)->get_double();
        mu_assert_double_eq(expect, res);
    };
    TEST_DOUBLE(0.0, "0.0");
}

MU_TEST_SUITE(parser_suit)
{
    MU_RUN_TEST(test_base_null_object);
    MU_RUN_TEST(test_string_object);
    MU_RUN_TEST(test_string_parse);
    MU_RUN_TEST(test_double_parse);
}

int main()
{
    MU_RUN_SUITE(parser_suit);
    MU_REPORT();
    return minunit_status;
}
