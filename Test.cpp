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
    typedef Table<int, char, const char*> TestTable;
    TestTable table;

    auto updateRow = [&](TestTable::Index index, int v1, char v2, const char* v3) {
        table.update(index, v1);
        table.update(index, v2);
        table.update(index, v3);
    };

    updateRow(table.insert(), 1, '2', "3");
    updateRow(table.insert(), 4, '5', "6");
    updateRow(table.insert(), 7, '8', "9");

    std::vector<std::string> headers = table.headers();
    for(auto name : headers) {
        std::cout << name << "|";
    }
    std::cout << std::endl;

    table.select<Printer, int, char>(Printer());

    return 0;
}

