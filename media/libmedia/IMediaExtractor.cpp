/*
 * Copyright (C) 2009 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "BpMediaExtractor"
#include <utils/Log.h>

#include <stdint.h>
#include <time.h>
#include <sys/types.h>

#include <binder/IPCThreadState.h>
#include <binder/Parcel.h>
#include <binder/PermissionCache.h>
#include <android/IMediaExtractor.h>
#include <media/stagefright/MetaData.h>

namespace android {

enum {
    COUNTTRACKS = IBinder::FIRST_CALL_TRANSACTION,
    GETTRACK,
    GETTRACKMETADATA,
    GETMETADATA,
    FLAGS,
    SETMEDIACAS,
    NAME,
    GETMETRICS,
    SETENTRYPOINT,
    SETLOGSESSIONID
};

class BpMediaExtractor : public BpInterface<IMediaExtractor> {
public:
    explicit BpMediaExtractor(const sp<IBinder>& impl)
        : BpInterface<IMediaExtractor>(impl)
    {
    }

    virtual size_t countTracks() {
        ALOGV("countTracks");
        Parcel data, reply;
        data.writeInterfaceToken(BpMediaExtractor::getInterfaceDescriptor());
        status_t ret = remote()->transact(COUNTTRACKS, data, &reply);
        size_t numTracks = 0;
        if (ret == NO_ERROR) {
            numTracks = reply.readUint32();
        }
        return numTracks;
    }
    virtual sp<IMediaSource> getTrack(size_t index) {
        ALOGV("getTrack(%zu)", index);
        Parcel data, reply;
        data.writeInterfaceToken(BpMediaExtractor::getInterfaceDescriptor());
        data.writeUint32(index);
        status_t ret = remote()->transact(GETTRACK, data, &reply);
        if (ret == NO_ERROR) {
            return interface_cast<IMediaSource>(reply.readStrongBinder());
        }
        return NULL;
    }

    virtual sp<MetaData> getTrackMetaData(
            size_t index, uint32_t flags) {
        ALOGV("getTrackMetaData(%zu, %u)", index, flags);
        Parcel data, reply;
        data.writeInterfaceToken(BpMediaExtractor::getInterfaceDescriptor());
        data.writeUint32(index);
        data.writeUint32(flags);
        status_t ret = remote()->transact(GETTRACKMETADATA, data, &reply);
        if (ret == NO_ERROR) {
            return MetaData::createFromParcel(reply);
        }
        return NULL;
    }

    virtual sp<MetaData> getMetaData() {
        ALOGV("getMetaData");
        Parcel data, reply;
        data.writeInterfaceToken(BpMediaExtractor::getInterfaceDescriptor());
        status_t ret = remote()->transact(GETMETADATA, data, &reply);
        if (ret == NO_ERROR) {
            return MetaData::createFromParcel(reply);
        }
        return NULL;
    }

    virtual status_t getMetrics(Parcel * reply) {
        Parcel data;
        data.writeInterfaceToken(BpMediaExtractor::getInterfaceDescriptor());
        status_t ret = remote()->transact(GETMETRICS, data, reply);
        if (ret == NO_ERROR) {
            return OK;
        }
        return UNKNOWN_ERROR;
    }

    virtual uint32_t flags() const {
        ALOGV("flags");
        Parcel data, reply;
        data.writeInterfaceToken(BpMediaExtractor::getInterfaceDescriptor());
        status_t ret = remote()->transact(FLAGS, data, &reply);
        int flgs = 0;
        if (ret == NO_ERROR) {
            flgs = reply.readUint32();
        }
        return flgs;
    }

    virtual status_t setMediaCas(const HInterfaceToken &casToken) {
        ALOGV("setMediaCas");

        Parcel data, reply;
        data.writeInterfaceToken(BpMediaExtractor::getInterfaceDescriptor());
        data.writeByteVector(casToken);

        status_t err = remote()->transact(SETMEDIACAS, data, &reply);
        if (err != NO_ERROR) {
            return err;
        }
        return reply.readInt32();
    }

    virtual String8 name() {
        Parcel data, reply;
        data.writeInterfaceToken(BpMediaExtractor::getInterfaceDescriptor());
        status_t ret = remote()->transact(NAME, data, &reply);
        String8 nm;
        if (ret == NO_ERROR) {
            nm = reply.readString8();
        }
        return nm;
    }

    virtual status_t setEntryPoint(EntryPoint entryPoint) {
        Parcel data, reply;
        data.writeInterfaceToken(BpMediaExtractor::getInterfaceDescriptor());
        data.writeInt32(static_cast<int32_t>(entryPoint));
        return remote()->transact(SETENTRYPOINT, data, &reply);
    }

    virtual status_t setLogSessionId(const String8& logSessionId) {
        Parcel data, reply;
        data.writeInterfaceToken(BpMediaExtractor::getInterfaceDescriptor());
        data.writeString8(logSessionId);
        return remote()->transact(SETLOGSESSIONID, data, &reply);
    }
};

IMPLEMENT_META_INTERFACE(MediaExtractor, "android.media.IMediaExtractor");

#undef LOG_TAG
#define LOG_TAG "BnMediaExtractor"

status_t BnMediaExtractor::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
        case COUNTTRACKS: {
            ALOGV("countTracks");
            CHECK_INTERFACE(IMediaExtractor, data, reply);
            size_t numTracks = countTracks();
            if (numTracks > INT32_MAX) {
                numTracks = 0;
            }
            reply->writeUint32(uint32_t(numTracks));
            return NO_ERROR;
        }
        case GETTRACK: {
            ALOGV("getTrack()");
            CHECK_INTERFACE(IMediaExtractor, data, reply);
            uint32_t idx;
            if (data.readUint32(&idx) == NO_ERROR) {
                const sp<IMediaSource> track = getTrack(size_t(idx));
                registerMediaSource(this, track);
                return reply->writeStrongBinder(IInterface::asBinder(track));
            }
            return UNKNOWN_ERROR;
        }
        case GETTRACKMETADATA: {
            ALOGV("getTrackMetaData");
            CHECK_INTERFACE(IMediaExtractor, data, reply);
            uint32_t idx;
            uint32_t flags;
            if (data.readUint32(&idx) == NO_ERROR &&
                    data.readUint32(&flags) == NO_ERROR) {
                sp<MetaData> meta = getTrackMetaData(idx, flags);
                if (meta == NULL) {
                    return UNKNOWN_ERROR;
                }
                meta->writeToParcel(*reply);
                return NO_ERROR;
            }
            return UNKNOWN_ERROR;
        }
        case GETMETADATA: {
            ALOGV("getMetaData");
            CHECK_INTERFACE(IMediaExtractor, data, reply);
            sp<MetaData> meta = getMetaData();
            if (meta != NULL) {
                meta->writeToParcel(*reply);
                return NO_ERROR;
            }
            return UNKNOWN_ERROR;
        }
        case GETMETRICS: {
            CHECK_INTERFACE(IMediaExtractor, data, reply);
            status_t ret = getMetrics(reply);
            return ret;
        }
        case FLAGS: {
            ALOGV("flags");
            CHECK_INTERFACE(IMediaExtractor, data, reply);
            reply->writeUint32(this->flags());
            return NO_ERROR;
        }
        case SETMEDIACAS: {
            ALOGV("setMediaCas");
            CHECK_INTERFACE(IMediaExtractor, data, reply);

            HInterfaceToken casToken;
            status_t err = data.readByteVector(&casToken);
            if (err != NO_ERROR) {
                ALOGE("Error reading casToken from parcel");
                return err;
            }

            reply->writeInt32(setMediaCas(casToken));
            return OK;
        }
        case NAME: {
            ALOGV("name");
            CHECK_INTERFACE(IMediaExtractor, data, reply);
            String8 nm = name();
            reply->writeString8(nm);
            return NO_ERROR;
        }
        case SETENTRYPOINT: {
            ALOGV("setEntryPoint");
            CHECK_INTERFACE(IMediaExtractor, data, reply);
            int32_t entryPoint;
            status_t err = data.readInt32(&entryPoint);
            if (err == OK) {
                setEntryPoint(EntryPoint(entryPoint));
            }
            return err;
        }
        case SETLOGSESSIONID: {
            ALOGV("setLogSessionId");
            CHECK_INTERFACE(IMediaExtractor, data, reply);
            String8 logSessionId;
            status_t status = data.readString8(&logSessionId);
            if (status == OK) {
              setLogSessionId(logSessionId);
            }
            return status;
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

typedef struct {
    String8 mime;
    String8 name;
    String8 sourceDescription;
    pid_t owner;
    wp<IMediaExtractor> extractor;
    Vector<wp<IMediaSource>> tracks;
    Vector<String8> trackDescriptions;
    String8 toString() const;
    time_t when;
} ExtractorInstance;

String8 ExtractorInstance::toString() const {
    String8 str;
    char timeString[32];
    strftime(timeString, sizeof(timeString), "%m-%d %T", localtime(&when));
    str.append(timeString);
    str.append(": ");
    str.append(name);
    str.append(" for mime ");
    str.append(mime);
    str.append(", source ");
    str.append(sourceDescription);
    str.append(String8::format(", pid %d: ", owner));
    if (extractor.promote() == NULL) {
        str.append("deleted\n");
    } else {
        str.append("active\n");
    }
    for (size_t i = 0; i < tracks.size(); i++) {
        const String8 desc = trackDescriptions.itemAt(i);
        str.appendFormat("    track {%s} ", desc.c_str());
        wp<IMediaSource> wSource = tracks.itemAt(i);
        if (wSource == NULL) {
            str.append(": null\n");
        } else {
            const sp<IMediaSource> source = wSource.promote();
            if (source == NULL) {
                str.append(": deleted\n");
            } else {
                str.appendFormat(": active\n");
            }
        }
    }
    return str;
}

static Vector<ExtractorInstance> sExtractors;
static Mutex sExtractorsLock;

void registerMediaSource(
        const sp<IMediaExtractor> &ex,
        const sp<IMediaSource> &source) {
    Mutex::Autolock lock(sExtractorsLock);
    for (size_t i = 0; i < sExtractors.size(); i++) {
        ExtractorInstance &instance = sExtractors.editItemAt(i);
        sp<IMediaExtractor> extractor = instance.extractor.promote();
        if (extractor != NULL && extractor == ex) {
            if (instance.tracks.size() > 5) {
                instance.tracks.resize(5);
                instance.trackDescriptions.resize(5);
            }
            instance.tracks.push_front(source);
            if (source != NULL) {
                instance.trackDescriptions.push_front(source->getFormat()->toString());
            } else {
                instance.trackDescriptions.push_front(String8());
            }
            break;
        }
    }
}

void registerMediaExtractor(
        const sp<IMediaExtractor> &extractor,
        const sp<DataSource> &source,
        const char *mime) {
    ExtractorInstance ex;
    ex.mime = mime == NULL ? "NULL" : mime;
    ex.name = extractor->name();
    ex.sourceDescription = source->toString();
    ex.owner = IPCThreadState::self()->getCallingPid();
    ex.extractor = extractor;
    ex.when = time(NULL);

    {
        Mutex::Autolock lock(sExtractorsLock);
        if (sExtractors.size() > 10) {
            sExtractors.resize(10);
        }
        sExtractors.push_front(ex);
    }
}

status_t dumpExtractors(int fd, const Vector<String16>&) {
    String8 out;
    const IPCThreadState* ipc = IPCThreadState::self();
    const int pid = ipc->getCallingPid();
    const int uid = ipc->getCallingUid();
    if (!PermissionCache::checkPermission(String16("android.permission.DUMP"), pid, uid)) {
        out.appendFormat("Permission Denial: "
                "can't dump MediaExtractor from pid=%d, uid=%d\n", pid, uid);
    } else {
        out.append("Recent extractors, most recent first:\n");
        {
            Mutex::Autolock lock(sExtractorsLock);
            for (size_t i = 0; i < sExtractors.size(); i++) {
                const ExtractorInstance &instance = sExtractors.itemAt(i);
                out.append("  ");
                out.append(instance.toString());
            }
        }
    }
    write(fd, out.c_str(), out.size());
    return OK;
}


}  // namespace android

