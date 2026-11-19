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

#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_common_event_manager.h"

#include "location_log.h"
#include "securec.h"
#include "system_ability_definition.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "agnss_ni_manager.h"

namespace OHOS {
namespace Location {
constexpr uint32_t MAX_RETRY_TIMES = 3;

GnssCommonEventManager* GnssCommonEventManager::GetInstance()
{
    static GnssCommonEventManager data;
    return &data;
}

GnssCommonEventManager::GnssCommonEventManager()
{}

GnssCommonEventManager::~GnssCommonEventManager()
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    std::unique_lock<ffrt::mutex> lock(statusChangeListenerMutex_);
    if (samgrProxy != nullptr) {
        samgrProxy->UnSubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, statusChangeListener_);
        return;
    }
    if (statusChangeListener_ != nullptr) {
        statusChangeListener_ = nullptr;
    }
}

void GnssCommonEventManager::SubscribeSaStatusChangeListner()
{
    LBSLOGI(GNSS, "GnssCommonEventManager SubscribeSaStatusChangeListner");
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        LBSLOGE(GNSS, "failed to get samgrProxy");
        return;
    }
    std::unique_lock<ffrt::mutex> lock(statusChangeListenerMutex_);
    statusChangeListener_ = new SystemAbilityStatusChangeListener();
    if (statusChangeListener_ == nullptr) {
        LBSLOGE(GNSS, "statusChangeListener_ is nullptr!");
        return;
    }
    int32_t ret = samgrProxy->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, statusChangeListener_);
    if (ret != ERR_OK) {
        LBSLOGE(GNSS, "subscribe systemAbilityId: call manager service failed!");
        return;
    }
}

void GnssCommonEventManager::RegisterAgnssNiEvent()
{
    if (subscriber_) {
        CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    }
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SMS_WAPPUSH_RECEIVE_COMPLETED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SMS_RECEIVE_COMPLETED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriber_ = std::make_shared<GnssCommonEventSubscriber>(subscriberInfo);

    uint32_t count = 0;
    bool result = false;
    while (!result && count <= MAX_RETRY_TIMES) {
        result = CommonEventManager::SubscribeCommonEvent(subscriber_);
        count++;
    }
    if (count > MAX_RETRY_TIMES || !result) {
        LBSLOGE(GNSS, "Failed to subscriber gnss event");
    } else {
        LBSLOGI(GNSS, "success to subscriber gnss event");
    }
}


void GnssCommonEventManager::RegisterNiResponseEvent()
{
    if (niResponseSubscriber_) {
        CommonEventManager::UnSubscribeCommonEvent(niResponseSubscriber_);
    }
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(AGNSS_NI_ACCEPT_EVENT);
    matchingSkills.AddEvent(AGNSS_NI_REJECT_EVENT);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPermission("ohos.permission.PUBLISH_LOCATION_EVENT");
    niResponseSubscriber_ = std::make_shared<GnssCommonEventSubscriber>(subscriberInfo);

    uint32_t count = 0;
    bool result = false;
    while (!result && count <= MAX_RETRY_TIMES) {
        result = CommonEventManager::SubscribeCommonEvent(niResponseSubscriber_);
        count++;
    }
    if (count > MAX_RETRY_TIMES || !result) {
        LBSLOGE(GNSS, "Failed to subscriber ni response event");
    } else {
        LBSLOGI(GNSS, "success to subscriber ni response event");
    }
}

void GnssCommonEventManager::UnRegisterAgnssNiEvent()
{
    if (subscriber_ == nullptr) {
        LBSLOGE(GNSS, "UnRegisterAgnssNiEvent subscriber_ is null");
        return;
    }
    bool result = CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    subscriber_ = nullptr;
    LBSLOGI(GNSS, "unSubscriber gnss event, result = %{public}d", result);
}

void GnssCommonEventManager::UnRegisterNiResponseEvent()
{
    if (niResponseSubscriber_ == nullptr) {
        LBSLOGE(GNSS, "UnRegisterAgnssNiEvent subscriber_ is null");
        return;
    }
    bool result = CommonEventManager::UnSubscribeCommonEvent(niResponseSubscriber_);
    niResponseSubscriber_ = nullptr;
    LBSLOGI(GNSS, "unSubscriber ni response event, result = %{public}d", result);
}

void SystemAbilityStatusChangeListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    LBSLOGI(GNSS, "GnssCommonEventManager systemAbilityId:%{public}d", systemAbilityId);
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        LBSLOGE(GNSS, "systemAbilityId is not COMMON_EVENT_SERVICE_ID");
        return;
    }
    GnssCommonEventManager::GetInstance()->RegisterAgnssNiEvent();
    GnssCommonEventManager::GetInstance()->RegisterNiResponseEvent();
    AGnssNiManager::GetInstance()->Run();
}

void SystemAbilityStatusChangeListener::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    LBSLOGI(GNSS, "GnssCommonEventManager systemAbilityId:%{public}d", systemAbilityId);
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        LBSLOGE(GNSS, "systemAbilityId is not COMMON_EVENT_SERVICE_ID");
        return;
    }
    GnssCommonEventManager::GetInstance()->UnRegisterAgnssNiEvent();
    GnssCommonEventManager::GetInstance()->UnRegisterNiResponseEvent();
}
}  // namespace Location
}  // namespace OHOS
#endif
