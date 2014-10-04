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

#ifndef AUTOJSONCXX_QT_TYPES_HPP_29A4C106C1B1
#define AUTOJSONCXX_QT_TYPES_HPP_29A4C106C1B1

#include <autojsoncxx/autojsoncxx.hpp>

#include <QString>
#include <QVector>
#include <QList>
#include <QSharedPointer>
#include <QScopedPointer>
#include <QMap>
#include <QHash>

namespace autojsoncxx {

template <>
class SAXEventHandler<QString> : public BaseSAXEventHandler<SAXEventHandler<QString> > {
private:
    QString* m_value;

public:
    explicit SAXEventHandler(QString* v)
        : m_value(v)
    {
    }

    bool String(const char* str, autojsoncxx::utility::SizeType size, bool)
    {
        *m_value = QString::fromUtf8(str, size);
        return true;
    }

    static const char* type_name()
    {
        return "string";
    }
};

template <class Writer>
struct Serializer<Writer, QString> {
    void operator()(Writer& w, const QString& str)
    {
        QByteArray utf8 = str.toUtf8();
        w.String(utf8.constData(), utf8.size());
    }
};

template <class T>
class SAXEventHandler<QVector<T> >
    : public VectorBaseSAXEventHandler<T, SAXEventHandler<QVector<T> > > {

public:
    typedef QVector<T> vector_type;
    typedef VectorBaseSAXEventHandler<T, SAXEventHandler<QVector<T> > > base_type;

private:
    vector_type* m_value;

public:
    explicit SAXEventHandler(vector_type* v)
        : m_value(v)
    {
    }

    void Push(const T& c)
    {
        m_value->push_back(c);
    }

#if AUTOJSONCXX_HAS_RVALUE

    void Push(T&& c)
    {
        m_value->push_back(AUTOJSONCXX_MOVE(c));
    }

#endif

    bool CheckLength(SizeType) const
    {
        return true;
    }

    size_t ExpectedLength() const
    {
        return 0;
    }

    size_t GetCurrentSize() const
    {
        return m_value->size();
    }
};

template <class Writer, class T>
struct Serializer<Writer, QVector<T> >
    : public ContainerSerializer<Writer, QVector<T>,
                                 typename QVector<T>::value_type,
                                 typename QVector<T>::const_iterator> {
};

template <class T>
class SAXEventHandler<QList<T> >
    : public VectorBaseSAXEventHandler<T, SAXEventHandler<QList<T> > > {

public:
    typedef QList<T> vector_type;
    typedef VectorBaseSAXEventHandler<T, SAXEventHandler<QList<T> > > base_type;

private:
    vector_type* m_value;

public:
    explicit SAXEventHandler(vector_type* v)
        : m_value(v)
    {
    }

    void Push(const T& c)
    {
        m_value->push_back(c);
    }

#if AUTOJSONCXX_HAS_RVALUE

    void Push(T&& c)
    {
        m_value->push_back(AUTOJSONCXX_MOVE(c));
    }

#endif

    bool CheckLength(SizeType) const
    {
        return true;
    }

    size_t ExpectedLength() const
    {
        return 0;
    }

    size_t GetCurrentSize() const
    {
        return m_value->size();
    }
};

template <class Writer, class T>
struct Serializer<Writer, QList<T> >
    : public ContainerSerializer<Writer, QList<T>,
                                 typename QList<T>::value_type,
                                 typename QList<T>::const_iterator> {
};

template <class T>
class SAXEventHandler<QSharedPointer<T> >
    : public NullableBaseSAXEventHandler<T, SAXEventHandler<QSharedPointer<T> > > {

public:
    typedef QSharedPointer<T> smart_pointer_type;

private:
    smart_pointer_type* m_value;

public:
    explicit SAXEventHandler(smart_pointer_type* v)
        : m_value(v)
    {
    }

    bool IsNull() const
    {
        return m_value->isNull();
    }

    T* Initialize()
    {
        m_value->reset(new T());
        return m_value->data();
    }

    void SetNull()
    {
        m_value->reset(0);
    }
};

template <class Writer, class T>
struct Serializer<Writer, QSharedPointer<T> > {
    void operator()(Writer& w, const QSharedPointer<T>& ptr) const
    {
        if (!ptr)
            w.Null();
        else
            Serializer<Writer, T>()(w, *ptr);
    }
};

namespace utility {
    namespace traits {
        template <class T>
        struct is_simple_type<QSharedPointer<T> > : public is_simple_type<T> {
        };
    }
}

template <class T>
class SAXEventHandler<QScopedPointer<T> >
    : public NullableBaseSAXEventHandler<T, SAXEventHandler<QScopedPointer<T> > > {

public:
    typedef QScopedPointer<T> smart_pointer_type;

private:
    smart_pointer_type* m_value;

public:
    explicit SAXEventHandler(smart_pointer_type* v)
        : m_value(v)
    {
    }

    bool IsNull() const
    {
        return m_value->isNull();
    }

    T* Initialize()
    {
        m_value->reset(new T());
        return m_value->data();
    }

    void SetNull()
    {
        m_value->reset(0);
    }
};

template <class Writer, class T>
struct Serializer<Writer, QScopedPointer<T> > {
    void operator()(Writer& w, const QScopedPointer<T>& ptr) const
    {
        if (!ptr)
            w.Null();
        else
            Serializer<Writer, T>()(w, *ptr);
    }
};

namespace utility {
    namespace traits {
        template <class T>
        struct is_simple_type<QScopedPointer<T> > : public is_simple_type<T> {
        };
    }
}

template <class ElementType, class Derived>
class QMapBaseSAXEventHandler {
private:
    QString key;
    ElementType value;

