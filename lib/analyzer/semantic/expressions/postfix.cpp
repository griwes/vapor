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

#include "vapor/analyzer/expressions/postfix.h"
#include "vapor/analyzer/expressions/call.h"
#include "vapor/analyzer/expressions/expression_list.h"
#include "vapor/analyzer/expressions/identifier.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/semantic/overloads.h"
#include "vapor/parser.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> postfix_expression::_analyze(analysis_context & ctx)
    {
        return _base_expr->analyze(ctx)
            .then([&] {
                auto expr_ctx = get_context();
                expr_ctx.push_back(this);

                return when_all(fmap(_arguments, [&](auto && expr) {
                    expr->set_context(expr_ctx);
                    return expr->analyze(ctx);
                }));
            })
            .then([&] {
                if (!_modifier)
                {
                    this->_set_type(_base_expr->get_type());
                    return make_ready_future();
                }

                if (_modifier == lexer::token_type::dot)
                {
                    return _base_expr->get_type()
                        ->get_scope()
                        ->get_future(*_accessed_member)
                        .then([](auto && symb) { return symb->get_expression_future(); })
                        .then([&](auto && var) {
                            _referenced_expression = var;
                            this->_set_type(_referenced_expression.get()->get_type());
                        });
                }

                return resolve_overload(ctx, get_ast_info()->range, _base_expr.get(), *_modifier, fmap(_arguments, [](auto && arg) { return arg.get(); }))
                    .then([&](std::unique_ptr<expression> call_expr) {
                        if (auto call_expr_downcasted = call_expr->as<call_expression>())
                        {
                            call_expr_downcasted->set_ast_info(get_ast_info().get());
                        }
                        _call_expression = std::move(call_expr);
                        return _call_expression->analyze(ctx);
                    })
                    .then([&] { this->_set_type(_call_expression->get_type()); });
            });
    }
}
}
