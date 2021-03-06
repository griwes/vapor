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

#include "type.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class function;

    // TODO: combined_overload_set and combined_overload_set_type
    // those need to be distinct from normal ones for code generation reasons (SCOPES!)
    // this is not a crucial feature *right now*, but it will be, and rather soon
    // this is just a note so that I remember why the hell I can't use the existing types
    //
    // random bikeshedding thoughts:
    // actually I might be able to do this differently, on the function level
    // i.e. create a proxy_function that generates a function in current scope that calls
    // a function that's in a different scope
    // but that's going to be funny

    class overload_set_type : public type
    {
    public:
        overload_set_type(scope * lex_scope) : type{ lex_scope }
        {
        }

        void add_function(function * fn);

        virtual std::string explain() const override
        {
            return "overload set (TODO: add location and member info)";
        }

        virtual void print(std::ostream & os, print_context ctx) const override;

        virtual future<std::vector<function *>> get_candidates(lexer::token_type bracket) const override;

    private:
        virtual void _codegen_type(ir_generation_context &) const override;

        virtual std::u32string _codegen_name(ir_generation_context & ctx) const override
        {
            if (!_codegen_type_name)
            {
                _codegen_type_name = U"overload_set_" + utf32(std::to_string(ctx.overload_set_index++));
            }

            return *_codegen_type_name;
        }

        mutable std::mutex _functions_lock;
        mutable optional<std::u32string> _codegen_type_name;
        std::vector<function *> _functions;
    };
}
}
