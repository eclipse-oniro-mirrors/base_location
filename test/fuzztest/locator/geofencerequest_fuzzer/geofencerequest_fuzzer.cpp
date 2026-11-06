/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "geofencerequest_fuzzer.h"

#include "parcel.h"
#include "geofence.h"
#include "geofence_request.h"
#include "nlohmann/json.hpp"

namespace OHOS {
    using namespace OHOS::Location;
    const int MIN_DATA_LEN = 8;
    const int TRANSITION_EVENT_COUNT = 32;
    const int COORDINATE_TYPE_COUNT = 3;
    const int LIST_COUNT_VARIANTS = 3;
    const int TRANSITION_LIST_EXCEED_COUNT = 7;
    const int NOTIFY_LIST_EXCEED_COUNT = 7;

    uint8_t FuzzByte(const uint8_t* data, size_t size, int& index)
    {
        if (data == nullptr || size == 0) {
            return 0;
        }
        if (index < 0 || index >= static_cast<int>(size)) {
            index = 0;
        }
        return data[index++];
    }

    GeoFence BuildGeoFence(const uint8_t* data, size_t size, int& index)
    {
        GeoFence geofence = {0.0, 0.0, 0.0, 0.0, WGS84};
        geofence.latitude = static_cast<double>(FuzzByte(data, size, index));
        geofence.longitude = static_cast<double>(FuzzByte(data, size, index));
        geofence.radius = static_cast<double>(FuzzByte(data, size, index));
        geofence.expiration = static_cast<double>(FuzzByte(data, size, index));
        geofence.coordinateSystemType = static_cast<CoordinateSystemType>(
            FuzzByte(data, size, index) % COORDINATE_TYPE_COUNT);
        return geofence;
    }

    void CheckGeofenceParcel(const uint8_t* data, size_t size, int& index)
    {
        Geofence geofence;
        geofence.SetLatitude(static_cast<double>(FuzzByte(data, size, index)));
        geofence.SetLongitude(static_cast<double>(FuzzByte(data, size, index)));
        geofence.SetRadius(static_cast<double>(FuzzByte(data, size, index)));
        geofence.SetExpiration(static_cast<double>(FuzzByte(data, size, index)));
        geofence.SetCoordinateSystemType(static_cast<CoordinateSystemType>(
            FuzzByte(data, size, index) % COORDINATE_TYPE_COUNT));
        Parcel fenceParcel;
        geofence.Marshalling(fenceParcel);
        Geofence* fenceResult = Geofence::Unmarshalling(fenceParcel);
        if (fenceResult != nullptr) {
            delete fenceResult;
            fenceResult = nullptr;
        }
    }

    void CheckGeofenceRequestParcel(const uint8_t* data, size_t size, int& index, int listCount)
    {
        auto request = std::make_unique<GeofenceRequest>();
        request->SetGeofence(BuildGeoFence(data, size, index));
        request->SetScenario(static_cast<int>(FuzzByte(data, size, index)));
        request->SetLoiterTimeMs(static_cast<int>(FuzzByte(data, size, index)));
        std::vector<GeofenceTransitionEvent> eventList;
        for (int i = 0; i < listCount; i++) {
            eventList.push_back(static_cast<GeofenceTransitionEvent>(
                FuzzByte(data, size, index) % TRANSITION_EVENT_COUNT + 1));
        }
        request->SetGeofenceTransitionEventList(eventList);
        std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>> notifyList;
        for (int i = 0; i < listCount; i++) {
            notifyList.push_back(nullptr);
        }
        request->SetNotificationRequestList(notifyList);
        request->SetFenceId(static_cast<int>(FuzzByte(data, size, index)));
        request->SetBundleName("geofenceBundle");
        request->SetFenceExtensionAbilityName("FenceExtensionAbility");
        request->SetUid(static_cast<int32_t>(FuzzByte(data, size, index)));
        request->SetPid(static_cast<int32_t>(FuzzByte(data, size, index)));
        request->SetTokenId(static_cast<uint32_t>(FuzzByte(data, size, index)));
        request->SetTokenIdEx(static_cast<uint32_t>(FuzzByte(data, size, index)));
        request->SetFirstTokenId(static_cast<uint32_t>(FuzzByte(data, size, index)));
        request->SetAppAliveStatus(FuzzByte(data, size, index) & 1);
        request->SetRequestExpirationTimeStamp(static_cast<int64_t>(FuzzByte(data, size, index)));
        request->IsTokenIdValid();
        Parcel requestParcel;
        request->Marshalling(requestParcel);
        GeofenceRequest* requestResult = GeofenceRequest::Unmarshalling(requestParcel);
        if (requestResult != nullptr) {
            delete requestResult;
            requestResult = nullptr;
        }
    }

