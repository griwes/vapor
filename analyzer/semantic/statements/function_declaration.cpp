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

#include <numeric>

#include "vapor/analyzer/function.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/statements/block.h"
#include "vapor/analyzer/statements/function_declaration.h"
#include "vapor/analyzer/variables/overload_set.h"
#include "vapor/codegen/ir/function.h"
#include "vapor/parser/lambda_expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> function_declaration::_analyze(analysis_context & ctx)
    {
        _function = make_function("overloadable function",
            nullptr,
            {},
            [=, name = _parse.name.string](ir_generation_context & ctx) {
                return codegen::ir::function{ U"operator()",
                    {},
                    fmap(_argument_list,
                        [&](auto && arg) { return get<std::shared_ptr<codegen::ir::variable>>(get<codegen::ir::value>(arg.variable->codegen_ir(ctx))); }),
                    _body->codegen_return(ctx),
                    _body->codegen_ir(ctx) };
            },
            _parse.range);
        _function->set_name(U"operator()");
        _overload_set->add_function(this);

        auto initial_future = [&] {
            if (_return_type)
            {
                return (*_return_type)->analyze(ctx).then([&] {
                    auto var = (*_return_type)->get_variable();

                    assert(var->get_type() == builtin_types().type.get());
                    assert(var->is_constant());

                    _function->set_return_type(dynamic_cast<type_variable *>(var)->get_value());
                });
            }

            return make_ready_future();
        }();

        return initial_future.then([&] { return when_all(fmap(_argument_list, [&](auto && arg) { return arg.type_expression->analyze(ctx); })); })
            .then([&] {
                auto arg_variables = fmap(_argument_list, [&](auto && arg) -> variable * {
                    arg.variable->set_type(arg.type_expression->get_variable());
                    return arg.variable.get();
                });

                _function->set_arguments(std::move(arg_variables));

                return _body->analyze(ctx);
            })
            .then([&] {
                fmap(_return_type, [&](auto && ret_type) {
                    auto explicit_type = ret_type->get_variable();
                    auto type_var = static_cast<type_variable *>(explicit_type);

                    assert(type_var->get_value() == _body->return_type());

                    return unit{};
                });

                _function->set_body(_body.get());

                if (!_function->return_type().try_get())
                {
                    _function->set_return_type(_body->return_type());
                }
            });
    }
}
}