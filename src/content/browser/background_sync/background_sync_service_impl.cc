// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/background_sync/background_sync_service_impl.h"

#include "background_sync_registration_handle.h"
#include "base/memory/weak_ptr.h"
#include "base/stl_util.h"
#include "content/browser/background_sync/background_sync_context_impl.h"
#include "content/public/browser/browser_thread.h"

namespace content {

namespace {

// TODO(iclelland): Move these converters to mojo::TypeConverter template
// specializations.

BackgroundSyncRegistrationOptions ToBackgroundSyncRegistrationOptions(
    const SyncRegistrationPtr& in) {
  BackgroundSyncRegistrationOptions out;

  out.tag = in->tag;
  out.min_period = in->min_period_ms;
  out.power_state = static_cast<SyncPowerState>(in->power_state);
  out.network_state = static_cast<SyncNetworkState>(in->network_state);
  out.periodicity = static_cast<SyncPeriodicity>(in->periodicity);
  return out;
}

SyncRegistrationPtr ToMojoRegistration(
    const BackgroundSyncRegistrationHandle& in) {
  SyncRegistrationPtr out(content::SyncRegistration::New());
  out->handle_id = in.handle_id();
  out->tag = in.options()->tag;
  out->min_period_ms = in.options()->min_period;
  out->periodicity = static_cast<content::BackgroundSyncPeriodicity>(
      in.options()->periodicity);
  out->power_state =
      static_cast<content::BackgroundSyncPowerState>(in.options()->power_state);
  out->network_state = static_cast<content::BackgroundSyncNetworkState>(
      in.options()->network_state);
  return out.Pass();
}

}  // namespace

#define COMPILE_ASSERT_MATCHING_ENUM(mojo_name, manager_name) \
  COMPILE_ASSERT(static_cast<int>(content::mojo_name) ==      \
                     static_cast<int>(content::manager_name), \
                 mismatching_enums)

// TODO(iclelland): Move these tests somewhere else
COMPILE_ASSERT_MATCHING_ENUM(BACKGROUND_SYNC_ERROR_NONE,
                             BACKGROUND_SYNC_STATUS_OK);
COMPILE_ASSERT_MATCHING_ENUM(BACKGROUND_SYNC_ERROR_STORAGE,
                             BACKGROUND_SYNC_STATUS_STORAGE_ERROR);
COMPILE_ASSERT_MATCHING_ENUM(BACKGROUND_SYNC_ERROR_NOT_FOUND,
                             BACKGROUND_SYNC_STATUS_NOT_FOUND);
COMPILE_ASSERT_MATCHING_ENUM(BACKGROUND_SYNC_ERROR_NO_SERVICE_WORKER,
                             BACKGROUND_SYNC_STATUS_NO_SERVICE_WORKER);
COMPILE_ASSERT_MATCHING_ENUM(BACKGROUND_SYNC_ERROR_NOT_ALLOWED,
                             BACKGROUND_SYNC_STATUS_NOT_ALLOWED);
COMPILE_ASSERT_MATCHING_ENUM(BACKGROUND_SYNC_ERROR_MAX,
                             BACKGROUND_SYNC_STATUS_NOT_ALLOWED);

COMPILE_ASSERT_MATCHING_ENUM(BACKGROUND_SYNC_NETWORK_STATE_ANY,
                             SyncNetworkState::NETWORK_STATE_ANY);
COMPILE_ASSERT_MATCHING_ENUM(BACKGROUND_SYNC_NETWORK_STATE_AVOID_CELLULAR,
                             SyncNetworkState::NETWORK_STATE_AVOID_CELLULAR);
COMPILE_ASSERT_MATCHING_ENUM(BACKGROUND_SYNC_NETWORK_STATE_ONLINE,
                             SyncNetworkState::NETWORK_STATE_ONLINE);
COMPILE_ASSERT_MATCHING_ENUM(BACKGROUND_SYNC_NETWORK_STATE_MAX,
                             SyncNetworkState::NETWORK_STATE_ONLINE);

COMPILE_ASSERT_MATCHING_ENUM(BACKGROUND_SYNC_POWER_STATE_AUTO,
                             SyncPowerState::POWER_STATE_AUTO);
COMPILE_ASSERT_MATCHING_ENUM(BACKGROUND_SYNC_POWER_STATE_AVOID_DRAINING,
                             SyncPowerState::POWER_STATE_AVOID_DRAINING);
COMPILE_ASSERT_MATCHING_ENUM(BACKGROUND_SYNC_POWER_STATE_MAX,
                             SyncPowerState::POWER_STATE_AVOID_DRAINING);

COMPILE_ASSERT_MATCHING_ENUM(BACKGROUND_SYNC_PERIODICITY_PERIODIC,
                             SyncPeriodicity::SYNC_PERIODIC);
COMPILE_ASSERT_MATCHING_ENUM(BACKGROUND_SYNC_PERIODICITY_ONE_SHOT,
                             SyncPeriodicity::SYNC_ONE_SHOT);
COMPILE_ASSERT_MATCHING_ENUM(BACKGROUND_SYNC_PERIODICITY_MAX,
                             SyncPeriodicity::SYNC_ONE_SHOT);

BackgroundSyncServiceImpl::~BackgroundSyncServiceImpl() {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  DCHECK(background_sync_context_->background_sync_manager());
}

BackgroundSyncServiceImpl::BackgroundSyncServiceImpl(
    BackgroundSyncContextImpl* background_sync_context,
    mojo::InterfaceRequest<BackgroundSyncService> request)
    : background_sync_context_(background_sync_context),
      binding_(this, request.Pass()),
      weak_ptr_factory_(this) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  DCHECK(background_sync_context);

