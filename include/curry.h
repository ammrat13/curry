/**
 * \file curry.h
 * \brief Library to curry C functions
 *
 * This library assumes that all arguments are `uint64_t`, and it assumes that
 * the function returns in %rax. It allows the user to pass the first few
 * arguments and receive a function pointer that can be used on the remaining
 * arguments. That function pointer is dynamically allocated, and it is freed
 * just before it returns.
 */
#pragma once

#include <stdarg.h>
#include <stddef.h>

/**
 * \brief Variadic version of `vcurry`
 * \see vcurry
 */
void *curry(void *fn, size_t nargs_now, size_t nargs_later, ...);

/**
 * \brief Curries a function
 *
 * This function can fail, either if memory allocation fails or if the number of
 * arguments is to large to be managable.
 *
 * This function modifies `args_now`, but does not call `va_end` on it. It's the
 * caller's job to free that structure.
 *
 * \param [in] fn The function to curry
 * \param [in] nargs_now The number of arguments passed to this call to `curry`
 * via `args_now`
 * \param [in] nargs_later The number of arguments that will be passed when the
 * returned function pointer is called
 * \param [in] args_now The arguments to be remembered on the returned function
 * \return A function pointer, or `NULL` on failure
 */
void *vcurry(void *fn, size_t nargs_now, size_t nargs_later, va_list args_now);

/**
 * \brief Maximum number of arguments that can be passed to `curry`
 * \details This limit applies to both `nargs_now` and `nargs_later`.
 * \see vcurry
 * \see curry
 */
#define CURRY_NARGS_MAX (32)
