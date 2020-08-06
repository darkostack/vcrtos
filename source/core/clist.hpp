#ifndef CORE_CLIST_HPP
#define CORE_CLIST_HPP

#include <stddef.h>

#include <vcrtos/clist.h>

#include "core/list.hpp"

namespace vc {

class Clist : public List
{
public:
    Clist(void) { next = NULL; }

    void right_push(Clist *node)
    {
        if (this->next)
        {
            node->next = this->next->next;
            this->next->next = node;
        }
        else
        {
            node->next = node;
        }

        this->next = node;
    }

    void left_push(Clist *node)
    {
        if (this->next)
        {
            node->next = this->next->next;
            this->next->next = node;
        }
        else
        {
            node->next = node;
            this->next = node;
        }
    }

    Clist *left_pop(void)
    {
        if (this->next)
        {
            Clist *first = static_cast<Clist *>(this->next->next);

            if (this->next == first)
            {
                this->next = NULL;
            }
            else
            {
                this->next->next = first->next;
            }

            return first;
        }
        else
        {
            return NULL;
        }
    }

    void left_pop_right_push(void)
    {
        if (this->next)
        {
            this->next = this->next->next;
        }
    }

    Clist *left_peek(void)
    {
        if (this->next)
        {
            return static_cast<Clist *>(this->next->next);
        }

        return NULL;
    }

    Clist *right_peek(void) { return static_cast<Clist *>(this->next); }

    Clist *right_pop(void)
    {
        if (this->next)
        {
            List *last = static_cast<List *>(this->next);

            while (this->next->next != last)
            {
                this->left_pop_right_push();
            }

            return this->left_pop();
        }
        else
        {
            return NULL;
        }
    }

    Clist *find_before(const Clist *node)
    {
        Clist *pos = static_cast<Clist *>(this->next);

        if (!pos)
        {
            return NULL;
        }

        do
        {
            pos = static_cast<Clist *>(pos->next);

            if (pos->next == node)
            {
                return pos;
            }

        } while (pos != this->next);

        return NULL;
    }

    Clist *find(const Clist *node)
    {
        Clist *tmp = this->find_before(node);

        if (tmp)
        {
            return static_cast<Clist *>(tmp->next);
        }
        else
        {
            return NULL;
        }
    }

    Clist *remove(Clist *node)
    {
        if (this->next)
        {
            if (this->next->next == node)
            {
                return this->left_pop();
            }
            else
            {
                Clist *tmp = this->find_before(node);

                if (tmp)
                {
                    tmp->next = tmp->next->next;

                    if (node == this->next)
                    {
                        this->next = tmp;
                    }
                    return node;
                }
            }
        }

        return NULL;
    }

    size_t count(void)
    {
        Clist *node = static_cast<Clist *>(this->next);
        size_t cnt  = 0;

        if (node)
        {
            do
            {
                node = static_cast<Clist *>(node->next);
                ++cnt;
            } while (node != this->next);
        }

        return cnt;
    }
};

} // namespace vc

#endif /* CORE_CLIST_HPP */
