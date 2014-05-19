// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RECT_PACK_H_
#define RECT_PACK_H_

// "A Thousand Ways to Pack the Bin - A Practical Approach to Two-Dimensional
// Rectangle Bin Packing", Jukka Jylänki, February 27, 2010.
// http://clb.demon.fi/files/RectangleBinPack.pdf
// This is "SKYLINE-BL" without a waste map.

#include <vector>

namespace core {

class RectPack {
 public:
  RectPack(int width, int height);

  // Finds a suitable position for the given rectangle.
  bool Insert(int width, int height, int* x, int* y);

 private:
  int Fit(int skyline_node_index, int width, int height);

  // Merges all skyline nodes that are at the same level.
  void Merge();

  struct Node {
    Node(int x, int y, int width) : x(x), y(y), width(width) {}

    int x;      // The starting x-coordinate (leftmost).
    int y;      // The y-coordinate of the skyline level line.
    int width;  // The line width. The ending coordinate (inclusive) will be
                // x+width-1.
  };

  int width_;                  // width (in pixels) of the underlying texture
  int height_;                 // height (in pixels) of the underlying texture
  std::vector<Node> skyline_;  // node of the skyline algorithm
};

inline RectPack::RectPack(int width, int height)
    : width_(width), height_(height) {
  skyline_.push_back(Node(0, 0, width));
}

inline bool RectPack::Insert(int width, int height, int* out_x, int* out_y) {
  *out_x = 0;
  *out_y = 0;

  int best_height = INT_MAX;
  int best_index = -1;
  int best_width = INT_MAX;
  for (int i = 0; i < static_cast<int>(skyline_.size()); ++i) {
    int y = Fit(i, width, height);
    if (y >= 0) {
      Node* node = &skyline_[i];
      if (((y + height) < best_height) ||
          (((y + height) == best_height) && (node->width < best_width))) {
        best_height = y + height;
        best_index = i;
        best_width = node->width;
        *out_x = node->x;
        *out_y = y;
      }
    }
  }

  if (best_index == -1) {
    return false;
  }

  Node newNode(*out_x, *out_x + height, width);
  skyline_.insert(skyline_.begin() + best_index, newNode);

  for (int i = best_index + 1; i < static_cast<int>(skyline_.size()); ++i) {
    Node* node = &skyline_[i];
    Node* prev = &skyline_[i - 1];
    if (node->x < (prev->x + prev->width)) {
      int shrink = prev->x + prev->width - node->x;
      node->x += shrink;
      node->width -= shrink;
      if (node->width <= 0) {
        skyline_.erase(skyline_.begin() + i);
        --i;
      } else {
        break;
      }
    } else {
      break;
    }
  }

  Merge();
  return true;
}

inline int RectPack::Fit(int skyline_node_index, int width, int height) {
  const Node& baseNode = skyline_[skyline_node_index];

  int width_remaining = width;
  int i = skyline_node_index;

  if (baseNode.x + width > width_)
    return -1;

  int y = baseNode.y;
  while (width_remaining > 0) {
    const Node& node = skyline_[i];
    if (node.y > y) {
      y = node.y;
    }

    if (y + height > height_) {
      return -1;
    }

    width_remaining -= node.width;
    ++i;
  }

  return y;
}

inline void RectPack::Merge() {
  for (int i = 0; i < static_cast<int>(skyline_.size() - 1); ++i) {
    Node* node = &skyline_[i];
    Node* next = &skyline_[i + 1];
    if (node->y == next->y) {
      node->width += next->width;
      skyline_.erase(skyline_.begin() + i + 1);
      --i;
    }
  }
}

}  // namespace core

#endif  // RECT_PACK_H_
