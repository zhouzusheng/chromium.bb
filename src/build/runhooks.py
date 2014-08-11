#!/usr/bin/env python

# Copyright (C) 2014 Bloomberg L.P. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# This script implements the bare bones "gclient runhooks" behavior so that we
# don't need to have gclient installed everywhere.  Also, arguments passed to
# this script will be forwarded to gyp.

import os, sys, subprocess

scriptDir = os.path.dirname(os.path.realpath(__file__))


def execInShell(cmd):
  print "Executing '" + " ".join(cmd) + "'"
  sys.stdout.flush()
  return subprocess.call(cmd, shell=True)


def dummyVar(s):
  return s


def loadHooksForSolution(solution):
  name = solution['name']
  deps = solution['deps_file']
  scope = {
    'Var': dummyVar,
  }
  execfile(os.path.join(name, deps), scope)
  return scope['hooks']


def main(args):
  # Need to be in the root directory
  os.chdir(os.path.join(scriptDir, os.pardir, os.pardir))

  scope = {}
  execfile('.gclient', scope)
  for sln in scope['solutions']:
    hooks = loadHooksForSolution(sln)

    for hook in hooks:
      cmd = hook['action']
      if hook['name'] == 'gyp':
        cmd.extend(args)
      rc = execInShell(cmd)
      if 0 != rc:
        return rc

  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))

