// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_FOCUS_H_
#define UI_FOCUS_H_

class Dockable;

Dockable* GetFocusedContents();
void SetFocusedContents(Dockable* contents);

#endif  // UI_FOCUS_H_
