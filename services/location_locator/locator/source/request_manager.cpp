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
#include "privacy_error.h"

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
#ifdef DEVICE_STANDBY_ENABLE
#include "standby_service_client.h"
#endif

#ifdef RES_SCHED_SUPPROT
#include "res_type.h"
#include "res_sched_client.h"
#endif

#include "location_data_rdb_manager.h"

namespace OHOS {
namespace Location {
ffrt::mutex RequestManager::requestMutex_;
const int MAX_LOCATION_ERROR_CALLBACK_NUM = 1000;

RequestManager* RequestManager::GetInstance()
{
    static RequestManager data;
    return &data;
}

RequestManager::RequestManager()
{
    isDeviceIdleMode_.store(false);
    isDeviceStillState_.store(false);
    auto locatorDftManager = LocatorDftManager::GetInstance();
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

bool RequestManager::UpdateUsingPermission(std::shared_ptr<Request> request, const bool isStart)
{
    std::unique_lock<ffrt::mutex> lock(permissionRecordMutex_, std::defer_lock);
    lock.lock();
    if (request == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "request is null");
        lock.unlock();
        return false;
    }
    bool ret = UpdateUsingApproximatelyPermission(request, isStart);
    lock.unlock();
    return ret;
}

bool RequestManager::UpdateUsingApproximatelyPermission(std::shared_ptr<Request> request, const bool isStart)
{
    auto locatorAbility = LocatorAbility::GetInstance();
    uint32_t callingTokenId = request->GetTokenId();
    int ret;
    if (isStart && !request->GetApproximatelyPermState()) {
        IncreaseWorkingPidsCount(request->GetPid());
        bool isNeedStart = IsNeedStartUsingPermission(request->GetPid());
        if (isNeedStart) {
            ret = PrivacyKit::StartUsingPermission(callingTokenId, ACCESS_APPROXIMATELY_LOCATION, request->GetPid());
            if (ret != ERRCODE_SUCCESS && ret != Security::AccessToken::ERR_PERMISSION_ALREADY_START_USING &&
                locatorAbility->IsHapCaller(request->GetTokenId())) {
                DecreaseWorkingPidsCount(request->GetPid());
                LBSLOGE(REQUEST_MANAGER, "StartUsingPermission failed ret=%{public}d", ret);
                return false;
            }
        }
        request->SetApproximatelyPermState(true);
        ret = locatorAbility->UpdatePermissionUsedRecord(request->GetTokenId(),
            ACCESS_APPROXIMATELY_LOCATION, request->GetPermUsedType(), 1, 0);
        if (ret != ERRCODE_SUCCESS && locatorAbility->IsHapCaller(callingTokenId)) {
            LBSLOGE(REQUEST_MANAGER, "UpdatePermissionUsedRecord failed ret=%{public}d", ret);
            return false;
        }
    } else if (!isStart && request->GetApproximatelyPermState()) {
        DecreaseWorkingPidsCount(request->GetPid());
        bool isNeedStop = IsNeedStopUsingPermission(request->GetPid());
        if (isNeedStop) {
            ret = PrivacyKit::StopUsingPermission(callingTokenId, ACCESS_APPROXIMATELY_LOCATION, request->GetPid());
            if (ret != ERRCODE_SUCCESS && locatorAbility->IsHapCaller(callingTokenId)) {
                LBSLOGE(REQUEST_MANAGER, "StopUsingPermission failed ret=%{public}d", ret);
                return false;
            }
        }
        request->SetApproximatelyPermState(false);
    }
    return true;
}

void RequestManager::IncreaseWorkingPidsCount(const pid_t pid)
{
    std::unique_lock<ffrt::mutex> uniquelock(workingPidsCountMutex_);
    if (workingPidsCountMap_.count(pid) > 0) {
        workingPidsCountMap_[pid] = workingPidsCountMap_[pid] + 1;
    } else {
        workingPidsCountMap_.insert(std::make_pair(pid, 1));
    }
}

void RequestManager::DecreaseWorkingPidsCount(const pid_t pid)
{
    std::unique_lock<ffrt::mutex> uniquelock(workingPidsCountMutex_);
    if (workingPidsCountMap_.count(pid) > 0) {
        workingPidsCountMap_[pid] = workingPidsCountMap_[pid] - 1;
        if (workingPidsCountMap_[pid] < 0) {
            LBSLOGE(REQUEST_MANAGER, "working pid less 0, pid=%{public}d", pid);
        }
    }
}

bool RequestManager::IsNeedStartUsingPermission(const pid_t pid)
{
    std::unique_lock<ffrt::mutex> uniquelock(workingPidsCountMutex_);
    if (workingPidsCountMap_.count(pid) <= 0) {
        return false;
    }
    if (workingPidsCountMap_[pid] == 1) {
        return true;
    }
    return false;
}

bool RequestManager::IsNeedStopUsingPermission(const pid_t pid)
{
    std::unique_lock<ffrt::mutex> uniquelock(workingPidsCountMutex_);
    if (workingPidsCountMap_.count(pid) <= 0) {
        return false;
    }
    if (workingPidsCountMap_[pid] <= 0) {
        workingPidsCountMap_.erase(pid);
        return true;
    }
    return false;
}

void RequestManager::HandleStartLocating(std::shared_ptr<Request> request)
{
    auto locatorAbility = LocatorAbility::GetInstance();
    auto locatorDftManager = LocatorDftManager::GetInstance();
    // restore request to all request list
    bool isNewRequest = RestorRequest(request);
    // update request map
    if (isNewRequest) {
        locatorAbility->RegisterPermissionCallback(request->GetTokenId(),
            {ACCESS_APPROXIMATELY_LOCATION, ACCESS_LOCATION, ACCESS_BACKGROUND_LOCATION});
        UpdateRequestRecord(request, true);
        locatorDftManager->LocationSessionStart(request);
    }
    // process location request
    HandleRequest();
}

bool RequestManager::RestorRequest(std::shared_ptr<Request> newRequest)
{
    std::unique_lock lock(requestMutex_);

    auto locatorAbility = LocatorAbility::GetInstance();
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
    auto requestWithSameCallback = &(iterator->second);
    for (auto iter = requestWithSameCallback->begin(); iter != requestWithSameCallback->end(); ++iter) {
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
    requestWithSameCallback->push_back(newRequest);
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
    auto locatorAbility = LocatorAbility::GetInstance();
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
    LBSLOGD(REQUEST_MANAGER, "%{public}s ability current request size %{public}zu",
        abilityName.c_str(), list->size());
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
    LBSLOGD(REQUEST_MANAGER, "%{public}s ability request size %{public}zu",
        abilityName.c_str(), list->size());
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
    auto locatorAbility = LocatorAbility::GetInstance();
    std::unique_lock<ffrt::mutex> lock(requestMutex_, std::defer_lock);
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
        LBSLOGE(REQUEST_MANAGER, "this callback has no record in receiver map");
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
    LBSLOGD(REQUEST_MANAGER, "get %{public}zu dead request", deadRequests->size());
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
        UpdateUsingPermission(request, false);
        if (request->GetLocatorCallBack() != nullptr && request->GetLocatorCallbackRecipient() != nullptr) {
            request->GetLocatorCallBack()->AsObject()->RemoveDeathRecipient(request->GetLocatorCallbackRecipient());
        }
        auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
        locatorBackgroundProxy->OnDeleteRequestRecord(request);
    }
}

void RequestManager::HandleRequest()
{
    auto locatorAbility = LocatorAbility::GetInstance();
    std::unique_lock<ffrt::mutex> lock(requestMutex_, std::defer_lock);
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
        if (!AddRequestToWorkRecord(abilityName, request, workRecord)) {
            WriteLocationInnerEvent(REMOVE_REQUEST, {"PackageName", request->GetPackageName(),
                    "abilityName", abilityName, "requestAddress", request->GetUuid()});
            UpdateUsingPermission(request, false);
            continue;
        }
        if (!ActiveLocatingStrategies(request)) {
            continue;
        }
        LBSLOGD(REQUEST_MANAGER, "add pid:%{public}d uid:%{public}d %{public}s", request->GetPid(), request->GetUid(),
            request->GetPackageName().c_str());
    }
    LBSLOGD(REQUEST_MANAGER, "detect %{public}s ability requests(size:%{public}zu) work record:%{public}s",
        abilityName.c_str(), list.size(), workRecord->ToString().c_str());

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
    auto fusionController = FusionController::GetInstance();
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
    if (!request->GetIsRequesting()) {
        return false;
    }
    // for frozen app, do not add to workRecord
    if (ProxyFreezeManager::GetInstance()->IsProxyPid(request->GetPid())) {
        WriteLocationInnerEvent(LBS_REQUEST_FAIL_DETAIL, {"REQ_APP_NAME", request->GetPackageName(), "REQ_INFO",
            request->ToString().c_str(), "TRANS_ID", request->GetUuid(), "ERR_CODE", 
            std::to_string(ERRCODE_LOCATING_FREEZE)});
        return false;
    }
    AppIdentity identity;
    identity.SetUid(request->GetUid());
    identity.SetTokenId(request->GetTokenId());
    int currentUserId = LocatorBackgroundProxy::GetInstance()->getCurrentUserId();
    if (!CommonUtils::IsAppBelongCurrentAccount(identity, currentUserId)) {
        LBSLOGD(REPORT_MANAGER, "AddRequestToWorkRecord uid: %{public}d ,CheckAppIsCurrentUser fail",
            request->GetUid());
        WriteLocationInnerEvent(LBS_REQUEST_FAIL_DETAIL, {"REQ_APP_NAME", request->GetPackageName(), "REQ_INFO",
            request->ToString().c_str(), "TRANS_ID", request->GetUuid(), 
            "ERR_CODE", std::to_string(LOCATION_ERRCODE_NOT_CURRENT_USER_ID)});
        return false;
    }
    // for once_request app, if it has timed out, do not add to workRecord
    int64_t curTime = CommonUtils::GetCurrentTime();
    if (request->GetRequestConfig()->GetFixNumber() == 1 &&
        fabs(curTime - request->GetRequestConfig()->GetTimeStamp()) >
        (request->GetRequestConfig()->GetTimeOut() / MILLI_PER_SEC)) {
        LBSLOGE(LOCATOR, "%{public}d has timed out.", request->GetPid());
        WriteLocationInnerEvent(LBS_REQUEST_FAIL_DETAIL, {"REQ_APP_NAME", request->GetPackageName(),"REQ_INFO",
            request->ToString().c_str(), "TRANS_ID", request->GetUuid(), "ERR_CODE",
            std::to_string(LOCATION_ERRCODE_REQUEST_TIMEOUT)});
        return false;
    }
    return true;
}

void RequestManager::IsStandby()
{
#ifdef DEVICE_STANDBY_ENABLE
    LBSLOGI(LOCATOR, "%{public}s called", __func__);
    bool isStandby = false;
    DevStandbyMgr::StandbyServiceClient& standbyServiceClient = DevStandbyMgr::StandbyServiceClient::GetInstance();
    ErrCode code = standbyServiceClient.IsDeviceInStandby(isStandby);
    if (code == ERR_OK && isStandby) {
        isDeviceIdleMode_.store(true);
        LBSLOGI(LOCATOR, "isStandby = true");
        return;
    }
#endif
    isDeviceIdleMode_.store(false);
    LBSLOGI(LOCATOR, "isStandby = false");
}

bool RequestManager::AddRequestToWorkRecord(std::string abilityName, std::shared_ptr<Request>& request,
    std::shared_ptr<WorkRecord>& workRecord)
{
    if (request == nullptr || !IsRequestAvailable(request)) {
        return false;
    }
    if (LocationDataRdbManager::QuerySwitchState() != ENABLED &&
        !LocatorAbility::GetInstance()->GetLocationSwitchIgnoredFlag(request->GetTokenId())) {
        RequestManager::GetInstance()->ReportLocationError(LOCATING_FAILED_LOCATION_SWITCH_OFF, request);
        WriteLocationInnerEvent(LBS_REQUEST_FAIL_DETAIL, {"REQ_APP_NAME", request->GetPackageName(), "REQ_INFO",
            request->ToString().c_str(), "TRANS_ID", request->GetUuid(), "ERR_CODE", 
            std::to_string(ERRCODE_SWITCH_OFF)});
        LBSLOGE(LOCATOR, "%{public}s line:%{public}d the location switch is off", __func__, __LINE__);
        return false;
    }
    uint32_t tokenId = request->GetTokenId();
    uint32_t firstTokenId = request->GetFirstTokenId();
    // if location access permission granted, add request info to work record
    if (!PermissionManager::CheckLocationPermission(tokenId, firstTokenId) &&
        !PermissionManager::CheckApproximatelyPermission(tokenId, firstTokenId)) {
        RequestManager::GetInstance()->ReportLocationError(LOCATING_FAILED_LOCATION_PERMISSION_DENIED, request);
        WriteLocationInnerEvent(LBS_REQUEST_FAIL_DETAIL, {"REQ_APP_NAME", request->GetPackageName(), "REQ_INFO",
            request->ToString().c_str(), "TRANS_ID", request->GetUuid(), "ERR_CODE",
            std::to_string(LOCATION_ERRCODE_PERMISSION_DENIED)});
        LBSLOGI(LOCATOR, "CheckLocationPermission return false, Id=%{public}d", tokenId);
        return false;
    }
    std::string bundleName = "";
    pid_t uid = request->GetUid();
    pid_t pid = request->GetPid();
    if (!CommonUtils::GetBundleNameByUid(uid, bundleName)) {
        LBSLOGD(REPORT_MANAGER, "Fail to Get bundle name: uid = %{public}d.", uid);
    }
    auto requestConfig = request->GetRequestConfig();
    if (requestConfig == nullptr) {
        return false;
    }
    if (requestConfig->GetFixNumber() == 0 && ReportManager::GetInstance()->IsAppBackground(bundleName, tokenId,
        request->GetTokenIdEx(), uid, pid) &&
        !PermissionManager::CheckBackgroundPermission(tokenId, firstTokenId)) {
        RequestManager::GetInstance()->ReportLocationError(LOCATING_FAILED_BACKGROUND_PERMISSION_DENIED, request);
        WriteLocationInnerEvent(LBS_REQUEST_FAIL_DETAIL, {"REQ_APP_NAME", request->GetPackageName(), "TRANS_ID", 
            request->GetUuid(), "REQ_INFO", request->ToString().c_str(),  "ERR_CODE", 
            std::to_string(LOCATION_ERRCODE_BACKGROUND_CONTINUE_PERMISSION_DENIED)});
        LBSLOGE(REPORT_MANAGER, "CheckBackgroundPermission return false, Id=%{public}d", tokenId);
        return false;
    }
    if (HookUtils::ExecuteHookWhenAddWorkRecord(isDeviceStillState_.load(), isDeviceIdleMode_.load(),
        abilityName, bundleName)) {
        WriteLocationInnerEvent(LBS_REQUEST_FAIL_DETAIL, {"REQ_APP_NAME", request->GetPackageName(), "TRANS_ID",
            request->GetUuid(), "REQ_INFO", request->ToString().c_str(), "ERR_CODE", 
            std::to_string(LOCATION_ERRCODE_DEVICE_IDLE)});
        LBSLOGI(REQUEST_MANAGER, "Enter idle and still status, not add request");
        return false;
    }
    if (!UpdateUsingPermission(request, true)) {
        RequestManager::GetInstance()->ReportLocationError(LOCATING_FAILED_LOCATION_PERMISSION_DENIED, request);
        WriteLocationInnerEvent(LBS_REQUEST_FAIL_DETAIL, {"REQ_APP_NAME", request->GetPackageName(), "REQ_INFO", 
            request->ToString().c_str(), "TRANS_ID", request->GetUuid(), "ERR_CODE", 
            std::to_string(LOCATION_ERRCODE_USING_PERMISSION)});
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
    if (!SaLoadWithStatistic::InitLocationSa(systemAbilityId)) {
        return ;
    }
    sptr<IRemoteObject> remoteObject = CommonUtils::GetRemoteObject(systemAbilityId, CommonUtils::InitDeviceId());
    if (remoteObject == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s: remote obj is nullptr", __func__);
        return;
    }
    LBSLOGI(LOCATOR, "%{public}s: %{public}s workRecord uid_ size %{public}d, time %{public}s",
        __func__, abilityName.c_str(), workRecord.Size(), std::to_string(CommonUtils::GetCurrentTimeMilSec()).c_str());
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
}

sptr<IRemoteObject> RequestManager::GetRemoteObject(std::string abilityName)
{
    sptr<IRemoteObject> remoteObject = nullptr;
    auto locatorAbility = LocatorAbility::GetInstance();
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
    LocatorAbility::GetInstance()->ApplyRequests(1);
}

bool RequestManager::IsUidInProcessing(int32_t uid)
{
    std::unique_lock<ffrt::mutex> lock(runningUidsMutex_);
    auto iter = runningUidMap_.find(uid);
    if (iter == runningUidMap_.end()) {
        return false;
    }
    return true;
}

void RequestManager::UpdateRunningUids(const std::shared_ptr<Request>& request, std::string abilityName, bool isAdd)
{
    std::unique_lock<ffrt::mutex> lock(runningUidsMutex_);
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
            WriteLocationRequestEvent(request->GetPackageName(), abilityName);
            ReportDataToResSched("start", pid, uid);
        }
    } else {
        WriteLocationInnerEvent(REMOVE_REQUEST, {"PackageName", request->GetPackageName(),
                    "abilityName", abilityName, "requestAddress", request->GetUuid()});
        uidCount -= 1;
        if (uidCount == 0) {
            WriteAppLocatingStateEvent("stop", pid, uid);
            ReportDataToResSched("stop", pid, uid);
        }
    }
    if (uidCount > 0) {
        runningUidMap_.insert(std::make_pair(uid, uidCount));
    }
}

