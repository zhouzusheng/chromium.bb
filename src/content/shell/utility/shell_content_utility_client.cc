// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/utility/shell_content_utility_client.h"

#include "base/bind.h"
#include "base/memory/scoped_ptr.h"

// SHEZ: Remove test-only code.
// #include "content/public/test/test_mojo_app.h"

#include "mojo/shell/static_application_loader.h"

#include <chrome/common/chrome_paths.h>
#include <chrome/common/chrome_utility_messages.h>
#include <chrome/utility/printing_handler.h>
#include <content/public/utility/content_utility_client.h>
#include <content/public/utility/utility_thread.h>
#include <ipc/ipc_message_macros.h>

namespace content {

namespace {

// SHEZ: Remove test-only code
//scoped_ptr<mojo::ApplicationDelegate> CreateTestApp() {
//  return scoped_ptr<mojo::ApplicationDelegate>(new TestMojoApp);
//}

bool Send(IPC::Message* message) {
  return content::UtilityThread::Get()->Send(message);
}

}  // namespace

ShellContentUtilityClient::ShellContentUtilityClient() {
  d_handlers.push_back(new PrintingHandler());
}

ShellContentUtilityClient::~ShellContentUtilityClient() {
}

void ShellContentUtilityClient::RegisterMojoApplications(
    StaticMojoApplicationMap* apps) {
  // SHEZ: Remove test-only code
  // apps->insert(
  //     std::make_pair(GURL(kTestMojoAppUrl), base::Bind(&CreateTestApp)));
}

bool ShellContentUtilityClient::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ShellContentUtilityClient, message)
    IPC_MESSAGE_HANDLER(ChromeUtilityMsg_StartupPing, onStartupPing)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  for (Handlers::iterator it = d_handlers.begin(); !handled && it != d_handlers.end(); ++it)
    handled = (*it)->OnMessageReceived(message);

  return handled;
}

void ShellContentUtilityClient::onStartupPing() {
  Send(new ChromeUtilityHostMsg_ProcessStarted);
}

}  // namespace content
