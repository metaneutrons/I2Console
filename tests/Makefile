# Host tests. Deliberately separate from the firmware build: these compile with
# the host compiler for the host architecture, and nothing here is flashed.
#
#   make          build and run
#   make coverage build, run, and fail below the line coverage floor
#
# The floor is hard. A coverage number that only gets printed is a number
# nobody reads.

CC      ?= cc
SRC_DIR := ../src
BUILD   := build

# Modules under test: those with no hardware dependency. circular_buffer.c
# includes nothing but its own header, which is why it is here and the rest of
# src/ is not.
MODULES := $(SRC_DIR)/circular_buffer.c
TESTS   := test_circular_buffer.c

CFLAGS  := -std=c11 -Wall -Wextra -Werror -I$(SRC_DIR) -g -O0
COVFLAGS := --coverage

# Every line of the modules under test has to be covered. They are small,
# dependency-free and entirely exercised by the tests, so anything below this
# means a test was deleted rather than that the target is unrealistic.
COVERAGE_FLOOR ?= 100

.PHONY: all test coverage clean

all: test

$(BUILD):
	@mkdir -p $(BUILD)

test: | $(BUILD)
	$(CC) $(CFLAGS) -o $(BUILD)/tests $(TESTS) $(MODULES)
	$(BUILD)/tests

coverage: | $(BUILD)
	$(CC) $(CFLAGS) $(COVFLAGS) -o $(BUILD)/tests-cov $(TESTS) $(MODULES)
	$(BUILD)/tests-cov
	gcovr --root .. --filter '.*/src/circular_buffer\.c' \
	      --fail-under-line $(COVERAGE_FLOOR) \
	      --txt --print-summary

clean:
	rm -rf $(BUILD) *.gcda *.gcno *.gcov