void RequestManager::ReportDataToResSched(std::string state, const pid_t pid, const pid_t uid)
{
#ifdef RES_SCHED_SUPPROT
    std::unordered_map<std::string, std::string> payload;
    payload["pid"] = std::to_string(pid);
    payload["uid"] = std::to_string(uid);
    payload["state"] = state;
    uint32_t type = ResourceSchedule::ResType::RES_TYPE_LOCATION_STATUS_CHANGE;
    int64_t value =  ResourceSchedule::ResType::LocationStatus::APP_LOCATION_STATUE_CHANGE;
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, value, payload);
#endif
}

void RequestManager::RegisterLocationErrorCallback(
    sptr<ILocatorCallback> callback, AppIdentity identity)
{
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow)
        LocatorErrCallbackDeathRecipient(identity.GetTokenId()));
    callback->AsObject()->AddDeathRecipient(death);
    std::shared_ptr<LocationErrRequest> locatorErrRequest = std::make_shared<LocationErrRequest>();
    locatorErrRequest->SetUid(identity.GetUid());
    locatorErrRequest->SetPid(identity.GetPid());
    locatorErrRequest->SetLocatorErrCallbackRecipient(death);
    std::unique_lock<ffrt::mutex> lock(locationErrorCallbackMutex_);
    if (locationErrorCallbackMap_.size() <= MAX_LOCATION_ERROR_CALLBACK_NUM) {
        locationErrorCallbackMap_[callback->AsObject()] = locatorErrRequest;
    } else {
        LBSLOGE(LOCATOR, "RegisterLocationErrorCallback num max");
        return;
    }
    LBSLOGD(LOCATOR, "after RegisterLocationErrorCallback, callback size:%{public}zu",
        locationErrorCallbackMap_.size());
}

