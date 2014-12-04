# Copyright (C) 2013 Bloomberg L.P. All rights reserved.
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

import sys, os

def writeBindingCCFile(fCC, symbols):
  fCC.write('// generated file -- DO NOT EDIT\n')
  fCC.write('#include <windows.h>\n')
  fCC.write('#include <EGL/egl.h>\n')
  fCC.write('#include <GLES2/gl2.h>\n')
  fCC.write('#include <GLES2/gl2ext.h>\n')
  fCC.write('#include <GLES3/gl3.h>\n')
  fCC.write('#include <event_tracer.h>\n')
  fCC.write('\n')
  fCC.write('extern "C" {\n')
  fCC.write('\n')
  fCC.write('typedef __eglMustCastToProperFunctionPointerType angleFunctionPointer;\n')
  fCC.write('\n')
  fCC.write('angleFunctionPointer __stdcall blpangle_getProcAddress(const char* name)\n')
  fCC.write('{\n')
  fCC.write('    struct Func {\n')
  fCC.write('        const char* name;\n')
  fCC.write('        angleFunctionPointer address;\n')
  fCC.write('    };\n')
  fCC.write('\n')
  fCC.write('    static const Func funcs[] = {\n')
  for symbol in symbols:
    fCC.write('        {"' + symbol + '", (angleFunctionPointer)' + symbol + '},\n')
  fCC.write('    };\n')
  fCC.write('\n')
  fCC.write('    for (int i = 0; i < sizeof(funcs) / sizeof(Func); ++i) {\n')
  fCC.write('        if (0 == strcmp(name, funcs[i].name)) {\n')
  fCC.write('          return funcs[i].address;\n')
  fCC.write('        }\n')
  fCC.write('    }\n')
  fCC.write('\n')
  fCC.write('    return eglGetProcAddress(name);\n')
  fCC.write('}\n')
  fCC.write('\n')
  fCC.write('}  /* extern C */ \n')


def doMain(args):
  inputDefs = []
  bindingsCCFile = None

  for i in range(len(args)):
    if args[i].endswith('.def'):
      inputDefs.append(args[i])
    elif args[i] == '--output-bindings-cc':
      bindingsCCFile = args[i+1]

  assert(inputDefs)
  assert(bindingsCCFile)

  symbols = []

  for inputDef in inputDefs:
    with open(inputDef, 'r') as f:
      for ln in f.readlines():
        ln = ln.strip()
        if -1 == ln.find('@'): continue
        if -1 != ln.find('NONAME'): continue
        symbol = ln.split('@')[0].strip()
        symbols.append(symbol)

  with open(bindingsCCFile, 'w') as fCC:
    writeBindingCCFile(fCC, symbols)


if __name__ == '__main__':
  doMain(sys.argv[1:])
