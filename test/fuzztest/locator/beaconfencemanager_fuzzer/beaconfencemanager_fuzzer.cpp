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

#include "beaconfencemanager_fuzzer.h"

#include "app_identity.h"
#include "beacon_fence.h"
#include "beacon_fence_manager.h"
#include "beacon_fence_request.h"
#include "location_gnss_geofence_callback_napi.h"

namespace OHOS {
    using namespace OHOS::Location;
    const int MIN_DATA_LEN = 8;
    const int IBEACON_PAYLOAD_LEN = 18;
    const int SAME_BUNDLE_ADD_LIMIT = 10;

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

    std::shared_ptr<BeaconFence> BuildBeaconFence(const uint8_t* data, size_t size, int& index)
    {
        auto beaconFence = std::make_shared<BeaconFence>();
        beaconFence->SetIdentifier("beaconFenceIdentifier");
        beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
        BeaconManufactureData manufactureData;
        manufactureData.manufactureId = static_cast<int32_t>(FuzzByte(data, size, index));
        for (int i = 0; i < IBEACON_PAYLOAD_LEN; i++) {
            manufactureData.manufactureData.push_back(FuzzByte(data, size, index));
            manufactureData.manufactureDataMask.push_back(FuzzByte(data, size, index) & 1);
        }
        beaconFence->SetBeaconManufactureData(manufactureData);
        return beaconFence;
    }

    std::shared_ptr<BeaconFenceRequest> BuildBeaconFenceRequest(const uint8_t* data, size_t size, int& index)
    {
        auto beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
        std::shared_ptr<BeaconFence> beaconFence = BuildBeaconFence(data, size, index);
        beaconFenceRequest->SetBeaconFence(beaconFence);
        beaconFenceRequest->SetFenceExtensionAbilityName("FenceExtensionAbility");
        beaconFenceRequest->SetBundleName("beaconBundle");
        beaconFenceRequest->SetFenceId(std::to_string(static_cast<int>(FuzzByte(data, size, index))));
        auto callback = sptr<LocationGnssGeofenceCallbackNapi>(
            new (std::nothrow) LocationGnssGeofenceCallbackNapi());
        if (callback != nullptr && callback->AsObject() != nullptr) {
            beaconFenceRequest->SetBeaconFenceTransitionCallback(callback->AsObject());
        }
        return beaconFenceRequest;
    }

    void CheckDuplicateAndLimitAdd(BeaconFenceManager* manager, const uint8_t* data, size_t size, int& index)
    {
        AppIdentity identity;
        identity.SetBundleName("beaconBundle");
        std::shared_ptr<BeaconFenceRequest> nullRequest = nullptr;
        manager->AddBeaconFence(nullRequest, identity);
        auto beaconFenceRequest = BuildBeaconFenceRequest(data, size, index);
        manager->AddBeaconFence(beaconFenceRequest, identity);
        manager->AddBeaconFence(beaconFenceRequest, identity);
        for (int i = 0; i < SAME_BUNDLE_ADD_LIMIT; i++) {
            auto distinctRequest = BuildBeaconFenceRequest(data, size, index);
            manager->AddBeaconFence(distinctRequest, identity);
        }
    }

    void CheckRemoveByFenceAndBundle(BeaconFenceManager* manager, const uint8_t* data, size_t size, int& index)
    {
        AppIdentity identity;
        identity.SetBundleName("beaconBundle");
        auto beaconFenceRequest = BuildBeaconFenceRequest(data, size, index);
        manager->AddBeaconFence(beaconFenceRequest, identity);
        std::shared_ptr<BeaconFence> addedFence = beaconFenceRequest->GetBeaconFence();
        manager->RemoveBeaconFence(addedFence, beaconFenceRequest->GetBundleName());
        manager->RemoveBeaconFence(addedFence, "otherBundle");
        manager->AddBeaconFence(beaconFenceRequest, identity);
    }

    void CheckRemoveByCallbackAndPackage(BeaconFenceManager* manager, const uint8_t* data, size_t size, int& index)
    {
        AppIdentity identity;
        identity.SetBundleName("beaconBundle");
        auto beaconFenceRequest = BuildBeaconFenceRequest(data, size, index);
        manager->AddBeaconFence(beaconFenceRequest, identity);
        auto callback = sptr<LocationGnssGeofenceCallbackNapi>(
            new (std::nothrow) LocationGnssGeofenceCallbackNapi());
        if (callback != nullptr && callback->AsObject() != nullptr) {
            manager->RemoveBeaconFenceRequestByCallback(callback->AsObject());
            sptr<IRemoteObject> registered = beaconFenceRequest->GetBeaconFenceTransitionCallback();
            manager->RemoveBeaconFenceRequestByCallback(registered);
        }
        std::string packageName = "beaconBundle";
        manager->RemoveBeaconFenceByPackageName(packageName);
        std::string missingPackage = "missingBundle";
        manager->RemoveBeaconFenceByPackageName(missingPackage);
    }

#ifdef BLUETOOTH_ENABLE
    void CheckBeaconBleCallback(const uint8_t* data)
    {
        int index = 0;
        BeaconBleCallbackWapper wapper;
        Bluetooth::BleScanResult scanResult;
        wapper.OnScanCallback(scanResult);
        wapper.OnFoundOrLostCallback(scanResult, data[index++]);
        std::vector<Bluetooth::BleScanResult> batchResults;
        batchResults.push_back(scanResult);
        wapper.OnBleBatchScanResultsEvent(batchResults);
        int32_t resultCode = static_cast<int32_t>(data[index++]);
        bool isStartScan = (data[index++] & 1) != 0;
        wapper.OnStartOrStopScanEvent(resultCode, isStartScan);
        BeaconFenceManager::GetInstance()->ReportFoundOrLost(scanResult, data[index++]);
    }

    void CheckBeaconFenceDeathRecipient()
    {
        auto callback = sptr<LocationGnssGeofenceCallbackNapi>(
            new (std::nothrow) LocationGnssGeofenceCallbackNapi());
        if (callback == nullptr || callback->AsObject() == nullptr) {
            return;
        }
        BeaconFenceCallbackDeathRecipient recipient;
        wptr<IRemoteObject> watcher(callback->AsObject());
        recipient.OnRemoteDied(watcher);
    }
#endif

    bool BeaconFenceManagerFuzzTest(const uint8_t* data, size_t size)
    {
        if (data == nullptr || size < MIN_DATA_LEN) {
            return false;
        }
        BeaconFenceManager* manager = BeaconFenceManager::GetInstance();
        if (manager == nullptr) {
            return false;
        }
        int index = 0;
        CheckDuplicateAndLimitAdd(manager, data, size, index);
        CheckRemoveByFenceAndBundle(manager, data, size, index);
        CheckRemoveByCallbackAndPackage(manager, data, size, index);
#ifdef BLUETOOTH_ENABLE
        CheckBeaconBleCallback(data);
        CheckBeaconFenceDeathRecipient();
#endif
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::BeaconFenceManagerFuzzTest(data, size);
    return 0;
}
