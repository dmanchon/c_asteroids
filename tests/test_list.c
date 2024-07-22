#include "unity/unity.h"
#include "list.h"
#include <stdlib.h>

void test_list_correctly_init(void) {
    list *l = list_init();
    TEST_ASSERT_EQUAL_INT32(0, l->size);

    list_free(l);
}


int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_list_correctly_init);
    return UNITY_END();
}