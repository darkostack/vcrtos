#ifndef CORE_NEW_HPP
#define CORE_NEW_HPP

#include <stddef.h>

inline void *operator new(size_t, void *p) throw()
{
    return p;
}

#endif // CORE_NEW_HPP_
