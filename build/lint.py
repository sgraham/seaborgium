import fnmatch
import os
import sys

matches = []
for root, dirnames, files in os.walk('src'):
  for f in fnmatch.filter(files, '*.cc') + fnmatch.filter(files, '*.h'):
    if f.endswith('zevv-peep.h'):
      # Generated.
      continue
    if f.endswith('entry_linux.cc'):
      # Not used yet.
      continue
    matches.append(os.path.join(root, f))
args = [
  sys.executable,
  os.path.join(
    os.path.dirname(__file__),
    '..',
    'third_party',
    'cpplint',
    'cpplint.py'),
  '--root=src',
  # We don't have any include tree.
  # We don't have the _EQ variants it wants.
  '--filter=-build/include,-readability/check',
  ] + matches
os.system(' '.join(args))
