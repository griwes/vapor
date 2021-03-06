/**
 * Vapor Compiler Licence
 *
 * Copyright © 2015-2017 Michał "Griwes" Dominiak
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

#include "vapor/parser/statement.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    bool operator==(const statement & lhs, const statement & rhs)
    {
        return lhs.range == rhs.range && lhs.statement_value == rhs.statement_value;
    }

    statement parse_statement(context & ctx)
    {
        statement ret;

        if (!peek(ctx))
        {
            throw expectation_failure{ "statement" };
        }

        auto type = peek(ctx)->type;

        switch (type)
        {
            case lexer::token_type::function:
            {
                auto func = parse_function(ctx);

                fmap(func,
                    make_overload_set(
                        [&](function_definition & func) {
                            ret.range = func.range;
                            ret.statement_value = std::move(func);
                            return unit{};
                        },
                        [&](function_declaration & func) {
                            assert("function declaration not allowed in the context; TODO: make this a sensible error");
                            return unit{};
                        }));

                return ret;
            }

            case lexer::token_type::if_:
            {
                auto if_ = parse_if_statement(ctx);
                ret.range = if_.range;
                ret.statement_value = std::move(if_);
                return ret;
            }

            case lexer::token_type::let:
                ret.statement_value = parse_declaration(ctx);
                break;

            case lexer::token_type::return_:
                ret.statement_value = parse_return_expression(ctx);
                break;

            case lexer::token_type::struct_:
                ret.statement_value = parse_struct_declaration(ctx);
                break;

            case lexer::token_type::with:
                ret.statement_value = parse_template_declaration(ctx);
                break;

            case lexer::token_type::default_:
                ret.statement_value = parse_default_instance(ctx);
                break;

            default:
                ret.statement_value = parse_expression_list(ctx);
        }

        auto end = expect(ctx, lexer::token_type::semicolon).range.end();
        visit(
            [&](const auto & value) -> unit {
                ret.range = { value.range.start(), end };
                return {};
            },
            ret.statement_value);

        return ret;
    }

    void print(const statement & stmt, std::ostream & os, print_context ctx)
    {
        os << styles::def << ctx << styles::rule_name << "statement";
        print_address_range(os, stmt);
        os << '\n';

        fmap(stmt.statement_value, [&](const auto & value) -> unit {
            print(value, os, ctx.make_branch(true));
            return {};
        });
    }
}
}
