/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2019 Michał "Griwes" Dominiak
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

#include <numeric>

#include "vapor/analyzer/expressions/entity.h"
#include "vapor/analyzer/expressions/expression.h"
#include "vapor/analyzer/expressions/overload_set.h"
#include "vapor/analyzer/expressions/type.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/precontext.h"
#include "vapor/analyzer/semantic/function.h"
#include "vapor/analyzer/semantic/symbol.h"
#include "vapor/analyzer/types/overload_set.h"
#include "vapor/analyzer/types/unresolved.h"
#include "vapor/codegen/ir/type.h"

#include "types/overload_set.pb.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<std::vector<function *>> overload_set_type::get_candidates(lexer::token_type bracket) const
    {
        if (bracket == lexer::token_type::round_bracket_open)
        {
            return make_ready_future([&] { return _oset->get_overloads(); }());
        }

        assert(0);
        return make_ready_future(std::vector<function *>{});
    }

    void overload_set_type::_codegen_type(ir_generation_context & ctx) const
    {
        auto actual_type = *_codegen_t;
        auto members = fmap(_oset->get_overloads(), [&](auto && fn) {
            ctx.add_generated_function(fn);
            return codegen::ir::member{ fn->codegen_ir(ctx) };
        });
        auto type = codegen::ir::variable_type{
            _codegen_name(ctx), get_scope()->codegen_ir(), 0, std::move(members)
        };

        auto scopes = get_scope()->codegen_ir();
        scopes.emplace_back(type.name, codegen::ir::scope_type::type);

        fmap(type.members, [&](auto && member) {
            fmap(member,
                make_overload_set(
                    [&](codegen::ir::function & fn) {
                        fn.scopes = scopes;
                        fn.parent_type = actual_type;
                        fn.is_exported = _is_exported;

                        return unit{};
                    },
                    [&](auto &&) {
                        assert(0);
                        return unit{};
                    }));
            return unit{};
        });

        *actual_type = std::move(type);
    }

    void overload_set_type::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::type << "overload set type";
        os << styles::def << " @ " << styles::address << this << styles::def << ":\n";

        std::size_t idx = 0;
        for (auto && function : _oset->get_overloads())
        {
            function->print(os, ctx.make_branch(++idx == _oset->get_overloads().size()));
        }
    }

    std::unique_ptr<google::protobuf::Message> overload_set_type::_user_defined_interface() const
    {
        auto t = std::make_unique<proto::overload_set_type>();

        for (auto && func : _oset->get_overloads())
        {
            auto fn = t->add_functions();
            fn->set_allocated_return_type(func->get_return_type()
                                              .try_get()
                                              .value()
                                              ->as<type_expression>()
                                              ->get_value()
                                              ->generate_interface_reference()
                                              .release());

            for (auto && param : func->parameters())
            {
                auto & par = *fn->add_parameters();
                par.set_allocated_type(param->get_type()->generate_interface_reference().release());
            }

            fn->set_is_member(func->is_member());
        }

        return t;
    }
}
}
