import fnmatch
import os
import sys

matches = []
for root, dirnames, files in os.walk('src'):
  for f in fnmatch.filter(files, '*.cc') + fnmatch.filter(files, '*.h'):
    matches.append(os.path.join(root, f))

for match in matches:
  args = [
    os.path.join(
        os.path.dirname(__file__),
        '..', 'third_party', 'clang-format', 'clang-format.exe'),
    '-i',
    match
  ]
  os.system(' '.join(args))
