# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'test_shell_windows_resource_files': [
      'resources/test_shell.rc',
      '../../glue/resources/pan_east.cur',
      '../../glue/resources/pan_middle.cur',
      '../../glue/resources/pan_north.cur',
      '../../glue/resources/pan_north_east.cur',
      '../../glue/resources/pan_north_west.cur',
      '../../glue/resources/pan_south.cur',
      '../../glue/resources/pan_south_east.cur',
      '../../glue/resources/pan_south_west.cur',
      '../../glue/resources/pan_west.cur',
      'resources/small.ico',
      'resources/test_shell.ico',
      'resource.h',
    ],
  },
  'targets': [
    {
      'target_name': 'test_shell_common',
      'type': 'static_library',
      'variables': {
        'chromium_code': 1,
      },
      'dependencies': [
        '../build/temp_gyp/googleurl.gyp:googleurl',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/base.gyp:base_i18n',
        '<(DEPTH)/base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '<(DEPTH)/gpu/gpu.gyp:gles2_c_lib',
        '<(DEPTH)/media/media.gyp:media',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/testing/gmock.gyp:gmock',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:inspector_resources',
        '<(DEPTH)/third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit',
        '<(DEPTH)/ui/native_theme/native_theme.gyp:native_theme',
        '<(DEPTH)/v8/tools/gyp/v8.gyp:v8',
        '<(DEPTH)/webkit/support/webkit_support.gyp:glue',
        '<(DEPTH)/webkit/support/webkit_support.gyp:user_agent',
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_base',
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_gpu',
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_media',
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_resources',
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_storage',
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_support_common',
      ],
      'sources': [
        'mac/test_shell_webview.h',
        'mac/test_shell_webview.mm',
        'accessibility_ui_element.cc',
        'accessibility_ui_element.h',
        'drop_delegate.cc',
        'drop_delegate.h',
        'mock_spellcheck.cc',
        'mock_spellcheck.h',
        'notification_presenter.cc',
        'notification_presenter.h',
        'resource.h',
        'test_navigation_controller.cc',
        'test_navigation_controller.h',
        'test_shell.cc',
        'test_shell.h',
        'test_shell_devtools_agent.cc',
        'test_shell_devtools_agent.h',
        'test_shell_devtools_callargs.cc',
        'test_shell_devtools_callargs.h',
        'test_shell_devtools_client.cc',
        'test_shell_devtools_client.h',
        'test_shell_gtk.cc',
        'test_shell_x11.cc',
        'test_shell_mac.mm',
        'test_shell_platform_delegate.h',
        'test_shell_platform_delegate_gtk.cc',
        'test_shell_platform_delegate_mac.mm',
        'test_shell_platform_delegate_win.cc',
        'test_shell_switches.cc',
        'test_shell_switches.h',
        'test_shell_win.cc',
        'test_shell_webkit_init.cc',
        'test_shell_webkit_init.h',
        'test_shell_webthemecontrol.h',
        'test_shell_webthemecontrol.cc',
        'test_shell_webthemeengine.h',
        'test_shell_webthemeengine.cc',
        'test_webview_delegate.cc',
        'test_webview_delegate.h',
        'test_webview_delegate_mac.mm',
        'test_webview_delegate_gtk.cc',
        'test_webview_delegate_win.cc',
        'webview_host.h',
        'webview_host_gtk.cc',
        'webview_host_mac.mm',
        'webview_host_win.cc',
        'webwidget_host.h',
        'webwidget_host.cc',
        'webwidget_host_gtk.cc',
        'webwidget_host_mac.mm',
        'webwidget_host_win.cc',
      ],
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit',
        '<(DEPTH)/webkit/support/webkit_support.gyp:glue',
        '<(DEPTH)/webkit/support/webkit_support.gyp:user_agent',
      ],
      'conditions': [
        ['toolkit_uses_gtk == 1', {
          'dependencies': [
            'test_shell_resources',
            '<(DEPTH)/build/linux/system.gyp:gtk',
            '<(DEPTH)/tools/xdisplaycheck/xdisplaycheck.gyp:xdisplaycheck',
          ],
          # for:  test_shell_gtk.cc
          'cflags': ['-Wno-multichar'],
        }],
        ['OS=="win"', {
          'msvs_disabled_warnings': [ 4800 ],
          'link_settings': {
            'libraries': [
              '-lcomctl32.lib',
            ],
          },
          'include_dirs': [
            '<(DEPTH)/third_party/wtl/include',
            '.',
          ],
          'dependencies': [
            '<(DEPTH)/breakpad/breakpad.gyp:breakpad_handler',
          ],
        }, {  # else: OS!=win
          'sources/': [
            ['exclude', '_webtheme(control|engine)\.(cc|h)$'],
          ],
          'sources!': [
            'drop_delegate.cc',
          ],
        }],
      ],
    },
    {
      'target_name': 'test_shell',
      'type': 'executable',
      'variables': {
        'chromium_code': 1,
      },
      'mac_bundle': 1,
      'dependencies': [
        'test_shell_common',
        '<(DEPTH)/net/net.gyp:net_test_support',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/third_party/mesa/mesa.gyp:osmesa',
        '<(DEPTH)/tools/imagediff/image_diff.gyp:image_diff',
        '<(DEPTH)/ui/gl/gl.gyp:gl',
      ],
      'defines': [
        # Technically not a unit test but require functions available only to
        # unit tests.
        'UNIT_TEST'
      ],
      'sources': [
        'test_shell_main.cc',
      ],
      'mac_bundle_resources': [
        '../../data/test_shell/',
        'mac/English.lproj/InfoPlist.strings',
        'mac/English.lproj/MainMenu.xib',
        'mac/Info.plist',
        'mac/test_shell.icns',
        'resources/AHEM____.TTF',
      ],
      'mac_bundle_resources!': [
        # TODO(mark): Come up with a fancier way to do this (mac_info_plist?)
        # that automatically sets the correct INFOPLIST_FILE setting and adds
        # the file to a source group.
        'mac/Info.plist',
      ],
      'xcode_settings': {
        'INFOPLIST_FILE': '<(DEPTH)/webkit/tools/test_shell/mac/Info.plist',
      },
      'conditions': [
        ['OS=="win"', {
          'resource_include_dirs': [
            '<(SHARED_INTERMEDIATE_DIR)/webkit',
          ],
          'sources': [
            '<@(test_shell_windows_resource_files)',
            # TODO:  It would be nice to have these pulled in
            # automatically from direct_dependent_settings in
            # their various targets (net.gyp:net_resources, etc.),
            # but that causes errors in other targets when
            # resulting .res files get referenced multiple times.
            '<(SHARED_INTERMEDIATE_DIR)/net/net_resources.rc',
            '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_chromium_resources.rc',
            '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_strings_en-US.rc',
            '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_unscaled_resources.rc',
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
        }],
        ['toolkit_uses_gtk == 1', {
          'conditions': [
            ['linux_use_tcmalloc==1', {
              'dependencies': [
                '<(DEPTH)/base/allocator/allocator.gyp:allocator',
              ],
            }],
          ],
          'dependencies': [
            '<(DEPTH)/build/linux/system.gyp:gtk',
            'test_shell_resources',
            'test_shell_pak',
          ],
        }],
        ['OS=="mac"', {
          'product_name': 'TestShell',
          'dependencies': [
            '<(DEPTH)/third_party/mesa/mesa.gyp:osmesa',
          ],
          'variables': {
            'repack_path': '../../../tools/grit/grit/format/repack.py',
          },
          'actions': [
            {
              # TODO(mark): Make this work with more languages than the
              # hardcoded en-US.
              'action_name': 'repack_locale',
              'variables': {
                'pak_inputs': [
                  '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_chromium_resources.pak',
                  '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_strings_en-US.pak',
                  '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_resources_100_percent.pak',
                ],
              },
              'inputs': [
                '<(repack_path)',
                '<@(pak_inputs)',
              ],
              'outputs': [
                '<(INTERMEDIATE_DIR)/repack/test_shell.pak',
              ],
              'action': ['python', '<(repack_path)', '<@(_outputs)', '<@(pak_inputs)'],
              'process_outputs_as_mac_bundle_resources': 1,
            },
          ],
          'copies': [
            {
              # Copy FFmpeg binaries for audio/video support.
              'destination': '<(PRODUCT_DIR)/TestShell.app/Contents/MacOS/',
              'files': [
                '<(PRODUCT_DIR)/ffmpegsumo.so',
              ],
            },
          ],
        }, { # OS != "mac"
          'dependencies': [
            '<(DEPTH)/net/net.gyp:net_resources',
            '<(DEPTH)/ui/ui.gyp:ui_resources',
            '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_resources',
            '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_strings',
          ]
        }],
      ],
    },
  ],
  'conditions': [
    ['os_posix == 1 and OS != "mac"', {
      'targets': [
        {
          'target_name': 'test_shell_resources',
          'type': 'none',
          'variables': {
            'grit_grd_file': './test_shell_resources.grd',
            'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/test_shell',
          },
          'actions': [
            {
              'action_name': 'test_shell_resources',
              'includes': [ '../../../build/grit_action.gypi' ],
            },
          ],
          'includes': [ '../../../build/grit_target.gypi' ],
        },
      ],
    }],
  ],
}
