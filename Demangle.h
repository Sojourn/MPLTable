#ifndef DEMANGLE_H
#define DEMANGLE_H

#ifdef __GNUG__
#include <cstdlib>
#include <string>
#include <memory>
#include <cxxabi.h>

inline std::string demangle(const char* name)
{
    int status = 0;
    std::unique_ptr<char, void(*)(void*)> result {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };

    return (status==0) ? result.get() : name ;
}

#else

inline std::string demangle(const char* name)
{
    return name;
}

#endif

#endif // DEMANGLE_H
