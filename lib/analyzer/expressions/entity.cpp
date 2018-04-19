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

#include "vapor/analyzer/expressions/entity.h"
#include "vapor/analyzer/expressions/overload_set.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/precontext.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/types/unresolved.h"

#include "entity.pb.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    entity::entity(type * t, std::unique_ptr<expression> wrapped) : expression{ t }, _wrapped{ std::move(wrapped) }
    {
    }

    entity::entity(std::unique_ptr<type> t, std::unique_ptr<expression> wrapped) : expression{ t.get() }, _owned{ std::move(t) }, _wrapped{ std::move(wrapped) }
    {
    }

    entity::entity(std::shared_ptr<unresolved_type> res, std::unique_ptr<expression> wrapped) : _unresolved{ std::move(res) }, _wrapped{ std::move(wrapped) }
    {
        if (auto t = _unresolved.value()->get_resolved())
        {
            _set_type(t);
        }
    }

    void entity::print(std::ostream & os, print_context ctx) const
    {
        assert(0);
    }

    std::unique_ptr<expression> entity::_clone_expr_with_replacement(replacements & repl) const
    {
        assert(0);
    }

    statement_ir entity::_codegen_ir(ir_generation_context & ctx) const
    {
        assert(0);
    }

    std::unique_ptr<entity> get_entity(precontext & ctx, const proto::entity & ent, const std::map<std::string, const proto::entity *> & associated)
    {
        auto type = get_imported_type_ref(ctx, ent.type());

        auto expr = [&]() -> std::unique_ptr<expression> {
            switch (ent.value_case())
            {
                case proto::entity::kTypeValue:
                    return get_imported_type(ctx, ent.type_value());

                case proto::entity::kOverloadSet:
                {
                    assert(ent.associated_entities_size() == 1);
                    auto type = import_overload_set_type(ctx, associated.at(ent.associated_entities(0))->type_value().overload_set());
                    return std::make_unique<overload_set>(std::move(type));
                }

                default:
                    throw exception{ logger::fatal } << "unknown expression kind of symbol `" << ctx.current_scope.top() << "." << ctx.current_symbol
                                                     << "` in imported file " << ctx.current_file.top();
            }
        }();

        return std::get<0>(fmap(type, [&](auto && type) { return std::make_unique<entity>(std::move(type), std::move(expr)); }));
    }
}
}
