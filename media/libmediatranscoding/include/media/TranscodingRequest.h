/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef ANDROID_MEDIA_TRANSCODING_REQUEST_H
#define ANDROID_MEDIA_TRANSCODING_REQUEST_H

#include <aidl/android/media/TranscodingRequestParcel.h>

namespace android {

using ::aidl::android::media::TranscodingRequestParcel;

// Helper class for duplicating a TranscodingRequestParcel
class TranscodingRequest : public TranscodingRequestParcel {
public:
    TranscodingRequest() = default;
    TranscodingRequest(const TranscodingRequestParcel& parcel) { setTo(parcel); }
    TranscodingRequest& operator=(const TranscodingRequest& request) {
        setTo(request);
        return *this;
    }

private:
    void setTo(const TranscodingRequestParcel& parcel) {
        sourceFilePath = parcel.sourceFilePath;
        sourceFd = parcel.sourceFd;
        destinationFilePath = parcel.destinationFilePath;
        destinationFd = parcel.destinationFd;
        clientUid = parcel.clientUid;
        clientPid = parcel.clientPid;
        clientPackageName = parcel.clientPackageName;
        transcodingType = parcel.transcodingType;
        requestedVideoTrackFormat = parcel.requestedVideoTrackFormat;
        priority = parcel.priority;
        requestProgressUpdate = parcel.requestProgressUpdate;
        requestSessionEventUpdate = parcel.requestSessionEventUpdate;
        isForTesting = parcel.isForTesting;
        testConfig = parcel.testConfig;
    }
};

}  // namespace android
#endif  // ANDROID_MEDIA_TRANSCODING_REQUEST_H
