/**
 * Vapor Compiler Licence
 *
 * Copyright © 2018 Michał "Griwes" Dominiak
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

#include "../expressions/type.h"
#include "type.h"

#include <variant>

namespace reaver::vapor::proto
{
class user_defined_reference;
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    struct precontext;

    class unresolved_type : public std::enable_shared_from_this<unresolved_type>
    {
    public:
        unresolved_type(const proto::user_defined_reference * reference, scope * lex_scope);
        future<> resolve(analysis_context &);

        type * get_resolved() const
        {
            return _resolved;
        }

        std::unique_ptr<expression> get_expression();

    private:
        type * _resolved = nullptr;
        scope * _lex_scope = nullptr;
        std::unique_ptr<proto::user_defined_reference> _reference;

        std::optional<future<>> _analysis_future;
    };

    using imported_type = std::variant<type *, std::shared_ptr<unresolved_type>>;

    std::unique_ptr<expression> get_imported_type(precontext &, const proto::type &);
    imported_type get_imported_type_ref(precontext &, const proto::type_reference &);
    std::unique_ptr<expression> get_imported_type_ref_expr(precontext &, const proto::type_reference &);
}
}
