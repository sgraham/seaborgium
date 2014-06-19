// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_GEOMETRIC_TYPES_H_
#define CORE_GEOMETRIC_TYPES_H_

class Point;

class Rect {
 public:
  Rect() : x(0), y(0), w(-1), h(-1) {}
  Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
  Rect Expand(const Rect& by) const {
    return Rect(x - by.x, y - by.y, w + by.x + by.w, h + by.y + by.w);
  }
  Rect Contract(const Rect& by) const {
    return Rect(x + by.x, y + by.y, w - by.x - by.w, h - by.y - by.h);
  }
  bool Contains(const Point& point) const;
  Rect RelativeTo(const Rect& other) const {
    return Rect(x - other.x, y - other.y, w, h);
  }
  float x, y, w, h;
};

class Point {
 public:
  Point() : x(0), y(0) {}
  Point(float x, float y) : x(x), y(y) {}
  Point Subtract(const Point& other) const {
    return Point(x - other.x, y - other.y);
  }
  Point Scale(float scale) const {
    return Point(x * scale, y * scale);
  }
  Point RelativeTo(const Rect& rect) const;
  float x, y;
};

inline bool Rect::Contains(const Point& point) const {
  return point.x >= x &&
         point.x < x + w &&
         point.y >= y &&
         point.y < y + h;
}

inline Point Point::RelativeTo(const Rect& rect) const {
  return Point(x - rect.x, y - rect.y);
}

#endif  // CORE_GEOMETRIC_TYPES_H_