  binding_.set_connection_error_handler(
      base::Bind(&BackgroundSyncServiceImpl::OnConnectionError,
                 base::Unretained(this) /* the channel is owned by this */));
}

void BackgroundSyncServiceImpl::OnConnectionError() {
  background_sync_context_->ServiceHadConnectionError(this);
  // |this| is now deleted.
}

void BackgroundSyncServiceImpl::Register(content::SyncRegistrationPtr options,
                                         int64_t sw_registration_id,
                                         bool requested_from_service_worker,
                                         const RegisterCallback& callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);

  BackgroundSyncRegistrationOptions mgr_options =
      ToBackgroundSyncRegistrationOptions(options);

  BackgroundSyncManager* background_sync_manager =
      background_sync_context_->background_sync_manager();
  DCHECK(background_sync_manager);
  background_sync_manager->Register(
      sw_registration_id, mgr_options, requested_from_service_worker,
      base::Bind(&BackgroundSyncServiceImpl::OnRegisterResult,
                 weak_ptr_factory_.GetWeakPtr(), callback));
}

void BackgroundSyncServiceImpl::Unregister(
    BackgroundSyncRegistrationHandle::HandleId handle_id,
    int64_t sw_registration_id,
    const UnregisterCallback& callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);

  BackgroundSyncRegistrationHandle* registration =
      active_handles_.Lookup(handle_id);
  if (!registration) {
    callback.Run(BACKGROUND_SYNC_ERROR_NOT_ALLOWED);
    return;
  }

  registration->Unregister(
      sw_registration_id,
      base::Bind(&BackgroundSyncServiceImpl::OnUnregisterResult,
                 weak_ptr_factory_.GetWeakPtr(), callback));
}

void BackgroundSyncServiceImpl::GetRegistration(
    BackgroundSyncPeriodicity periodicity,
    const mojo::String& tag,
    int64_t sw_registration_id,
    const GetRegistrationCallback& callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  BackgroundSyncManager* background_sync_manager =
      background_sync_context_->background_sync_manager();
  DCHECK(background_sync_manager);
  background_sync_manager->GetRegistration(
      sw_registration_id, tag.get(), static_cast<SyncPeriodicity>(periodicity),
      base::Bind(&BackgroundSyncServiceImpl::OnRegisterResult,
                 weak_ptr_factory_.GetWeakPtr(), callback));
}

void BackgroundSyncServiceImpl::GetRegistrations(
    BackgroundSyncPeriodicity periodicity,
    int64_t sw_registration_id,
    const GetRegistrationsCallback& callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  BackgroundSyncManager* background_sync_manager =
      background_sync_context_->background_sync_manager();
  DCHECK(background_sync_manager);
  background_sync_manager->GetRegistrations(
      sw_registration_id, static_cast<SyncPeriodicity>(periodicity),
      base::Bind(&BackgroundSyncServiceImpl::OnGetRegistrationsResult,
                 weak_ptr_factory_.GetWeakPtr(), callback));
}

