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

#include "locatorrequireddatamanager_fuzzer.h"

#include "bluetooth_scan_result.h"
#include "bluetooth_scan_result_callback_napi.h"
#include "constant_definition.h"
#include "locating_required_data.h"
#include "locating_required_data_callback_napi.h"
#include "locating_required_data_config.h"
#include "locator_required_data_manager.h"

namespace OHOS {
    using namespace OHOS::Location;
    const int MIN_DATA_LEN = 8;
    const int DATA_TYPE_COUNT = 3;
    const int SCAN_INTERVAL_MS = 100;
    const int SCAN_TIMEOUT_MS = 100;

    std::shared_ptr<LocatingRequiredDataConfig> BuildDataConfig(const uint8_t* data, int& index)
    {
        auto dataConfig = std::make_shared<LocatingRequiredDataConfig>();
        dataConfig->SetType(static_cast<int>(data[index++] % DATA_TYPE_COUNT + 1));
        dataConfig->SetNeedStartScan(data[index++] & 1);
        dataConfig->SetIsWlanMatchCalled(data[index++] & 1);
        dataConfig->SetScanIntervalMs(SCAN_INTERVAL_MS);
        dataConfig->SetScanTimeoutMs(SCAN_TIMEOUT_MS);
        return dataConfig;
    }

    void CheckRegisterTypeBranches(LocatorRequiredDataManager* manager, const uint8_t* data)
    {
        int index = 0;
        AppIdentity identity;
        identity.SetPid(static_cast<pid_t>(data[index++]));
        identity.SetUid(static_cast<pid_t>(data[index++]));
        identity.SetBundleName("locatorBundle");
        auto callback = sptr<LocatingRequiredDataCallbackNapi>(
            new (std::nothrow) LocatingRequiredDataCallbackNapi());
        std::shared_ptr<LocatingRequiredDataConfig> nullConfig = nullptr;
        manager->RegisterCallback(identity, nullConfig, nullptr);
        if (callback == nullptr || callback->AsObject() == nullptr) {
            return;
        }
        auto wifiConfig = std::make_shared<LocatingRequiredDataConfig>();
        wifiConfig->SetType(LocatingRequiredDataType::WIFI);
        wifiConfig->SetNeedStartScan(data[index++] & 1);
        wifiConfig->SetIsWlanMatchCalled(data[index++] & 1);
        wifiConfig->SetScanIntervalMs(SCAN_INTERVAL_MS);
        wifiConfig->SetScanTimeoutMs(SCAN_TIMEOUT_MS);
        manager->RegisterCallback(identity, wifiConfig, callback->AsObject());
        auto bluetoothConfig = std::make_shared<LocatingRequiredDataConfig>();
        bluetoothConfig->SetType(LocatingRequiredDataType::BLUE_TOOTH);
        manager->RegisterCallback(identity, bluetoothConfig, callback->AsObject());
    }

    void CheckReportWithRegisteredCallbacks(LocatorRequiredDataManager* manager, const uint8_t* data)
    {
        int index = 0;
        AppIdentity identity;
        identity.SetBundleName("locatorBundle");
        auto dataConfig = BuildDataConfig(data, index);
        auto callback = sptr<LocatingRequiredDataCallbackNapi>(
            new (std::nothrow) LocatingRequiredDataCallbackNapi());
        if (callback == nullptr || callback->AsObject() == nullptr) {
            return;
        }
        manager->AddScanCallback(identity, dataConfig, callback->AsObject());
        auto secondCallback = sptr<LocatingRequiredDataCallbackNapi>(
            new (std::nothrow) LocatingRequiredDataCallbackNapi());
        if (secondCallback != nullptr && secondCallback->AsObject() != nullptr) {
            manager->AddScanCallback(identity, dataConfig, secondCallback->AsObject());
        }
        std::vector<std::shared_ptr<LocatingRequiredData>> requiredDataList;
        auto requiredData = std::make_shared<LocatingRequiredData>();
        requiredData->SetType(dataConfig->GetType());
        requiredDataList.push_back(requiredData);
        manager->ReportData(requiredDataList, dataConfig->GetType());
        manager->ReportData(requiredDataList, data[index++ % DATA_TYPE_COUNT + 1]);
        manager->GetCallbackConfig(callback->AsObject());
        manager->GetCallbackConfig(nullptr);
        manager->UnregisterCallback(callback->AsObject());
        manager->UnregisterCallback(callback->AsObject());
    }

    void CheckWifiAndStillState(LocatorRequiredDataManager* manager, const uint8_t* data)
    {
        int index = 0;
        int fixNumber = static_cast<int>(data[index++]);
        bool isRepeat = (data[index++] & 1) != 0;
        manager->StartWifiScan(fixNumber, isRepeat);
        manager->TriggerWifiScan();
        manager->IsWifiConnecting();
        manager->IsBluetoothConnecting();
        std::string bssid;
        manager->GetCurrentWifiBssidForLocating(bssid);
        manager->SyncStillMovementState(data[index++] & 1);
        manager->IsStill();
        manager->GetlastStillTime();
        manager->UpdateWifiScanCompleteTimestamp();
        manager->GetWifiScanCompleteTimestamp();
        manager->SendWifiScanEvent();
        int timeoutMs = static_cast<int>(data[index++]);
        bool needRetryScan = (data[index++] & 1) != 0;
        manager->SendGetWifiListEvent(timeoutMs, needRetryScan);
        manager->RemoveGetWifiListEvent();
    }

