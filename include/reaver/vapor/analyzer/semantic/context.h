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

#include "../simplification/context.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class type;

    class analysis_context
    {
    public:
        analysis_context() : results{ std::make_shared<cached_results>() }, simplification_ctx{ std::make_shared<simplification_context>(*results) }
        {
        }

        std::shared_ptr<cached_results> results;
        std::shared_ptr<simplification_context> simplification_ctx;
        std::unordered_map<std::size_t, std::shared_ptr<type>> sized_integers;
        bool entry_point_marked = false;
        bool entry_variable_marked = false;
    };
}
}
