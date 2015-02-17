// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FOCUS_H_
#define FOCUS_H_

class Widget;

Widget* GetFocusedContents();
void SetFocusedContents(Widget* contents);

#endif  // FOCUS_H_
