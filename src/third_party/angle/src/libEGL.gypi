# Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'conditions':
    [
        ['OS=="win"',
        {
            'targets':
            [
                {
                    'target_name': 'libEGL_static',
                    'type': 'static_library',
                    'dependencies': [ 'commit_id' ],
                    'include_dirs':
                    [
                        '.',
                        '../include',
                        'libGLESv2',
                    ],
                    'sources':
                    [
                        '<!@(python <(angle_path)/enumerate_files.py \
                             -dirs common libEGL ../include \
                             -excludes common/version.h libEGL/dllmain.cpp \
                             -types *.cpp *.h)',
                    ],
                    'defines':
                    [
                        'GL_APICALL=',
                        'GL_GLEXT_PROTOTYPES=',
                        'EGLAPI=',
                    ],
                    'includes': [ '../build/common_defines.gypi', ],
                    'link_settings':
                    {
                        'libraries':
                        [
                            '-ld3d9.lib',
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
                    'target_name': 'libEGL',
                    'type': 'loadable_module',
                    'dependencies': [ 'libGLESv2_shared', 'libEGL_static' ],
                    'include_dirs':
                    [
                        '.',
                        '../include',
                    ],
                    'sources':
                    [
                        'common/version.h',
                        'libEGL/dllmain.cpp',
                        'libEGL/libEGL.def',
                        'libEGL/libEGL.rc',
                    ],
                },
            ],
        },
        ],
    ],
}
