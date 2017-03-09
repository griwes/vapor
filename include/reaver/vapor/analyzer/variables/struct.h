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

#include "../types/struct.h"
#include "variable.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class struct_variable : public variable
    {
    public:
        struct_variable(std::shared_ptr<struct_type> type, std::vector<std::unique_ptr<variable>> fields) : _type{ type }
        {
            auto members = _type->get_data_members();

            auto repl = replacements{};
            fields.reserve(members.size());
            std::transform(members.begin() + fields.size(), members.end(), std::back_inserter(fields), [&](auto && member) {
                auto def = member->get_default_value();
                assert(def);
                return def->get_variable()->clone_with_replacement(repl);
            });

            assert(fields.size() == members.size());

            _fields_in_order.reserve(fields.size());

            for (std::size_t i = 0; i < fields.size(); ++i)
            {
                _fields_in_order.push_back(fields[i].get());
                _fields[members[i]] = std::move(fields[i]);
            }
        }

        virtual type * get_type() const override
        {
            return _type.get();
        }

        virtual bool is_constant() const override
        {
            return std::all_of(_fields_in_order.begin(), _fields_in_order.end(), [](auto && field) { return field->is_constant(); });
        }

        virtual variable * get_member(const member_variable * var) const override
        {
            auto it = _fields.find(var);
            if (it != _fields.end())
            {
                return it->second.get();
            }

            return nullptr;
        }

        virtual variable * get_member(const std::u32string & name) const override
        {
            auto it = std::find_if(_fields.begin(), _fields.end(), [&](auto && elem) { return elem.first->get_name() == name; });
            if (it == _fields.end())
            {
                return nullptr;
            }

            return it->second.get();
        }

    private:
        virtual std::unique_ptr<variable> _clone_with_replacement(replacements & repl) const override
        {
            return std::make_unique<struct_variable>(
                _type, fmap(_fields_in_order, [&](auto && field) { return repl.variables.at(field)->clone_with_replacement(repl); }));
        }

        virtual variable_ir _codegen_ir(ir_generation_context & ctx) const override
        {
            return codegen::ir::struct_value{ fmap(_fields_in_order, [&](auto && field) { return get<codegen::ir::value>(field->codegen_ir(ctx)); }) };
        }

        std::shared_ptr<struct_type> _type;
        std::unordered_map<const member_variable *, std::unique_ptr<variable>> _fields;
        std::vector<variable *> _fields_in_order;
    };

    inline auto make_struct_variable(std::shared_ptr<struct_type> type, std::vector<std::unique_ptr<variable>> fields)
    {
        return std::make_unique<struct_variable>(type, std::move(fields));
    }
}
}
