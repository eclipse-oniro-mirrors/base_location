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

#include "fusionfencerequest_fuzzer.h"

#include "parcel.h"
#include "fusion_fence_request.h"
#include "geofence.h"

namespace OHOS {
    using namespace OHOS::Location;
    const int MIN_DATA_LEN = 8;
    const int SCENE_TYPE_COUNT = 4;
    const int WIFI_FINGERPRINT_TYPE_COUNT = 2;
    const int GNSS_FENCE_TYPE_COUNT = 2;
    const int FENCE_TYPE_COUNT = 15;
    const int TRANSITION_EVENT_COUNT = 32;
    const int LIST_COUNT_VARIANTS = 3;
    const int ADDITIONS_EXCEED_COUNT = 101;
    const int MAC_EXCEED_COUNT = 151;
    const int WIFI_EXCEED_COUNT = 151;
    const int CELL_EXCEED_COUNT = 1001;

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

    FusionFencePoint BuildFusionFencePoint(const uint8_t* data, size_t size, int& index)
    {
        FusionFencePoint point;
        point.latitude = static_cast<double>(FuzzByte(data, size, index));
        point.longitude = static_cast<double>(FuzzByte(data, size, index));
        return point;
    }

    std::shared_ptr<FusionFenceCellInfo> BuildCellInfo(const uint8_t* data, size_t size, int& index,
        int additions)
    {
        auto cellInfo = std::make_shared<FusionFenceCellInfo>();
        cellInfo->timeSinceBoot = static_cast<int64_t>(FuzzByte(data, size, index));
        cellInfo->cellId = static_cast<int64_t>(FuzzByte(data, size, index));
        cellInfo->lac = static_cast<int32_t>(FuzzByte(data, size, index));
        cellInfo->mcc = static_cast<int32_t>(FuzzByte(data, size, index));
        cellInfo->mnc = static_cast<int32_t>(FuzzByte(data, size, index));
        cellInfo->rat = static_cast<int32_t>(FuzzByte(data, size, index));
        cellInfo->signalIntensity = static_cast<int32_t>(FuzzByte(data, size, index));
        cellInfo->arfcn = static_cast<int32_t>(FuzzByte(data, size, index));
        cellInfo->pci = static_cast<int32_t>(FuzzByte(data, size, index));
        cellInfo->tac = static_cast<int32_t>(FuzzByte(data, size, index));
        for (int i = 0; i < additions; i++) {
            cellInfo->additionsMap["key" + std::to_string(i)] = "value";
        }
        return cellInfo;
    }

    std::shared_ptr<FusionFenceWifi> BuildWifiFence(const uint8_t* data, size_t size, int& index,
        int featureCount, int macCount)
    {
        auto wifiFence = std::make_shared<FusionFenceWifi>();
        wifiFence->type = static_cast<WifiFingerprintType>(
            FuzzByte(data, size, index) % WIFI_FINGERPRINT_TYPE_COUNT + 1);
        for (int i = 0; i < featureCount; i++) {
            WirelessSignalFeature feature;
            feature.rssiAvg = static_cast<int32_t>(FuzzByte(data, size, index));
            feature.rssiStandardDeviation = static_cast<double>(FuzzByte(data, size, index));
            for (int j = 0; j < macCount; j++) {
                feature.mac.push_back("00:11:22:33:44:55");
            }
            wifiFence->wifiFeatures.push_back(feature);
        }
        return wifiFence;
    }

    std::shared_ptr<FusionFenceGnss> BuildGnssFence(const uint8_t* data, size_t size, int& index,
        bool withCircular, int pointCount)
    {
        auto gnssFence = std::make_shared<FusionFenceGnss>();
        gnssFence->gnssFenceType = static_cast<GnssFenceType>(
            FuzzByte(data, size, index) % GNSS_FENCE_TYPE_COUNT + 1);
        if (withCircular) {
            auto circularFence = std::make_shared<Geofence>();
            circularFence->SetLatitude(static_cast<double>(FuzzByte(data, size, index)));
            circularFence->SetLongitude(static_cast<double>(FuzzByte(data, size, index)));
            gnssFence->circularFence = circularFence;
        }
        for (int i = 0; i < pointCount; i++) {
            gnssFence->polygon.push_back(BuildFusionFencePoint(data, size, index));
        }
        return gnssFence;
    }

    void CheckFusionFenceTransition(const uint8_t* data, size_t size, int& index)
    {
        FusionFenceTransition transition;
        transition.identifier = "transition";
        transition.scene = static_cast<FusionFenceScene>(
            FuzzByte(data, size, index) % SCENE_TYPE_COUNT + 1);
        transition.transitionEvent = static_cast<GeofenceTransitionEvent>(
            FuzzByte(data, size, index) % TRANSITION_EVENT_COUNT + 1);
        Parcel transitionParcel;
        transition.Marshalling(transitionParcel);
        FusionFenceTransition* transitionResult = FusionFenceTransition::Unmarshalling(transitionParcel);
        if (transitionResult != nullptr) {
            delete transitionResult;
            transitionResult = nullptr;
        }
    }

