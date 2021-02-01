/**
 * Copyright 2021, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "TunerFilter"

#include "TunerFilter.h"

using ::android::hardware::hidl_handle;
using ::android::hardware::tv::tuner::V1_0::DemuxFilterMainType;
using ::android::hardware::tv::tuner::V1_0::DemuxFilterSettings;
using ::android::hardware::tv::tuner::V1_0::DemuxMmtpFilterType;
using ::android::hardware::tv::tuner::V1_0::DemuxTsFilterSettings;
using ::android::hardware::tv::tuner::V1_0::DemuxTsFilterType;
using ::android::hardware::tv::tuner::V1_0::Result;
namespace android {

TunerFilter::TunerFilter(
        sp<IFilter> filter, sp<IFilterCallback> callback) {
    mFilter = filter;
    mFilter_1_1 = ::android::hardware::tv::tuner::V1_1::IFilter::castFrom(filter);
    mFilterCallback = callback;
}

TunerFilter::~TunerFilter() {
    mFilter = nullptr;
    mFilter_1_1 = nullptr;
    mFilterCallback = nullptr;
}

DemuxFilterAvSettings TunerFilter::getAvSettings(const TunerFilterSettings& settings) {
    DemuxFilterAvSettings av {
        .isPassthrough = settings.get<TunerFilterSettings::av>().isPassthrough,
    };
    return av;
}

Status TunerFilter::getId(int32_t* _aidl_return) {
    if (mFilter == nullptr) {
        ALOGE("IFilter is not initialized");
        return Status::fromServiceSpecificError(static_cast<int32_t>(Result::UNAVAILABLE));
    }

    Result res;
    mFilter->getId([&](Result r, uint32_t filterId) {
        res = r;
        mId = filterId;
    });
    if (res != Result::SUCCESS) {
        return Status::fromServiceSpecificError(static_cast<int32_t>(res));
    }
    *_aidl_return = mId;
    return Status::ok();
}

Status TunerFilter::getId64Bit(int64_t* _aidl_return) {
    if (mFilter_1_1 == nullptr) {
        ALOGE("IFilter_1_1 is not initialized");
        return Status::fromServiceSpecificError(static_cast<int32_t>(Result::UNAVAILABLE));
    }

    Result res;
    mFilter_1_1->getId64Bit([&](Result r, uint64_t filterId) {
        res = r;
        mId64Bit = filterId;
    });
    if (res != Result::SUCCESS) {
        return ::ndk::ScopedAStatus::fromServiceSpecificError(static_cast<int32_t>(res));
    }
    *_aidl_return = mId64Bit;
    return Status::ok();
}

Status TunerFilter::configure(const TunerFilterConfiguration& config) {
    if (mFilter == nullptr) {
        ALOGE("IFilter is not initialized");
        return Status::fromServiceSpecificError(static_cast<int32_t>(Result::UNAVAILABLE));
    }

    // TODO: more filter types.
    TunerFilterSettings tunerSettings;
    DemuxFilterSettings halSettings;
    switch (config.getTag()) {
        case TunerFilterConfiguration::ts: {
            uint16_t tpid = static_cast<uint16_t>(config.get<TunerFilterConfiguration::ts>().tpid);
            tunerSettings = config.get<TunerFilterConfiguration::ts>().filterSettings;
            DemuxTsFilterSettings ts {
                .tpid = tpid,
            };

            switch (tunerSettings.getTag()) {
                case TunerFilterSettings::av: {
                    ts.filterSettings.av(getAvSettings(tunerSettings));
                    break;
                }
            }
            halSettings.ts(ts);
            break;
        }
    }
    Result res = mFilter->configure(halSettings);
    if (res != Result::SUCCESS) {
        return Status::fromServiceSpecificError(static_cast<int32_t>(res));
    }
    return Status::ok();
}

Status TunerFilter::getAvSharedHandleInfo(TunerFilterSharedHandleInfo* _aidl_return) {
    if (mFilter_1_1 == nullptr) {
        ALOGE("IFilter_1_1 is not initialized");
        return Status::fromServiceSpecificError(static_cast<int32_t>(Result::UNAVAILABLE));
    }

    Result res;
    mFilter_1_1->getAvSharedHandle([&](Result r, hidl_handle avMemory, uint64_t avMemSize) {
        res = r;
        if (res == Result::SUCCESS) {
            TunerFilterSharedHandleInfo info{
                .handle = dupToAidl(hidl_handle(avMemory.getNativeHandle())),
                .size = static_cast<int64_t>(avMemSize),
            };
            *_aidl_return = std::move(info);
        } else {
            _aidl_return = NULL;
        }
    });

    return Status::fromServiceSpecificError(static_cast<int32_t>(res));
}

Status TunerFilter::releaseAvHandle(
        const ::aidl::android::hardware::common::NativeHandle& handle, int64_t avDataId) {
    if (mFilter == nullptr) {
        ALOGE("IFilter is not initialized");
        return Status::fromServiceSpecificError(static_cast<int32_t>(Result::UNAVAILABLE));
    }

    Result res = mFilter->releaseAvHandle(hidl_handle(makeFromAidl(handle)), avDataId);
    if (res != Result::SUCCESS) {
        return Status::fromServiceSpecificError(static_cast<int32_t>(res));
    }
    return Status::ok();
}


Status TunerFilter::start() {
    if (mFilter == nullptr) {
        ALOGE("IFilter is not initialized");
        return Status::fromServiceSpecificError(static_cast<int32_t>(Result::UNAVAILABLE));
    }
    Result res = mFilter->start();
    if (res != Result::SUCCESS) {
        return Status::fromServiceSpecificError(static_cast<int32_t>(res));
    }
    return Status::ok();
}

Status TunerFilter::stop() {
    if (mFilter == nullptr) {
        ALOGE("IFilter is not initialized");
        return Status::fromServiceSpecificError(static_cast<int32_t>(Result::UNAVAILABLE));
    }
    Result res = mFilter->stop();
    if (res != Result::SUCCESS) {
        return Status::fromServiceSpecificError(static_cast<int32_t>(res));
    }
    return Status::ok();
}

Status TunerFilter::flush() {
    if (mFilter == nullptr) {
        ALOGE("IFilter is not initialized");
        return Status::fromServiceSpecificError(static_cast<int32_t>(Result::UNAVAILABLE));
    }
    Result res = mFilter->flush();
    if (res != Result::SUCCESS) {
        return Status::fromServiceSpecificError(static_cast<int32_t>(res));
    }
    return Status::ok();
}

Status TunerFilter::close() {
    if (mFilter == nullptr) {
        ALOGE("IFilter is not initialized");
        return Status::fromServiceSpecificError(static_cast<int32_t>(Result::UNAVAILABLE));
    }
    Result res = mFilter->close();
    if (res != Result::SUCCESS) {
        return Status::fromServiceSpecificError(static_cast<int32_t>(res));
    }
    return Status::ok();
}

sp<IFilter> TunerFilter::getHalFilter() {
    return mFilter;
}

/////////////// FilterCallback ///////////////////////

void TunerFilter::FilterCallback::getMediaEvent(
        std::vector<DemuxFilterEvent::Event>& events, std::vector<TunerFilterEvent>& res) {
    for (DemuxFilterEvent::Event e : events) {
        DemuxFilterMediaEvent mediaEvent = e.media();
        TunerFilterMediaEvent tunerMedia;

        tunerMedia.streamId = static_cast<int>(mediaEvent.streamId);
        tunerMedia.isPtsPresent = mediaEvent.isPtsPresent;
        tunerMedia.pts = static_cast<long>(mediaEvent.pts);
        tunerMedia.dataLength = static_cast<int>(mediaEvent.dataLength);
        tunerMedia.offset = static_cast<int>(mediaEvent.offset);
        tunerMedia.isSecureMemory = mediaEvent.isSecureMemory;
        tunerMedia.avDataId = static_cast<long>(mediaEvent.avDataId);
        tunerMedia.mpuSequenceNumber = static_cast<int>(mediaEvent.mpuSequenceNumber);
        tunerMedia.isPesPrivateData = mediaEvent.isPesPrivateData;

        if (mediaEvent.avMemory.getNativeHandle() != nullptr) {
            tunerMedia.avMemory = dupToAidl(mediaEvent.avMemory.getNativeHandle());
        }

        TunerFilterEvent tunerEvent;
        tunerEvent.set<TunerFilterEvent::media>(std::move(tunerMedia));
        res.push_back(std::move(tunerEvent));
    }
}

Return<void> TunerFilter::FilterCallback::onFilterStatus(DemuxFilterStatus status) {
    mTunerFilterCallback->onFilterStatus((int)status);
    return Void();
}

Return<void> TunerFilter::FilterCallback::onFilterEvent(const DemuxFilterEvent& filterEvent) {
    std::vector<DemuxFilterEvent::Event> events = filterEvent.events;
    std::vector<TunerFilterEvent> tunerEvent;

    if (!events.empty()) {
        DemuxFilterEvent::Event event = events[0];
        switch (event.getDiscriminator()) {
            case DemuxFilterEvent::Event::hidl_discriminator::media: {
                getMediaEvent(events, tunerEvent);
                break;
            }
            default: {
                break;
            }
        }
    }

    mTunerFilterCallback->onFilterEvent(tunerEvent);
    return Void();
}

}  // namespace android
