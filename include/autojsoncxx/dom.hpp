// The MIT License (MIT)
//
// Copyright (c) 2014 Siyuan Ren (netheril96@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef AUTOJSONCXX_DOM_HPP_29A4C106C1B1
#define AUTOJSONCXX_DOM_HPP_29A4C106C1B1

#include <autojsoncxx/base.hpp>
#include <autojsoncxx/error.hpp>

#include <rapidjson/document.h>

#include <stack>
#include <cassert>

namespace autojsoncxx {
template <class Encoding, class Allocator, class StackAllocator>
class SAXEventHandler<rapidjson::GenericDocument<Encoding, Allocator, StackAllocator> > {
public:
    typedef rapidjson::GenericDocument<Encoding, Allocator, StackAllocator> document_type;
    typedef rapidjson::GenericValue<Encoding, Allocator> value_type;
    typedef typename value_type::Ch char_type;

private:
    // Workarounds for the lack of copy constructor for value_type
    // This class moves value around, like auto_ptr
    class value_shim {
    private:
        value_type internal;

    public:
        value_shim()
            : internal()
        {
        }

        value_shim(value_type& v)
        {
            internal.Swap(v);
        }

        value_shim(value_shim& that)
        {
            internal.Swap(that.internal);
        }

        value_type& value() AUTOJSONCXX_NOEXCEPT
        {
            return internal;
        }

        const value_type& value() const AUTOJSONCXX_NOEXCEPT
        {
            return internal;
        }

#if AUTOJSONCXX_HAS_RVALUE
    private:
        value_shim(value_shim&&);

        value_shim& operator=(value_shim&&);
#endif
    };

    class value_stack {
    private:
        std::stack<value_shim> internal_stack;

    public:
        bool empty() const AUTOJSONCXX_NOEXCEPT
        {
            return internal_stack.empty();
        }

        void emplace()
        {
            value_shim shim;
            internal_stack.push(shim);
        }

        void push(value_type& v)
        {
            value_shim shim(v);
            internal_stack.push(shim);
        }

        value_type& top()
        {
            return internal_stack.top().value();
        }

        const value_type& top() const
        {
            return internal_stack.top().value();
        }

        void pop()
        {
            internal_stack.pop();
        }

        std::size_t size() const AUTOJSONCXX_NOEXCEPT
        {
            return internal_stack.size();
        }
    };

private:
    value_stack working_stack;
    utility::scoped_ptr<error::ErrorBase> the_error;
    document_type* doc;

private:
    // Implicitly treat the document as the bottom of stack
    value_type* top_value()
    {
        if (working_stack.empty())
            return doc;
        return &working_stack.top();
    }

    bool set_dom_error(const char* message)
    {
        the_error.reset(new error::CorruptedDOMError(message));
        return false;
    }

    void pre_processing()
    {
        if (top_value()->IsArray())
            working_stack.emplace();
    }

    bool post_processing()
    {
        if (working_stack.empty())
            return true;

        value_type to_be_inserted;
        to_be_inserted.Swap(working_stack.top());
        working_stack.pop();

        if (!working_stack.empty() && working_stack.top().IsString()) {
            value_type key;
            key.Swap(working_stack.top());
            working_stack.pop();

            if (!top_value()->IsObject())
                return set_dom_error("DOM corrupted: Non-object types encountered where object is expected");
            top_value()->AddMember(key, to_be_inserted, doc->GetAllocator());
        }

        return true;
    }

public:
    explicit SAXEventHandler(document_type* value)
        : doc(value)
    {
    }

    bool Null()
    {
        pre_processing();
        top_value()->SetNull();
        return post_processing();
    }

    bool Bool(bool b)
    {
        pre_processing();
        top_value()->SetBool(b);
        return post_processing();
    }

    bool Int(int i)
    {
        pre_processing();
        top_value()->SetInt(i);
        return post_processing();
    }

    bool Uint(unsigned i)
    {
        pre_processing();
        top_value()->SetUint(i);
        return post_processing();
    }

    bool Int64(utility::int64_t i)
    {
        pre_processing();
        top_value()->SetInt64(i);
        return post_processing();
    }

    bool Uint64(utility::uint64_t i)
    {
        pre_processing();
        top_value()->SetUint64(i);
        return post_processing();
    }

    bool Double(double d)
    {
        pre_processing();
        top_value()->SetDouble(d);
        return post_processing();
    }

    bool String(const char_type* str, SizeType length, bool copy)
    {
        pre_processing();
        if (copy)
            top_value()->SetString(str, length, doc->GetAllocator());
        else
            top_value()->SetString(str, length);
        return post_processing();
    }

    bool Key(const char_type* str, SizeType length, bool copy)
    {
        if (!top_value()->IsObject())
            return set_dom_error("DOM corrupted: object types required");

        value_type key;
        if (copy)
            key.SetString(str, length, doc->GetAllocator());
        else
            key.SetString(str, length);

        working_stack.push(key);

        // push a placeholder as the corresponding value
        working_stack.emplace();
        return true;
    }

    bool StartArray()
    {
        top_value()->SetArray();
        return true;
    }

    bool EndArray(SizeType)
    {
        return post_processing();
    }

    bool StartObject()
    {
        top_value()->SetObject();
        return true;
    }

    bool EndObject(SizeType)
    {
        return post_processing();
    }

    bool HasError() const
    {
        return !this->the_error.empty();
    }

    bool ReapError(error::ErrorStack& errs)
    {
        if (this->the_error.empty())
            return false;

        errs.push(this->the_error.release());
        return true;
    }

    void PrepareForReuse()
    {
    }
};
}

#endif
