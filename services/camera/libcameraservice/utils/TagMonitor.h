/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef ANDROID_SERVERS_CAMERA_TAGMONITOR_H
#define ANDROID_SERVERS_CAMERA_TAGMONITOR_H

#include <vector>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <string>

#include <utils/RefBase.h>
#include <utils/Timers.h>

#include <media/RingBuffer.h>
#include <system/camera_metadata.h>
#include <system/camera_vendor_tags.h>
#include <camera/CameraMetadata.h>
#include <device3/InFlightRequest.h>

namespace android {

/**
 * A monitor for camera metadata values.
 * Tracks changes to specified metadata values over time, keeping a circular
 * buffer log that can be dumped at will. */
class TagMonitor {
  public:

    // Monitor argument
    static const std::string kMonitorOption;

    enum eventSource {
        REQUEST,
        RESULT
    };

    TagMonitor();

    TagMonitor(const TagMonitor& other);

    void initialize(metadata_vendor_id_t id) { mVendorTagId = id; }

    // Parse tag name list (comma-separated) and if valid, enable monitoring
    // If invalid, do nothing.
    // Recognizes "3a" as a shortcut for enabling tracking 3A state, mode, and
    // triggers
    void parseTagsToMonitor(std::string tagNames);

    // Disable monitoring; does not clear the event log
    void disableMonitoring();

    // Scan through the metadata and update the monitoring information
    void monitorMetadata(eventSource source, int64_t frameNumber,
            nsecs_t timestamp, const CameraMetadata& metadata,
            const std::unordered_map<std::string, CameraMetadata>& physicalMetadata,
            const camera3::camera_stream_buffer_t *outputBuffers = nullptr,
            uint32_t numOutputBuffers = 0, int32_t inputStreamId = -1);

    // Dump current event log to the provided fd
    void dumpMonitoredMetadata(int fd);

    // Dumps the latest monitored Tag events to the passed vector.
    // NOTE: The events are appended to the vector in reverser chronological order
    // (i.e. most recent first)
    void getLatestMonitoredTagEvents(std::vector<std::string> &out);

  private:
    // Dumps monitored tag events to the passed vector without acquiring
    // mMonitorMutex. mMonitorMutex must be acquired before calling this
    // function.
    void dumpMonitoredTagEventsToVectorLocked(std::vector<std::string> &out);

    static std::string getEventDataString(const uint8_t* data_ptr, uint32_t tag, int type,
            int count, int indentation);

    void monitorSingleMetadata(TagMonitor::eventSource source, int64_t frameNumber,
            nsecs_t timestamp, const std::string& cameraId, uint32_t tag,
            const CameraMetadata& metadata, const std::unordered_set<int32_t> &outputStreamIds,
            int32_t inputStreamId);

    std::atomic<bool> mMonitoringEnabled;
    std::mutex mMonitorMutex;

    // Current tags to monitor and record changes to
    std::vector<uint32_t> mMonitoredTagList;

    // Latest-seen values of tracked tags
    CameraMetadata mLastMonitoredRequestValues;
    CameraMetadata mLastMonitoredResultValues;

    std::unordered_map<std::string, CameraMetadata> mLastMonitoredPhysicalRequestKeys;
    std::unordered_map<std::string, CameraMetadata> mLastMonitoredPhysicalResultKeys;

    int32_t mLastInputStreamId = -1;
    std::unordered_set<int32_t> mLastStreamIds;

    /**
     * A monitoring event
     * Stores a new metadata field value and the timestamp at which it changed.
     * Copies the source metadata value array and frees it on destruct.
     */
    struct MonitorEvent {
        template<typename T>
        MonitorEvent(eventSource src, uint32_t frameNumber, nsecs_t timestamp,
                const T &newValue, const std::string& cameraId,
                const std::unordered_set<int32_t> &outputStreamIds, int32_t inputStreamId);
        ~MonitorEvent();

        eventSource source;
        uint32_t frameNumber;
        nsecs_t timestamp;
        std::string cameraId;
        uint32_t tag;
        uint8_t type;
        std::vector<uint8_t> newData;
        // NOTE: We want to print changes to outputStreamIds and inputStreamId in their own lines.
        // So any MonitorEvent where these fields are not the default value will have garbage
        // values for all fields other than source, frameNumber, timestamp, and cameraId.
        std::unordered_set<int32_t> outputStreamIds;
        int32_t inputStreamId = -1;
    };

    // A ring buffer for tracking the last kMaxMonitorEvents metadata changes
    static const int kMaxMonitorEvents = 100;
    RingBuffer<MonitorEvent> mMonitoringEvents;

    // 3A fields to use with the "3a" option
    static const char *k3aTags;
    metadata_vendor_id_t mVendorTagId;
};

} // namespace android

#endif