void BackgroundSyncServiceImpl::GetPermissionStatus(
    BackgroundSyncPeriodicity periodicity,
    int64_t sw_registration_id,
    const GetPermissionStatusCallback& callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);

  // TODO(iclelland): Implement a real policy. This is a stub implementation.
  // OneShot: crbug.com/482091
  // Periodic: crbug.com/482093
  callback.Run(BACKGROUND_SYNC_ERROR_NONE, PERMISSION_STATUS_GRANTED);
}

void BackgroundSyncServiceImpl::DuplicateRegistrationHandle(
    BackgroundSyncRegistrationHandle::HandleId handle_id,
    const DuplicateRegistrationHandleCallback& callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  BackgroundSyncManager* background_sync_manager =
      background_sync_context_->background_sync_manager();
  DCHECK(background_sync_manager);

  scoped_ptr<BackgroundSyncRegistrationHandle> registration_handle =
      background_sync_manager->DuplicateRegistrationHandle(handle_id);

  BackgroundSyncRegistrationHandle* handle_ptr = registration_handle.get();

  if (!registration_handle) {
    callback.Run(BACKGROUND_SYNC_ERROR_NOT_FOUND,
                 SyncRegistrationPtr(content::SyncRegistration::New()));
    return;
  }

  active_handles_.AddWithID(registration_handle.release(),
                            handle_ptr->handle_id());
  SyncRegistrationPtr mojoResult = ToMojoRegistration(*handle_ptr);
  callback.Run(BACKGROUND_SYNC_ERROR_NONE, mojoResult.Pass());
}

void BackgroundSyncServiceImpl::ReleaseRegistration(
    BackgroundSyncRegistrationHandle::HandleId handle_id) {
  if (!active_handles_.Lookup(handle_id)) {
    // TODO(jkarlin): Abort client.
    LOG(WARNING) << "Client attempted to release non-existing registration";
    return;
  }

  active_handles_.Remove(handle_id);
}

void BackgroundSyncServiceImpl::NotifyWhenFinished(
    BackgroundSyncRegistrationHandle::HandleId handle_id,
    const NotifyWhenFinishedCallback& callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  BackgroundSyncRegistrationHandle* registration =
      active_handles_.Lookup(handle_id);
  if (!registration) {
    callback.Run(BACKGROUND_SYNC_ERROR_NOT_ALLOWED,
                 BACKGROUND_SYNC_STATE_FAILED);
    return;
  }

  registration->NotifyWhenFinished(
      base::Bind(&BackgroundSyncServiceImpl::OnNotifyWhenFinishedResult,
                 weak_ptr_factory_.GetWeakPtr(), callback));
}

void BackgroundSyncServiceImpl::OnRegisterResult(
    const RegisterCallback& callback,
    BackgroundSyncStatus status,
    scoped_ptr<BackgroundSyncRegistrationHandle> result) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  BackgroundSyncRegistrationHandle* result_ptr = result.get();

  if (status != BACKGROUND_SYNC_STATUS_OK) {
    callback.Run(static_cast<content::BackgroundSyncError>(status),
                 SyncRegistrationPtr(content::SyncRegistration::New()));
    return;
  }

  active_handles_.AddWithID(result.release(), result_ptr->handle_id());
  SyncRegistrationPtr mojoResult = ToMojoRegistration(*result_ptr);
  callback.Run(static_cast<content::BackgroundSyncError>(status),
               mojoResult.Pass());
}

void BackgroundSyncServiceImpl::OnUnregisterResult(
    const UnregisterCallback& callback,
    BackgroundSyncStatus status) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  callback.Run(static_cast<content::BackgroundSyncError>(status));
}

void BackgroundSyncServiceImpl::OnGetRegistrationsResult(
    const GetRegistrationsCallback& callback,
    BackgroundSyncStatus status,
    scoped_ptr<ScopedVector<BackgroundSyncRegistrationHandle>>
        result_registrations) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  mojo::Array<content::SyncRegistrationPtr> mojo_registrations(0);
  for (BackgroundSyncRegistrationHandle* registration : *result_registrations) {
    active_handles_.AddWithID(registration, registration->handle_id());
    mojo_registrations.push_back(ToMojoRegistration(*registration));
  }

  result_registrations->weak_clear();

  callback.Run(static_cast<content::BackgroundSyncError>(status),
               mojo_registrations.Pass());
}

void BackgroundSyncServiceImpl::OnNotifyWhenFinishedResult(
    const NotifyWhenFinishedCallback& callback,
    BackgroundSyncStatus status,
    BackgroundSyncState sync_state) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  callback.Run(static_cast<content::BackgroundSyncError>(status), sync_state);
}

}  // namespace content