    void CheckGeofenceRequestExceedList(const uint8_t* data, size_t size, int& index)
    {
        auto request = std::make_unique<GeofenceRequest>();
        request->SetGeofence(BuildGeoFence(data, size, index));
        std::vector<GeofenceTransitionEvent> eventList;
        for (int i = 0; i < TRANSITION_LIST_EXCEED_COUNT; i++) {
            eventList.push_back(GEOFENCE_TRANSITION_EVENT_ENTER);
        }
        request->SetGeofenceTransitionEventList(eventList);
        std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>> notifyList;
        for (int i = 0; i < NOTIFY_LIST_EXCEED_COUNT; i++) {
            notifyList.push_back(nullptr);
        }
        request->SetNotificationRequestList(notifyList);
        nlohmann::json jsonObject;
        request->ToJson(jsonObject);
        Parcel requestParcel;
        request->Marshalling(requestParcel);
        GeofenceRequest* requestResult = GeofenceRequest::Unmarshalling(requestParcel);
        if (requestResult != nullptr) {
            delete requestResult;
            requestResult = nullptr;
        }
    }

    void CheckGeofenceRequestJson(const uint8_t* data)
    {
        int index = 0;
        auto request = std::make_unique<GeofenceRequest>();
        request->SetBundleName("jsonBundle");
        nlohmann::json jsonObject;
        request->ToJson(jsonObject);
        int scenarioValue = static_cast<int>(data[index++]);
        int loiterTimeValue = static_cast<int>(data[index++]);
        int fenceIdValue = static_cast<int>(data[index++]);
        int uidValue = static_cast<int>(data[index++]);
        int pidValue = static_cast<int>(data[index++]);
        uint32_t tokenIdValue = static_cast<uint32_t>(data[index++]);
        uint32_t tokenIdExValue = static_cast<uint32_t>(data[index++]);
        uint32_t firstTokenIdValue = static_cast<uint32_t>(data[index++]);
        nlohmann::json fullObject = {
            {"scenario", scenarioValue},
            {"loiterTimeMs", loiterTimeValue},
            {"fenceId", fenceIdValue},
            {"uid", uidValue},
            {"pid", pidValue},
            {"tokenId", tokenIdValue},
            {"tokenIdEx", tokenIdExValue},
            {"firstTokenId", firstTokenIdValue},
            {"bundleName", "jsonBundle"},
            {"fenceExtensionAbilityName", "FenceExtensionAbility"},
            {"appAliveStatus", true},
            {"requestExpirationTimeStamp", static_cast<int64_t>(scenarioValue)},
            {"geofence", {
                {"latitude", static_cast<double>(loiterTimeValue)},
                {"longitude", static_cast<double>(fenceIdValue)},
                {"radius", static_cast<double>(uidValue)},
                {"expiration", static_cast<double>(pidValue)},
                {"coordinateSystemType", static_cast<int>(tokenIdValue)}
            }},
            {"transitionStatusList", nlohmann::json::array({fenceIdValue})},
            {"wantAgent", nlohmann::json::object()},
            {"notificationRequestList", nlohmann::json::array({
                {{"notificationId", uidValue}}
            })}
        };
        std::shared_ptr<GeofenceRequest> fromJsonResult = GeofenceRequest::FromJson(fullObject);
        if (fromJsonResult != nullptr) {
            fromJsonResult->GetBundleName();
            fromJsonResult->GetFenceId();
        }
        std::shared_ptr<GeofenceRequest> fromEmptyResult = GeofenceRequest::FromJson(jsonObject);
        if (fromEmptyResult != nullptr) {
            fromEmptyResult->GetScenario();
        }
    }

    void CheckGeofenceRequestJsonWrongType()
    {
        nlohmann::json wrongTypeObject = {
            {"scenario", "text"},
            {"loiterTimeMs", "text"},
            {"fenceId", "text"},
            {"uid", "text"},
            {"pid", "text"},
            {"tokenId", "text"},
            {"geofence", {
                {"latitude", "text"},
                {"longitude", "text"},
                {"radius", "text"},
                {"expiration", "text"},
                {"coordinateSystemType", "text"}
            }},
            {"transitionStatusList", nlohmann::json::array({"text"})}
        };
        std::shared_ptr<GeofenceRequest> fromWrongResult = GeofenceRequest::FromJson(wrongTypeObject);
        if (fromWrongResult != nullptr) {
            fromWrongResult->GetScenario();
        }
    }

    void CheckGeofenceRequestRawParcel(const uint8_t* data, size_t size)
    {
        Parcel rawParcel;
        rawParcel.WriteBuffer(data, size);
        GeofenceRequest rawRequest;
        rawRequest.ReadFromParcel(rawParcel);
        rawRequest.ReadAppInfo(rawParcel);
    }

    bool GeofenceRequestFuzzTest(const uint8_t* data, size_t size)
    {
        if (data == nullptr || size < MIN_DATA_LEN) {
            return false;
        }
        int index = 0;
        CheckGeofenceParcel(data, size, index);
        int listCount = FuzzByte(data, size, index) % LIST_COUNT_VARIANTS;
        CheckGeofenceRequestParcel(data, size, index, listCount);
        CheckGeofenceRequestExceedList(data, size, index);
        CheckGeofenceRequestJson(data);
        CheckGeofenceRequestJsonWrongType();
        CheckGeofenceRequestRawParcel(data, size);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::GeofenceRequestFuzzTest(data, size);
    return 0;
}
