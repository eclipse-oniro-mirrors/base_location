/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_LOCATION_SA_LOAD_MANAGER_H
#define OHOS_LOCATION_SA_LOAD_MANAGER_H

#include <condition_variable>
#include <singleton.h>
#include "iremote_object.h"
#include "constant_definition.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
namespace Location {
class LocationSaLoadCallback : public SystemAbilityLoadCallbackStub {
public:
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject) override;
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
};

class LocationSaLoadManager {
public:
    LocationSaLoadManager();
    ~LocationSaLoadManager();
    LocationErrCode LoadLocationSa(int32_t systemAbilityId);
    LocationErrCode UnloadLocationSa(int32_t systemAbilityId);
    void LoadSystemAbilitySuccess();
    void LoadSystemAbilityFail();
    static bool CheckIfSystemAbilityAvailable(int32_t systemAbilityId);
    static bool InitLocationSa(int32_t systemAbilityId);
    static bool UnInitLocationSa(int32_t systemAbilityId);
    static LocationSaLoadManager* GetInstance();

private:
    void InitLoadState();
    LocationErrCode WaitLoadStateChange(int32_t systemAbilityId);

    std::condition_variable locatorCon_;
    std::mutex locatorMutex_;
    bool state_ = false;
};

class SaLoadWithStatistic {
public:
    SaLoadWithStatistic();
    ~SaLoadWithStatistic();

    static bool InitLocationSa(int32_t systemAbilityId);
    static bool UnInitLocationSa(int32_t systemAbilityId);
};
}  // namespace Location
}  // namespace OHOS
#endif // OHOS_LOCATION_SA_LOAD_MANAGER_H
