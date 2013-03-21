// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/**
 * @fileoverview Provides the TimelineProcess class.
 */
base.require('timeline_process_base');
base.exportTo('tracing', function() {

  /**
   * The TimelineProcess represents a single userland process in the
   * trace.
   * @constructor
   */
  function TimelineProcess(pid) {
    tracing.TimelineProcessBase.call(this);
    this.pid = pid;
  };

  /**
   * Comparison between processes that orders by pid.
   */
  TimelineProcess.compare = function(x, y) {
    return x.pid - y.pid;
  };

  TimelineProcess.prototype = {
    __proto__: tracing.TimelineProcessBase.prototype,

    compareTo: function(that) {
      return TimelineProcess.compare(this, that);
    },

    get userFriendlyName() {
      return this.pid;
    },

    get userFriendlyDetails() {
      return 'pid: ' + this.pid;
    },
  };

  return {
    TimelineProcess: TimelineProcess
  };
});
