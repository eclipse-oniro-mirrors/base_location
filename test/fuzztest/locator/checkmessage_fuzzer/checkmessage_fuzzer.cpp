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


#include "checkmessage_fuzzer.h"

#include "accesstoken_kit.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "message_option.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

#define private public
#include "locator_ability.h"
#undef private

namespace OHOS {
using namespace OHOS::Location;
const int32_t MAX_MEM_SIZE = 4 * 1024 * 1024;
const int32_t U32DATA_SIZE = 4;
const int32_t U64DATA_SIZE = 8;
const int32_t MIN_SIZE_NUM = 8;


uint32_t GetU32Data(const char* ptr)
{
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
}

uint64_t GetU64Data(const char* ptr)
{
    uint64_t u64data = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
    return (u64data << 32) | (ptr[4] << 24) | (ptr[5] << 16) | (ptr[6] << 8) | (ptr[7]);
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

bool CheckMessageFuzzTest(const char* data, size_t size)
{
    MessageParcel reply;
    AppIdentity identity;
    int offset = 0;
    if (size <= 32) {
        return true;
    }
    identity.SetPid(static_cast<pid_t>(GetU64Data(data)));
    identity.SetUid(static_cast<pid_t>(GetU64Data(data + (offset += U64DATA_SIZE))));
    identity.SetTokenId(GetU32Data(data + (offset += U64DATA_SIZE)));
    identity.SetTokenIdEx(GetU64Data(data + (offset += U32DATA_SIZE)));
    identity.SetFirstTokenId(GetU32Data(data + (offset += U64DATA_SIZE)));
    identity.SetBundleName(data + (offset += U32DATA_SIZE));

    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    locatorAbility->CheckLocationPermission(reply, identity);
    locatorAbility->CheckSettingsPermission(reply, identity);
    locatorAbility->CheckPreciseLocationPermissions(reply, identity);
    locatorAbility->CheckLocationSwitchState(reply);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::MIN_SIZE_NUM) {
        return 0;
    }
    char* ch = OHOS::ParseData(data, size);
    if (ch != nullptr) {
        OHOS::CheckMessageFuzzTest(ch, size);
        free(ch);
        ch = nullptr;
    }
    return 0;
}

