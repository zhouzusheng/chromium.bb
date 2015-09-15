// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_UTILITY_SHELL_CONTENT_UTILITY_CLIENT_H_
#define CONTENT_SHELL_UTILITY_SHELL_CONTENT_UTILITY_CLIENT_H_

#include "base/memory/scoped_vector.h"
#include "chrome/utility/utility_message_handler.h"
#include "content/public/utility/content_utility_client.h"

namespace content {

class ShellContentUtilityClient : public ContentUtilityClient {
 public:
  ShellContentUtilityClient();
  ~ShellContentUtilityClient() override;

  // ContentUtilityClient:
  void RegisterMojoApplications(StaticMojoApplicationMap* apps) override;
  bool OnMessageReceived(const IPC::Message& message) override;

  void onStartupPing();

 private:
  typedef ScopedVector<UtilityMessageHandler> Handlers;
  Handlers d_handlers;

  DISALLOW_COPY_AND_ASSIGN(ShellContentUtilityClient);
};

}  // namespace content

#endif  // CONTENT_SHELL_UTILITY_SHELL_CONTENT_UTILITY_CLIENT_H_
