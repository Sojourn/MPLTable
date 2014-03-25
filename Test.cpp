#include <iostream>
#include <string>
#include <vector>

#include "Table.h"

struct Printer
{
    template<class Index, class T>
    void operator()(Index, T t) const
    {
        std::cout << t << std::endl;
    }
};

int main()
{
    typedef Table<int, char, const char*, std::string> TestTable;
    TestTable table;

    table.insert(0, '\0', std::string("a"), "b");
    table.insert(0, '\0', std::string("c"), "d");

    table.select< Project<int, char, const char*, std::string> >(Printer());

    {
        table.select< Project<std::string> >([&](uint64_t, const std::string &value) {
            if(!value.empty()) {
                std::cout << value << std::endl;
            }
        });
    }

    return 0;
}

