# Copyright (C) 2015 Bloomberg L.P. All rights reserved.
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

{
  'targets': [
    {
      'target_name': 'blppdfutil',
      'type': 'shared_library',
      'dependencies': [
        'pdf.gyp:pdf',
        '../net/net.gyp:net_derived_sources',
        '../printing/printing.gyp:printing',
        '../blpwtk2/blpwtk2.gyp:blpwtk2_generate_sources'
      ],
      'sources': [
        'private/blppdfutil.rc',
        'private/blppdfutil_dllmain.cc',
        'public/blppdfutil_config.h',
        'public/blppdfutil_pdfutil.cc',
        'public/blppdfutil_pdfutil.h'
      ],
      'include_dirs': [
        'public',
        'private',
        '..'
      ],
      'defines': [
        'BUILDING_BLPPDFUTIL_SHARED',
        'BLPPDFUTIL_IMPLEMENTATION'
      ],
      'direct_dependent_settings': {
        'defines': [
          'USING_BLPPDFUTIL_SHARED'
        ],
        'include_dirs': [
          'public',
        ],
      }
    }
  ]
}

