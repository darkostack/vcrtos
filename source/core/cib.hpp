#ifndef CORE_CIB_HPP
#define CORE_CIB_HPP

#include <vcrtos/assert.h>
#include <vcrtos/cib.h>

namespace vc {

class Cib : public cib_t
{
public:
    explicit Cib(unsigned int size) { init(size); }

    void init(unsigned int size)
    {
        vcassert(!(size & (size - 1)));
        read_count = 0;
        write_count = 0;
        mask = size - 1;
    }

    unsigned int avail(void) { return write_count - read_count; }

    unsigned int get_read_count(void) { return read_count; }

    unsigned int get_write_count(void) { return write_count; }

    unsigned int get_mask(void) { return mask; }

    int get_unsafe(void) { return static_cast<int>(read_count++ & mask); }

    int put_unsafe(void) { return static_cast<int>(write_count++ & mask); }

    int full(void) { return avail() > static_cast<int>(mask); }

    int get(void)
    {
        if (avail())
        {
            return static_cast<int>(read_count++ & mask);
        }

        return -1;
    }

    int peek(void)
    {
        if (avail())
        {
            return static_cast<int>(read_count & mask);
        }

        return -1;
    }

    int put(void)
    {
        int available = avail();

        if (available <= static_cast<int>(mask))
        {
            return static_cast<int>(write_count++ & mask);
        }

        return -1;
    }
};

} // namespace vc

#endif /* CORE_COMMON_CIB_HPP */
