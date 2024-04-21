# Disable default rules
.SUFFIXES:

# Name of the library to build
LIBNAME := curry

# List of the object files that will be in the library
LIB_OFILES := src/curry.o

# List of test executable files. Each of these are independent.
TEST_EFILES :=
TEST_OFILES := $(TEST_EFILES:.elf=.o)
# The test runners have to be generated from the tests using unity's scripts
TEST_RUN_OFILES := $(TEST_EFILES:.elf=_run.o)
TEST_RUN_CFILES := $(TEST_EFILES:.elf=_run.c)

# Flags for building the library. The library doesn't link, so it has no linker
# flags.
LIB_CFLAGS := -Wall -Wextra -Werror -O2 -g -Iinclude/
# Flags for building the tests. It links with this library.
TEST_CFLAGS := -Wall -Wextra -Werror -O2 -g -Iinclude/ -Ithird-party/Unity/src/
TEST_LFLAGS := -L. -l$(LIBNAME)

.PHONY: all
all: lib$(LIBNAME).a

.PHONY: clean
clean:
	rm -fv \
		lib$(LIBNAME).a unity.o \
		$(LIB_OFILES) $(TEST_EFILES) $(TEST_OFILES) \
		$(TEST_RUN_OFILES) $(TEST_RUN_CFILES)

.PHONY: test
test: all $(TEST_EFILES)
	for test in $(TEST_EFILES); do \
		./$$test; \
	done

# The library just archives all the object files
lib$(LIBNAME).a: $(LIB_OFILES)
	$(AR) -rc $@ $^
# Test executables must link with unity, along with their generated runner
$(TEST_EFILES): %.elf: %.o %_run.o unity.o
	$(CC) $^ -o $@ $(TEST_LFLAGS)

# Special rule for the test framework. We don't want to be writing over their
# repository, so we just write into our own.
unity.o: third-party/Unity/src/unity.c
	$(CC) $(CFLAGS) -c $< -o $@
# Usually, just output the `.o` file in the same place as the `.c` file.
# However, we use different flags for the library and the tests
$(LIB_OFILES): %.o: %.c
	$(CC) $(LIB_CFLAGS) -c $< -o $@
$(TEST_OFILES) $(TEST_RUN_OFILES): %.o: %.c
	$(CC) $(TEST_CFLAGS) -c $< -o $@

$(TEST_RUN_CFILES): %_run.c: %.c
	./third-party/Unity/auto/generate_test_runner.rb $< $@
