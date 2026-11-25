#include <string.h>

#include "test_util.h"
#include "textutil.h"
#include "validation.h"

static void test_text_helpers(void)
{
    char text[32] = "   Alice Uwase  ";
    char small[4];

    text_trim(text);
    CHECK(strcmp(text, "Alice Uwase") == 0);

    text_copy(small, sizeof small, "Computer");
    CHECK(strcmp(small, "Com") == 0);

    CHECK(text_compare_ci("alice", "ALICE") == 0);
    CHECK(text_compare_ci("Alice", "Bob") < 0);
    CHECK(text_contains_ci("Computer Science", "science"));
    CHECK(!text_contains_ci("Computer Science", "math"));
}

static void test_field_rules(void)
{
    CHECK(valid_student_id(1));
    CHECK(valid_student_id(99999999));
    CHECK(!valid_student_id(0));
    CHECK(!valid_student_id(-5));

    CHECK(valid_year(1) && valid_year(6));
    CHECK(!valid_year(0) && !valid_year(7));

    CHECK(valid_text_field("Alice", 32));
    CHECK(!valid_text_field("", 32));
    CHECK(!valid_text_field("bad|name", 32));
    CHECK(!valid_text_field("abcdef", 6));
    CHECK(!valid_text_field("caf\xc3\xa9", 32));
}

static void test_marks_rules(void)
{
    CHECK(valid_max_marks(100));
    CHECK(!valid_max_marks(0));
    CHECK(!valid_max_marks(1001));
    CHECK(valid_marks(0, 100) && valid_marks(100, 100));
    CHECK(!valid_marks(-1, 100));
    CHECK(!valid_marks(101, 100));
}

static void test_parsing(void)
{
    int i = 0;
    double d = 0;

    CHECK(parse_int("42", &i) && i == 42);
    CHECK(parse_int("7  ", &i) && i == 7);
    CHECK(!parse_int("", &i));
    CHECK(!parse_int("12abc", &i));
    CHECK(!parse_int("99999999999", &i));

    CHECK(parse_double("75.5", &d) && d == 75.5);
    CHECK(!parse_double("abc", &d));
    CHECK(!parse_double("nan", &d));
}

int main(void)
{
    test_text_helpers();
    test_field_rules();
    test_marks_rules();
    test_parsing();
    return test_summary("test_validation");
}
