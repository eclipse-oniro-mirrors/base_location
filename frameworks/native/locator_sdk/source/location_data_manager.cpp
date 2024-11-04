/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "location_data_manager.h"

#include "uri.h"

#include "switch_callback_proxy.h"
#include "constant_definition.h"
#include "location_data_rdb_helper.h"
#include "location_data_rdb_manager.h"
#include "location_log.h"
#include "common_hisysevent.h"
#include "parameter.h"
#include "permission_manager.h"
namespace OHOS {
namespace Location {
const int APP_INFO_SIZE = 3;
const int APP_INFO_UID_INDEX = 0;
const int APP_INFO_TOKENID_INDEX = 1;
const int APP_INFO_BUNDLEID_INDEX = 2;
const int APP_INFO_LASTSTATE_INDEX = 3;
LocationDataManager* LocationDataManager::GetInstance()
{
    static LocationDataManager data;
    return &data;
}

LocationDataManager::LocationDataManager()
{
}

LocationDataManager::~LocationDataManager()
{
}

LocationErrCode LocationDataManager::ReportSwitchState(bool isEnabled)
{
    int state = isEnabled ? ENABLED : DISABLED;
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto item : switchCallbackMap_) {
        std::vector<std::string> appInfo = item.second;
        if (appInfo.size() < APP_INFO_SIZE) {
            continue;
        }
        int uid = std::stoi(appInfo[APP_INFO_UID_INDEX]);
        int tokenId = std::stoi(appInfo[APP_INFO_TOKENID_INDEX]);
        std::string bundleName = appInfo[APP_INFO_BUNDLEID_INDEX];
        int lastState = std::stoi(appInfo[APP_INFO_LASTSTATE_INDEX]);
        if (!PermissionManager::CheckIsSystemSa(tokenId) &&
            !CommonUtils::CheckAppForUser(uid, bundleName)) {
            LBSLOGE(LOCATOR, "It is not a listener of Current user, no need to report. uid : %{public}d", uid);
            continue;
        }
        if (state == lastState) {
            // current state is same to before, no need to report
            continue;
        }
        sptr<IRemoteObject> remoteObject = item.first;
        if (remoteObject == nullptr) {
            LBSLOGE(LOCATOR, "remoteObject callback is nullptr");
            continue;
        }
        auto callback = std::make_unique<SwitchCallbackProxy>(remoteObject);
        LBSLOGI(LOCATOR, "ReportSwitchState to uid : %{public}d , state = %{public}d", uid, state);
        callback->OnSwitchChange(state);
        appInfo[APP_INFO_LASTSTATE_INDEX] = std::to_string(state);
        switchCallbackMap_[remoteObject] = appInfo;
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocationDataManager::RegisterSwitchCallback(const sptr<IRemoteObject>& callback,
    AppIdentity& identity)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR, "register an invalid switch callback");
        return ERRCODE_INVALID_PARAM;
    }
    std::unique_lock<std::mutex> lock(mutex_);
    auto iter = switchCallbackMap_.find(callback);
    if (iter != switchCallbackMap_.end()) {
        LBSLOGE(LOCATOR, "callback has registered");
        return ERRCODE_SUCCESS;
    }
    std::vector<std::string> appInfo;
    appInfo.push_back(std::to_string(identity.GetUid()));
    appInfo.push_back(std::to_string(identity.GetTokenId()));
    appInfo.push_back(identity.GetBundleName());
    appInfo.push_back(std::to_string((DEFAULT_SWITCH_STATE)));
    switchCallbackMap_[callback] = appInfo;
    LBSLOGD(LOCATOR, "after uid:%{public}d register, switch callback size:%{public}s",
        identity.GetUid(), std::to_string(switchCallbackMap_.size()).c_str());
    if (!IsSwitchObserverReg()) {
        RegisterLocationSwitchObserver();
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocationDataManager::UnregisterSwitchCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR, "unregister an invalid switch callback");
        return ERRCODE_INVALID_PARAM;
    }
    std::unique_lock<std::mutex> lock(mutex_);
    auto iter = switchCallbackMap_.find(callback);
    if (iter != switchCallbackMap_.end()) {
        switchCallbackMap_.erase(iter);
    }
    LBSLOGD(LOCATOR, "after unregister, switch callback size:%{public}s",
        std::to_string(switchCallbackMap_.size()).c_str());
    return ERRCODE_SUCCESS;
}

bool LocationDataManager::IsSwitchObserverReg()
{
    std::unique_lock<std::mutex> lock(isSwitchObserverRegMutex_);
    return isSwitchObserverReg_;
}

void LocationDataManager::SetIsSwitchObserverReg(bool isSwitchObserverReg)
{
    std::unique_lock<std::mutex> lock(isSwitchObserverRegMutex_);
    isSwitchObserverReg_ = isSwitchObserverReg;
}

bool LocationDataManager::IsFirstReport()
{
    std::unique_lock<std::mutex> lock(isFirstReportMutex_);
    return isFirstReport_;
}

void LocationDataManager::SetIsFirstReport(bool isFirstReport)
{
    std::unique_lock<std::mutex> lock(isFirstReportMutex_);
    isFirstReport_ = isFirstReport;
}

void LocationDataManager::RegisterLocationSwitchObserver()
{
    auto eventCallback = [](const char *key, const char *value, void *context) {
        int32_t state = DEFAULT_SWITCH_STATE;
        state = LocationDataRdbManager::QuerySwitchState();
        auto manager = LocationDataManager::GetInstance();
        if (manager->IsFirstReport()) {
            LBSLOGI(LOCATOR, "first switch callback, no need to report");
            manager->SetIsFirstReport(false);
            return;
        }
        if (state == DEFAULT_SWITCH_STATE) {
            LBSLOGE(LOCATOR, "LOCATION_SWITCH_MODE changed. state %{public}d. do not report", state);
            return;
        }
        bool switchState = (state == ENABLED);
        LBSLOGI(LOCATOR, "LOCATION_SWITCH_MODE changed. switchState %{public}d", switchState);
        manager->ReportSwitchState(switchState);
    };

    int ret = WatchParameter(LOCATION_SWITCH_MODE, eventCallback, nullptr);
    if (ret != SUCCESS) {
        LBSLOGE(LOCATOR, "WatchParameter fail");
        return;
    }
    SetIsSwitchObserverReg(true);
    return;
}
}  // namespace Location
}  // namespace OHOS
