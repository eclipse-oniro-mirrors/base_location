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

#include "request_manager.h"

#include "privacy_kit.h"

#include "common_utils.h"
#include "constant_definition.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_ability_proxy.h"
#endif
#include "fusion_controller.h"
#include "location_log.h"
#include "location_sa_load_manager.h"
#include "locator_ability.h"
#include "locator_background_proxy.h"
#include "locator_event_manager.h"
#ifdef FEATURE_NETWORK_SUPPORT
#include "network_ability_proxy.h"
#endif
#ifdef FEATURE_PASSIVE_SUPPORT
#include "passive_ability_proxy.h"
#endif
#include "request_config.h"
#include "location_log_event_ids.h"
#include "common_hisysevent.h"
#include "hook_utils.h"
#include "permission_manager.h"

#ifdef RES_SCHED_SUPPROT
#include "res_type.h"
#include "res_sched_client.h"
#endif

namespace OHOS {
namespace Location {
std::mutex RequestManager::requestMutex_;
RequestManager::RequestManager()
{
    auto locatorDftManager = DelayedSingleton<LocatorDftManager>::GetInstance();
    if (locatorDftManager != nullptr) {
        locatorDftManager->Init();
    }
}

RequestManager::~RequestManager()
{
}

bool RequestManager::InitSystemListeners()
{
    LBSLOGI(REQUEST_MANAGER, "Init system listeners.");
    return true;
}

void RequestManager::UpdateUsingPermission(std::shared_ptr<Request> request)
{
    std::unique_lock<std::mutex> lock(permissionRecordMutex_, std::defer_lock);
    lock.lock();
    if (request == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "request is null");
        lock.unlock();
        return;
    }
    LBSLOGD(REQUEST_MANAGER, "UpdateUsingPermission : tokenId = %{public}d, firstTokenId = %{public}d",
        request->GetTokenId(), request->GetFirstTokenId());
    UpdateUsingApproximatelyPermission(request);
    lock.unlock();
}

void RequestManager::UpdateUsingApproximatelyPermission(std::shared_ptr<Request> request)
{
    uint32_t callingTokenId = request->GetTokenId();
    uint32_t callingFirstTokenid = request->GetFirstTokenId();
    int32_t uid = request->GetUid();
    if (IsUidInProcessing(uid) &&
        PermissionManager::CheckApproximatelyPermission(callingTokenId, callingFirstTokenid)) {
        if (!request->GetApproximatelyPermState()) {
            PrivacyKit::StartUsingPermission(callingTokenId, ACCESS_APPROXIMATELY_LOCATION);
            request->SetApproximatelyPermState(true);
        }
    } else {
        if (request->GetApproximatelyPermState()) {
            PrivacyKit::StopUsingPermission(callingTokenId, ACCESS_APPROXIMATELY_LOCATION);
            request->SetApproximatelyPermState(false);
        }
    }
}

void RequestManager::HandleStartLocating(std::shared_ptr<Request> request)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    auto locatorDftManager = DelayedSingleton<LocatorDftManager>::GetInstance();
    if (locatorAbility == nullptr || locatorDftManager == nullptr) {
        return;
    }
    // restore request to all request list
    bool isNewRequest = RestorRequest(request);
    // update request map
    if (isNewRequest) {
        locatorAbility->RegisterPermissionCallback(request->GetTokenId(),
            {ACCESS_APPROXIMATELY_LOCATION, ACCESS_LOCATION, ACCESS_BACKGROUND_LOCATION});
        UpdateRequestRecord(request, true);
        UpdateUsingPermission(request);
        locatorDftManager->LocationSessionStart(request);
    }
    // process location request
    HandleRequest();
}

