#ifndef VCRTOS_LIST_H
#define VCRTOS_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct list_node
{
    struct list_node *next;
} list_node_t;

#ifdef __cplusplus
}
#endif

#endif /* VCRTOS_LIST_H */
