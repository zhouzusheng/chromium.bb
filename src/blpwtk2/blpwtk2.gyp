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
  'targets': [
    {
      'target_name': 'blpwtk2_all',
      'type': 'none',
      'dependencies': [
        '../content/content_shell_and_tests.gyp:content_shell',
        '../chrome/chrome_blpwtk2.gyp:chrome_blpwtk2',
        '../v8/src/d8.gyp:d8',
      ],
      'conditions': [
        ['bb_ppapi_examples==1', {
          'dependencies': [
            '../ppapi/ppapi_internal.gyp:ppapi_example_mouse_cursor',
            '../ppapi/ppapi_internal.gyp:ppapi_example_mouse_lock',
            '../ppapi/ppapi_internal.gyp:ppapi_example_gamepad',
            '../ppapi/ppapi_internal.gyp:ppapi_example_c_stub',
            '../ppapi/ppapi_internal.gyp:ppapi_example_cc_stub',
            '../ppapi/ppapi_internal.gyp:ppapi_example_audio',
            '../ppapi/ppapi_internal.gyp:ppapi_example_audio_input',
            '../ppapi/ppapi_internal.gyp:ppapi_example_file_chooser',
            '../ppapi/ppapi_internal.gyp:ppapi_example_graphics_2d',
            '../ppapi/ppapi_internal.gyp:ppapi_example_ime',
            '../ppapi/ppapi_internal.gyp:ppapi_example_paint_manager',
            '../ppapi/ppapi_internal.gyp:ppapi_example_input',
            '../ppapi/ppapi_internal.gyp:ppapi_example_post_message',
            '../ppapi/ppapi_internal.gyp:ppapi_example_scroll',
            '../ppapi/ppapi_internal.gyp:ppapi_example_simple_font',
            '../ppapi/ppapi_internal.gyp:ppapi_example_url_loader',
            '../ppapi/ppapi_internal.gyp:ppapi_example_url_loader_file',
            #'../ppapi/ppapi_internal.gyp:ppapi_example_gles2',
            #'../ppapi/ppapi_internal.gyp:ppapi_example_video_decode',
            #'../ppapi/ppapi_internal.gyp:ppapi_example_vc',
            '../ppapi/ppapi_internal.gyp:ppapi_example_enumerate_devices',
            '../ppapi/ppapi_internal.gyp:ppapi_example_flash_topmost',
            '../ppapi/ppapi_internal.gyp:ppapi_example_printing',
          ],
        }],
      ],
    },
  ],
}
