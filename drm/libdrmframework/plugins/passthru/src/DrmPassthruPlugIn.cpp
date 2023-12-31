/*
 * Copyright (C) 2010 The Android Open Source Project
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
#define LOG_TAG "DrmPassthruPlugIn"
#include <utils/Log.h>

#include <android-base/strings.h>
#include <drm/DrmRights.h>
#include <drm/DrmConstraints.h>
#include <drm/DrmMetadata.h>
#include <drm/DrmInfo.h>
#include <drm/DrmInfoEvent.h>
#include <drm/DrmInfoStatus.h>
#include <drm/DrmConvertedStatus.h>
#include <drm/DrmInfoRequest.h>
#include <drm/DrmSupportInfo.h>
#include <DrmPassthruPlugIn.h>

#include <filesystem>

using namespace android;


// This extern "C" is mandatory to be managed by TPlugInManager
extern "C" IDrmEngine* create() {
    return new DrmPassthruPlugIn();
}

// This extern "C" is mandatory to be managed by TPlugInManager
extern "C" void destroy(IDrmEngine* pPlugIn) {
    delete pPlugIn;
    pPlugIn = NULL;
}

DrmPassthruPlugIn::DrmPassthruPlugIn()
    : DrmEngineBase() {

}

DrmPassthruPlugIn::~DrmPassthruPlugIn() {

}

DrmMetadata* DrmPassthruPlugIn::onGetMetadata(int /*uniqueId*/, const String8* /*path*/) {
    return NULL;
}

DrmConstraints* DrmPassthruPlugIn::onGetConstraints(
        int uniqueId, const String8* /*path*/, int /*action*/) {
    ALOGV("DrmPassthruPlugIn::onGetConstraints From Path: %d", uniqueId);
    DrmConstraints* drmConstraints = new DrmConstraints();

    String8 value("dummy_available_time");
    char* charValue = NULL;
    charValue = new char[value.length() + 1];
    strncpy(charValue, value.c_str(), value.length());
    charValue[value.length()] = '\0';

    //Just add dummy available time for verification
    drmConstraints->put(&(DrmConstraints::LICENSE_AVAILABLE_TIME), charValue);
    delete[] charValue;
    return drmConstraints;
}

DrmInfoStatus* DrmPassthruPlugIn::onProcessDrmInfo(int uniqueId, const DrmInfo* drmInfo) {
    ALOGV("DrmPassthruPlugIn::onProcessDrmInfo - Enter : %d", uniqueId);
    DrmInfoStatus* drmInfoStatus = NULL;
    if (NULL != drmInfo) {
        switch (drmInfo->getInfoType()) {
        case DrmInfoRequest::TYPE_REGISTRATION_INFO: {
            const DrmBuffer* emptyBuffer = new DrmBuffer();
            drmInfoStatus = new DrmInfoStatus(DrmInfoStatus::STATUS_OK,
                    DrmInfoRequest::TYPE_REGISTRATION_INFO, emptyBuffer, drmInfo->getMimeType());
            break;
        }
        case DrmInfoRequest::TYPE_UNREGISTRATION_INFO: {
            const DrmBuffer* emptyBuffer = new DrmBuffer();
            drmInfoStatus = new DrmInfoStatus(DrmInfoStatus::STATUS_OK,
                    DrmInfoRequest::TYPE_UNREGISTRATION_INFO, emptyBuffer, drmInfo->getMimeType());
            break;
        }
        case DrmInfoRequest::TYPE_RIGHTS_ACQUISITION_INFO: {
            String8 licenseString("dummy_license_string");
            const int bufferSize = licenseString.size();
            char* data = NULL;
            data = new char[bufferSize];
            memcpy(data, licenseString.c_str(), bufferSize);
            const DrmBuffer* buffer = new DrmBuffer(data, bufferSize);
            drmInfoStatus = new DrmInfoStatus(DrmInfoStatus::STATUS_OK,
                    DrmInfoRequest::TYPE_RIGHTS_ACQUISITION_INFO, buffer, drmInfo->getMimeType());
            break;
        }
        }
    }
    ALOGV("DrmPassthruPlugIn::onProcessDrmInfo - Exit");
    return drmInfoStatus;
}

status_t DrmPassthruPlugIn::onSetOnInfoListener(
            int uniqueId, const IDrmEngine::OnInfoListener* /*infoListener*/) {
    ALOGV("DrmPassthruPlugIn::onSetOnInfoListener : %d", uniqueId);
    return DRM_NO_ERROR;
}

status_t DrmPassthruPlugIn::onInitialize(int uniqueId) {
    ALOGV("DrmPassthruPlugIn::onInitialize : %d", uniqueId);
    return DRM_NO_ERROR;
}

