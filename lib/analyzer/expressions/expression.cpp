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

#include "vapor/analyzer/expressions/expression.h"
#include "vapor/analyzer/expressions/binary.h"
#include "vapor/analyzer/expressions/boolean.h"
#include "vapor/analyzer/expressions/closure.h"
#include "vapor/analyzer/expressions/expression_list.h"
#include "vapor/analyzer/expressions/import.h"
#include "vapor/analyzer/expressions/integer.h"
#include "vapor/analyzer/expressions/member_access.h"
#include "vapor/analyzer/expressions/postfix.h"
#include "vapor/analyzer/expressions/struct.h"
#include "vapor/analyzer/expressions/unary.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<expression> preanalyze_expression(const parser::_v1::expression & expr, scope * lex_scope)
    {
        return get<0>(fmap(expr.expression_value,
            make_overload_set(
                [](const parser::string_literal & string) -> std::unique_ptr<expression> {
                    assert(0);
                    return nullptr;
                },

                [](const parser::integer_literal & integer) -> std::unique_ptr<expression> { return make_integer_constant(integer); },

                [](const parser::boolean_literal & boolean) -> std::unique_ptr<expression> { return make_boolean_constant(boolean); },

                [&](const parser::postfix_expression & postfix) -> std::unique_ptr<expression> {
                    auto pexpr = preanalyze_postfix_expression(postfix, lex_scope);
                    return pexpr;
                },

                [](const parser::import_expression & import) -> std::unique_ptr<expression> {
                    assert(0);
                    return std::unique_ptr<import_expression>();
                },

                [&](const parser::lambda_expression & lambda_expr) -> std::unique_ptr<expression> {
                    auto lambda = preanalyze_closure(lambda_expr, lex_scope);
                    return lambda;
                },

                [](const parser::unary_expression & unary_expr) -> std::unique_ptr<expression> {
                    assert(0);
                    return std::unique_ptr<unary_expression>();
                },

                [&](const parser::binary_expression & binary_expr) -> std::unique_ptr<expression> {
                    auto binexpr = preanalyze_binary_expression(binary_expr, lex_scope);
                    return binexpr;
                },

                [&](const parser::struct_literal & struct_lit) -> std::unique_ptr<expression> {
                    auto struct_expr = preanalyze_struct_literal(struct_lit, lex_scope);
                    return struct_expr;
                },

                [&](const parser::member_expression & mexpr) -> std::unique_ptr<expression> {
                    auto memexpr = preanalyze_member_access_expression(mexpr, lex_scope);
                    return memexpr;
                },

                [](auto &&) -> std::unique_ptr<expression> { assert(0); })));
    }
}
}
