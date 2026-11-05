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

#include "commonutils_fuzzer.h"

#include "app_identity.h"
#include "common_utils.h"

namespace OHOS {
    using namespace OHOS::Location;
    const int MIN_DATA_LEN = 8;
    const int EDM_POLICY_COUNT = 4;
    const int LATCH_WAIT_TIME_MS = 1;
    const int RANDOM_RANGE = 256;

    void CheckAbilityConvert(const uint8_t* data, size_t size)
    {
        int index = 0;
        CommonUtils::AbilityConvertToId(GNSS_ABILITY);
        CommonUtils::AbilityConvertToId(NETWORK_ABILITY);
        CommonUtils::AbilityConvertToId(PASSIVE_ABILITY);
        CommonUtils::AbilityConvertToId(FUSED_ABILITY);
        CommonUtils::AbilityConvertToId(GEO_ABILITY);
        std::string unknownAbility(reinterpret_cast<const char*>(data), size);
        CommonUtils::AbilityConvertToId(unknownAbility);
        CommonUtils::GetCapability(GNSS_ABILITY);
        CommonUtils::GetCapability(unknownAbility);
        CommonUtils::GetCapabilityToString(NETWORK_ABILITY, static_cast<uint32_t>(data[index++]));
        CommonUtils::GetLabel(GNSS_ABILITY);
        CommonUtils::GetLabel(unknownAbility);
    }

    void CheckMathAndTime(const uint8_t* data)
    {
        int index = 0;
        double latitudeA = static_cast<double>(data[index++] % RANDOM_RANGE);
        double longitudeA = static_cast<double>(data[index++] % RANDOM_RANGE);
        double latitudeB = static_cast<double>(data[index++] % RANDOM_RANGE);
        double longitudeB = static_cast<double>(data[index++] % RANDOM_RANGE);
        CommonUtils::DoubleEqual(latitudeA, latitudeB);
        CommonUtils::CalDistance(latitudeA, longitudeA, latitudeB, longitudeB);
        CommonUtils::DoubleRandom(latitudeA, latitudeB);
        int minRandom = static_cast<int>(data[index++]);
        int maxRandom = static_cast<int>(data[index++]);
        CommonUtils::IntRandom(minRandom, maxRandom);
        CommonUtils::GetCurrentTime();
        CommonUtils::GetCurrentTimeMilSec();
        CommonUtils::GetCurrentTimeStamp();
    }

    void CheckStringParse(const uint8_t* data, size_t size)
    {
        std::string input(reinterpret_cast<const char*>(data), size);
        CommonUtils::Split(input, ",");
        CommonUtils::isValidInteger(input);
        uint8_t mac[MAC_LEN] = {0};
        CommonUtils::GetMacArray(input, mac);
        CommonUtils::ConvertStringToDigit(input);
        int32_t digitRet = 0;
        CommonUtils::ConvertStringToDigit(input, digitRet);
        CommonUtils::IsValidForStoull(input, size);
        CommonUtils::IsStrValidForStoi(input);
        CommonUtils::IsExistFile(input);
        CommonUtils::GenerateUuid();
    }

    void CheckUserAndConfig(const uint8_t* data)
    {
        int index = 0;
        std::vector<int> activeIds;
        CommonUtils::GetActiveUserIds(activeIds);
        CommonUtils::GetAllUserId(activeIds);
        int userId = 0;
        CommonUtils::GetCurrentUserId(userId);
        CommonUtils::GetUserIdByUid(static_cast<int>(data[index++]));
        std::string bundleName;
        CommonUtils::CheckAppForUser(static_cast<int32_t>(data[index++]), bundleName);
        CommonUtils::CheckAppForUsers(static_cast<int32_t>(data[index++]), activeIds, bundleName);
        AppIdentity identity;
        identity.SetUid(static_cast<pid_t>(data[index++]));
        identity.SetBundleName(bundleName);
        CommonUtils::IsAppBelongActiveAccounts(identity, activeIds);
        CommonUtils::IsAppBelongCurrentAccount(identity);
        std::string value;
        CommonUtils::GetConfigFromJson(GNSS_ABILITY, value);
        std::string parameterValue;
        CommonUtils::GetStringParameter("const.product.name", parameterValue);
        std::string edmName;
        CommonUtils::GetEdmPolicy(edmName);
        CommonUtils::ErrCodeToLocationErrCode(static_cast<ErrCode>(data[index++] % EDM_POLICY_COUNT));
    }

    void CheckCountDownLatch(const uint8_t* data)
    {
        int index = 0;
        CountDownLatch latch;
        latch.SetCount(static_cast<int>(data[index++]));
        latch.GetCount();
        latch.Wait(LATCH_WAIT_TIME_MS);
        latch.CountDown();
    }

    bool CommonUtilsFuzzTest(const uint8_t* data, size_t size)
    {
        if (data == nullptr || size < MIN_DATA_LEN) {
            return false;
        }
        CheckAbilityConvert(data, size);
        CheckMathAndTime(data);
        CheckStringParse(data, size);
        CheckUserAndConfig(data);
        CheckCountDownLatch(data);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::CommonUtilsFuzzTest(data, size);
    return 0;
}
