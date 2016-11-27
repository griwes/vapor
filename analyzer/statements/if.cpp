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

#include "vapor/analyzer/statements/if.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/variables/boolean.h"
#include "vapor/parser/lambda_expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    void if_statement::print(std::ostream & os, std::size_t indent) const
    {
        auto in = std::string(indent, ' ');
        os << in << "if statement at " << _parse.range << '\n';
        os << in << "condition:\n";
        os << in << "{\n";
        _condition->print(os, indent + 4);
        os << in << "}\n";

        os << in << "then-block:\n";
        os << in << "{\n";
        _then_block->print(os, indent + 4);
        os << in << "}\n";

        fmap(_else_block, [&](auto && block) {
            os << in << "else-block:\n";
            os << in << "{\n";
            block->print(os, indent + 4);
            os << in << "}\n";
            return unit{};
        });
    }

    statement_ir if_statement::_codegen_ir(ir_generation_context & ctx) const
    {
        // instructions returning those labels are silly
        // really need to do this better at some point
        // (future refactor perhaps)
        // (perhaps once I have tuples, so they can return unit type values)

        auto condition_instructions = _condition->codegen_ir(ctx);
        auto condition_variable = condition_instructions.back().result;

        auto negated_variable = codegen::ir::make_variable(builtin_types().boolean->codegen_type(ctx));
        auto negation = codegen::ir::instruction{
            {}, {}, { boost::typeindex::type_id<codegen::ir::boolean_negation_instruction>() }, { condition_variable }, negated_variable
        };

        auto else_label = U"__else_" + boost::locale::conv::utf_to_utf<char32_t>(std::to_string(ctx.label_index++));
        auto after_else_label = U"__after_else_" + boost::locale::conv::utf_to_utf<char32_t>(std::to_string(ctx.label_index++));

        auto then_instructions = _then_block->codegen_ir(ctx);
        auto else_instructions = [&]() {
            statement_ir ir;
            if (_else_block)
            {
                ir = _else_block.get()->codegen_ir(ctx);
            }
            else
            {
                ir = statement_ir{ { {}, {}, { boost::typeindex::type_id<codegen::ir::noop_instruction>() }, {}, codegen::ir::label{ else_label, {} } } };
            }

            ir.front().label = else_label;
            return ir;
        }();

        auto jump = codegen::ir::instruction{ {},
            {},
            { boost::typeindex::type_id<codegen::ir::jump_instruction>() },
            { negated_variable, codegen::ir::label{ else_label, {} } },
            codegen::ir::label{ else_label, {} } };

        auto jump_over_else = codegen::ir::instruction{ {},
            {},
            { boost::typeindex::type_id<codegen::ir::jump_instruction>() },
            { codegen::ir::boolean_value{ true }, codegen::ir::label{ after_else_label, {} } },
            codegen::ir::label{ after_else_label, {} } };

        auto after_else = codegen::ir::instruction{
            after_else_label, {}, { boost::typeindex::type_id<codegen::ir::noop_instruction>() }, {}, codegen::ir::label{ after_else_label, {} }
        };

        statement_ir ret;
        ret.reserve(condition_instructions.size() + 4 + then_instructions.size() + else_instructions.size());
        std::move(condition_instructions.begin(), condition_instructions.end(), std::back_inserter(ret));
        ret.push_back(std::move(negation));
        ret.push_back(std::move(jump));
        std::move(then_instructions.begin(), then_instructions.end(), std::back_inserter(ret));
        ret.push_back(std::move(jump_over_else));
        std::move(else_instructions.begin(), else_instructions.end(), std::back_inserter(ret));
        ret.push_back(std::move(after_else));
        return ret;
    }
}
}