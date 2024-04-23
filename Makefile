# Disable default rules
.SUFFIXES:

# Where to install
PREFIX ?= /usr/local/

# Name of the library to build
LIBNAME := curry

# List of the object files that will be in the library
LIB_OFILES := src/curry.o
LIB_DFILES := $(LIB_OFILES:.o=.d)
LIB_CFILES := $(LIB_OFILES:.o=.c)
# The library has some assembly files. List them here
SLIB_OFILES := src/curry_trampoline.o
SLIB_SFILES := $(SLIB_OFILES:.o=.S)

# List of test executable files. Each of these are independent.
TEST_EFILES := \
	test/suite_basic.elf \
	test/suite_size.elf \
	test/suite_overflow.elf \
	test/suite_chain.elf
TEST_OFILES := $(TEST_EFILES:.elf=.o)
TEST_DFILES := $(TEST_EFILES:.elf=.d)
TEST_CFILES := $(TEST_EFILES:.elf=.c)
# The test runners have to be generated from the tests using unity's scripts
TEST_RUN_OFILES := $(TEST_EFILES:.elf=_run.o)
TEST_RUN_DFILES := $(TEST_EFILES:.elf=_run.d)
TEST_RUN_CFILES := $(TEST_EFILES:.elf=_run.c)

# Flags for building the library. The library doesn't link, so it has no linker
# flags. Change the optimization flags depending on whether DEBUG is set.
LIB_CFLAGS := \
	-Wall -Wextra -Werror -Iinclude/ \
	$(if $(findstring undefined,$(origin DEBUG)), -O2 -DNDEBUG, -Og -g)
# Flags for building the tests. It links with this library.
TEST_CFLAGS := -Wall -Wextra -Werror -Og -g -Iinclude/ -Ithird-party/Unity/src/

.PHONY: all
all: lib$(LIBNAME).a

.PHONY: install
install: all
	install -D -m 644 lib$(LIBNAME).a $(PREFIX)/lib/lib$(LIBNAME).a
	install -D -m 644 include/curry.h $(PREFIX)/include/curry.h

.PHONY: clean
clean:
	rm -fv \
		lib$(LIBNAME).a unity.o \
		$(LIB_OFILES) $(LIB_DFILES) $(SLIB_OFILES) \
		$(TEST_EFILES) $(TEST_OFILES) $(TEST_DFILES) \
		$(TEST_RUN_OFILES) $(TEST_RUN_DFILES) $(TEST_RUN_CFILES)

.PHONY: test
test: all $(TEST_EFILES)
	for test in $(TEST_EFILES); do \
		./$$test; \
	done

.PHONY: format format-check
format:
	clang-format -i $(LIB_CFILES) $(TEST_CFILES)
format-check:
	clang-format -n -Werror $(LIB_CFILES) $(TEST_CFILES)

# The library just archives all the object files
lib$(LIBNAME).a: $(LIB_OFILES) $(SLIB_OFILES)
	$(AR) -rc $@ $^
# Test executables must link with unity, along with their generated runner. They
# also depend on the library itself.
$(TEST_EFILES): %.elf: %.o %_run.o unity.o lib$(LIBNAME).a
	$(CC) $^ -o $@ -L. -l$(LIBNAME)

# Special rule for the test framework. We don't want to be writing over their
# repository, so we just write into our own.
unity.o: third-party/Unity/src/unity.c
	$(CC) $(CFLAGS) -c $< -o $@
# Usually, just output the `.o` file in the same place as the `.c` file.
# However, we use different flags for the library and the tests
$(LIB_OFILES): %.o: %.c
	$(CC) $(LIB_CFLAGS) -MMD -c $< -o $@
$(SLIB_OFILES): %.o: %.S
	$(CC) -c $< -o $@
$(TEST_OFILES) $(TEST_RUN_OFILES): %.o: %.c
	$(CC) $(TEST_CFLAGS) -MMD -c $< -o $@

$(TEST_RUN_CFILES): %_run.c: %.c
	./third-party/Unity/auto/generate_test_runner.rb $< $@

-include $(LIB_DFILES) $(TEST_DFILES) $(TEST_RUN_DFILES)
