# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
    'gin_gen_path': '<(SHARED_INTERMEDIATE_DIR)/gin/',
  },
  'targets': [
    {
      'target_name': 'gin',
      'type': '<(component)',
      'dependencies': [
        '../base/base.gyp:base',
        '../blpwtk2/blpwtk2.gyp:blpwtk2_generate_sources',
        '../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../v8/tools/gyp/v8.gyp:v8',
      ],
      'export_dependent_settings': [
        '../base/base.gyp:base',
        '../v8/tools/gyp/v8.gyp:v8',
      ],
      'defines': [
        'GIN_IMPLEMENTATION',
      ],
      'sources': [
        'arguments.cc',
        'arguments.h',
        'array_buffer.cc',
        'array_buffer.h',
        'context_holder.cc',
        'converter.cc',
        'converter.h',
        'debug_impl.cc',
        'debug_impl.h',
        'dictionary.cc',
        'dictionary.h',
        'function_template.cc',
        'function_template.h',
        'gin_export.h',
        'handle.h',
        'interceptor.cc',
        'interceptor.h',
        'isolate_holder.cc',
        'modules/console.cc',
        'modules/console.h',
        'modules/file_module_provider.cc',
        'modules/file_module_provider.h',
        'modules/module_registry.cc',
        'modules/module_registry.h',
        'modules/module_registry_observer.h',
        'modules/module_runner_delegate.cc',
        'modules/module_runner_delegate.h',
        'modules/timer.cc',
        'modules/timer.h',
        'object_template_builder.cc',
        'object_template_builder.h',
        'per_context_data.cc',
        'per_context_data.h',
        'per_isolate_data.cc',
        'per_isolate_data.h',
        'public/context_holder.h',
        'public/debug.h',
        'public/gin_embedders.h',
        'public/isolate_holder.h',
        'public/v8_platform.h',
        'public/wrapper_info.h',
        'runner.cc',
        'runner.h',
        'run_microtasks_observer.cc',
        'run_microtasks_observer.h',
        'shell_runner.cc',
        'shell_runner.h',
        'try_catch.cc',
        'try_catch.h',
        'v8_initializer.h',
        'v8_initializer.cc',
        'v8_isolate_memory_dump_provider.cc',
        'v8_isolate_memory_dump_provider.h',
        'v8_platform.cc',
        'wrappable.cc',
        'wrappable.h',
        'wrapper_info.cc',
      ],
      'conditions': [
        ['v8_use_external_startup_data==1 and OS=="win"', {
          'dependencies': [
            '../crypto/crypto.gyp:crypto',
          ],
          'defines': [
            'V8_VERIFY_EXTERNAL_STARTUP_DATA',
          ]
        }],
      ],
    },
    {
      'target_name': 'gin_v8_snapshot_fingerprint',
      'type': 'none',
      'variables': {
        'snapshot_file': '<(PRODUCT_DIR)/snapshot_blob<(bb_version_suffix).bin',
        'natives_file': '<(PRODUCT_DIR)/natives_blob<(bb_version_suffix).bin',
        'output_file': '<(gin_gen_path)/v8_snapshot_fingerprint.cc',
      },
      'includes': [ '../gin/fingerprint/fingerprint_v8_snapshot.gypi' ],
    },
    {
      'target_name': 'gin_shell',
      'type': 'executable',
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:base_i18n',
        '../v8/tools/gyp/v8.gyp:v8',
        'gin',
      ],
      'sources': [
        'shell/gin_main.cc',
      ],
      'msvs_settings': {
        'VCLinkerTool': {
          'SubSystem': '1', # /SUBSYSTEM:CONSOLE
        },
      },
    },
    {
      'target_name': 'gin_test',
      'type': 'static_library',
      'dependencies': [
        '../v8/tools/gyp/v8.gyp:v8',
        'gin',
      ],
      'export_dependent_settings': [
        'gin',
      ],
      'sources': [
        'test/file.cc',
        'test/file.h',
        'test/file_runner.cc',
        'test/file_runner.h',
        'test/gc.cc',
        'test/gc.h',
        'test/gtest.cc',
        'test/gtest.h',
        'test/v8_test.cc',
        'test/v8_test.h',
      ],
    },
    {
      'target_name': 'gin_unittests',
      'type': 'executable',
      'dependencies': [
        '../v8/tools/gyp/v8.gyp:v8',
        'gin_test',
      ],
      'sources': [
        'converter_unittest.cc',
        'interceptor_unittest.cc',
        'modules/module_registry_unittest.cc',
        'modules/timer_unittest.cc',
        'per_context_data_unittest.cc',
        'shell_runner_unittest.cc',
        'shell/gin_shell_unittest.cc',
        'test/run_all_unittests.cc',
        'test/run_js_tests.cc',
        'v8_isolate_memory_dump_provider_unittest.cc',
        'wrappable_unittest.cc',
      ],
    },
  ],
}
