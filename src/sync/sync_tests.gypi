# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    # Test support files for the 'sync_core' target.
    {
      'target_name': 'test_support_sync_core',
      'type': 'static_library',
      'variables': { 'enable_wexit_time_destructors': 1, },
      'include_dirs': [
        '..',
      ],
      'defines': [
        'SYNC_TEST'
      ],
      'dependencies': [
        '../base/base.gyp:base',
        'sync',
      ],
      'export_dependent_settings': [
        'sync',
      ],
      'sources': [
        'internal_api/public/base/model_type_test_util.cc',
        'internal_api/public/base/model_type_test_util.h',
        'js/js_test_util.cc',
        'js/js_test_util.h',
        'sessions/test_util.cc',
        'sessions/test_util.h',
        'test/callback_counter.h',
        'test/engine/fake_model_worker.cc',
        'test/engine/fake_model_worker.h',
        'test/engine/fake_sync_scheduler.cc',
        'test/engine/fake_sync_scheduler.h',
        'test/engine/mock_connection_manager.cc',
        'test/engine/mock_connection_manager.h',
        'test/engine/syncer_command_test.cc',
        'test/engine/syncer_command_test.h',
        'test/engine/test_directory_setter_upper.cc',
        'test/engine/test_directory_setter_upper.h',
        'test/engine/test_id_factory.h',
        'test/engine/test_syncable_utils.cc',
        'test/engine/test_syncable_utils.h',
        'test/fake_encryptor.cc',
        'test/fake_encryptor.h',
        'test/fake_sync_encryption_handler.cc',
        'test/fake_sync_encryption_handler.h',
        'test/null_directory_change_delegate.cc',
        'test/null_directory_change_delegate.h',
        'test/null_transaction_observer.cc',
        'test/null_transaction_observer.h',
        'test/sessions/test_scoped_session_event_listener.h',
        'test/test_directory_backing_store.cc',
        'test/test_directory_backing_store.h',
        'test/test_transaction_observer.cc',
        'test/test_transaction_observer.h',
        'util/test_unrecoverable_error_handler.cc',
        'util/test_unrecoverable_error_handler.h',
      ],
    },

    # Test support files for the python sync test server.
    {
      'target_name': 'test_support_sync_testserver',
      'type': 'static_library',
      'variables': { 'enable_wexit_time_destructors': 1, },
      'include_dirs': [
        '..',
      ],
      'dependencies': [
        '../base/base.gyp:base',
        # The sync test server uses Python modules generated by the sync protos.
        '../third_party/protobuf/protobuf.gyp:py_proto',
        'sync',
      ],
      'export_dependent_settings': [
        '../base/base.gyp:base',
      ],
      'sources': [
        'test/local_sync_test_server.cc',
        'test/local_sync_test_server.h',
      ],
    },

    # Test support files for the 'sync_notifier' target.
    {
      'target_name': 'test_support_sync_notifier',
      'type': 'static_library',
      'include_dirs': [
        '..',
      ],
      'defines': [
        'SYNC_TEST'
      ],
      'dependencies': [
        '../third_party/cacheinvalidation/cacheinvalidation.gyp:cacheinvalidation_proto_cpp',
        'sync',
      ],
      'export_dependent_settings': [
        '../third_party/cacheinvalidation/cacheinvalidation.gyp:cacheinvalidation_proto_cpp',
        'sync',
      ],
      'sources': [
        'notifier/fake_invalidation_state_tracker.cc',
        'notifier/fake_invalidation_state_tracker.h',
        'notifier/fake_invalidator.cc',
        'notifier/fake_invalidator.h',
        'notifier/fake_invalidation_handler.cc',
        'notifier/fake_invalidation_handler.h',
        'notifier/invalidator_test_template.cc',
        'notifier/invalidator_test_template.h',
        'notifier/object_id_invalidation_map_test_util.cc',
        'notifier/object_id_invalidation_map_test_util.h',
      ],
    },

    # Test support files for the 'sync_internal_api' target.
    {
      'target_name': 'test_support_sync_internal_api',
      'type': 'static_library',
      'variables': { 'enable_wexit_time_destructors': 1, },
      'include_dirs': [
        '..',
      ],
      'defines': [
        'SYNC_TEST'
      ],
      'dependencies': [
        '../base/base.gyp:base',
        'sync',
        'test_support_sync_core',
      ],
      'export_dependent_settings': [
        'sync',
        'test_support_sync_core',
      ],
      'sources': [
        'internal_api/public/base/invalidation_test_util.cc',
        'internal_api/public/base/invalidation_test_util.h',
        'internal_api/public/test/fake_sync_manager.h',
        'internal_api/public/test/test_entry_factory.h',
        'internal_api/public/test/test_internal_components_factory.h',
        'internal_api/public/test/test_user_share.h',
        'internal_api/test/fake_sync_manager.cc',
        'internal_api/test/test_entry_factory.cc',
        'internal_api/test/test_internal_components_factory.cc',
        'internal_api/test/test_user_share.cc',
      ],
    },

    # Test support files for the 'sync_api' target.
    {
      'target_name': 'test_support_sync_api',
      'type': 'static_library',
      'include_dirs': [
        '..',
      ],
      'defines': [
        'SYNC_TEST'
      ],
      'dependencies': [
        'sync',
      ],
      'export_dependent_settings': [
        'sync',
      ],
      'sources': [
        'api/fake_syncable_service.cc',
        'api/fake_syncable_service.h',
        'api/sync_error_factory_mock.cc',
        'api/sync_error_factory_mock.h',
      ],
    },

    # Unit tests for the 'sync_core' target.  This cannot be a static
    # library because the unit test files have to be compiled directly
    # into the executable, so we push the target files to the
    # depending executable target via direct_dependent_settings.
    {
      'target_name': 'sync_core_tests',
      'type': 'none',
      # We only want unit test executables to include this target.
      'suppress_wildcard': 1,
      'dependencies': [
        '../base/base.gyp:base',
        '../sql/sql.gyp:sql',
        'sync',
        'test_support_sync_core',
      ],
      'conditions': [
        ['OS=="linux" and chromeos==1', {
          # Required by get_session_name_unittest.cc on Chrome OS.
          'dependencies': [
            '../chromeos/chromeos.gyp:chromeos',
          ],
        }],
      ],
      # Propagate all dependencies since the actual compilation
      # happens in the dependents.
      'export_dependent_settings': [
        '../base/base.gyp:base',
        '../sql/sql.gyp:sql',
        'sync',
        'test_support_sync_core',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '..',
        ],
        'sources': [
          'internal_api/public/base/cancelation_signal_unittest.cc',
          'internal_api/public/base/enum_set_unittest.cc',
          'internal_api/public/base/node_ordinal_unittest.cc',
          'internal_api/public/base/ordinal_unittest.cc',
          'internal_api/public/base/unique_position_unittest.cc',
          'internal_api/public/engine/model_safe_worker_unittest.cc',
          'internal_api/public/util/immutable_unittest.cc',
          'internal_api/public/util/weak_handle_unittest.cc',
          'engine/apply_control_data_updates_unittest.cc',
          'engine/apply_updates_and_resolve_conflicts_command_unittest.cc',
          'engine/backoff_delay_provider_unittest.cc',
          'engine/download_unittest.cc',
          'engine/model_changing_syncer_command_unittest.cc',
          'engine/process_commit_response_command_unittest.cc',
          'engine/process_updates_command_unittest.cc',
          'engine/store_timestamps_command_unittest.cc',
          'engine/sync_scheduler_unittest.cc',
          'engine/syncer_proto_util_unittest.cc',
          'engine/syncer_unittest.cc',
          'engine/traffic_recorder_unittest.cc',
          'js/js_arg_list_unittest.cc',
          'js/js_event_details_unittest.cc',
          'js/sync_js_controller_unittest.cc',
          'protocol/proto_enum_conversions_unittest.cc',
          'protocol/proto_value_conversions_unittest.cc',
          'sessions/nudge_tracker_unittest.cc',
          'sessions/ordered_commit_set_unittest.cc',
          'sessions/status_controller_unittest.cc',
          'sessions/sync_session_unittest.cc',
          'syncable/directory_backing_store_unittest.cc',
          'syncable/model_type_unittest.cc',
          'syncable/nigori_util_unittest.cc',
          'syncable/parent_child_index_unittest.cc',
          'syncable/syncable_enum_conversions_unittest.cc',
          'syncable/syncable_id_unittest.cc',
          'syncable/syncable_unittest.cc',
          'syncable/syncable_util_unittest.cc',
          'util/cryptographer_unittest.cc',
          'util/data_type_histogram_unittest.cc',
          'util/get_session_name_unittest.cc',
          'util/nigori_unittest.cc',
          'util/protobuf_unittest.cc',
        ],
        'conditions': [
          ['OS == "ios" and coverage != 0', {
            'sources!': [
              # These sources can't be built with coverage due to a toolchain
              # bug: http://openradar.appspot.com/radar?id=1499403
              'engine/syncer_unittest.cc',

              # These tests crash when run with coverage turned on due to an
              # issue with llvm_gcda_increment_indirect_counter:
              # http://crbug.com/156058
              'syncable/directory_backing_store_unittest.cc',
            ],
          }],
        ],
      },
    },

    # Unit tests for the 'sync_notifier' target.  This cannot be a static
    # library because the unit test files have to be compiled directly
    # into the executable, so we push the target files to the
    # depending executable target via direct_dependent_settings.
    {
      'target_name': 'sync_notifier_tests',
      'type': 'none',
      # We only want unit test executables to include this target.
      'suppress_wildcard': 1,
      'dependencies': [
        '../base/base.gyp:base',
        '../jingle/jingle.gyp:notifier_test_util',
        '../third_party/cacheinvalidation/cacheinvalidation.gyp:cacheinvalidation',
        '../third_party/libjingle/libjingle.gyp:libjingle',
        'sync',
        'test_support_sync_notifier',
      ],
      # Propagate all dependencies since the actual compilation
      # happens in the dependents.
      'export_dependent_settings': [
        '../base/base.gyp:base',
        '../jingle/jingle.gyp:notifier_test_util',
        '../third_party/cacheinvalidation/cacheinvalidation.gyp:cacheinvalidation',
        '../third_party/libjingle/libjingle.gyp:libjingle',
        'sync',
        'test_support_sync_notifier',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '..',
        ],
        'conditions': [
          ['OS != "android"', {
            'sources': [
              'notifier/ack_tracker_unittest.cc',
              'notifier/fake_invalidator_unittest.cc',
              'notifier/invalidation_notifier_unittest.cc',
              'notifier/invalidator_registrar_unittest.cc',
              'notifier/non_blocking_invalidator_unittest.cc',
              'notifier/p2p_invalidator_unittest.cc',
              'notifier/push_client_channel_unittest.cc',
              'notifier/registration_manager_unittest.cc',
              'notifier/sync_invalidation_listener_unittest.cc',
              'notifier/sync_system_resources_unittest.cc',
            ],
          }],
        ],
      },
    },

    # Unit tests for the 'sync_internal_api' target.  This cannot be a static
    # library because the unit test files have to be compiled directly
    # into the executable, so we push the target files to the
    # depending executable target via direct_dependent_settings.
    {
      'target_name': 'sync_internal_api_tests',
      'type': 'none',
      # We only want unit test executables to include this target.
      'suppress_wildcard': 1,
      'dependencies': [
        '../base/base.gyp:base',
        '../net/net.gyp:net',
        'sync',
        'test_support_sync_internal_api',
      ],
      # Propagate all dependencies since the actual compilation
      # happens in the dependents.
      'export_dependent_settings': [
        '../base/base.gyp:base',
        '../net/net.gyp:net',
        'sync',
        'test_support_sync_internal_api',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '..',
        ],
        'sources': [
          'internal_api/debug_info_event_listener_unittest.cc',
          'internal_api/http_bridge_unittest.cc',
          'internal_api/js_mutation_event_observer_unittest.cc',
          'internal_api/js_sync_encryption_handler_observer_unittest.cc',
          'internal_api/js_sync_manager_observer_unittest.cc',
          'internal_api/public/change_record_unittest.cc',
          'internal_api/public/sessions/sync_session_snapshot_unittest.cc',
          'internal_api/syncapi_server_connection_manager_unittest.cc',
          'internal_api/sync_encryption_handler_impl_unittest.cc',
          'internal_api/sync_manager_impl_unittest.cc',
        ],
        'conditions': [
          ['OS == "ios"', {
            'sources!': [
              'internal_api/http_bridge_unittest.cc',
            ],
          }],
        ],
      },
    },

    # Unit tests for the 'sync_api' target.  This cannot be a static
    # library because the unit test files have to be compiled directly
    # into the executable, so we push the target files to the
    # depending executable target via direct_dependent_settings.
    {
      'target_name': 'sync_api_tests',
      'type': 'none',
      # We only want unit test executables to include this target.
      'suppress_wildcard': 1,
      'dependencies': [
        '../base/base.gyp:base',
        'sync',
        'test_support_sync_internal_api',
      ],
      # Propagate all dependencies since the actual compilation
      # happens in the dependents.
      'export_dependent_settings': [
        '../base/base.gyp:base',
        'sync',
        'test_support_sync_internal_api',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '..',
        ],
        'sources': [
          'api/sync_change_unittest.cc',
          'api/sync_error_unittest.cc',
          'api/sync_merge_result_unittest.cc',
        ],
      },
    },

    # The unit test executable for sync tests.
    {
      'target_name': 'sync_unit_tests',
      'type': '<(gtest_target_type)',
      # Typed-parametrized tests generate exit-time destructors.
      'variables': { 'enable_wexit_time_destructors': 0, },
      'defines': [
        'SYNC_TEST',
      ],
      'dependencies': [
        '../base/base.gyp:run_all_unittests',
        'sync_api_tests',
        'sync_core_tests',
        'sync_internal_api_tests',
        'sync_notifier_tests',
      ],
      'conditions': [
        # TODO(akalin): This is needed because histogram.cc uses
        # leak_annotations.h, which pulls this in.  Make 'base'
        # propagate this dependency.
        ['OS=="linux" and linux_use_tcmalloc==1', {
          'dependencies': [
            '../base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['OS == "android" and gtest_target_type == "shared_library"', {
          'dependencies': [
            '../testing/android/native_test.gyp:native_test_native_code',
          ],
        }],
      ],
    },

    # Test support files for using the Test Accounts service.
    {
      'target_name': 'test_support_accounts_client',
      'type': 'static_library',
      'direct_dependent_settings': {
        'include_dirs': [
          '..',
        ],
      },
      'dependencies': [
        '../base/base.gyp:base',
        '../net/net.gyp:net',
      ],
      'sources': [
        'test/accounts_client/test_accounts_client.cc',
        'test/accounts_client/test_accounts_client.h',
        'test/accounts_client/url_request_context_getter.cc',
        'test/accounts_client/url_request_context_getter.h',
      ],
    },

    # The Sync end-to-end (and associated infrastructure) tests.
    {
      'target_name': 'sync_endtoend_tests',
      'type': '<(gtest_target_type)',
      'dependencies': [
        '../base/base.gyp:run_all_unittests',
        '../url/url.gyp:url_lib',
        'test_support_accounts_client',
      ],
      'sources': [
        'test/accounts_client/test_accounts_client_unittest.cc',
      ],
    },

  ],
  'conditions': [
    ['OS != "ios"', {
      'targets': [
        {
          'target_name': 'sync_tools_helper',
          'type': 'static_library',
          'defines': [
            'SYNC_IMPLEMENTATION',
          ],
          'include_dirs': [
            '..',
          ],
          'dependencies': [
            '../base/base.gyp:base',
            'sync',
          ],
          'export_dependent_settings': [
            '../base/base.gyp:base',
            'sync',
          ],
          'sources': [
            'tools/null_invalidation_state_tracker.cc',
            'tools/null_invalidation_state_tracker.h',
          ],
        },

        # A tool that can be used to launch a python sync server instance.
        {
          'target_name': 'run_sync_testserver',
          'type': 'executable',
          'dependencies': [
            '../base/base.gyp:base',
            'test_support_sync_testserver',
          ],
          'sources': [
            'tools/testserver/run_sync_testserver.cc',
          ],
        },

        # A tool to listen to sync notifications and print them out.
        {
          'target_name': 'sync_listen_notifications',
          'type': 'executable',
          'defines': [
            'SYNC_TEST',
          ],
          'dependencies': [
            '../base/base.gyp:base',
            '../jingle/jingle.gyp:notifier',
            '../net/net.gyp:net',
            'sync',
            'sync_tools_helper',
          ],
          'sources': [
            'tools/sync_listen_notifications.cc',
          ],
        },

        # A standalone command-line sync client.
        {
          'target_name': 'sync_client',
          'type': 'executable',
          'defines': [
            'SYNC_TEST',
          ],
          'dependencies': [
            '../base/base.gyp:base',
            '../jingle/jingle.gyp:notifier',
            '../net/net.gyp:net',
            'sync',
            'sync_tools_helper',
            'test_support_sync_core'
          ],
          'sources': [
            'tools/sync_client.cc',
          ],
        },
      ],
    }],
    ['OS == "android"', {
      'targets': [
        {
          'target_name': 'sync_javatests',
          'type': 'none',
          'variables': {
            'java_in_dir': '../sync/android/javatests',
          },
          'dependencies': [
            'sync_java',
            'sync_java_test_support',
            '../base/base.gyp:base_java_test_support',
          ],
          'includes': [ '../build/java.gypi' ],
        },
        {
          'target_name': 'sync_java_test_support',
          'type': 'none',
          'variables': {
            'package_name': 'sync_java_test_support',
            'java_in_dir': '../sync/test/android/javatests',
          },
          'dependencies': [
            'sync_java',
          ],
          'includes': [ '../build/java.gypi' ],
        },
      ],
    }],
    # Special target to wrap a gtest_target_type==shared_library
    # sync_unit_tests into an android apk for execution.
    ['OS == "android" and gtest_target_type == "shared_library"', {
      'targets': [
        {
          'target_name': 'sync_unit_tests_apk',
          'type': 'none',
          'dependencies': [
            'sync_unit_tests',
          ],
          'variables': {
            'test_suite_name': 'sync_unit_tests',
            'input_shlib_path': '<(SHARED_LIB_DIR)/<(SHARED_LIB_PREFIX)sync_unit_tests<(SHARED_LIB_SUFFIX)',
          },
          'includes': [ '../build/apk_test.gypi' ],
        },
      ],
    }],
  ],
}