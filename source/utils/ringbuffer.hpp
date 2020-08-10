#ifndef UTILS_RINGBUFFER_HPP
#define UTILS_RINGBUFFER_HPP

#include <vcrtos/config.h>

namespace vc {
namespace utils {

class RingBuffer
{
public:
    explicit RingBuffer(char *buf, unsigned size)
    {
        init(buf, size);
    }

    void init(char *buf, unsigned size)
    {
        _buf = buf;
        _size = size;
        _start = 0;
        _avail = 0;
    }

    int add_one(char byte);

    unsigned add(const char *buf, unsigned size);

    int get_one(void);

    unsigned get(char *buf, unsigned size);

    unsigned remove(unsigned size);

    int is_empyt(void) { return _avail == 0; }

    int is_full(void) { return _avail == _size; }

    unsigned int free(void) { return _size - _avail; }

    unsigned int avail(void) { return _avail; }

    int peek_one(void);

    unsigned peek(char *buf, unsigned size);

private:
    void add_tail(char byte);

    char get_head(void);

    char *_buf;
    unsigned int _size;
    unsigned int _start;
    unsigned int _avail;
};

} // namespace utils
} // namespace vc

#endif /* UTILS_RINGBUFFER_HPP */
