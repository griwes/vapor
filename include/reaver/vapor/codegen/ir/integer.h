/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2017 Michał "Griwes" Dominiak
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation is required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 **/

#pragma once

#include <boost/multiprecision/cpp_int.hpp>

#include <reaver/optional.h>

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    namespace ir
    {
        struct integer_value
        {
            boost::multiprecision::cpp_int value;
            optional<std::size_t> size = none;
        };

        struct integer_addition_instruction
        {
        };
        struct integer_subtraction_instruction
        {
        };
        struct integer_multiplication_instruction
        {
        };
        struct integer_equal_comparison_instruction
        {
        };
        struct integer_less_comparison_instruction
        {
        };
        struct integer_less_equal_comparison_instruction
        {
        };
    }
}
}
