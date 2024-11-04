#ifndef KNN_API_H
#define KNN_API_H

#include <stddef.h>
#include <stdint.h>

// Opaque pointer to the KDTree object
typedef struct KNN_Tree KNN_Tree;

// Initialize the KDTree with a specified capacity
KNN_Tree* knn_create_tree(size_t capacity);

// Insert a point into the KDTree with an associated ID
void knn_insert_point(KNN_Tree* tree, float x, float y, size_t id);

// Build the KDTree after inserting points
void knn_build_tree(KNN_Tree* tree);

// Find the nearest neighbor to a given (x, y) point, returning the ID of the nearest point
int knn_find_nearest(KNN_Tree* tree, float x, float y, size_t* nearest_id, float* distance);

// Free the KDTree
void knn_free_tree(KNN_Tree* tree);

#endif // KNN_API_H
