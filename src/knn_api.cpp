#include <vector>
#include <cmath>
#include <cstddef>
#include <cstdint>

extern "C" {
#include "knn_api.h"
}

#include <nanoflann.hpp>

// Structure to hold 2D points with IDs
struct PointCloud {
    struct Point { float x, y; size_t id; }; // Include an ID with each point
    std::vector<Point> pts;

    inline size_t kdtree_get_point_count() const { return pts.size(); }
    inline float kdtree_get_pt(const size_t idx, const size_t dim) const {
        return (dim == 0) ? pts[idx].x : pts[idx].y;
    }
    template <class BBOX> bool kdtree_get_bbox(BBOX&) const { return false; }
};

// Define the KDTree type with L2 (Euclidean) distance
using KDTree = nanoflann::KDTreeSingleIndexAdaptor<
    nanoflann::L2_Simple_Adaptor<float, PointCloud>,
    PointCloud,
    2 // 2D points
>;

// Internal structure to manage KDTree and point cloud
struct KNN_Tree {
    PointCloud cloud;
    KDTree* tree;
    bool tree_built;

    KNN_Tree(size_t capacity) : cloud(), tree(nullptr), tree_built(false) {
        cloud.pts.reserve(capacity);
    }
};

extern "C" {

// Initialize the KDTree with a specified capacity
KNN_Tree* knn_create_tree(size_t capacity) {
    return new KNN_Tree(capacity);
}

// Insert a point into the KDTree with an associated ID
void knn_insert_point(KNN_Tree* tree, float x, float y, size_t id) {
    tree->cloud.pts.push_back({x, y, id}); // Store point with ID
    tree->tree_built = false; // Mark tree as unbuilt after adding points
}

// Build the KDTree after inserting points
void knn_build_tree(KNN_Tree* tree) {
    if (tree->tree) delete tree->tree;
    tree->tree = new KDTree(2, tree->cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    tree->tree->buildIndex();
    tree->tree_built = true;
}

// Find the nearest neighbor to a given (x, y) point
int knn_find_nearest(KNN_Tree* tree, float x, float y, size_t* nearest_id, float* distance) {
    if (!tree->tree_built) return -1; // Return error if tree is not built

    float query_pt[2] = {x, y};
    nanoflann::KNNResultSet<float> resultSet(1);
    size_t nearest_index;
    resultSet.init(&nearest_index, distance);

    tree->tree->findNeighbors(resultSet, query_pt, nanoflann::SearchParams(10));

    // Retrieve the ID of the nearest point
    *nearest_id = tree->cloud.pts[nearest_index].id;
    *distance = std::sqrt(*distance); // Return actual Euclidean distance
    return 0; // Success
}

// Free the KDTree
void knn_free_tree(KNN_Tree* tree) {
    delete tree->tree;
    delete tree;
}

} // extern "C"
