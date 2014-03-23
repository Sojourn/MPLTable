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
#include <boost/mpl/placeholders.hpp>

using namespace boost;

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
        Func func;

        template<typename T>
        void operator()(Wrap<T>)
        {
            func(entry.index, column<T>(entry.row));
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

    template<typename T>
    bool select(Index index, T *value) const
    {
        TableEntry key;
        key.index = index;
        auto iter = std::lower_bound(table_.begin(), table_.end(), key);
        if(iter == table_.end() || (iter->index != index)) {
            return false;
        } else {
            *value = column<T>(iter->row);
            return true;
        }
    }

    template<typename Func, typename... Projection>
    void select(Func func) const
    {
        typedef mpl::vector<Projection...> ProjectionVector;

        for(const auto &row : table_) {
            Projector<Func> projector{row, func};
            mpl::for_each<ProjectionVector, Wrap<mpl::placeholders::_1> >
                    (std::ref(projector));
        }
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
            column<T>(iter->row) = value;
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

    template<typename T>
    static T& column(Column<T>& t)
    {
        return t.field;
    }

    template<typename T>
    static const T &column(const Column<T>& t)
    {
        return t.field;
    }

private:
    std::vector<TableEntry> table_;
    std::vector<std::string> headers_;
    uint64_t next_;
};

#endif // TABLE_H
