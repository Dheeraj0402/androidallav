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

#include <type_traits>
#include "Configuration.h"
#include <stdint.h>
#include <media/nblog/NBLog.h>

namespace android {

struct FastThreadDumpState;

// Represents a single state of a FastThread
struct FastThreadState {
    using Command = uint32_t;
    static const Command
        INITIAL = 0,            // used only for the initial state
        HOT_IDLE = 1,           // do nothing
        COLD_IDLE = 2,          // wait for the futex
        IDLE = 3,               // either HOT_IDLE or COLD_IDLE
        EXIT = 4;               // exit from thread
        // additional values defined per subclass
    Command     mCommand = INITIAL;       // current command
    int32_t*    mColdFutexAddr = nullptr; // for COLD_IDLE only, pointer to the associated futex
    unsigned    mColdGen = 0;             // increment when COLD_IDLE is requested
                                          // so it's only performed once

    // This might be a one-time configuration rather than per-state
    FastThreadDumpState* mDumpState = nullptr; // if non-NULL, then update dump state periodically
    NBLog::Writer* mNBLogWriter = nullptr; // non-blocking logger

    // returns NULL if command belongs to a subclass
    static const char *commandToString(Command command);
};  // struct FastThreadState

// No virtuals.
static_assert(!std::is_polymorphic_v<FastThreadState>);

}  // namespace android
