#ifndef ALLOC_H
#define ALLOC_H

void *alloc_blob_node(struct repository *r);
void *alloc_tree_node(struct repository *r);
void *alloc_commit_node(struct repository *r);
void *alloc_tag_node(struct repository *r);
void *alloc_object_node(struct repository *r);
void alloc_report(struct repository *r);
unsigned int alloc_commit_index(struct repository *r);

void *allocate_alloc_state(void);
void clear_alloc_state(struct alloc_state *s);

extern struct alloc_state the_repository_blob_state;
extern struct alloc_state the_repository_tree_state;
extern struct alloc_state the_repository_commit_state;
extern struct alloc_state the_repository_tag_state;
extern struct alloc_state the_repository_object_state;

#endif
