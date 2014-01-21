#if ! defined CLNODE_H
#define CLNODE_H

struct clnode { struct clnode *prev, *next; };
typedef struct clnode clnode_t;

static clnode_t *clnode_init(clnode_t *n)
{ return n->prev = n->next = n; }

static clnode_t *clnode_next(clnode_t *n)
{ return n->next; }

static clnode_t *clnode_prev(clnode_t *n)
{ return n->prev; }

static int clnode_singleton(clnode_t *n)
{ return n->prev == n->next; }

static void clnode_splice(clnode_t *a, clnode_t *b)
{
	clnode_t *atail = a->prev, *btail = b->prev;
	a->prev     = btail;
	btail->next = a;
	b->prev     = atail;
	atail->next = b;
}

static clnode_t *clnode_remove(clnode_t *n)
{ clnode_splice(n, clnode_next(n)); }

static clnode_t *clnode_append(clnode_t *head, clnode_t *n)
{ clnode_splice(head, n); }

#endif
