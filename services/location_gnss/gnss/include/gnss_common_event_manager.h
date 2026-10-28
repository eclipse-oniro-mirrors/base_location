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

#ifndef GNSS_COMMON_EVENT_MANAGER_H
#define GNSS_COMMON_EVENT_MANAGER_H
#ifdef FEATURE_GNSS_SUPPORT

#include <mutex>
#include "ffrt.h"
#include "gnss_common_event_subscriber.h"
#include "subability_common.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace Location {

class SystemAbilityStatusChangeListener : public SystemAbilityStatusChangeStub {
public:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
};

class GnssCommonEventManager {
public:
    GnssCommonEventManager();
    ~GnssCommonEventManager();
    static GnssCommonEventManager* GetInstance();

    void SubscribeSaStatusChangeListner();
    void RegisterAgnssNiEvent();
    void UnRegisterAgnssNiEvent();
    void RegisterNiResponseEvent();
    void UnRegisterNiResponseEvent();

private:
    std::shared_ptr<GnssCommonEventSubscriber> subscriber_ = nullptr;
    std::shared_ptr<GnssCommonEventSubscriber> niResponseSubscriber_ = nullptr;
    sptr<SystemAbilityStatusChangeListener> statusChangeListener_ = nullptr;
    ffrt::mutex statusChangeListenerMutex_;
};
} // namespace Location
} // namespace OHOS

#endif // FEATURE_GNSS_SUPPORT
#endif // GNSS_COMMON_EVENT_MANAGER_H
