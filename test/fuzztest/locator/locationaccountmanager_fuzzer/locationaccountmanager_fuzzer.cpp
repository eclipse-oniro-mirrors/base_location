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

#include "locationaccountmanager_fuzzer.h"

#include "location_account_manager.h"

#include <cstdint>
#include <vector>

namespace OHOS {
using namespace OHOS::Location;
const int MIN_DATA_LEN = 2;

bool LocationAccountManagerFuzzerTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < MIN_DATA_LEN)) {
        return false;
    }
    LocationAccountManager* accountManager = LocationAccountManager::GetInstance();
    if (accountManager == nullptr) {
        return false;
    }
    int32_t userId = static_cast<int32_t>(data[0]);
    int32_t removedUserId = static_cast<int32_t>(data[1]);
    accountManager->GetActiveUserIds();
    accountManager->OnUserSwitch(userId);
    accountManager->GetActiveUserIds();
    accountManager->OnUserRemove(removedUserId);
    accountManager->GetActiveUserIds();
    accountManager->OnUserSwitch(removedUserId);
    accountManager->OnUserRemove(userId);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::MIN_DATA_LEN) {
        return 0;
    }
    OHOS::LocationAccountManagerFuzzerTest(data, size);
    return 0;
}