bool RequestManager::RestorRequest(std::shared_ptr<Request> newRequest)
{
    std::unique_lock lock(requestMutex_);

    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        return false;
    }
    auto receivers = locatorAbility->GetReceivers();
    if (receivers == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "receivers is empty");
        return false;
    }
    if (newRequest == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "newRequest is empty");
        return false;
    }
    newRequest->SetRequesting(true);
    sptr<IRemoteObject> newCallback = newRequest->GetLocatorCallBack()->AsObject();

    LBSLOGI(REQUEST_MANAGER, "add request:%{public}s", newRequest->ToString().c_str());
    // if callback and request config type is same, take new request configuration over the old one in request list
    // otherwise, add restore the new request in the list.
    auto iterator = receivers->find(newCallback);
    if (iterator == receivers->end()) {
        std::list<std::shared_ptr<Request>> requestList;
        requestList.push_back(newRequest);
        receivers->insert(make_pair(newCallback, requestList));
        LBSLOGD(REQUEST_MANAGER, "add new receiver with new callback");
        return true;
    }

    sptr<RequestConfig> newConfig = newRequest->GetRequestConfig();
    std::list<std::shared_ptr<Request>> requestWithSameCallback = iterator->second;
    for (auto iter = requestWithSameCallback.begin(); iter != requestWithSameCallback.end(); ++iter) {
        auto request = *iter;
        if (request == nullptr) {
            continue;
        }
        auto requestConfig = request->GetRequestConfig();
        if (requestConfig == nullptr || newConfig == nullptr) {
            continue;
        }
        if (newConfig->IsSame(*requestConfig)) {
            request->SetRequestConfig(*newConfig);
            LBSLOGI(REQUEST_MANAGER, "find same type request, update request configuration");
            return false;
        }
    }
    requestWithSameCallback.push_back(newRequest);
    LBSLOGD(REQUEST_MANAGER, "add new receiver with old callback");
    return true;
}

void RequestManager::UpdateRequestRecord(std::shared_ptr<Request> request, bool shouldInsert)
{
    std::shared_ptr<std::list<std::string>> proxys = std::make_shared<std::list<std::string>>();
    request->GetProxyName(proxys);
    if (proxys->empty()) {
        LBSLOGE(REQUEST_MANAGER, "can not get proxy name according to request configuration");
        return;
    }

    for (std::list<std::string>::iterator iter = proxys->begin(); iter != proxys->end(); ++iter) {
        std::string abilityName = *iter;
        UpdateRequestRecord(request, abilityName, shouldInsert);
    }
}

void RequestManager::UpdateRequestRecord(std::shared_ptr<Request> request, std::string abilityName, bool shouldInsert)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "locatorAbility is null");
        return;
    }
    std::unique_lock lock(requestMutex_);
    auto requests = locatorAbility->GetRequests();
    if (requests == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "requests map is empty");
        return;
    }
    auto mapIter = requests->find(abilityName);
    if (mapIter == requests->end()) {
        LBSLOGE(REQUEST_MANAGER, "can not find %{public}s ability request list.", abilityName.c_str());
        return;
    }

    auto list = &(mapIter->second);
    LBSLOGD(REQUEST_MANAGER, "%{public}s ability current request size %{public}s",
        abilityName.c_str(), std::to_string(list->size()).c_str());
    if (shouldInsert) {
        list->push_back(request);
        HandleChrEvent(*list);
        UpdateRunningUids(request, abilityName, true);
    } else {
        for (auto iter = list->begin(); iter != list->end();) {
            auto findRequest = *iter;
            if (request == findRequest) {
                iter = list->erase(iter);
                UpdateRunningUids(findRequest, abilityName, false);
                LBSLOGD(REQUEST_MANAGER, "find request");
            } else {
                ++iter;
            }
        }
    }
    LBSLOGD(REQUEST_MANAGER, "%{public}s ability request size %{public}s",
        abilityName.c_str(), std::to_string(list->size()).c_str());
}