    SAXEventHandler<ElementType> internal_handler;
    utility::scoped_ptr<error::ErrorBase> the_error;
    std::stack<signed char> state;
    // A stack of StartArray() and StartObject() event
    // must be recorded, so we know when the current
    // element has been fully parsed, and needs to be
    // pushed back into the container

    bool emplace_when_time_is_right()
    {
        if (state.size() == 1 && state.top() == internal::OBJECT) {
            if (!static_cast<Derived*>(this)->Emplace(key, AUTOJSONCXX_MOVE_IF_NOEXCEPT(value))) {
                the_error.reset(new error::DuplicateKeyError(key.toStdString()));
                return false;
            }

            value = ElementType();
            internal_handler.PrepareForReuse();
        }
        return true;
    }

    bool check_depth(const char* type)
    {
        if (state.empty()) {
            the_error.reset(new error::TypeMismatchError("object", type));
            return false;
        }
        return true;
    }

    bool checked_event_forwarding(bool success)
    {
        if (success)
            return emplace_when_time_is_right();

        set_member_error();
        return false;
    }

    void set_member_error()
    {
        this->the_error.reset(new error::ObjectMemberError(key.toStdString()));
    }

public:
    explicit QMapBaseSAXEventHandler()
        : key()
        , value()
        , internal_handler(&value)
    {
    }

    bool Null()
    {
        return check_depth("null") && checked_event_forwarding(internal_handler.Null());
    }

    bool Bool(bool b)
    {
        return check_depth("bool") && checked_event_forwarding(internal_handler.Bool(b));
    }

    bool Int(int i)
    {
        return check_depth("int") && checked_event_forwarding(internal_handler.Int(i));
    }

    bool Uint(unsigned i)
    {
        return check_depth("unsigned") && checked_event_forwarding(internal_handler.Uint(i));
    }

    bool Int64(utility::int64_t i)
    {
        return check_depth("int64_t") && checked_event_forwarding(internal_handler.Int64(i));
    }

    bool Uint64(utility::uint64_t i)
    {
        return check_depth("uint64_t") && checked_event_forwarding(internal_handler.Uint64(i));
    }

