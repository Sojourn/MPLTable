#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>

#include "Demangle.h"

#include <boost/mpl/vector.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/inherit.hpp>
#include <boost/mpl/inherit_linearly.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/placeholders.hpp>

using namespace boost;

template<class... Columns>
struct Project
{
    typedef mpl::vector<Columns...> Projection;
};

template<typename... Columns>
class Table
{
public:
    typedef uint64_t Index;

    template<typename T>
    struct Column
    {
        T field;
    };

    typedef mpl::vector<Columns...> ColumnVector;

    typedef typename mpl::inherit_linearly<ColumnVector,
        mpl::inherit< mpl::_1, Column<mpl::_2> >
        >::type Row;

    struct TableEntry
    {
        Index index;
        Row row;

        template<typename T>
        T& get()
        {
            Column<T> &column = row;
            return column.field;
        }

        template<typename T>
        const T &get() const
        {
            const Column<T> &column = row;
            return column.field;
        }

        bool operator <(const TableEntry &rhs) const { return index < rhs.index; }
        bool operator==(const TableEntry &rhs) const { return index == rhs.index; }
        bool operator!=(const TableEntry &rhs) const { return !(*this == rhs); }
    };

    template<typename T>
    struct Wrap {};

    struct HeaderRecorder
    {
        std::vector<std::string> &result;

        template<typename T>
        void operator()(Wrap<T>)
        {
            const char *name = typeid(T).name();
            result.push_back(demangle(name));
        }
    };

    template<typename Func>
    struct Projector
    {
        const TableEntry &entry;
        Func &func;

        template<typename T>
        void operator()(Wrap<T>)
        {
            func(entry.index, entry.get<T>());
        }
    };

    struct EntryBuilder
    {
        TableEntry &entry;

        template<class Arg>
        void build(Arg arg)
        {
            entry.get<Arg>() = arg;
        }

        template<class Arg, class... Args>
        void build(Arg arg, Args... args)
        {
            build(arg);
            if(isEmpty<Args...>()) {
                build(args...);
            }
        }

        template<typename... Args>
        static bool isEmpty()
        {
            typedef mpl::vector<Args...> ArgVector;
            return mpl::size<ArgVector>::value > 0;
        }
    };

public:
    Table() :
        next_(0)
    {
        HeaderRecorder func{headers_};
        mpl::for_each<ColumnVector, Wrap<mpl::placeholders::_1> >
                (std::ref(func));
    }

    const std::vector<std::string> &headers() const
    {
        return headers_;
    }

    size_t count() const
    {
        return table_.size();
    }

    Index insert()
    {
        TableEntry entry;
        entry.index = next_++;
        table_.push_back(entry);
        return entry.index;
    }

    template<class... Args>
    Index insert(Args... args)
    {
        TableEntry entry;
        EntryBuilder builder{entry};
        builder.build(args...);

        entry.index = next_++;
        table_.push_back(entry);
        return entry.index;
    }

    template<typename Project_, typename Func>
    void select(Func func) const
    {
        for(const auto &entry : table_) {
            Projector<Func> projector{entry, func};
            mpl::for_each<typename Project_::Projection, Wrap<mpl::placeholders::_1> >
                    (std::ref(projector));
        }
    }

    typename std::vector<TableEntry>::iterator begin()
    {
        return table_.begin();
    }

    typename std::vector<TableEntry>::const_iterator begin() const
    {
        return table_.begin();
    }

    typename std::vector<TableEntry>::iterator end()
    {
        return table_.end();
    }

    typename std::vector<TableEntry>::const_iterator end() const
    {
        return table_.end();
    }

    template<typename T>
    bool update(Index index, const T &value)
    {
        TableEntry key;
        key.index = index;
        auto iter = std::lower_bound(table_.begin(), table_.end(), key);
        if(iter == table_.end() || (iter->index != index)) {
            return false;
        } else {
            iter->get<T>() = value;
            return true;
        }
    }

    bool remove(Index index)
    {
        TableEntry key;
        key.index = index;
        auto iter = std::lower_bound(table_.begin(), table_.end(), key);
        if(iter == table_.end() || (iter->index != index)) {
            return false;
        } else {
            table_.erase(iter);
            return true;
        }
    }

private:
    std::vector<TableEntry> table_;
    std::vector<std::string> headers_;
    uint64_t next_;
};

#endif // TABLE_H