void RequestManager::UnRegisterLocationErrorCallback(
    sptr<ILocatorCallback> callback)
{
    std::unique_lock<ffrt::mutex> lock(locationErrorCallbackMutex_);
    auto iter = locationErrorCallbackMap_.find(callback->AsObject());
    if (iter != locationErrorCallbackMap_.end()) {
        auto locatorErrorCallback = iter->first;
        auto locatorErrRequest = iter->second;
        locatorErrorCallback->RemoveDeathRecipient(locatorErrRequest->GetLocatorErrCallbackRecipient());
        locationErrorCallbackMap_.erase(iter);
    }
    LBSLOGD(LOCATOR, "after UnRegisterLocationErrorCallback, callback size:%{public}zu",
        locationErrorCallbackMap_.size());
}

void RequestManager::ReportLocationError(const int errorCode, std::shared_ptr<Request> request)
{
    if (errorCode == LOCATING_FAILED_INTERNET_ACCESS_FAILURE) {
        auto locatorCallback = request->GetLocatorCallBack();
        bool isNeedLocation = true;
        if (request->GetRequestConfig() != nullptr) {
            isNeedLocation = request->GetRequestConfig()->GetIsNeedLocation();
        }
        if (locatorCallback != nullptr && isNeedLocation) {
            locatorCallback->OnErrorReport(LocationErrCode::ERRCODE_LOCATING_NETWORK_FAIL);
        } else {
            LBSLOGE(REQUEST_MANAGER, "RequestManager null LocatorCallback");
        }
    }
    std::unique_lock<ffrt::mutex> lock(locationErrorCallbackMutex_);
    for (auto iter : locationErrorCallbackMap_) {
        auto locatorErrRequest = iter.second;
        if (locatorErrRequest == nullptr) {
            continue;
        }
        if (ProxyFreezeManager::GetInstance()->IsProxyPid(locatorErrRequest->GetPid()) ||
            (request->GetUid() != 0 && (request->GetUid() != locatorErrRequest->GetUid()))) {
            continue;
        }
        if (locatorErrRequest->GetLastReportErrcode() != LOCATING_FAILED_DEFAULT &&
            locatorErrRequest->GetLastReportErrcode() == errorCode) {
            continue;
        }
        sptr<ILocatorCallback> locatorErrorCallback = iface_cast<ILocatorCallback>(iter.first);
        if (locatorErrorCallback == nullptr) {
            continue;
        }
        locatorErrorCallback->OnErrorReport(errorCode);
        locatorErrRequest->SetLastReportErrcode(errorCode);
    }
}

