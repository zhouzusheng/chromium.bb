// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TimingInput_h
#define TimingInput_h

#include "core/CoreExport.h"
#include "core/animation/Timing.h"
#include "wtf/Allocator.h"

namespace blink {

class Dictionary;
class KeyframeEffectOptions;

class CORE_EXPORT TimingInput {
    STATIC_ONLY(TimingInput);
public:
    static Timing convert(const KeyframeEffectOptions& timingInput);
    static Timing convert(double duration);

    static void setStartDelay(Timing&, double startDelay);
    static void setEndDelay(Timing&, double endDelay);
    static void setFillMode(Timing&, const String& fillMode);
    static void setIterationStart(Timing&, double iterationStart);
    static void setIterationCount(Timing&, double iterationCount);
    static void setIterationDuration(Timing&, double iterationDuration);
    static void setPlaybackRate(Timing&, double playbackRate);
    static void setPlaybackDirection(Timing&, const String& direction);
    static void setTimingFunction(Timing&, const String& timingFunctionString);
};

} // namespace blink

#endif
