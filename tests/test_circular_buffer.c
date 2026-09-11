// Host tests for the circular buffer.
//
// The buffer is the one module in this firmware with no hardware dependency at
// all: circular_buffer.c includes nothing but its own header. It is also the
// module whose behaviour is easiest to get subtly wrong, because it implements
// a drop-oldest policy rather than refusing a write when full.
//
// No test framework on purpose. A single-file harness keeps the firmware
// repository free of a host-only dependency, and there is not enough here to
// justify one.

#include "circular_buffer.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;
static int checks = 0;

#define CHECK(cond)                                                            \
    do {                                                                       \
        checks++;                                                              \
        if (!(cond)) {                                                         \
            failures++;                                                        \
            printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);           \
        }                                                                      \
    } while (0)

#define CHECK_EQ(actual, expected)                                             \
    do {                                                                       \
        checks++;                                                              \
        unsigned long a_ = (unsigned long)(actual);                            \
        unsigned long e_ = (unsigned long)(expected);                          \
        if (a_ != e_) {                                                        \
            failures++;                                                        \
            printf("  FAIL %s:%d: %s is %lu, expected %lu\n", __FILE__,        \
                   __LINE__, #actual, a_, e_);                                 \
        }                                                                      \
    } while (0)

static void test_init_is_empty(void)
{
    uint8_t storage[8];
    circular_buffer_t cb;
    circular_buffer_init(&cb, storage, sizeof(storage));

    CHECK_EQ(circular_buffer_available(&cb), 0);
    CHECK_EQ(circular_buffer_free(&cb), sizeof(storage));

    uint8_t out = 0xAA;
    CHECK(!circular_buffer_pop(&cb, &out));
    // A failed pop must not touch the caller's variable.
    CHECK_EQ(out, 0xAA);
}

static void test_fifo_order(void)
{
    uint8_t storage[4];
    circular_buffer_t cb;
    circular_buffer_init(&cb, storage, sizeof(storage));

    for (uint8_t i = 1; i <= 4; i++) {
        CHECK(circular_buffer_push(&cb, i));
    }
    CHECK_EQ(circular_buffer_available(&cb), 4);
    CHECK_EQ(circular_buffer_free(&cb), 0);

    for (uint8_t i = 1; i <= 4; i++) {
        uint8_t out = 0;
        CHECK(circular_buffer_pop(&cb, &out));
        CHECK_EQ(out, i);
    }
    CHECK_EQ(circular_buffer_available(&cb), 0);
}

static void test_drop_oldest_when_full(void)
{
    // The documented policy: a write into a full buffer succeeds and discards
    // the oldest byte. This is what prevents the buffer deadlock the README
    // describes, and it is the behaviour most likely to be broken by a
    // well-meaning "return false when full" change.
    uint8_t storage[3];
    circular_buffer_t cb;
    circular_buffer_init(&cb, storage, sizeof(storage));

    for (uint8_t i = 1; i <= 3; i++) {
        CHECK(circular_buffer_push(&cb, i));
    }
    // Buffer holds 1,2,3. Pushing 4 must drop 1, not refuse.
    CHECK(circular_buffer_push(&cb, 4));
    CHECK_EQ(circular_buffer_available(&cb), 3);

    uint8_t out = 0;
    CHECK(circular_buffer_pop(&cb, &out));
    CHECK_EQ(out, 2);
    CHECK(circular_buffer_pop(&cb, &out));
    CHECK_EQ(out, 3);
    CHECK(circular_buffer_pop(&cb, &out));
    CHECK_EQ(out, 4);
    CHECK(!circular_buffer_pop(&cb, &out));
}

static void test_wraparound_many_times(void)
{
    // Drives head and tail past the end repeatedly, which is where an
    // off-by-one in the modulo arithmetic would show up.
    uint8_t storage[5];
    circular_buffer_t cb;
    circular_buffer_init(&cb, storage, sizeof(storage));

    for (int round = 0; round < 100; round++) {
        uint8_t value = (uint8_t)(round & 0xFF);
        CHECK(circular_buffer_push(&cb, value));
        uint8_t out = 0;
        CHECK(circular_buffer_pop(&cb, &out));
        CHECK_EQ(out, value);
        CHECK_EQ(circular_buffer_available(&cb), 0);
    }
}

static void test_overrun_keeps_the_newest(void)
{
    // Push far more than capacity and check that exactly the last `size`
    // bytes survive, in order.
    enum { SIZE = 4, TOTAL = 50 };
    uint8_t storage[SIZE];
    circular_buffer_t cb;
    circular_buffer_init(&cb, storage, sizeof(storage));

    for (int i = 0; i < TOTAL; i++) {
        CHECK(circular_buffer_push(&cb, (uint8_t)i));
    }
    CHECK_EQ(circular_buffer_available(&cb), SIZE);

    for (int i = TOTAL - SIZE; i < TOTAL; i++) {
        uint8_t out = 0;
        CHECK(circular_buffer_pop(&cb, &out));
        CHECK_EQ(out, (uint8_t)i);
    }
}

static void test_clear_discards_content(void)
{
    uint8_t storage[4];
    circular_buffer_t cb;
    circular_buffer_init(&cb, storage, sizeof(storage));

    CHECK(circular_buffer_push(&cb, 0x11));
    CHECK(circular_buffer_push(&cb, 0x22));
    CHECK_EQ(circular_buffer_available(&cb), 2);

    circular_buffer_clear(&cb);
    CHECK_EQ(circular_buffer_available(&cb), 0);
    CHECK_EQ(circular_buffer_free(&cb), sizeof(storage));

    uint8_t out = 0;
    CHECK(!circular_buffer_pop(&cb, &out));

    // Usable again after a clear.
    CHECK(circular_buffer_push(&cb, 0x33));
    CHECK(circular_buffer_pop(&cb, &out));
    CHECK_EQ(out, 0x33);
}

static void test_free_and_available_are_complementary(void)
{
    uint8_t storage[6];
    circular_buffer_t cb;
    circular_buffer_init(&cb, storage, sizeof(storage));

    for (size_t i = 0; i < sizeof(storage); i++) {
        CHECK_EQ(circular_buffer_available(&cb) + circular_buffer_free(&cb),
                 sizeof(storage));
        CHECK(circular_buffer_push(&cb, (uint8_t)i));
    }
    CHECK_EQ(circular_buffer_available(&cb) + circular_buffer_free(&cb),
             sizeof(storage));

    // Still complementary once the drop-oldest path has run.
    CHECK(circular_buffer_push(&cb, 0xFF));
    CHECK_EQ(circular_buffer_available(&cb) + circular_buffer_free(&cb),
             sizeof(storage));
}

static void test_size_one_buffer(void)
{
    // The degenerate case: every push after the first goes through the
    // drop-oldest path.
    uint8_t storage[1];
    circular_buffer_t cb;
    circular_buffer_init(&cb, storage, sizeof(storage));

    CHECK(circular_buffer_push(&cb, 0x41));
    CHECK_EQ(circular_buffer_available(&cb), 1);
    CHECK(circular_buffer_push(&cb, 0x42));
    CHECK_EQ(circular_buffer_available(&cb), 1);

    uint8_t out = 0;
    CHECK(circular_buffer_pop(&cb, &out));
    CHECK_EQ(out, 0x42);
    CHECK(!circular_buffer_pop(&cb, &out));
}

int main(void)
{
    struct {
        const char *name;
        void (*fn)(void);
    } tests[] = {
        {"init is empty", test_init_is_empty},
        {"fifo order", test_fifo_order},
        {"drop oldest when full", test_drop_oldest_when_full},
        {"wraparound many times", test_wraparound_many_times},
        {"overrun keeps the newest", test_overrun_keeps_the_newest},
        {"clear discards content", test_clear_discards_content},
        {"free and available are complementary",
         test_free_and_available_are_complementary},
        {"size one buffer", test_size_one_buffer},
    };

    const size_t count = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < count; i++) {
        int before = failures;
        tests[i].fn();
        printf("%s %s\n", failures == before ? "ok  " : "FAIL", tests[i].name);
    }

    printf("\n%d checks in %zu tests, %d failures\n", checks, count, failures);
    return failures == 0 ? 0 : 1;
}
