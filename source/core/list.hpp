#ifndef CORE_LIST_HPP
#define CORE_LIST_HPP

#include <vcrtos/list.h>

namespace vc {

class List : public list_node_t
{
public:
    List(void) { next = NULL; }

    void add(List *node)
    {
        node->next = this->next;
        this->next = node;
    }

    List *remove_head(void)
    {
        List *head = static_cast<List *>(this->next);

        if (head)
        {
            this->next = head->next;
        }

        return head;
    }

    static List *remove(List *list, List *node)
    {
        while (list->next)
        {
            if (list->next == node)
            {
                list->next = node->next;
                return node;
            }
            list = static_cast<List *>(list->next);
        }

        return static_cast<List *>(list->next);
    }
};

} // namespace vc

#endif /* CORE_LIST_HPP */