void RequestManager::HandleChrEvent(std::list<std::shared_ptr<Request>> requests)
{
    if (requests.size() > LBS_REQUEST_MAX_SIZE) {
        std::vector<std::string> names;
        std::vector<std::string> values;
        int index = 0;
        for (auto it = requests.begin(); it != requests.end(); ++it, ++index) {
            auto request = *it;
            if (request == nullptr) {
                continue;
            }
            names.push_back(std::to_string(index));
            std::string packageName = request->GetPackageName();
            values.push_back(packageName);
        }
        WriteLocationInnerEvent(LBS_REQUEST_TOO_MUCH, names, values);
    }
}

void RequestManager::HandleStopLocating(sptr<ILocatorCallback> callback)
{
    if (callback == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "stop locating but callback is null");
        return;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "locatorAbility is null");
        return;
    }
    std::unique_lock<std::mutex> lock(requestMutex_, std::defer_lock);
    lock.lock();
    auto receivers = locatorAbility->GetReceivers();
    if (receivers == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "receivers map is empty");
        lock.unlock();
        return;
    }
    sptr<IRemoteObject> deadCallback = callback->AsObject();
    // get dead request list
    LBSLOGD(REQUEST_MANAGER, "stop callback");
    auto iterator = receivers->find(deadCallback);
    if (iterator == receivers->end()) {
        LBSLOGD(REQUEST_MANAGER, "this callback has no record in receiver map");
        lock.unlock();
        return;
    }

    auto requests = iterator->second;
    auto deadRequests = std::make_shared<std::list<std::shared_ptr<Request>>>();
    for (auto iter = requests.begin(); iter != requests.end(); ++iter) {
        auto request = *iter;
        locatorAbility->UnregisterPermissionCallback(request->GetTokenId());
        deadRequests->push_back(request);
        HookUtils::ExecuteHookWhenStopLocation(request);
        LBSLOGI(REQUEST_MANAGER, "remove request:%{public}s", request->ToString().c_str());
    }
    LBSLOGD(REQUEST_MANAGER, "get %{public}s dead request", std::to_string(deadRequests->size()).c_str());
    // update request map
    if (deadRequests->size() == 0) {
        lock.unlock();
        return;
    }
    iterator->second.clear();
    receivers->erase(iterator);
    lock.unlock();
    DeleteRequestRecord(deadRequests);
    deadRequests->clear();
    // process location request
    HandleRequest();
}

void RequestManager::DeleteRequestRecord(std::shared_ptr<std::list<std::shared_ptr<Request>>> requests)
{
    for (auto iter = requests->begin(); iter != requests->end(); ++iter) {
        auto request = *iter;
        UpdateRequestRecord(request, false);
        UpdateUsingPermission(request);
        auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
        if (locatorBackgroundProxy == nullptr) {
            LBSLOGE(REQUEST_MANAGER, "DeleteRequestRecord: LocatorBackgroundProxy is nullptr.");
            break;
        }
        locatorBackgroundProxy.get()->OnDeleteRequestRecord(request);
    }
}

void RequestManager::HandleRequest()
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "locatorAbility is null");
        return;
    }
    std::unique_lock<std::mutex> lock(requestMutex_, std::defer_lock);
    lock.lock();
    auto requests = locatorAbility->GetRequests();
    lock.unlock();
    if (requests == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "requests map is empty");
        return;
    }
    std::map<std::string, std::list<std::shared_ptr<Request>>>::iterator iter;
    for (iter = requests->begin(); iter != requests->end(); ++iter) {
        std::string abilityName = iter->first;
        std::list<std::shared_ptr<Request>> requestList = iter->second;
        HandleRequest(abilityName, requestList);
    }
}

