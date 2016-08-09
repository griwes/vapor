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

#pragma once

#include "../parser/block.h"
#include "scope.h"
#include "statement.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class block;
            std::shared_ptr<block> preanalyze_block(const parser::block &, std::shared_ptr<scope>);

            class block
            {
            public:
                block(const parser::block & parse, std::shared_ptr<scope> lex_scope) : _parse{ parse }, _scope{ std::make_shared<scope>(std::move(lex_scope)) }
                {
                    _statements = fmap(_parse.block_value, [&](auto && row) {
                        return fmap(row, make_overload_set(
                            [&](const parser::block & block) {
                                return preanalyze_block(block, _scope);
                            },
                            [&](const parser::statement & statement) {
                                return preanalyze_statement(statement, _scope);
                            }
                        ));
                    });

                    _value_expr = fmap(_parse.value_expression, [&](auto && val_expr) {
                        return preanalyze_expression(val_expr, _scope);
                    });
                }

            private:
                const parser::block & _parse;
                std::shared_ptr<scope> _scope;
                std::vector<variant<
                    std::shared_ptr<block>,
                    statement
                >> _statements;
                optional<expression> _value_expr;
            };

            std::shared_ptr<block> preanalyze_block(const parser::block & parse, std::shared_ptr<scope> lex_scope)
            {
                return std::make_shared<block>(parse, std::move(lex_scope));
            }
        }}
    }
}

