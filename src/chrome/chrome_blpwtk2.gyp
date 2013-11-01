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

{
  'variables': {
    # This turns on e.g. the filename-based detection of which
    # platforms to include source files on (e.g. files ending in
    # _mac.h or _mac.cc are only compiled on MacOSX).
    'chromium_code': 1,
   },
  'targets': [
    {
      'target_name': 'chrome_blpwtk2',
      'type': 'static_library',
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:base_i18n',
        '../base/base.gyp:base_prefs',
        '../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../build/temp_gyp/googleurl.gyp:googleurl',
        '../components/components.gyp:browser_context_keyed_service',
        '../components/components.gyp:user_prefs',
        '../content/content.gyp:content',
        '../ipc/ipc.gyp:ipc',
        '../net/net.gyp:net',
        '../sync/sync.gyp:sync',
        '../skia/skia.gyp:skia',
        '../third_party/hunspell/hunspell.gyp:hunspell',
        '../third_party/icu/icu.gyp:icui18n',
        '../third_party/icu/icu.gyp:icuuc',
        '../third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit',
      ],
      'include_dirs': [ '..' ],
      'sources': [
        'browser/spellchecker/spellcheck_custom_dictionary.cc',
        'browser/spellchecker/spellcheck_custom_dictionary.h',
        'browser/spellchecker/spellcheck_dictionary.h',
        'browser/spellchecker/spellcheck_factory.cc',
        'browser/spellchecker/spellcheck_factory.h',
        'browser/spellchecker/spellcheck_host_metrics.cc',
        'browser/spellchecker/spellcheck_host_metrics.h',
        'browser/spellchecker/spellcheck_hunspell_dictionary.cc',
        'browser/spellchecker/spellcheck_hunspell_dictionary.h',
        'browser/spellchecker/spellcheck_message_filter.cc',
        'browser/spellchecker/spellcheck_message_filter.h',
        'browser/spellchecker/spellcheck_service.cc',
        'browser/spellchecker/spellcheck_service.h',
        'browser/spellchecker/spelling_service_client.cc',
        'browser/spellchecker/spelling_service_client.h',
        'browser/spellchecker/spelling_service_feedback.cc',
        'browser/spellchecker/spelling_service_feedback.h',
        'browser/spellchecker/word_trimmer.cc',
        'browser/spellchecker/word_trimmer.h',
        'common/spellcheck_common.cc',
        'common/spellcheck_common.h',
        'common/spellcheck_messages.h',
        'common/spellcheck_result.h',
        'renderer/spellchecker/custom_dictionary_engine.cc',
        'renderer/spellchecker/custom_dictionary_engine.h',
        'renderer/spellchecker/hunspell_engine.cc',
        'renderer/spellchecker/hunspell_engine.h',
        'renderer/spellchecker/spellcheck_provider.cc',
        'renderer/spellchecker/spellcheck_provider.h',
        'renderer/spellchecker/spellcheck.cc',
        'renderer/spellchecker/spellcheck.h',
        'renderer/spellchecker/spellcheck_language.cc',
        'renderer/spellchecker/spellcheck_language.h',
        'renderer/spellchecker/spellcheck_worditerator.cc',
        'renderer/spellchecker/spellcheck_worditerator.h',
        'renderer/spellchecker/spelling_engine.h',

        # Needed by spellchecker
        'common/chrome_constants.cc',
        'common/chrome_constants.h',
        'common/chrome_switches.cc',
        'common/chrome_switches.h',
        'common/common_message_generator.cc',
        'common/common_message_generator.h',
        'common/pref_names.cc',
        'common/pref_names.h',
      ],
    },
  ],
}