    bool Double(double d)
    {
        return check_depth("double") && checked_event_forwarding(internal_handler.Double(d));
    }

    bool String(const char* str, SizeType length, bool copy)
    {
        return check_depth("string") && checked_event_forwarding(internal_handler.String(str, length, copy));
    }

    bool Key(const char* str, SizeType length, bool copy)
    {
        if (state.size() > 1)
            return checked_event_forwarding(internal_handler.Key(str, length, copy));

        key = QString::fromUtf8(str, length);
        return true;
    }

    bool StartArray()
    {
        if (!check_depth("array"))
            return false;
        state.push(internal::ARRAY);
        return checked_event_forwarding(internal_handler.StartArray());
    }

    bool EndArray(SizeType length)
    {
        assert(state.top() == internal::ARRAY);
        state.pop();
        return check_depth("array") && checked_event_forwarding(internal_handler.EndArray(length));
    }

    bool StartObject()
    {
        state.push(internal::OBJECT);
        if (state.size() > 1)
            return checked_event_forwarding(internal_handler.StartObject());
        return true;
    }

    bool EndObject(SizeType length)
    {
        assert(state.top() == internal::OBJECT);
        state.pop();
        if (!state.empty())
            return checked_event_forwarding(internal_handler.EndObject(length));
        return true;
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
        internal_handler.ReapError(errs);
        return true;
    }

    void PrepareForReuse()
    {
    }
};

template <class Writer, class MapType, class ElementType, class ConstIteratorType>
struct QMapSerializer {
    void operator()(Writer& w, const MapType& map) const
    {

        w.StartObject();

        for (ConstIteratorType it = map.begin(), end = map.end(); it != end; ++it) {
            QByteArray utf8 = it.key().toUtf8();
            w.Key(utf8.constData(), utf8.size());
            Serializer<Writer, ElementType>()(w, it.value());
        }

        w.EndObject();
    }
};

template <class ValueType>
class SAXEventHandler<QMap<QString, ValueType> > : public QMapBaseSAXEventHandler<ValueType, SAXEventHandler<QMap<QString, ValueType> > > {
public:
    typedef QMap<QString, ValueType> map_type;

private:
    map_type* m_value;

public:
    explicit SAXEventHandler(map_type* v)
        : m_value(v)
    {
    }

    bool Emplace(const QString& key, const ValueType& value)
    {
        m_value->insert(key, value);
        return true;
    }

#if AUTOJSONCXX_HAS_RVALUE
    bool Emplace(const QString& key, ValueType&& value)
    {
        m_value->insert(key, std::move(value));
        return true;
    }
#endif
};

template <class Writer, class ValueType>
struct Serializer<Writer, QMap<QString, ValueType> > : public QMapSerializer<Writer, QMap<QString, ValueType>, ValueType,
                                                                             typename QMap<QString, ValueType>::const_iterator> {
};

template <class ValueType>
class SAXEventHandler<QHash<QString, ValueType> > : public QMapBaseSAXEventHandler<ValueType, SAXEventHandler<QHash<QString, ValueType> > > {
public:
    typedef QHash<QString, ValueType> map_type;

private:
    map_type* m_value;

public:
    explicit SAXEventHandler(map_type* v)
        : m_value(v)
    {
    }

    bool Emplace(const QString& key, const ValueType& value)
    {
        m_value->insert(key, value);
        return true;
    }

#if AUTOJSONCXX_HAS_RVALUE
    bool Emplace(const QString& key, ValueType&& value)
    {
        m_value->insert(key, std::move(value));
        return true;
    }
#endif
};

template <class Writer, class ValueType>
struct Serializer<Writer, QHash<QString, ValueType> > : public QMapSerializer<Writer, QHash<QString, ValueType>, ValueType,
                                                                              typename QHash<QString, ValueType>::const_iterator> {
};
}

#endif
