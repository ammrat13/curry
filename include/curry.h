/**
 * \file curry.h
 * \brief Library to curry C functions
 *
 * This library assumes that all arguments are `uint64_t`. It allows the user to
 * pass the first few arguments and receive a function pointer that can be used
 * on the remaining arguments. That function pointer is dynamically allocated,
 * and it is freed just before it returns.
 */
#pragma once

#include <stddef.h>

/**
 * \brief Curries a function
 *
 * This function can fail, either if memory allocation fails or if the number of
 * arguments is to large to be managable.
 *
 * \param [in] fn The function to curry
 * \param [in] nargs_now The number of arguments passed to this call to `curry`
 * \param [in] nargs_later The number of arguments that will be passed when the
 * returned function pointer is called
 * \return A function pointer, or `NULL` on failure
 */
void *curry(void *fn, size_t nargs_now, size_t nargs_later, ...);

/**
 * \brief Maximum number of arguments that can be passed to `curry`
 * \details This limit applies to both `nargs_now` and `nargs_later`.
 * \see curry
 */
#define CURRY_NARGS_MAX (32)
