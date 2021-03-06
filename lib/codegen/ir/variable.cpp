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

#include "vapor/codegen/ir/variable.h"
#include "vapor/codegen/ir/type.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    std::shared_ptr<ir::variable_type> ir::get_type(const ir::value & val)
    {
        return get<0>(fmap(val,
            make_overload_set([](const std::shared_ptr<variable> & var) { return var->type; },
                [](const ir::integer_value &) { return ir::builtin_types().integer; },
                [](auto &&) {
                    assert(0);
                    return std::shared_ptr<ir::variable_type>();
                })));
    }
}
}
