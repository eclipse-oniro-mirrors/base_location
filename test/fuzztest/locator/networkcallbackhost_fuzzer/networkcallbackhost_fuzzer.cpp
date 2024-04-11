/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "networkcallbackhost_fuzzer.h"

#include "accesstoken_kit.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "message_option.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "locator_ability.h"

#ifdef FEATURE_NETWORK_SUPPORT
#include "network_callback_host.h"
#endif
#include "permission_manager.h"

namespace OHOS {
using namespace OHOS::Location;
const int32_t MAX_MEM_SIZE = 4 * 1024 * 1024;
const int32_t CODE_MAX = 50;
const int32_t CODE_MIN = 1;
const int32_t MIN_SIZE_NUM = 4;
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
        .processName = "GnssAbility_FuzzTest",
        .aplStr = "system_basic",
    };
    auto tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

uint32_t GetU32Data(const char* ptr)
{
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
}

char* ParseData(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return nullptr;
    }

    if (size > MAX_MEM_SIZE) {
        return nullptr;
    }

    char* ch = (char *)malloc(size + 1);
    if (ch == nullptr) {
        return nullptr;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        free(ch);
        ch = nullptr;
        return nullptr;
    }
    return ch;
}

#ifdef FEATURE_NETWORK_SUPPORT
bool NetworkCallbackHostFuzzTest(const char* data, size_t size)
{
    uint32_t code = GetU32Data(data) % (CODE_MAX - CODE_MIN + 1) + CODE_MIN;
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.ILocatorCallback");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;
    auto callback = sptr<NetworkCallbackHost>(new (std::nothrow) NetworkCallbackHost());
    callback->OnRemoteRequest(code, requestParcel, reply, option);
    return true;
}
#endif
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::MockNativePermission();
    if (size < OHOS::MIN_SIZE_NUM) {
        return 0;
    }
    char* ch = OHOS::ParseData(data, size);
    if (ch != nullptr) {
#ifdef FEATURE_NETWORK_SUPPORT
        OHOS::NetworkCallbackHostFuzzTest(ch, size);
#endif
        free(ch);
        ch = nullptr;
    }
    return 0;
}

