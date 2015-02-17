#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import shutil
import stat
import sys

def main(args):
  executor = WinTool()
  exit_code = executor.Dispatch(args)
  if exit_code is not None:
    sys.exit(exit_code)


class WinTool(object):
  def Dispatch(self, args):
    """Dispatches a string command to a method."""
    if len(args) < 1:
      raise Exception("Not enough arguments")

    method = "Exec%s" % self._CommandifyName(args[0])
    return getattr(self, method)(*args[1:])

  def _CommandifyName(self, name_string):
    """Transforms a tool name like recursive-mirror to RecursiveMirror."""
    return name_string.title().replace('-', '')

  def ExecRecursiveMirror(self, source, dest):
    """Emulation of rm -rf out && cp -af in out."""
    if os.path.exists(dest):
      if os.path.isdir(dest):
        def _on_error(fn, path, excinfo):
          # The operation failed, possibly because the file is set to
          # read-only. If that's why, make it writable and try the op again.
          if not os.access(path, os.W_OK):
            os.chmod(path, stat.S_IWRITE)
          fn(path)
        shutil.rmtree(dest, onerror=_on_error)
      else:
        if not os.access(dest, os.W_OK):
          # Attempt to make the file writable before deleting it.
          os.chmod(dest, stat.S_IWRITE)
        os.unlink(dest)

    if os.path.isdir(source):
      shutil.copytree(source, dest)
    else:
      shutil.copy2(source, dest)

  def ExecStamp(self, path):
    """Simple stamp command."""
    open(path, 'w').close()


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
