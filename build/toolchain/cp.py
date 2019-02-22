#!/usr/bin/env python

import os
import shutil
import sys

src, dst = sys.argv[1:]

if os.path.exists(dst):
  if os.path.isdir(dst):
    shutil.rmtree(dst)
  else:
    os.remove(dst)

if os.path.isdir(src):
  shutil.copytree(src, dst)
else:
  shutil.copy2(src, dst)
