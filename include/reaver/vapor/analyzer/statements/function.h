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

#pragma once

#include <memory>

#include "../expressions/expression.h"
#include "../function.h"
#include "../semantic/parameter_list.h"
#include "../statements/block.h"
#include "../statements/statement.h"
#include "../symbol.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class function_definition;
    class overload_set;

    class function_definition : public statement
    {
    public:
        function_definition(ast_node parse,
            std::u32string name,
            parameter_list params,
            optional<std::unique_ptr<expression>> return_type,
            std::unique_ptr<block> body,
            std::unique_ptr<scope> scope);

        function * get_function() const
        {
            return _function.get();
        }

        virtual void print(std::ostream & os, print_context ctx) const override;

    private:
        virtual future<> _analyze(analysis_context &) override;

        virtual std::unique_ptr<statement> _clone_with_replacement(replacements &) const override
        {
            return make_null_statement();
        }

        virtual future<statement *> _simplify(recursive_context) override;
        virtual statement_ir _codegen_ir(ir_generation_context &) const override;

        std::u32string _name;
        parameter_list _parameter_list;

        optional<std::unique_ptr<expression>> _return_type;
        std::unique_ptr<block> _body;
        std::unique_ptr<scope> _scope;
        std::unique_ptr<function> _function;
        std::shared_ptr<overload_set> _overload_set;
        std::unique_ptr<expression> _self;
    };
}
}

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct function_definition;
}
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<function_definition> preanalyze_function_definition(const parser::function_definition & func, scope *& lex_scope);
}
}
