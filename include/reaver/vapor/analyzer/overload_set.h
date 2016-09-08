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

#include <memory>

#include "variable.h"
#include "function.h"
#include "statement.h"
#include "block.h"
#include "symbol.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class overload_set_type : public type
            {
            public:
                void add_function(std::shared_ptr<function> fn)
                {
                    _functions.push_back(std::move(fn));
                }

                virtual std::string explain() const override
                {
                    assert(0);
                }

            private:
                std::vector<std::shared_ptr<function>> _functions;
            };

            class function_declaration;

            class overload_set : public variable
            {
            public:
                overload_set() : _type{ std::make_shared<overload_set_type>() }
                {
                }

                void add_function(std::shared_ptr<function_declaration> fn);

                virtual std::shared_ptr<type> get_type() const override
                {
                    return _type;
                }

            private:
                std::vector<std::shared_ptr<function_declaration>> _overloads;
                std::shared_ptr<overload_set_type> _type;
            };

            class function_declaration : public statement, public std::enable_shared_from_this<function_declaration>
            {
            public:
                function_declaration(const parser::function & parse, std::shared_ptr<scope> scope)
                    : _parse{ parse }, _scope{ scope }
                {
                    assert(!_parse.arguments);

                    _body = preanalyze_block(*_parse.body, scope);
                    auto set = _scope->get_or_init(_parse.name.string,
                        [&]{ return make_symbol(_parse.name.string, std::make_shared<overload_set>()); });
                }

                std::shared_ptr<function> get_function() const
                {
                    return _function;
                }

            private:
                virtual future<> _analyze() override
                {
                    return _body->analyze().then([&]{
                        _function = make_function(_body->return_or_value_type(), {}, [](){ assert(!"implement functions at all"); });

                        auto set = _scope->get(_parse.name.string);
                        auto overloads = std::dynamic_pointer_cast<overload_set>(set->get_variable());
                        assert(overloads);
                        overloads->add_function(shared_from_this());
                    });
                }

                const parser::function & _parse;

                std::shared_ptr<block> _body;
                std::shared_ptr<scope> _scope;
                std::shared_ptr<function> _function;
            };

            inline void overload_set::add_function(std::shared_ptr<function_declaration> fn)
            {
                _type->add_function(fn->get_function());
                _overloads.push_back(std::move(fn));
            }

            inline std::shared_ptr<function_declaration> preanalyze_function(const parser::function & func, std::shared_ptr<scope> & lex_scope)
            {
                return std::make_shared<function_declaration>(func, lex_scope);
            }
        }}
    }
}

