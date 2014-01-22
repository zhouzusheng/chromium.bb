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
  'target_defaults': {
    'conditions': [
      # TODO(jschuh): Remove this after crbug.com/173851 gets fixed.
      ['OS=="win" and target_arch=="x64"', {
        'msvs_settings': {
          'VCCLCompilerTool': {
            'AdditionalOptions': ['/bigobj'],
          },
        },
      }],
    ],
  },
  'variables': {
    'angle_bindings_cc': '<(SHARED_INTERMEDIATE_DIR)/blpwtk2/private/blpangle_bindings.cc',
    'products_h': '<(SHARED_INTERMEDIATE_DIR)/blpwtk2/public/blpwtk2_products.h',
    'version_h': '<(SHARED_INTERMEDIATE_DIR)/blpwtk2/public/blpwtk2_version.h',
    'version_cc': '<(SHARED_INTERMEDIATE_DIR)/blpwtk2/public/blpwtk2_version.cc',
  },
  'targets': [
    {
      'target_name': 'blpwtk2_generate_sources',
      'type': 'none',
      'actions': [
        {
          'action_name': 'Generate version files',
          'inputs': [
            'gen_version.py',
          ],
          'outputs': [
            '<(products_h)',
            '<(version_h)',
            '<(version_cc)',
          ],
          'action': [
            'python',
            '<@(_inputs)',
            '--output-products', '<(products_h)',
            '--output-version-h', '<(version_h)',
            '--output-version-cc', '<(version_cc)',
            '--version', '<(bb_version)',
          ],
          'msvs_cygwin_shell': 1,
        },
        {
          'action_name': 'Generate angle bindings',
          'inputs': [
            'gen_angle_bindings.py',
            '<(angle_path)/src/libGLESv2/libGLESv2.def',
            '<(angle_path)/src/libEGL/libEGL.def',
          ],
          'outputs': [
            '<(angle_bindings_cc)',
          ],
          'action': [
            'python',
            '<@(_inputs)',
            '--output-bindings-cc', '<(angle_bindings_cc)',
          ],
          'msvs_cygwin_shell': 1,
        },
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(SHARED_INTERMEDIATE_DIR)/blpwtk2/public',
          '<(SHARED_INTERMEDIATE_DIR)/blpwtk2/private',
        ],
      },
    },
    {
      'target_name': 'blpwtk2',
      'type': 'shared_library',
      'dependencies': [
        'blpwtk2_generate_sources',
        'blpangle',
        '../chrome/chrome_blpwtk2.gyp:chrome_blpwtk2',
        '../content/content.gyp:content_app',
        '../content/content.gyp:content_browser',
        '../content/content.gyp:content_common',
        '../content/content.gyp:content_gpu',
        '../content/content.gyp:content_plugin',
        '../content/content.gyp:content_ppapi_plugin',
        '../content/content.gyp:content_renderer',
        '../content/content.gyp:content_utility',
        '../content/content.gyp:content_worker',
        '../base/base.gyp:base',
        '../ipc/ipc.gyp:ipc',
        '../net/net.gyp:net',
        '../skia/skia.gyp:skia',
        '../ui/gl/gl.gyp:gl',
        '../ui/ui.gyp:ui',
        '../url/url.gyp:url_lib',
        '../v8/tools/gyp/v8.gyp:v8',
        '../webkit/support/webkit_support.gyp:webkit_support',
        '../sandbox/sandbox.gyp:sandbox',
      ],
      'conditions': [
        ['OS=="win" and win_use_allocator_shim==1', {
          'dependencies': [
            '../base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['bb_version!=""', {
          'product_name': 'blpwtk2.<(bb_version)',
        }],
      ],
      'sources': [
        '<(products_h)',
        '<(version_h)',
        '<(version_cc)',
        '../content/app/startup_helper_win.cc',
        'public/blpwtk2_config.h',
        'public/blpwtk2.h',
        'public/blpwtk2_string.cc',
        'public/blpwtk2_string.h',
        'public/blpwtk2_stringref.cc',
        'public/blpwtk2_stringref.h',
        'public/blpwtk2_webnode.cc',
        'public/blpwtk2_webnode.h',
        'public/blpwtk2_webelement.cc',
        'public/blpwtk2_webelement.h',
        'public/blpwtk2_webdocument.cc',
        'public/blpwtk2_webdocument.h',
        'public/blpwtk2_webframe.cc',
        'public/blpwtk2_webframe.h',
        'public/blpwtk2_webview.cc',
        'public/blpwtk2_webview.h',
        'public/blpwtk2_webviewcreateparams.cc',
        'public/blpwtk2_webviewcreateparams.h',
        'public/blpwtk2_webviewdelegate.cc',
        'public/blpwtk2_webviewdelegate.h',
        'public/blpwtk2_toolkit.cc',
        'public/blpwtk2_toolkit.h',
        'public/blpwtk2_toolkitcreateparams.cc',
        'public/blpwtk2_toolkitcreateparams.h',
        'public/blpwtk2_toolkitfactory.cc',
        'public/blpwtk2_toolkitfactory.h',
        'public/blpwtk2_threadmode.cc',
        'public/blpwtk2_threadmode.h',
        'public/blpwtk2_pumpmode.cc',
        'public/blpwtk2_pumpmode.h',
        'public/blpwtk2_constants.cc',
        'public/blpwtk2_constants.h',
        'public/blpwtk2_contextmenuitem.cc',
        'public/blpwtk2_contextmenuitem.h',
        'public/blpwtk2_contextmenuparams.cc',
        'public/blpwtk2_contextmenuparams.h',
        'public/blpwtk2_httptransactionhandler.cc',
        'public/blpwtk2_httptransactionhandler.h',
        'public/blpwtk2_httptransaction.cc',
        'public/blpwtk2_httptransaction.h',
        'public/blpwtk2_mediarequest.cc',
        'public/blpwtk2_mediarequest.h',
        'public/blpwtk2_newviewparams.cc',
        'public/blpwtk2_newviewparams.h',
        'public/blpwtk2_profile.cc',
        'public/blpwtk2_profile.h',
        'public/blpwtk2_proxyconfig.cc',
        'public/blpwtk2_proxyconfig.h',
        'public/blpwtk2_refcountedobject.cc',
        'public/blpwtk2_refcountedobject.h',
        'public/blpwtk2_spellcheckconfig.cc',
        'public/blpwtk2_spellcheckconfig.h',
        'public/blpwtk2_textdirection.cc',
        'public/blpwtk2_textdirection.h',
        'private/blpwtk2.rc',
        'private/blpwtk2_resourcecontextimpl.cc',
        'private/blpwtk2_resourcecontextimpl.h',
        'private/blpwtk2_urlrequestcontextgetterimpl.cc',
        'private/blpwtk2_urlrequestcontextgetterimpl.h',
        'private/blpwtk2_browsercontextimpl.cc',
        'private/blpwtk2_browsercontextimpl.h',
        'private/blpwtk2_contentbrowserclientimpl.cc',
        'private/blpwtk2_contentbrowserclientimpl.h',
        'private/blpwtk2_contentrendererclientimpl.cc',
        'private/blpwtk2_contentrendererclientimpl.h',
        'private/blpwtk2_renderviewobserverimpl.cc',
        'private/blpwtk2_renderviewobserverimpl.h',
        'private/blpwtk2_mediaobserverimpl.cc',
        'private/blpwtk2_mediaobserverimpl.h',
        'private/blpwtk2_mediarequestimpl.cc',
        'private/blpwtk2_mediarequestimpl.h',
        'private/blpwtk2_contentmaindelegateimpl.cc',
        'private/blpwtk2_contentmaindelegateimpl.h',
        'private/blpwtk2_browsermainrunner.cc',
        'private/blpwtk2_browsermainrunner.h',
        'private/blpwtk2_inprocessrenderer.cc',
        'private/blpwtk2_inprocessrenderer.h',
        'private/blpwtk2_inprocessrendererhost.cc',
        'private/blpwtk2_inprocessrendererhost.h',
        'private/blpwtk2_browserthread.cc',
        'private/blpwtk2_browserthread.h',
        'private/blpwtk2_ncdragutil.cc',
        'private/blpwtk2_ncdragutil.h',
        'private/blpwtk2_networkdelegateimpl.cc',
        'private/blpwtk2_networkdelegateimpl.h',
        'private/blpwtk2_profileimpl.cc',
        'private/blpwtk2_profileimpl.h',
        'private/blpwtk2_profilemanager.cc',
        'private/blpwtk2_profilemanager.h',
        'private/blpwtk2_proxyconfigimpl.cc',
        'private/blpwtk2_proxyconfigimpl.h',
        'private/blpwtk2_rendererinfomap.cc',
        'private/blpwtk2_rendererinfomap.h',
        'private/blpwtk2_webviewimpl.cc',
        'private/blpwtk2_webviewimpl.h',
        'private/blpwtk2_webviewimplclient.cc',
        'private/blpwtk2_webviewimplclient.h',
        'private/blpwtk2_webframeimpl.cc',
        'private/blpwtk2_webframeimpl.h',
        'private/blpwtk2_webviewproxy.cc',
        'private/blpwtk2_webviewproxy.h',
        'private/blpwtk2_mainmessagepump.cc',
        'private/blpwtk2_mainmessagepump.h',
        'private/blpwtk2_webcontentsviewdelegateimpl.cc',
        'private/blpwtk2_webcontentsviewdelegateimpl.h',
        'private/blpwtk2_devtoolsfrontendhostdelegateimpl.cc',
        'private/blpwtk2_devtoolsfrontendhostdelegateimpl.h',
        'private/blpwtk2_devtoolshttphandlerdelegateimpl.cc',
        'private/blpwtk2_devtoolshttphandlerdelegateimpl.h',
        'private/blpwtk2_httptransactionfactoryimpl.cc',
        'private/blpwtk2_httptransactionfactoryimpl.h',
        'private/blpwtk2_httptransactionimpl.cc',
        'private/blpwtk2_httptransactionimpl.h',
        'private/blpwtk2_toolkitimpl.cc',
        'private/blpwtk2_toolkitimpl.h',
        'private/blpwtk2_statics.cc',
        'private/blpwtk2_statics.h',
        'private/blpwtk2_dllmain.cc',
        'private/blpwtk2_prefstore.cc',
        'private/blpwtk2_prefstore.h',
        'private/blpwtk2_printutil.cc',
        'private/blpwtk2_printutil.h',
        'private/blpwtk2_findonpage.cc',
        'private/blpwtk2_findonpage.h',
      ],
      'include_dirs': [
        'public',
        'private',
        '..',
      ],
      'configurations': {
        'Debug_Base': {
          'msvs_settings': {
            'VCLinkerTool': {
              'LinkIncremental': '<(msvs_large_module_debug_link_mode)',
            },
          },
        },
      },
      'defines': [
        'BUILDING_BLPWTK2_SHARED',
        'BLPWTK2_IMPLEMENTATION',
      ],
      'direct_dependent_settings': {
        'defines': [
          'USING_BLPWTK2_SHARED',
        ],
        'include_dirs': [
          'public',
          '<(SHARED_INTERMEDIATE_DIR)/blpwtk2/public',
        ],
        'include_dirs!': [
          '../third_party/wtl/include',
          '..',
          '../third_party/khronos',
          '../gpu',
          '../third_party/WebKit/Source/Platform/chromium',
        ],
      },
    },
    {
      'target_name': 'blpangle',
      'type': 'loadable_module',
      'dependencies': [
        'blpwtk2_generate_sources',
        '<(angle_path)/src/build_angle.gyp:libGLESv2_static',
        '<(angle_path)/src/build_angle.gyp:libEGL_static',
      ],
      'conditions': [
        ['bb_version!=""', {
          'product_name': 'blpangle.<(bb_version)',
        }],
      ],
      'sources': [
        '<(angle_bindings_cc)',
        'angle/blpangle_dllmain.cc',
        'angle/blpangle.def',
        'angle/blpangle.rc',
      ],
    },
    {
      'target_name': 'blpwtk2_subprocess',
      'type': 'executable',
      'msvs_settings': {
        'VCLinkerTool': {
          'SubSystem': '2',  # Set /SUBSYSTEM:WINDOWS
        },
      },
      'conditions': [
        ['bb_version!=""', {
          'product_name': 'blpwtk2_subprocess.<(bb_version)',
        }],
      ],
      'dependencies': [
        'blpwtk2_generate_sources',
        '../sandbox/sandbox.gyp:sandbox',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        '../content/app/startup_helper_win.cc',
        'subprocess/main.cc',
        'subprocess/resources.rc',
      ],
    },
    {
      'target_name': 'blpwtk2_shell',
      'type': 'executable',
      'dependencies': [
        'blpwtk2',
        '../v8/tools/gyp/v8.gyp:v8',
      ],
      'msvs_settings': {
        'VCLinkerTool': {
          'SubSystem': '2',  # Set /SUBSYSTEM:WINDOWS
        },
      },
      'sources': [
        'shell/main.cc',
      ],
    },
    {
      'target_name': 'blpwtk2_devtools',
      'type': 'none',
      'dependencies': [
        '../content/browser/devtools/devtools_resources.gyp:devtools_resources',
      ],
      'variables': {
        'repack_path': '<(DEPTH)/tools/grit/grit/format/repack.py',
      },
      'actions': [
        {
          'action_name': 'repack_blpwtk2_devtools',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/webkit/devtools_resources.pak',
            ],
          },
          'inputs': [
            '<(repack_path)',
            '<@(pak_inputs)',
          ],
          'action': ['python', '<(repack_path)', '<@(_outputs)',
                     '<@(pak_inputs)'],
          'conditions': [
            ['bb_version!=""', {
              'outputs': [
                '<(PRODUCT_DIR)/blpwtk2_devtools.<(bb_version).pak',
              ],
            }, {
              'outputs': [
                '<(PRODUCT_DIR)/blpwtk2_devtools.pak',
              ],
            }],
          ],
        },
      ],
    },
    {
      'target_name': 'blpwtk2_all',
      'type': 'none',
      'dependencies': [
        'blpwtk2_subprocess',
        'blpwtk2_shell',
        'blpwtk2_devtools',
        '../content/content.gyp:content_shell',
        '../chrome/chrome_blpwtk2.gyp:chrome_blpwtk2',
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