void RequestManager::HandleRequest(std::string abilityName, std::list<std::shared_ptr<Request>> list)
{
    // generate work record, and calculate interval
    std::shared_ptr<WorkRecord> workRecord = std::make_shared<WorkRecord>();
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        auto request = *iter;
        if (!AddRequestToWorkRecord(request, workRecord)) {
            continue;
        }
        if (!ActiveLocatingStrategies(request)) {
            continue;
        }
        LBSLOGD(REQUEST_MANAGER, "add pid:%{public}d uid:%{public}d %{public}s", request->GetPid(), request->GetUid(),
            request->GetPackageName().c_str());
    }
    LBSLOGD(REQUEST_MANAGER, "detect %{public}s ability requests(size:%{public}s) work record:%{public}s",
        abilityName.c_str(), std::to_string(list.size()).c_str(), workRecord->ToString().c_str());

    ProxySendLocationRequest(abilityName, *workRecord);
}

bool RequestManager::ActiveLocatingStrategies(const std::shared_ptr<Request>& request)
{
    if (request == nullptr) {
        return false;
    }
    auto requestConfig = request->GetRequestConfig();
    if (requestConfig == nullptr) {
        return false;
    }
    int requestType = requestConfig->GetScenario();
    if (requestType == SCENE_UNSET) {
        requestType = requestConfig->GetPriority();
    }
    auto fusionController = DelayedSingleton<FusionController>::GetInstance();
    if (fusionController != nullptr) {
        fusionController->ActiveFusionStrategies(requestType);
    }
    return true;
}

/**
 * determine whether the request is valid.
 */
bool RequestManager::IsRequestAvailable(std::shared_ptr<Request>& request)
{
    // for frozen app, do not add to workRecord
    if (DelayedSingleton<LocatorAbility>::GetInstance()->IsProxyPid(request->GetPid())) {
        return false;
    }
    // for once_request app, if it has timed out, do not add to workRecord
    int64_t curTime = CommonUtils::GetCurrentTime();
    if (request->GetRequestConfig()->GetFixNumber() == 1 &&
        fabs(curTime - request->GetRequestConfig()->GetTimeStamp()) >
        (request->GetRequestConfig()->GetTimeOut() / MILLI_PER_SEC)) {
        LBSLOGE(LOCATOR, "%{public}d has timed out.", request->GetPid());
        return false;
    }
    return true;
}

bool RequestManager::AddRequestToWorkRecord(std::shared_ptr<Request>& request,
    std::shared_ptr<WorkRecord>& workRecord)
{
    if (request == nullptr) {
        return false;
    }
    UpdateUsingPermission(request);
    if (!request->GetIsRequesting()) {
        return false;
    }
    if (!IsRequestAvailable(request)) {
        return false;
    }
    uint32_t tokenId = request->GetTokenId();
    uint32_t firstTokenId = request->GetFirstTokenId();
    // if location access permission granted, add request info to work record
    if (!PermissionManager::CheckLocationPermission(tokenId, firstTokenId) &&
        !PermissionManager::CheckApproximatelyPermission(tokenId, firstTokenId)) {
        LBSLOGI(LOCATOR, "CheckLocationPermission return false, tokenId=%{public}d", tokenId);
        return false;
    }
    std::string bundleName = "";
    pid_t uid = request->GetUid();
    if (!CommonUtils::GetBundleNameByUid(uid, bundleName)) {
        LBSLOGD(REPORT_MANAGER, "Fail to Get bundle name: uid = %{public}d.", uid);
    }
    auto reportManager = DelayedSingleton<ReportManager>::GetInstance();
    if (reportManager != nullptr) {
        if (reportManager->IsAppBackground(bundleName, tokenId,
            request->GetTokenIdEx(), uid)&&
            !PermissionManager::CheckBackgroundPermission(tokenId, firstTokenId)) {
            return false;
        }
    }
    auto requestConfig = request->GetRequestConfig();
    if (requestConfig == nullptr) {
        return false;
    }
    // add request info to work record
    if (workRecord != nullptr) {
        request->SetNlpRequestType();
        workRecord->Add(request);
    }
    return true;
}

