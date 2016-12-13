# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,  # Use higher warning level.
    'chromium_enable_vtune_jit_for_v8%': 0,  # enable the vtune support for V8 engine.
    'directxsdk_exists': '<!pymod_do_main(dir_exists ../third_party/directxsdk)',
  },
  'target_defaults': {
    'defines': ['CONTENT_IMPLEMENTATION'],
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
  'conditions': [
    ['OS != "ios"', {
      'includes': [
        '../build/win_precompile.gypi',
        'content_resources.gypi',
      ],
    }],
    # In component mode, we build all of content as a single DLL.
    # However, in the static mode, we need to build content as multiple
    # targets in order to prevent dependencies from getting introduced
    # upstream unnecessarily (e.g., content_renderer depends on allocator
    # and chrome_exe depends on content_common but we don't want
    # chrome_exe to have to depend on allocator).
    ['component=="static_library"', {
      'target_defines': [
        'COMPILE_CONTENT_STATICALLY',
      ],
      'targets': [
        {
          # GN version: //content
          'target_name': 'content',
          'type': 'none',
          'dependencies': [
            'content_browser',
            'content_common',
          ],
          'export_dependent_settings': [
            'content_common',
          ],
          'conditions': [
            ['OS != "ios"', {
              'dependencies': [
                'content_child',
                'content_gpu',
                'content_plugin',
#                'content_ppapi_plugin',
                'content_renderer',
                'content_utility',
              ],
            }],
          ],
        },
        {
          # GN version: //content/app:browser
          'target_name': 'content_app_browser',
          'type': 'static_library',
          'variables': { 'enable_wexit_time_destructors': 1, },
          'includes': [
            'content_app.gypi',
          ],
          'dependencies': [
            'content_common',
          ],
          'export_dependent_settings': [
            'content_common',
          ],
          'conditions': [
            ['chrome_multiple_dll', {
              'defines': [
                'CHROME_MULTIPLE_DLL_BROWSER',
              ],
            }],
          ],
        },
        {
          # GN version: //content/app:child
          'target_name': 'content_app_child',
          'type': 'static_library',
          'variables': { 'enable_wexit_time_destructors': 1, },
          'includes': [
            'content_app.gypi',
          ],
          'dependencies': [
            'content_common',
          ],
          'export_dependent_settings': [
            'content_common',
          ],
          'conditions': [
            ['chrome_multiple_dll', {
              'defines': [
                'CHROME_MULTIPLE_DLL_CHILD',
              ],
            }],
          ],
        },
        {
          # GN version: //content/app:both
          'target_name': 'content_app_both',
          'type': 'static_library',
          'variables': { 'enable_wexit_time_destructors': 1, },
          'includes': [
            'content_app.gypi',
          ],
          'dependencies': [
            'content_common',
          ],
          'export_dependent_settings': [
            'content_common',
          ],
        },
        {
          # GN version: //content/browser and //content/public/browser
          'target_name': 'content_browser',
          'type': 'static_library',
          'variables': { 'enable_wexit_time_destructors': 1, },
          'includes': [
            'content_browser.gypi',
            # Disable LTO due to ELF section name out of range
            # crbug.com/422251
          ],
          'dependencies': [
            'content_common',
          ],
          'export_dependent_settings': [
            'content_common',
          ],
          'conditions': [
            ['java_bridge==1', {
              'dependencies': [
                'content_child',
              ]
            }],
            ['OS=="android"', {
              'dependencies': [
                'content_gpu',
                'content_utility',
              ],
            }],
            ['OS != "ios"', {
              'dependencies': [
                'content_resources',
              ],
            }],
          ],
        },
        {
          # GN version: //content/common and //content/public/common
          'target_name': 'content_common',
          'type': 'static_library',
          'variables': { 'enable_wexit_time_destructors': 1, },
          'includes': [
            'content_common.gypi',
          ],
          'conditions': [
            ['OS != "ios"', {
              'dependencies': [
                'content_resources',
              ],
            }],
          ],
          # Disable c4267 warnings until we fix size_t to int truncations.
          'msvs_disabled_warnings': [ 4267, ],
        },
      ],
      'conditions': [
        ['OS != "ios"', {
          'targets': [
            {
              # GN version: //content/child and //content/public/child
              'target_name': 'content_child',
              'type': 'static_library',
              'variables': { 'enable_wexit_time_destructors': 1, },
              'includes': [
                'content_child.gypi',
              ],
              'dependencies': [
                'content_resources',
              ],
              # Disable c4267 warnings until we fix size_t to int truncations.
              'msvs_disabled_warnings': [ 4267, ],
            },
            {
              # GN version: //content/gpu
              'target_name': 'content_gpu',
              'type': 'static_library',
              'variables': { 'enable_wexit_time_destructors': 1, },
              'includes': [
                'content_gpu.gypi',
              ],
              'dependencies': [
                'content_child',
                'content_common',
              ],
            },
            {
              # GN version: //content/plugin and //content/public/plugin
              'target_name': 'content_plugin',
              'type': 'static_library',
              'variables': { 'enable_wexit_time_destructors': 1, },
              'includes': [
                'content_plugin.gypi',
              ],
              'dependencies': [
                'content_child',
                'content_common',
              ],
            },
            {
              # GN version: //content/renderer and //content/public/renderer
              'target_name': 'content_renderer',
              'type': 'static_library',
              'variables': { 'enable_wexit_time_destructors': 1, },
              'includes': [
                'content_renderer.gypi',
              ],
              'dependencies': [
                'content_child',
                'content_common',
                'content_resources',
              ],
              'export_dependent_settings': [
                'content_common',
              ],
              'conditions': [
                ['chromium_enable_vtune_jit_for_v8==1', {
                  'dependencies': [
                    '../v8/src/third_party/vtune/v8vtune.gyp:v8_vtune',
                  ],
                }],
              ],
            },
            {
              # GN version: //content/utility and //content/public/utility
              'target_name': 'content_utility',
              'type': 'static_library',
              'variables': { 'enable_wexit_time_destructors': 1, },
              'includes': [
                'content_utility.gypi',
              ],
              'dependencies': [
                'content_child',
                'content_common',
                'content_common_mojo_bindings.gyp:content_common_mojo_bindings',
              ],
            },
          ],
        }],
      ],
    },
    {  # component != static_library
      'targets': [
        {
          # GN version: //content
          'target_name': 'content',
          'type': 'shared_library',
          'variables': { 'enable_wexit_time_destructors': 1, },
          'dependencies': [
            'content_resources',
          ],
          'conditions': [
            ['chromium_enable_vtune_jit_for_v8==1', {
              'dependencies': [
                '../v8/src/third_party/vtune/v8vtune.gyp:v8_vtune',
              ],
            }],
          ],
          'includes': [
            'content_app.gypi',
            'content_browser.gypi',
            'content_child.gypi',
            'content_common.gypi',
            'content_gpu.gypi',
            'content_plugin.gypi',
#			'content_ppapi_plugin.gypi',
            'content_renderer.gypi',
            'content_utility.gypi',
          ],
          'msvs_settings': {
            'VCLinkerTool': {
              'conditions': [
                ['incremental_chrome_dll==1', {
                  'UseLibraryDependencyInputs': "true",
                }],
              ],
            },
          },
        },
        {
          # GN version: //content/app:browser
          'target_name': 'content_app_browser',
          'type': 'none',
          'dependencies': ['content', 'content_browser'],
        },
        {
          # GN version: //content/app:child
          'target_name': 'content_app_child',
          'type': 'none',
          'dependencies': ['content', 'content_child'],
        },
        {
          # GN version: //content/app:both
          'target_name': 'content_app_both',
          'type': 'none',
          'dependencies': ['content'],
          'export_dependent_settings': ['content'],
        },
        {
          # GN version: //content/browser and //content/public/browser
          'target_name': 'content_browser',
          'type': 'none',
          'dependencies': ['content'],
          'export_dependent_settings': ['content'],
        },
        {
          # GN version: //content/common and //content/public/common
          'target_name': 'content_common',
          'type': 'none',
          'dependencies': ['content', 'content_resources'],
          # Disable c4267 warnings until we fix size_t to int truncations.
          'msvs_disabled_warnings': [ 4267, ],
          'export_dependent_settings': ['content'],
        },
        {
          # GN Version: //content/child
          'target_name': 'content_child',
          'type': 'none',
          'dependencies': ['content'],
        },
        {
          # GN version: //content/gpu
          'target_name': 'content_gpu',
          'type': 'none',
          'dependencies': ['content'],
        },
        {
          # GN version: //content/plugin
          'target_name': 'content_plugin',
          'type': 'none',
          'dependencies': ['content'],
        },
        {
          # GN version: //content/renderer and //content/public/renderer
          'target_name': 'content_renderer',
          'type': 'none',
          'dependencies': ['content'],
          'export_dependent_settings': ['content'],
        },
        {
          # GN version: //content/utility
          'target_name': 'content_utility',
          'type': 'none',
          'dependencies': ['content'],
          'export_dependent_settings': ['content'],
        },
      ],
    }],
  ],
}
