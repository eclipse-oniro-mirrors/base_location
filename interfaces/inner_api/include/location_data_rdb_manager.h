/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef LOCATION_DATA_RDB_MANAGER_H
#define LOCATION_DATA_RDB_MANAGER_H
#include <unistd.h>
#include <string>
#include <mutex>

#include "constant_definition.h"

namespace OHOS {
namespace Location {
const std::string LOCATION_DATA_COLUMN_ENABLE = "location_switch_enable";
const std::string LOCATION_WORKING_STATE = "location_working_state";
class LocationDataRdbManager {
public:
    static std::string GetLocationDataUri(std::string key);
    static int QuerySwitchState();
    static LocationErrCode SetSwitchStateToDb(int modeValue);
    static bool SetLocationWorkingState(int32_t state);
    static bool GetLocationWorkingState(int32_t& state);
    static int GetSwitchMode();
    static bool SetSwitchStateToSyspara(int value);
    static bool ClearSwitchMode();
    static std::string GetLocationDataSecureUri(std::string key);
    static bool SetLocationEnhanceStatus(int32_t state);
    static bool GetLocationEnhanceStatus(int32_t& state);
    static void SyncSwitchStatus();
private:
    static std::mutex locationSwitchModeMutex_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_DATA_RDB_MANAGER_H
