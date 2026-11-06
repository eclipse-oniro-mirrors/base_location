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

#include "bluetoothsearchmanager_fuzzer.h"

#include "app_identity.h"
#include "bluetooth_scan_result.h"
#include "bluetooth_scan_result_callback_napi.h"
#include "bluetooth_search_manager.h"
#include "bluetooth_search_request_params.h"

namespace OHOS {
    using namespace OHOS::Location;
    const int MIN_DATA_LEN = 8;
    const int DEVICE_ID_ARRAY_SIZE = 2;
    const int RANDOM_RANGE = 256;

    void CheckBluetoothSearchStartAndStop(const uint8_t* data)
    {
        int index = 0;
        auto& manager = BluetoothSearchManager::GetInstance();
        AppIdentity identity;
        identity.SetPid(static_cast<pid_t>(data[index++]));
        identity.SetUid(static_cast<pid_t>(data[index++]));
        BluetoothSearchRequestParams params;
        params.rssiThreshold = static_cast<int32_t>(data[index++]) - RANDOM_RANGE;
        for (int i = 0; i < DEVICE_ID_ARRAY_SIZE; i++) {
            params.deviceIdArray.push_back("00:11:22:33:44:55");
        }
        manager.StartBluetoothSearch(nullptr, identity, params);
        auto callback = sptr<BluetoothScanResultCallbackNapi>(
            new (std::nothrow) BluetoothScanResultCallbackNapi());
        if (callback != nullptr) {
            manager.StartBluetoothSearch(callback, identity, params);
            if (callback->AsObject() != nullptr) {
                manager.StopBluetoothSearch(callback->AsObject().GetRefPtr());
            }
        }
        manager.StopBluetoothSearch(nullptr);
    }

    void CheckBluetoothSearchReport(const uint8_t* data, size_t size)
    {
        int index = 0;
        auto& manager = BluetoothSearchManager::GetInstance();
        auto scanResult = std::make_unique<BluetoothScanResult>();
        scanResult->SetDeviceId("00:11:22:33:44:55");
        scanResult->SetDeviceName("deviceName");
        scanResult->SetRssi(static_cast<int64_t>(data[index++]));
        std::vector<uint8_t> scanData(data, data + size);
        scanResult->SetData(scanData);
        scanResult->SetConnectable(data[index++] & 1);
        manager.ReportBluetoothScanResult(scanResult);
        auto nullResult = std::unique_ptr<BluetoothScanResult>(nullptr);
        manager.ReportBluetoothScanResult(nullResult);
    }

    void CheckBluetoothSearchDeathRecipient()
    {
        auto callback = sptr<BluetoothScanResultCallbackNapi>(
            new (std::nothrow) BluetoothScanResultCallbackNapi());
        if (callback == nullptr || callback->AsObject() == nullptr) {
            return;
        }
        BluetoothSearchManager::BluetoothSearchCallbackDeathRecipient recipient;
        wptr<IRemoteObject> watcher(callback->AsObject());
        recipient.OnRemoteDied(watcher);
    }

    bool BluetoothSearchManagerFuzzTest(const uint8_t* data, size_t size)
    {
        if (data == nullptr || size < MIN_DATA_LEN) {
            return false;
        }
        CheckBluetoothSearchStartAndStop(data);
        CheckBluetoothSearchReport(data, size);
        CheckBluetoothSearchDeathRecipient();
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::BluetoothSearchManagerFuzzTest(data, size);
    return 0;
}