void RequestManager::UpdateLocationError(std::shared_ptr<Request> request)
{
    std::unique_lock<ffrt::mutex> lock(locationErrorCallbackMutex_);
    for (auto iter : locationErrorCallbackMap_) {
        auto locatorErrRequest = iter.second;
        if (request->GetUid() != 0 && (request->GetUid() == locatorErrRequest->GetUid())) {
            locatorErrRequest->SetLastReportErrcode(LOCATING_FAILED_DEFAULT);
        }
    }
}

void RequestManager::SyncStillMovementState(bool state)
{
    bool newDeviceState = false;
    bool oldDeviceState = false;
    oldDeviceState = isDeviceStillState_.load() && isDeviceIdleMode_.load();
    isDeviceStillState_.store(state);
    newDeviceState = isDeviceStillState_.load() && isDeviceIdleMode_.load();
    if (newDeviceState != oldDeviceState) {
        HandleRequest();
    }
}

void RequestManager::SyncIdleState(bool state)
{
    bool newDeviceState = false;
    bool oldDeviceState = false;
    oldDeviceState = isDeviceStillState_.load() && isDeviceIdleMode_.load();
    isDeviceIdleMode_.store(state);
    LBSLOGI(REQUEST_MANAGER, "device idle mode change, isDeviceIdleMode_ %{public}d",
        isDeviceIdleMode_.load());
    newDeviceState = isDeviceStillState_.load() && isDeviceIdleMode_.load();
    if (newDeviceState != oldDeviceState) {
        HandleRequest();
    }
}