void RequestManager::ProxySendLocationRequest(std::string abilityName, WorkRecord& workRecord)
{
    int systemAbilityId = CommonUtils::AbilityConvertToId(abilityName);
    if (!LocationSaLoadManager::InitLocationSa(systemAbilityId)) {
        return ;
    }
    sptr<IRemoteObject> remoteObject = CommonUtils::GetRemoteObject(systemAbilityId, CommonUtils::InitDeviceId());
    if (remoteObject == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s: remote obj is nullptr", __func__);
        return;
    }
    workRecord.SetDeviceId(CommonUtils::InitDeviceId());
    if (abilityName == GNSS_ABILITY) {
#ifdef FEATURE_GNSS_SUPPORT
        std::unique_ptr<GnssAbilityProxy> gnssProxy = std::make_unique<GnssAbilityProxy>(remoteObject);
        gnssProxy->SendLocationRequest(workRecord);
#endif
    } else if (abilityName == NETWORK_ABILITY) {
#ifdef FEATURE_NETWORK_SUPPORT
        std::unique_ptr<NetworkAbilityProxy> networkProxy = std::make_unique<NetworkAbilityProxy>(remoteObject);
        networkProxy->SendLocationRequest(workRecord);
#endif
    } else if (abilityName == PASSIVE_ABILITY) {
#ifdef FEATURE_PASSIVE_SUPPORT
        std::unique_ptr<PassiveAbilityProxy> passiveProxy = std::make_unique<PassiveAbilityProxy>(remoteObject);
        passiveProxy->SendLocationRequest(workRecord);
#endif
    }
    auto fusionController = DelayedSingleton<FusionController>::GetInstance();
    if (fusionController != nullptr) {
        fusionController->Process(abilityName);
    }
}

sptr<IRemoteObject> RequestManager::GetRemoteObject(std::string abilityName)
{
    sptr<IRemoteObject> remoteObject = nullptr;
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "locatorAbility is null");
        return remoteObject;
    }
    auto remoteManagerMap = locatorAbility->GetProxyMap();
    if (remoteManagerMap == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "proxy map is empty");
        return remoteObject;
    }
    auto remoteObjectIter = remoteManagerMap->find(abilityName);
    if (remoteObjectIter == remoteManagerMap->end()) {
        LBSLOGE(REQUEST_MANAGER, "sa init fail!");
        return remoteObject;
    }
    remoteObject = remoteObjectIter->second;
    return remoteObject;
}

void RequestManager::HandlePowerSuspendChanged(int32_t pid, int32_t uid, int32_t state)
{
    if (!IsUidInProcessing(uid)) {
        LBSLOGD(REQUEST_MANAGER, "Current uid : %{public}d is not locating.", uid);
        return;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "locatorAbility is null");
        return;
    }
    auto requests = locatorAbility->GetRequests();
    if (requests == nullptr || requests->empty()) {
        LBSLOGE(REQUEST_MANAGER, "requests map is empty");
        return;
    }
    bool isActive = (state == static_cast<int>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND));
    for (auto mapIter = requests->begin(); mapIter != requests->end(); mapIter++) {
        auto list = mapIter->second;
        for (auto request : list) {
            std::string uid1 = std::to_string(request->GetUid());
            std::string uid2 = std::to_string(uid);
            std::string pid1 = std::to_string(request->GetPid());
            std::string pid2 = std::to_string(pid);
            if ((uid1.compare(uid2) != 0) || (pid1.compare(pid2) != 0)) {
                continue;
            }
            auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
            if (locatorBackgroundProxy != nullptr) {
                locatorBackgroundProxy.get()->OnSuspend(request, isActive);
            }
        }
    }
    if (DelayedSingleton<LocatorAbility>::GetInstance() != nullptr) {
        DelayedSingleton<LocatorAbility>::GetInstance().get()->ApplyRequests(1);
    }
}

