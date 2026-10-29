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

#include "appbackgroundstatusmanager_fuzzer.h"

#include "app_background_status_manager.h"

#include <cstdint>
#include <string>

namespace OHOS {
using namespace OHOS::Location;
const int MIN_DATA_LEN = 9;
const int32_t APP_BACKGROUND_STATUS_COUNT = 4;

bool AppBackgroundStatusManagerFuzzerTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < MIN_DATA_LEN)) {
        return false;
    }
    AppBackgroundStatusManager* appBackgroundStatusManager = AppBackgroundStatusManager::GetInstance();
    if (appBackgroundStatusManager == nullptr) {
        return false;
    }
    int index = 0;
    int32_t uid = data[index++];
    std::string bundleName(reinterpret_cast<const char*>(data), size);
    appBackgroundStatusManager->UpdateBackgroundAppStatues(
        uid, static_cast<int32_t>(data[index++] % APP_BACKGROUND_STATUS_COUNT));
    appBackgroundStatusManager->IsAppBackground(bundleName);
    appBackgroundStatusManager->IsAppBackground(uid, bundleName);
    appBackgroundStatusManager->UpdateBackgroundAppStatues(
        uid, static_cast<int32_t>(data[index++] % APP_BACKGROUND_STATUS_COUNT));
    appBackgroundStatusManager->IsAppBackground(uid, bundleName);
    pid_t taskUid = static_cast<pid_t>(data[index++]);
    pid_t taskPid = static_cast<pid_t>(data[index++]);
    appBackgroundStatusManager->IsAppInLocationContinuousTasks(taskUid, taskPid);
    uint32_t formTokenId = static_cast<uint32_t>(data[index++]);
    uint64_t formTokenIdEx = static_cast<uint64_t>(data[index++]);
    appBackgroundStatusManager->IsAppHasFormVisible(formTokenId, formTokenIdEx);
    pid_t processId = static_cast<pid_t>(data[index++]);
    uint32_t processTokenId = static_cast<uint32_t>(data[index]);
    bool defaultRunning = (data[index] & 1) == 1;
    appBackgroundStatusManager->IsProcessRunning(processId, processTokenId, defaultRunning);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::MIN_DATA_LEN) {
        return 0;
    }
    OHOS::AppBackgroundStatusManagerFuzzerTest(data, size);
    return 0;
}
