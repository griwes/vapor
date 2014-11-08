/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014 Michał "Griwes" Dominiak
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
#include <thread>
#include <atomic>

#include <boost/optional.hpp>

#include <reaver/semaphore.h>

#include "vapor/position.h"
#include "vapor/lexer/token.h"
#include "vapor/lexer/detail/lexer_node.h"
#include "vapor/lexer/errors.h"

namespace reaver
{
    namespace vapor
    {
        namespace lexer { inline namespace _v1
        {
            namespace _detail
            {

                class _iterator_backend
                {
                public:
                    friend class lexer::iterator;

                    template<typename Iter>
                    _iterator_backend(Iter begin, Iter end, std::shared_ptr<_lexer_node> & node) : _thread{ [&](){ _worker(begin, end); } }
                    {
                        _sem.wait();
                        node = std::move(_initial);
                    }

                    ~_iterator_backend()
                    {
                        _end_flag = true;
                        _thread.join();
                    }

                private:
                    template<typename Iter>
                    void _worker(Iter begin, Iter end, std::string filename = "")
                    {
                        std::shared_ptr<_lexer_node> node = nullptr;

                        position pos;
                        pos.offset = -1;
                        pos.column = 0;
                        pos.line = 1;
                        pos.file = std::move(filename);

                        auto get = [&]() -> boost::optional<char>
                        {
                            if (begin == end)
                            {
                                return {};
                            }

                            if (*begin == '\n')
                            {
                                pos.column = 0;
                                ++pos.line;
                            }
                            else
                            {
                                ++pos.column;
                            }

                            ++pos.offset;
                            return *begin++;
                        };

                        auto peek = [&](std::size_t x = 0) -> boost::optional<char>
                        {
                            for (std::size_t i = 0; i < x; ++i)
                            {
                                if (begin + i == end)
                                {
                                    return {};
                                }
                            }

                            return *(begin + x);
                        };

                        auto generate_token = [&](token_type type, position begin, position end, std::string string)
                        {
                            if (!node)
                            {
                                _initial = std::make_shared<_lexer_node>(token{ type, std::move(string), range(begin, end) }, _engine);
                                _sem.notify();
                                node = _initial;
                            }

                            else
                            {
                                node->_next = std::make_shared<_lexer_node>(token{ type, std::move(string), range(begin, end) }, _engine);
                                node->_sem.notify();
                                node = node->_next;
                            }
                        };

                        auto notify = [&]()
                        {
                            if (!node)
                            {
                                _sem.notify();
                                return;
                            }

                            node->_sem.notify();
                        };

                        auto is_white_space = [](char c)
                        {
                            return c == ' ' || c == '\t' || c == '\n' || c == '\r';
                        };

                        auto is_identifier_start = [](char c)
                        {
                            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
                        };

                        auto is_identifier_char = [&](char c)
                        {
                            return is_identifier_start(c) || (c >= '0' && c <= '9');
                        };

                        while (!_end_flag && begin != end)
                        {
                            auto next = get();

                            if (is_white_space(*next))
                            {
                                continue;
                            }

                            auto p = pos;
                            if (next == '/')
                            {
                                auto second = peek();

                                if (second == '/')
                                {
                                    while ((next = get()) && *next != '\n')
                                    {
                                    }

                                    continue;
                                }

                                if (second == '*')
                                {
                                    get();

                                    while ((next = get()) && (second = peek()) && next != '*' && second != '/')
                                    {
                                    }

                                    if (next && second && next == '*' && second == '/')
                                    {
                                        get();
                                        continue;
                                    }

                                    _engine.push(unterminated_comment{ { p, pos } });
                                    notify();
                                    return;
                                }
                            }

                            {
                                auto second = peek();
                                auto third = peek(1);

                                if (second && third && symbols3.find(*next) != symbols3.end() && symbols3[*next].find(*second) != symbols3[*next].end()
                                    && symbols3[*next][*second].find(*third) != symbols3[*next][*second].end())
                                {
                                    auto p = pos;
                                    generate_token(symbols3[*next][*second][*third], p, p + 3, { *next, *get(), *get() });
                                    continue;
                                }

                                else if (second && symbols2.find(*next) != symbols2.end() && symbols2[*next].find(*second) != symbols2[*next].end())
                                {
                                    auto p = pos;
                                    generate_token(symbols2[*next][*second], p, p + 2, { *next, *get() })   ;
                                    continue;
                                }

                                else if (symbols1.find(*next) != symbols1.end())
                                {
                                    auto p = pos;
                                    generate_token(symbols1[*next], p, p + 1, { *next });
                                    continue;
                                }
                            }

                            std::string variable_length;

                            if (next == '"')
                            {
                                auto second = peek();

                                while (next && second && (second != '"' || next == '\\') && (second != '\n' || next == '\\'))
                                {
                                    variable_length.push_back(*next);

                                    next = get();
                                    second = peek();
                                }

                                if (!next || second == '\n')
                                {
                                    _engine.push(unterminated_string{ { p, p + variable_length.size() } });
                                    notify();
                                    return;
                                }

                                variable_length.push_back(*next);
                                variable_length.push_back(*get());

                                generate_token(token_type::string, p, p + variable_length.size(), variable_length);
                                continue;
                            }

                            if (is_identifier_start(*next))
                            {
                                do
                                {
                                    variable_length.push_back(*next);
                                } while (peek() && is_identifier_char(*peek()) && (next = get()));

                                if (keywords.find(variable_length) != keywords.end())
                                {
                                    generate_token(keywords[variable_length], p, p + variable_length.size(), variable_length);
                                    continue;
                                }

                                generate_token(token_type::identifier, p, p + variable_length.size(), variable_length);
                                continue;
                            }
                        }

                        node->_done = true;
                        node->_sem.notify();
                    }

                    std::atomic<bool> _end_flag{ false };
                    std::mutex _m;
                    std::shared_ptr<_lexer_node> _initial = nullptr;
                    semaphore _sem;
                    std::thread _thread;
                    error_engine _engine;
                };
            }
        }}
    }
}
