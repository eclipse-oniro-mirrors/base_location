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

#include "selfrequestmanager_fuzzer.h"

#include "self_request_manager.h"

#include <cstdint>
#include <memory>
#include <string>

#include "accesstoken_kit.h"
#include "location.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "locator_ability.h"
#include "permission_manager.h"
#include "request.h"
#include "request_config.h"

namespace OHOS {
using namespace OHOS::Location;
const int MIN_DATA_LEN = 7;
const int32_t LOCATION_PERM_NUM = 4;

void MockNativePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "SelfRequestManager_FuzzTest",
        .aplStr = "system_basic",
    };
    auto tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    LocatorAbility::GetInstance()->EnableAbility(true);
}

bool SelfRequestManagerFuzzerTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < MIN_DATA_LEN)) {
        return false;
    }
    SelfRequestManager* selfRequestManager = SelfRequestManager::GetInstance();
    if (selfRequestManager == nullptr) {
        return false;
    }
    int index = 0;
    auto requestConfig = std::make_unique<RequestConfig>();
    std::shared_ptr<Request> request = std::make_shared<Request>();
    request->SetRequestConfig(*requestConfig);
    std::string packageName(reinterpret_cast<const char*>(data), size);
    request->SetPackageName(packageName);
    request->SetUid(data[index++]);
    request->SetPid(data[index++]);
    request->SetTokenId(data[index++]);
    request->SetTokenIdEx(data[index++]);
    request->SetFirstTokenId(data[index++]);
    request->SetRequesting((data[index++] & 1) == 1);

    selfRequestManager->StartSelfRequest(request);
    selfRequestManager->StopSelfRequest();
    selfRequestManager->ProcessStartSelfRequestEvent(request);
    selfRequestManager->ProcessStopSelfRequestEvent();
    selfRequestManager->ProcessStartSelfRequestEvent(nullptr);

    auto callback = sptr<SelfRequestManager::mLocatorCallback>(
        new (std::nothrow) SelfRequestManager::mLocatorCallback());
    if (callback == nullptr) {
        return false;
    }
    auto location = std::make_unique<OHOS::Location::Location>();
    callback->OnLocationReport(location);
    callback->OnLocatingStatusChange(data[0]);
    callback->OnErrorReport(data[1]);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::MIN_DATA_LEN) {
        return 0;
    }
    OHOS::MockNativePermission();
    OHOS::SelfRequestManagerFuzzerTest(data, size);
    return 0;
}
