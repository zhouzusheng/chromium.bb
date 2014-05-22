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

scriptDir = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(0, os.path.join(scriptDir, os.pardir, 'ui', 'gl'))

import generate_bindings  # from src/ui/gl

def writeBindingCCFile(fCC, symbols):
  symbolMapping = {}
  for funcSet in generate_bindings.FUNCTION_SETS:
    funcSet = funcSet[0]
    for func in funcSet:
      for name in func['names']:
        symbolMapping[name] = func

  # The following symbol mappings are for symbols that are present in the .def
  # files, but have no corresponding entry in src/ui/gl/generate_bindings.py,
  # so we provide them here.
  symbolMapping.update({
    'glTexImage3DOES': {
      'return_type': 'void',
      'arguments': 'GLenum target, GLint level, GLenum internalformat, '
                   'GLsizei width, GLsizei height, GLsizei depth, '
                   'GLint border, GLenum format, GLenum type, '
                   'const GLvoid* pixels',
    },
    'glReadnPixelsEXT': {
      'return_type': 'void',
      'arguments': 'GLint x, GLint y, GLsizei width, GLsizei height, '
                   'GLenum format, GLenum type, GLsizei bufSize, void *data',
    },
    'glGetnUniformfvEXT': {
      'return_type': 'void',
      'arguments': 'GLuint program, GLint location, GLsizei bufSize, '
                   'GLfloat* params',
    },
    'glGetnUniformivEXT': {
      'return_type': 'void',
      'arguments': 'GLuint program, GLint location, GLsizei bufSize, '
                   'GLint* params',
    },
    'SetTraceFunctionPointers': {
      'return_type': 'void',
      'arguments': 'GetCategoryEnabledFlagFunc get_category_enabled_flag, '
                   'AddTraceEventFunc add_trace_event_func',
    },
  })

  fCC.write('// generated file -- DO NOT EDIT\n')
  fCC.write('#include <windows.h>\n')
  fCC.write('\n')
  fCC.write('extern "C" {\n')
  fCC.write('\n')
  fCC.write('typedef void             GLvoid;\n')
  fCC.write('typedef char             GLchar;\n')
  fCC.write('typedef unsigned int     GLenum;\n')
  fCC.write('typedef unsigned char    GLboolean;\n')
  fCC.write('typedef unsigned int     GLbitfield;\n')
  fCC.write('typedef signed char      GLbyte;\n')
  fCC.write('typedef short            GLshort;\n')
  fCC.write('typedef int              GLint;\n')
  fCC.write('typedef int              GLsizei;\n')
  fCC.write('typedef unsigned char    GLubyte;\n')
  fCC.write('typedef unsigned short   GLushort;\n')
  fCC.write('typedef unsigned int     GLuint;\n')
  fCC.write('typedef float            GLfloat;\n')
  fCC.write('typedef float            GLclampf;\n')
  fCC.write('typedef __int32          GLfixed;\n')
  fCC.write('typedef signed long int  GLintptr;\n')
  fCC.write('typedef signed long int  GLsizeiptr;\n')
  fCC.write('\n')
  fCC.write('typedef HDC              EGLNativeDisplayType;\n')
  fCC.write('typedef HBITMAP          EGLNativePixmapType;\n')
  fCC.write('typedef HWND             EGLNativeWindowType;\n')
  fCC.write('typedef __int32          EGLint;\n')
  fCC.write('typedef unsigned int     EGLBoolean;\n')
  fCC.write('typedef unsigned int     EGLenum;\n')
  fCC.write('typedef void            *EGLConfig;\n')
  fCC.write('typedef void            *EGLContext;\n')
  fCC.write('typedef void            *EGLDisplay;\n')
  fCC.write('typedef void            *EGLSurface;\n')
  fCC.write('typedef void            *EGLClientBuffer;\n')
  fCC.write('\n')
  fCC.write('typedef const unsigned char* (*GetCategoryEnabledFlagFunc)(const char* name);\n')
  fCC.write('typedef void (*AddTraceEventFunc)(char phase, const unsigned char* categoryGroupEnabled, const char* name,\n')
  fCC.write('                                  unsigned long long id, int numArgs, const char** argNames,\n')
  fCC.write('                                  const unsigned char* argTypes, const unsigned long long* argValues,\n')
  fCC.write('                                  unsigned char flags);\n')
  fCC.write('\n')
  fCC.write('typedef void (__stdcall *angleFunctionPointer)(void);\n')
  fCC.write('typedef angleFunctionPointer __eglMustCastToProperFunctionPointerType;\n')
  fCC.write('\n')
  for sym in symbols:
    func = symbolMapping[sym]
    fCC.write(func['return_type'] + ' __stdcall ' + sym + '(' + func['arguments'] + ');\n')
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
