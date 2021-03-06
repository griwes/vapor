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

#include "vapor/parser/member_expression.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    member_expression parse_member_expression(context & ctx)
    {
        member_expression ret;

        auto start = expect(ctx, lexer::token_type::dot).range.start();
        ret.member_name = parse_literal<lexer::token_type::identifier>(ctx);
        ret.range = { start, ret.member_name.value.range.end() };

        return ret;
    }

    void print(const member_expression & mexpr, std::ostream & os, print_context ctx)
    {
        os << styles::def << ctx << styles::rule_name << "member-expression";
        print_address_range(os, mexpr);
        os << '\n';

        print(mexpr.member_name, os, ctx.make_branch(true));
    }
}
}