    void CheckFenceRequestRoundTrip(const uint8_t* data, size_t size, int& index, int listCount,
        bool withCircular)
    {
        auto request = std::make_unique<FusionFenceRequest>();
        request->SetIdentifier("fusionFence");
        request->SetScene(static_cast<FusionFenceScene>(
            FuzzByte(data, size, index) % SCENE_TYPE_COUNT + 1));
        request->SetFenceType(static_cast<int32_t>(FuzzByte(data, size, index) % FENCE_TYPE_COUNT));
        request->SetPoiType("poiType");
        request->SetPoiLocation(BuildFusionFencePoint(data, size, index));
        request->SetMonitorTransitionEvents(static_cast<int32_t>(FuzzByte(data, size, index)));
        request->SetLoiterTimeMs(static_cast<int64_t>(FuzzByte(data, size, index)));
        std::vector<std::shared_ptr<FusionFenceGnss>> gnssFences;
        for (int i = 0; i < listCount; i++) {
            gnssFences.push_back(BuildGnssFence(data, size, index, withCircular, listCount));
        }
        request->SetGnssFences(gnssFences);
        std::vector<std::shared_ptr<FusionFenceCell>> cellFences;
        for (int i = 0; i < listCount; i++) {
            auto cellFence = std::make_shared<FusionFenceCell>();
            cellFence->cellInfos.push_back(*BuildCellInfo(data, size, index, listCount));
            cellFences.push_back(cellFence);
        }
        request->SetCellFences(cellFences);
        std::vector<std::shared_ptr<FusionFenceWifi>> wifiFences;
        for (int i = 0; i < listCount; i++) {
            wifiFences.push_back(BuildWifiFence(data, size, index, listCount, listCount));
        }
        request->SetWifiFences(wifiFences);
        request->SetExpirationMs(static_cast<int64_t>(FuzzByte(data, size, index)));
        request->SetBundleName("fusionBundle");
        request->SetUid(static_cast<int32_t>(FuzzByte(data, size, index)));
        request->SetPid(static_cast<int32_t>(FuzzByte(data, size, index)));
        request->SetTokenId(static_cast<int64_t>(FuzzByte(data, size, index)));
        request->SetTokenIdEx(static_cast<int64_t>(FuzzByte(data, size, index)));
        request->SetFirstTokenId(static_cast<int64_t>(FuzzByte(data, size, index)));
        Parcel requestParcel;
        request->Marshalling(requestParcel);
        FusionFenceRequest* requestResult = FusionFenceRequest::Unmarshalling(requestParcel);
        if (requestResult != nullptr) {
            delete requestResult;
            requestResult = nullptr;
        }
    }

    void CheckExceedListRoundTrip()
    {
        auto cellFence = std::make_shared<FusionFenceCell>();
        FusionFenceCellInfo exceedCell;
        for (int i = 0; i < ADDITIONS_EXCEED_COUNT; i++) {
            exceedCell.additionsMap["key" + std::to_string(i)] = "value";
        }
        cellFence->cellInfos.push_back(exceedCell);
        for (int i = 0; i < CELL_EXCEED_COUNT; i++) {
            cellFence->cellInfos.push_back(FusionFenceCellInfo());
        }
        Parcel cellParcel;
        cellFence->Marshalling(cellParcel);
        FusionFenceCell cellResult;
        cellResult.ReadFromParcel(cellParcel);

        auto wifiFence = std::make_shared<FusionFenceWifi>();
        for (int i = 0; i < WIFI_EXCEED_COUNT; i++) {
            WirelessSignalFeature feature;
            for (int j = 0; j < MAC_EXCEED_COUNT; j++) {
                feature.mac.push_back("00:11:22:33:44:55");
            }
            wifiFence->wifiFeatures.push_back(feature);
        }
        Parcel wifiParcel;
        wifiFence->Marshalling(wifiParcel);
        FusionFenceWifi wifiResult;
        wifiResult.ReadFromParcel(wifiParcel);
    }

    void CheckFenceRequestRawParcel(const uint8_t* data, size_t size)
    {
        Parcel rawParcel;
        rawParcel.WriteBuffer(data, size);
        FusionFenceRequest rawRequest;
        rawRequest.ReadFromParcel(rawParcel);
        FusionFenceTransition rawTransition;
        rawTransition.ReadFromParcel(rawParcel);
    }

    bool FusionFenceRequestFuzzTest(const uint8_t* data, size_t size)
    {
        if (data == nullptr || size < MIN_DATA_LEN) {
            return false;
        }
        int index = 0;
        CheckFusionFenceTransition(data, size, index);
        int listCount = FuzzByte(data, size, index) % LIST_COUNT_VARIANTS;
        bool withCircular = (FuzzByte(data, size, index) & 1) != 0;
        CheckFenceRequestRoundTrip(data, size, index, listCount, withCircular);
        CheckFenceRequestRoundTrip(data, size, index, LIST_COUNT_VARIANTS - listCount, !withCircular);
        CheckExceedListRoundTrip();
        CheckFenceRequestRawParcel(data, size);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::FusionFenceRequestFuzzTest(data, size);
    return 0;
}
