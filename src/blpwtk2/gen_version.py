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

def getChromiumVersion():
  scriptDir = os.path.dirname(os.path.realpath(__file__))
  chromiumDir = os.path.abspath(os.path.join(scriptDir, os.pardir, os.pardir))
  for path in os.listdir(chromiumDir):
    if path.count('.') == 3:
      return path
  raise Exception("Could not find chromium version directory!")

def getVersionParts(version):
  chromiumVersion = getChromiumVersion()

  components = version.split('_')
  if len(components) == 2:
    if components[0] != chromiumVersion:
      raise Exception("bb_version does not match chromium version: " \
          + "bb_version(" + components[0] + ") chromiumVersion(" \
          + chromiumVersion + ")")
    bbPatch = components[1]
  else:
    bbPatch = "bb0"
    version = chromiumVersion + "_" + bbPatch

  return version, chromiumVersion, bbPatch

def writeProductsFile(f, version):
  productAppend = ""
  if version != "":
    productAppend = "." + version

  version, chromiumVersion, bbPatch = getVersionParts(version)
  chromiumVersionNumbers = chromiumVersion.split('.')
  bbPatchNumber = bbPatch[2:]

  # Make sure they are integers, otherwise we cannot embed them inside
  # FileVersion in the DLL rc files.
  int(chromiumVersionNumbers[0])
  int(chromiumVersionNumbers[1])
  int(chromiumVersionNumbers[2])
  int(chromiumVersionNumbers[3])
  int(bbPatchNumber)

  f.write('// generated file -- DO NOT EDIT\n')
  f.write('#ifndef INCLUDED_GENERATED_BLPWTK2_PRODUCTS\n')
  f.write('#define INCLUDED_GENERATED_BLPWTK2_PRODUCTS\n')
  f.write('\n')
  f.write('// Naming scheme based on http://www.chromium.org/developers/version-numbers\n')
  f.write('#define CHROMIUM_VERSION_MAJOR {}\n'.format(chromiumVersionNumbers[0]))
  f.write('#define CHROMIUM_VERSION_MINOR {}\n'.format(chromiumVersionNumbers[1]))
  f.write('#define CHROMIUM_VERSION_BUILD {}\n'.format(chromiumVersionNumbers[2]))
  f.write('#define CHROMIUM_VERSION_PATCH {}\n'.format(chromiumVersionNumbers[3]))
  f.write('\n')
  f.write('#define BB_PATCH_NUMBER {}\n'.format(bbPatchNumber))
  f.write('\n')
  f.write('#define BLPWTK2_VERSION "{}"\n'.format(version))
  f.write('#define BLPWTK2_RC_VERSION "{}, {}, {}, {}"\n'.format(chromiumVersionNumbers[0],
                                                                 chromiumVersionNumbers[2],
                                                                 chromiumVersionNumbers[3],
                                                                 bbPatchNumber))
  f.write('#define BLPWTK2_DLL_NAME "blpwtk2{}.dll"\n'.format(productAppend))
  f.write('#define BLPWTK2_SUBPROCESS_EXE_NAME "blpwtk2_subprocess{}.exe"\n'.format(productAppend))
  f.write('#define BLPWTK2_DEVTOOLS_PAK_NAME "blpwtk2_devtools{}.pak"\n'.format(productAppend))
  f.write('#define BLPANGLE_DLL_NAME "blpangle{}.dll"\n'.format(productAppend))
  f.write('#define BLPV8_DLL_NAME "blpv8{}.dll"\n'.format(productAppend))
  f.write('#define BLPPDFIUM_DLL_NAME "blppdfium{}.dll"\n'.format(productAppend))
  f.write('#define FFMPEGSUMO_DLL_NAME "ffmpegsumo{}.dll"\n'.format(productAppend))
  f.write('\n')
  f.write('#endif  // INCLUDED_GENERATED_BLPWTK2_PRODUCTS\n')

def writeVersionFiles(fH, fCC, version):
  version, chromiumVersion, bbPatch = getVersionParts(version)

  exportedSymbol =  'version_' + version.replace('.', '_')

  fH.write('// generated file -- DO NOT EDIT\n')
  fH.write('#ifndef INCLUDED_GENERATED_BLPWTK2_VERSION\n')
  fH.write('#define INCLUDED_GENERATED_BLPWTK2_VERSION\n')
  fH.write('\n')
  fH.write('#define CHROMIUM_VERSION "{}"\n'.format(chromiumVersion))
  fH.write('#define BB_PATCH_VERSION "{}"\n'.format(bbPatch))
  fH.write('\n')
  fH.write('namespace blpwtk2 {\n')
  fH.write('\n')
  fH.write('struct Version {\n')
  fH.write('    static const char* d_chromiumVersion;\n')
  fH.write('    static const char* d_bbPatchVersion;\n')
  fH.write('    BLPWTK2_EXPORT static const char* {}();\n'.format(exportedSymbol))
  fH.write('};  // Version\n')
  fH.write('\n')
  fH.write('// Force linker to pull in this component\'s object file.\n')
  fH.write('namespace {\n')
  fH.write('    extern const char* (*blpwtk2_version_assertion)() = \n')
  fH.write('        &Version::{};\n'.format(exportedSymbol))
  fH.write('}\n')
  fH.write('\n')
  fH.write('}  // blpwtk2\n')
  fH.write('\n')
  fH.write('#endif  // INCLUDED_GENERATED_BLPWTK2_VERSION\n')

  fCC.write('// generated file -- DO NOT EDIT\n')
  fCC.write('#include <blpwtk2_config.h>\n')
  fCC.write('#include <blpwtk2_version.h>\n')
  fCC.write('namespace blpwtk2 {\n')
  fCC.write('const char* Version::d_chromiumVersion = "{}";\n'.format(chromiumVersion))
  fCC.write('const char* Version::d_bbPatchVersion = "{}";\n'.format(bbPatch))
  fCC.write('const char* Version::{}()\n'.format(exportedSymbol))
  fCC.write('{\n')
  fCC.write('    return "{}";\n'.format(version))
  fCC.write('}\n')
  fCC.write('}  // blpwtk2\n')


def doMain(args):
  productsFile = None
  versionHFile = None
  versionCCFile = None
  version = None

  for i in range(len(args)):
    if args[i] == '--output-products':
      productsFile = args[i+1]
    elif args[i] == '--output-version-h':
      versionHFile = args[i+1]
    elif args[i] == '--output-version-cc':
      versionCCFile = args[i+1]
    elif args[i] == '--version':
      version = args[i+1]

  assert(productsFile != None)
  assert(versionHFile != None)
  assert(versionCCFile != None)
  assert(version != None)

  with open(productsFile, 'w') as f:
    writeProductsFile(f, version)

  with open(versionHFile, 'w') as fH:
    with open(versionCCFile, 'w') as fCC:
      writeVersionFiles(fH, fCC, version)

if __name__ == '__main__':
  doMain(sys.argv)

