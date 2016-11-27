/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/simplification/context.h"
#include "vapor/analyzer/expressions/expression.h"
#include "vapor/analyzer/symbol.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    simplification_context::~simplification_context() = default;

    void simplification_context::_handle_expressions(expression * ptr, future<expression *> & fut)
    {
        if (_statement_futures.find(ptr) != _statement_futures.end())
        {
            return;
        }

        _statement_futures.emplace(ptr, fut.then([](auto && expr) { return static_cast<statement *>(expr); }));
    }

    void simplification_context::keep_alive(statement * ptr)
    {
        std::lock_guard<std::mutex> lock{ _keep_alive_lock };
        auto inserted = _keep_alive_stmt.emplace(ptr).second;
        assert(inserted);
    }

    void simplification_context::keep_alive(variable * ptr)
    {
        std::lock_guard<std::mutex> lock{ _keep_alive_lock };
        auto inserted = _keep_alive_var.emplace(ptr).second;
        assert(inserted);
    }
}
}