    void CheckBluetoothScanFlow(LocatorRequiredDataManager* manager, const uint8_t* data)
    {
        int index = 0;
        manager->SetBluetoothScanStatus(data[index++] & 1);
        manager->GetBluetoothScanStatus();
        manager->StartBluetoothScan();
        manager->SendStartBluetoothScanEvent();
        manager->HandleRefreshBluetoothRequest();
        auto callback = sptr<BluetoothScanResultCallbackNapi>(
            new (std::nothrow) BluetoothScanResultCallbackNapi());
        AppIdentity identity;
        identity.SetPid(static_cast<pid_t>(data[index++]));
        if (callback != nullptr) {
            manager->StartScanBluetoothDevice(callback, identity);
            if (callback->AsObject() != nullptr) {
                manager->StopScanBluetoothDevice(callback->AsObject());
                manager->RemoveBluetoothScanCallback(callback->AsObject());
                manager->RemoveBluetoothScanCallbackDeathRecipientByCallback(callback->AsObject());
            }
        }
        manager->SendStopBluetoothScanEvent();
        manager->StoptBluetoothScan();
    }

    void CheckReportFlow(LocatorRequiredDataManager* manager, const uint8_t* data, size_t size)
    {
        int index = 0;
        auto bluetoothScanResult = std::make_unique<BluetoothScanResult>();
        bluetoothScanResult->SetDeviceId("00:11:22:33:44:55");
        bluetoothScanResult->SetDeviceName("deviceName");
        bluetoothScanResult->SetRssi(static_cast<int64_t>(data[index++]));
        std::vector<uint8_t> scanData(data, data + size);
        bluetoothScanResult->SetData(scanData);
        bluetoothScanResult->SetConnectable(data[index++] & 1);
        manager->ReportBluetoothScanResult(bluetoothScanResult);
    }

#ifdef BLUETOOTH_ENABLE
    void CheckBleCallbackWapper(const uint8_t* data)
    {
        int index = 0;
        LocatorBleCallbackWapper wapper;
        Bluetooth::BleScanResult scanResult;
        wapper.OnScanCallback(scanResult);
        wapper.OnFoundOrLostCallback(scanResult, data[index++]);
        std::vector<Bluetooth::BleScanResult> batchResults;
        batchResults.push_back(scanResult);
        wapper.OnBleBatchScanResultsEvent(batchResults);
        int32_t scanCode = static_cast<int32_t>(data[index++]);
        bool wapperScanStarted = (data[index++] & 1) != 0;
        wapper.OnStartOrStopScanEvent(scanCode, wapperScanStarted);
        wapper.GetLocatingRequiredDataByBle(scanResult);
        wapper.GetBluetoothScanResultByBle(scanResult);
    }

    void CheckBluetoothHostObserver(const uint8_t* data)
    {
        int index = 0;
        LocatorBluetoothHost host;
        Bluetooth::BluetoothRemoteDevice device;
        host.GetLocatingRequiredDataByBtHost(device);
        int transport = static_cast<int>(data[index++]);
        int status = static_cast<int>(data[index++]);
        host.OnStateChanged(transport, status);
        host.OnDiscoveryStateChanged(static_cast<int>(data[index++]));
        int rssi = static_cast<int>(data[index++]);
        int deviceClass = static_cast<int>(data[index++]);
        host.OnDiscoveryResult(device, rssi, "deviceName", deviceClass);
        host.OnPairRequested(device);
        int reqType = static_cast<int>(data[index++]);
        int passkey = static_cast<int>(data[index++]);
        host.OnPairConfirmed(device, reqType, passkey);
        host.OnScanModeChanged(static_cast<int>(data[index++]));
        host.OnDeviceNameChanged("deviceName");
        host.OnDeviceAddrChanged("00:11:22:33:44:55");
    }

    void CheckScanCallbackDeathRecipient()
    {
        auto callback = sptr<LocatingRequiredDataCallbackNapi>(
            new (std::nothrow) LocatingRequiredDataCallbackNapi());
        if (callback == nullptr || callback->AsObject() == nullptr) {
            return;
        }
        BluetoothScanCallbackDeathRecipient recipient;
        wptr<IRemoteObject> watcher(callback->AsObject());
        recipient.OnRemoteDied(watcher);
    }
#endif

    bool LocatorRequiredDataManagerFuzzTest(const uint8_t* data, size_t size)
    {
        if (data == nullptr || size < MIN_DATA_LEN) {
            return false;
        }
        auto manager = LocatorRequiredDataManager::GetInstance();
        if (manager == nullptr) {
            return false;
        }
        CheckRegisterTypeBranches(manager, data);
        CheckReportWithRegisteredCallbacks(manager, data);
        CheckWifiAndStillState(manager, data);
        CheckBluetoothScanFlow(manager, data);
        CheckReportFlow(manager, data, size);
#ifdef BLUETOOTH_ENABLE
        CheckBleCallbackWapper(data);
        CheckBluetoothHostObserver(data);
        CheckScanCallbackDeathRecipient();
#endif
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::LocatorRequiredDataManagerFuzzTest(data, size);
    return 0;
}
