# Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'target_defaults':
    {
        'defines':
        [
          'ANGLE_PRELOADED_D3DCOMPILER_MODULE_NAMES={ TEXT("d3dcompiler_46.dll"), TEXT("d3dcompiler_43.dll") }',
        ],
    },

    'conditions':
    [
        ['OS=="win"',
        {
            'targets':
            [
                {
                    'target_name': 'libGLESv2_static',
                    'type': 'static_library',
                    'dependencies': [ 'translator', 'commit_id', 'copy_compiler_dll' ],
                    'include_dirs':
                    [
                        '.',
                        '../include',
                        'libGLESv2',
                        '<(SHARED_INTERMEDIATE_DIR)',
                    ],
                    'sources': [ '<!@(python enumerate_files.py common libGLESv2 third_party/murmurhash -exclude common/version.h libGLESv2/dllmain.cpp -types *.cpp *.h *.hlsl *.vs *.ps *.bat)' ],
                    'msvs_disabled_warnings': [ 4267 ],
                    'link_settings':
                    {
                        'libraries':
                        [
                            '-ld3d9.lib',
                            '-ldxguid.lib',
                        ],
                    },
                    'configurations':
                    {
                        'Debug':
                        {
                            'defines':
                            [
                                'ANGLE_ENABLE_PERF',
                            ],
                        },
                    },
                },
                {
                    'target_name': 'libGLESv2_shared',
                    'product_name': 'libGLESv2',
                    'type': 'shared_library',
                    'dependencies': [ 'libGLESv2_static' ],
                    'include_dirs':
                    [
                        '.',
                        '../include',
                    ],
                    'sources':
                    [
                        'common/version.h',
                        'libGLESv2/dllmain.cpp',
                        'libGLESv2/libGLESv2.def',
                        'libGLESv2/libGLESv2.rc',
                    ],
                },
            ],
        },
        ],
    ],
}
