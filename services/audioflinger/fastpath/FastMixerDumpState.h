/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <stdint.h>
#include <type_traits>
#include <audio_utils/TimestampVerifier.h>
#include "Configuration.h"
#include "FastThreadDumpState.h"
#include "FastMixerState.h"

namespace android {

// Describes the underrun status for a single "pull" attempt
enum FastTrackUnderrunStatus {
    UNDERRUN_FULL,      // framesReady() is full frame count, no underrun
    UNDERRUN_PARTIAL,   // framesReady() is non-zero but < full frame count, partial underrun
    UNDERRUN_EMPTY,     // framesReady() is zero, total underrun
};

// Underrun counters are not reset to zero for new tracks or if track generation changes.
// This packed representation is used to keep the information atomic.
union FastTrackUnderruns {
    FastTrackUnderruns() { mAtomic = 0;
            static_assert(sizeof(FastTrackUnderruns) == sizeof(uint32_t), "FastTrackUnderrun"); }
    FastTrackUnderruns(const FastTrackUnderruns& copyFrom) : mAtomic(copyFrom.mAtomic) { }
    FastTrackUnderruns& operator=(const FastTrackUnderruns& rhs)
            { if (this != &rhs) mAtomic = rhs.mAtomic; return *this; }
    struct {
#define UNDERRUN_BITS 10
#define UNDERRUN_MASK ((1 << UNDERRUN_BITS) - 1)
        uint32_t mFull    : UNDERRUN_BITS; // framesReady() is full frame count
        uint32_t mPartial : UNDERRUN_BITS; // framesReady() is non-zero but < full frame count
        uint32_t mEmpty   : UNDERRUN_BITS; // framesReady() is zero
        FastTrackUnderrunStatus mMostRecent : 2;    // status of most recent framesReady()
    }        mBitFields;
private:
    uint32_t mAtomic;
};

// Represents the dump state of a fast track
struct FastTrackDump {
    FastTrackUnderruns  mUnderruns;
    size_t              mFramesReady = 0;    // most recent value only; no long-term statistics kept
    int64_t             mFramesWritten = 0;  // last value from track
};

// No virtuals.
static_assert(!std::is_polymorphic_v<FastTrackDump>);

struct FastMixerDumpState : FastThreadDumpState {
    void dump(int fd) const;    // should only be called on a stable copy, not the original

    double   mLatencyMs = 0.;     // measured latency, default of 0 if no valid timestamp read.
    uint32_t mWriteSequence = 0;  // incremented before and after each write()
    uint32_t mFramesWritten = 0;  // total number of frames written successfully
    uint32_t mNumTracks = 0;      // total number of active fast tracks
    uint32_t mWriteErrors = 0;    // total number of write() errors
    uint32_t mSampleRate = 0;
    size_t   mFrameCount = 0;
    uint32_t mTrackMask = 0;      // mask of active tracks
    FastTrackDump   mTracks[FastMixerState::kMaxFastTracks];

    // For timestamp statistics.
    TimestampVerifier<int64_t /* frame count */, int64_t /* time ns */> mTimestampVerifier;
};

// No virtuals.
static_assert(!std::is_polymorphic_v<FastMixerDumpState>);

}  // namespace android