void RequestManager::HandlePermissionChanged(uint32_t tokenId)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "HandlePermissionChanged locatorAbility is null");
        return;
    }
    auto requests = locatorAbility->GetRequests();
    if (requests == nullptr || requests->empty()) {
        LBSLOGE(REQUEST_MANAGER, "HandlePermissionChanged requests map is empty");
        return;
    }
    for (auto mapIter = requests->begin(); mapIter != requests->end(); mapIter++) {
        auto list = mapIter->second;
        for (auto request : list) {
            if (request == nullptr || tokenId != request->GetTokenId()) {
                continue;
            }
            auto backgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
            if (backgroundProxy != nullptr) {
                backgroundProxy.get()->UpdateListOnRequestChange(request);
            }
        }
    }
}

bool RequestManager::IsUidInProcessing(int32_t uid)
{
    std::unique_lock<std::mutex> lock(runningUidsMutex_);
    auto iter = runningUidMap_.find(uid);
    if (iter == runningUidMap_.end()) {
        return false;
    }
    return true;
}

void RequestManager::UpdateRunningUids(const std::shared_ptr<Request>& request, std::string abilityName, bool isAdd)
{
    std::unique_lock<std::mutex> lock(runningUidsMutex_);
    auto uid = request->GetUid();
    auto pid = request->GetPid();
    int32_t uidCount = 0;
    auto iter = runningUidMap_.find(uid);
    if (iter != runningUidMap_.end()) {
        uidCount = iter->second;
        runningUidMap_.erase(uid);
    }
    if (isAdd) {
        auto requestConfig = request->GetRequestConfig();
        WriteLocationInnerEvent(ADD_REQUEST, {
            "PackageName", request->GetPackageName(),
            "abilityName", abilityName,
            "requestAddress", request->GetUuid(),
            "scenario", std::to_string(requestConfig->GetScenario()),
            "priority", std::to_string(requestConfig->GetPriority()),
            "timeInterval", std::to_string(requestConfig->GetTimeInterval()),
            "maxAccuracy", std::to_string(requestConfig->GetMaxAccuracy())});
        uidCount += 1;
        if (uidCount == 1) {
            WriteAppLocatingStateEvent("start", pid, uid);
            ReportDataToResSched("start", uid);
        }
    } else {
        WriteLocationInnerEvent(REMOVE_REQUEST, {"PackageName", request->GetPackageName(),
                    "abilityName", abilityName, "requestAddress", request->GetUuid()});
        uidCount -= 1;
        if (uidCount == 0) {
            WriteAppLocatingStateEvent("stop", pid, uid);
            ReportDataToResSched("stop", uid);
        }
    }
    if (uidCount > 0) {
        runningUidMap_.insert(std::make_pair(uid, uidCount));
    }
}

void RequestManager::ReportDataToResSched(std::string state, const pid_t uid)
{
#ifdef RES_SCHED_SUPPROT
    std::unordered_map<std::string, std::string> payload;
    payload['uid'] = std::to_string(uid);
    payload['state'] = state;
    uint32_t type = ResourceSchedule::ResType::RES_TYPE_LOCATION_STATUS;
    int64_t value =  ResourceSchedule::ResType::LocationStatus::APP_LOCATION_STATE_CHANGE;
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, value, payload);
#endif
}

void RequestManager::UpdateLocationErrorCallbackToRequest(
    sptr<ILocatorCallback> callback, uint32_t tokenId, bool state)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "locatorAbility is null");
        return;
    }
    auto requests = locatorAbility->GetRequests();
    if (requests == nullptr || requests->empty()) {
        LBSLOGE(REQUEST_MANAGER, "requests map is empty");
        return;
    }
    for (auto mapIter = requests->begin(); mapIter != requests->end(); mapIter++) {
        auto list = mapIter->second;
        for (auto request : list) {
            if (request == nullptr || tokenId != request->GetTokenId()) {
                continue;
            }
            if (state) {
                request->SetLocationErrorCallBack(callback);
            } else {
                request->SetLocationErrorCallBack(nullptr);
            }
        }
    }
}
} // namespace Location
} // namespace OHOS