status_t DrmPassthruPlugIn::onTerminate(int uniqueId) {
    ALOGV("DrmPassthruPlugIn::onTerminate : %d", uniqueId);
    return DRM_NO_ERROR;
}

DrmSupportInfo* DrmPassthruPlugIn::onGetSupportInfo(int uniqueId) {
    ALOGV("DrmPassthruPlugIn::onGetSupportInfo : %d", uniqueId);
    DrmSupportInfo* drmSupportInfo = new DrmSupportInfo();
    // Add mimetype's
    drmSupportInfo->addMimeType(String8("application/vnd.passthru.drm"));
    // Add File Suffixes
    drmSupportInfo->addFileSuffix(String8(".passthru"));
    // Add plug-in description
    drmSupportInfo->setDescription(String8("Passthru plug-in"));
    return drmSupportInfo;
}

status_t DrmPassthruPlugIn::onSaveRights(int uniqueId, const DrmRights& /*drmRights*/,
            const String8& /*rightsPath*/, const String8& /*contentPath*/) {
    ALOGV("DrmPassthruPlugIn::onSaveRights : %d", uniqueId);
    return DRM_NO_ERROR;
}

DrmInfo* DrmPassthruPlugIn::onAcquireDrmInfo(int uniqueId, const DrmInfoRequest* drmInfoRequest) {
    ALOGV("DrmPassthruPlugIn::onAcquireDrmInfo : %d", uniqueId);
    DrmInfo* drmInfo = NULL;

    if (NULL != drmInfoRequest) {
        String8 dataString("dummy_acquistion_string");
        int length = dataString.length();
        char* data = NULL;
        data = new char[length];
        memcpy(data, dataString.c_str(), length);
        drmInfo = new DrmInfo(drmInfoRequest->getInfoType(),
            DrmBuffer(data, length), drmInfoRequest->getMimeType());
    }
    return drmInfo;
}

bool DrmPassthruPlugIn::onCanHandle(int /*uniqueId*/, const String8& path) {
    ALOGV("DrmPassthruPlugIn::canHandle: %s ", path.c_str());
    const auto extension = std::filesystem::path(path.c_str()).extension();
    return base::EqualsIgnoreCase(extension.string(), ".passthru");
}

String8 DrmPassthruPlugIn::onGetOriginalMimeType(int uniqueId,
            const String8& /*path*/, int /*fd*/) {
    ALOGV("DrmPassthruPlugIn::onGetOriginalMimeType() : %d", uniqueId);
    return String8("video/passthru");
}

int DrmPassthruPlugIn::onGetDrmObjectType(
            int uniqueId, const String8& /*path*/, const String8& /*mimeType*/) {
    ALOGV("DrmPassthruPlugIn::onGetDrmObjectType() : %d", uniqueId);
    return DrmObjectType::UNKNOWN;
}

int DrmPassthruPlugIn::onCheckRightsStatus(int uniqueId, const String8& /*path*/, int /*action*/) {
    ALOGV("DrmPassthruPlugIn::onCheckRightsStatus() : %d", uniqueId);
    int rightsStatus = RightsStatus::RIGHTS_VALID;
    return rightsStatus;
}

status_t DrmPassthruPlugIn::onConsumeRights(int uniqueId,
            sp<DecryptHandle>& /*decryptHandle*/, int /*action*/, bool /*reserve*/) {
    ALOGV("DrmPassthruPlugIn::onConsumeRights() : %d", uniqueId);
    return DRM_NO_ERROR;
}

status_t DrmPassthruPlugIn::onSetPlaybackStatus(int uniqueId,
            sp<DecryptHandle>& /*decryptHandle*/, int /*playbackStatus*/, int64_t /*position*/) {
    ALOGV("DrmPassthruPlugIn::onSetPlaybackStatus() : %d", uniqueId);
    return DRM_NO_ERROR;
}

bool DrmPassthruPlugIn::onValidateAction(int uniqueId,
            const String8& /*path*/, int /*action*/, const ActionDescription& /*description*/) {
    ALOGV("DrmPassthruPlugIn::onValidateAction() : %d", uniqueId);
    return true;
}

status_t DrmPassthruPlugIn::onRemoveRights(int uniqueId, const String8& /*path*/) {
    ALOGV("DrmPassthruPlugIn::onRemoveRights() : %d", uniqueId);
    return DRM_NO_ERROR;
}

status_t DrmPassthruPlugIn::onRemoveAllRights(int uniqueId) {
    ALOGV("DrmPassthruPlugIn::onRemoveAllRights() : %d", uniqueId);
    return DRM_NO_ERROR;
}

status_t DrmPassthruPlugIn::onOpenConvertSession(int uniqueId, int /*convertId*/) {
    ALOGV("DrmPassthruPlugIn::onOpenConvertSession() : %d", uniqueId);
    return DRM_NO_ERROR;
}

