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

#ifndef LOCATION_DATA_RDB_OBSERVER_H
#define LOCATION_DATA_RDB_OBSERVER_H

#include "data_ability_observer_stub.h"

namespace OHOS {
namespace Location {
class LocationDataRdbObserver : public AAFwk::DataAbilityObserverStub {
public:
    explicit LocationDataRdbObserver();
    ~LocationDataRdbObserver();
    void OnChange() override;
private:
    void HandleSwitchStateChanged();
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_DATA_RDB_OBSERVER_H