LocatorErrCallbackDeathRecipient::LocatorErrCallbackDeathRecipient(int32_t tokenId)
{
    tokenId_ = tokenId;
}

LocatorErrCallbackDeathRecipient::~LocatorErrCallbackDeathRecipient()
{
}

void LocatorErrCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    sptr<ILocatorCallback> callback = iface_cast<ILocatorCallback>(remote.promote());
    auto requestManager = RequestManager::GetInstance();
    if (requestManager != nullptr) {
        requestManager->UnRegisterLocationErrorCallback(callback);
        LBSLOGI(REQUEST_MANAGER, "locatorerr callback OnRemoteDied Id = %{public}d", tokenId_);
    }
}

LocationErrRequest::LocationErrRequest()
{
    uid_ = 0;
    pid_ = 0;
    lastReportErrcode_ = LOCATING_FAILED_DEFAULT;
    locatorErrCallbackRecipient_ = nullptr;
}

LocationErrRequest::~LocationErrRequest() {}

pid_t LocationErrRequest::GetUid()
{
    return uid_;
}

void LocationErrRequest::SetUid(pid_t uid)
{
    uid_ = uid;
}

pid_t LocationErrRequest::GetPid()
{
    return pid_;
}

void LocationErrRequest::SetPid(pid_t pid)
{
    pid_ = pid;
}

int32_t LocationErrRequest::GetLastReportErrcode()
{
    return lastReportErrcode_;
}

void LocationErrRequest::SetLastReportErrcode(int32_t lastReportErrcode)
{
    lastReportErrcode_ = lastReportErrcode;
}

void LocationErrRequest::SetLocatorErrCallbackRecipient(const sptr<IRemoteObject::DeathRecipient>& recipient)
{
    locatorErrCallbackRecipient_ = recipient;
}

sptr<IRemoteObject::DeathRecipient> LocationErrRequest::GetLocatorErrCallbackRecipient()
{
    return locatorErrCallbackRecipient_;
}
} // namespace Location
} // namespace OHOS