DrmConvertedStatus* DrmPassthruPlugIn::onConvertData(
            int uniqueId, int /*convertId*/, const DrmBuffer* inputData) {
    ALOGV("DrmPassthruPlugIn::onConvertData() : %d", uniqueId);
    DrmBuffer* convertedData = NULL;

    if (NULL != inputData && 0 < inputData->length) {
        int length = inputData->length;
        char* data = NULL;
        data = new char[length];
        convertedData = new DrmBuffer(data, length);
        memcpy(convertedData->data, inputData->data, length);
    }
    return new DrmConvertedStatus(DrmConvertedStatus::STATUS_OK, convertedData, 0 /*offset*/);
}

DrmConvertedStatus* DrmPassthruPlugIn::onCloseConvertSession(int uniqueId, int /*convertId*/) {
    ALOGV("DrmPassthruPlugIn::onCloseConvertSession() : %d", uniqueId);
    return new DrmConvertedStatus(DrmConvertedStatus::STATUS_OK, NULL, 0 /*offset*/);
}

status_t DrmPassthruPlugIn::onOpenDecryptSession(
            int uniqueId, sp<DecryptHandle>& decryptHandle, int /*fd*/, off64_t /*offset*/,
            off64_t /*length*/) {
    ALOGV("DrmPassthruPlugIn::onOpenDecryptSession() : %d", uniqueId);

#ifdef ENABLE_PASSTHRU_DECRYPTION
    decryptHandle->mimeType = String8("video/passthru");
    decryptHandle->decryptApiType = DecryptApiType::ELEMENTARY_STREAM_BASED;
    decryptHandle->status = DRM_NO_ERROR;
    decryptHandle->decryptInfo = NULL;
    return DRM_NO_ERROR;
#else
    (void)(decryptHandle.get()); // unused
#endif

    return DRM_ERROR_CANNOT_HANDLE;
}

status_t DrmPassthruPlugIn::onOpenDecryptSession(
            int /*uniqueId*/, sp<DecryptHandle>& /*decryptHandle*/, const char* /*uri*/) {
    return DRM_ERROR_CANNOT_HANDLE;
}

status_t DrmPassthruPlugIn::onCloseDecryptSession(int uniqueId, sp<DecryptHandle>& decryptHandle) {
    ALOGV("DrmPassthruPlugIn::onCloseDecryptSession() : %d", uniqueId);
    if (NULL != decryptHandle.get()) {
        if (NULL != decryptHandle->decryptInfo) {
            delete decryptHandle->decryptInfo; decryptHandle->decryptInfo = NULL;
        }
        decryptHandle.clear();
    }
    return DRM_NO_ERROR;
}

status_t DrmPassthruPlugIn::onInitializeDecryptUnit(int uniqueId,
        sp<DecryptHandle>& /*decryptHandle*/,
        int /*decryptUnitId*/, const DrmBuffer* /*headerInfo*/) {
    ALOGV("DrmPassthruPlugIn::onInitializeDecryptUnit() : %d", uniqueId);
    return DRM_NO_ERROR;
}

status_t DrmPassthruPlugIn::onDecrypt(int uniqueId, sp<DecryptHandle>& /*decryptHandle*/,
        int /*decryptUnitId*/, const DrmBuffer* encBuffer, DrmBuffer** decBuffer,
        DrmBuffer* /*IV*/) {
    ALOGV("DrmPassthruPlugIn::onDecrypt() : %d", uniqueId);
    /**
     * As a workaround implementation passthru would copy the given
     * encrypted buffer as it is to decrypted buffer. Note, decBuffer
     * memory has to be allocated by the caller.
     */
    if (NULL != (*decBuffer) && 0 < (*decBuffer)->length) {
        if ((*decBuffer)->length >= encBuffer->length) {
            memcpy((*decBuffer)->data, encBuffer->data, encBuffer->length);
            (*decBuffer)->length = encBuffer->length;
        } else {
            ALOGE("decBuffer size (%d) too small to hold %d bytes",
                (*decBuffer)->length, encBuffer->length);
            return DRM_ERROR_UNKNOWN;
        }
    }
    return DRM_NO_ERROR;
}

status_t DrmPassthruPlugIn::onFinalizeDecryptUnit(
            int uniqueId, sp<DecryptHandle>& /*decryptHandle*/, int /*decryptUnitId*/) {
    ALOGV("DrmPassthruPlugIn::onFinalizeDecryptUnit() : %d", uniqueId);
    return DRM_NO_ERROR;
}

ssize_t DrmPassthruPlugIn::onPread(int uniqueId, sp<DecryptHandle>& /*decryptHandle*/,
            void* /*buffer*/, ssize_t /*numBytes*/, off64_t /*offset*/) {
    ALOGV("DrmPassthruPlugIn::onPread() : %d", uniqueId);
    return 0;
}

