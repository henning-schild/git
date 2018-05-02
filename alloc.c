/*
 * alloc.c  - specialized allocator for internal objects
 *
 * Copyright (C) 2006 Linus Torvalds
 *
 * The standard malloc/free wastes too much space for objects, partly because
 * it maintains all the allocation infrastructure, but even more because it ends
 * up with maximal alignment because it doesn't know what the object alignment
 * for the new allocation is.
 *
 * TODO: Combine this with mem-pool?
 */
#include "cache.h"
#include "object.h"
#include "blob.h"
#include "tree.h"
#include "commit.h"
#include "tag.h"
#include "alloc.h"

#define BLOCKING 1024

union any_object {
	struct object object;
	struct blob blob;
	struct tree tree;
	struct commit commit;
	struct tag tag;
};

struct alloc_state {
	int count; /* total number of nodes allocated */
	int nr;    /* number of nodes left in current allocation */
	void *p;   /* first free node in current allocation */

	/* bookkeeping of allocations */
	void **slabs;
	int slab_nr, slab_alloc;
};

void *allocate_alloc_state(void)
{
	return xcalloc(1, sizeof(struct alloc_state));
}

void clear_alloc_state(struct alloc_state *s)
{
	while (s->slab_nr > 0) {
		s->slab_nr--;
		free(s->slabs[s->slab_nr]);
	}
}

static inline void *alloc_node(struct alloc_state *s, size_t node_size)
{
	void *ret;

	if (!s->nr) {
		s->nr = BLOCKING;
		s->p = xmalloc(BLOCKING * node_size);
	}
	s->nr--;
	s->count++;
	ret = s->p;
	s->p = (char *)s->p + node_size;
	memset(ret, 0, node_size);

	ALLOC_GROW(s->slabs, s->slab_nr + 1, s->slab_alloc);
	s->slabs[s->slab_nr++] = ret;

	return ret;
}

struct alloc_state the_repository_blob_state;
void *alloc_blob_node(struct repository *r)
{
	struct blob *b = alloc_node(r->parsed_objects->blob_state, sizeof(struct blob));
	b->object.type = OBJ_BLOB;
	return b;
}

struct alloc_state the_repository_tree_state;
void *alloc_tree_node(struct repository *r)
{
	struct tree *t = alloc_node(r->parsed_objects->tree_state, sizeof(struct tree));
	t->object.type = OBJ_TREE;
	return t;
}

struct alloc_state the_repository_tag_state;
void *alloc_tag_node(struct repository *r)
{
	struct tag *t = alloc_node(r->parsed_objects->tag_state, sizeof(struct tag));
	t->object.type = OBJ_TAG;
	return t;
}

struct alloc_state the_repository_object_state;
void *alloc_object_node(struct repository *r)
{
	struct object *obj = alloc_node(r->parsed_objects->object_state, sizeof(union any_object));
	obj->type = OBJ_NONE;
	return obj;
}

unsigned int alloc_commit_index(struct repository *r)
{
	return r->parsed_objects->commit_count++;
}

struct alloc_state the_repository_commit_state;
void *alloc_commit_node(struct repository *r)
{
	struct commit *c = alloc_node(r->parsed_objects->commit_state, sizeof(struct commit));
	c->object.type = OBJ_COMMIT;
	c->index = alloc_commit_index(r);
	c->graph_pos = COMMIT_NOT_FROM_GRAPH;
	c->generation = GENERATION_NUMBER_INFINITY;
	return c;
}

static void report(const char *name, unsigned int count, size_t size)
{
	fprintf(stderr, "%10s: %8u (%"PRIuMAX" kB)\n",
			name, count, (uintmax_t) size);
}

#define REPORT(name, type)	\
    report(#name, r->parsed_objects->name##_state->count, \
		  r->parsed_objects->name##_state->count * sizeof(type) >> 10)

void alloc_report(struct repository *r)
{
	REPORT(blob, struct blob);
	REPORT(tree, struct tree);
	REPORT(commit, struct commit);
	REPORT(tag, struct tag);
	REPORT(object, union any_object);
}
