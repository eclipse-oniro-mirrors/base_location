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

#include "locatingrequireddatacallbackhost_fuzzer.h"

#include "locating_required_data_callback_host.h"

#include <cstdint>
#include <memory>
#include <vector>

#include "i_locating_required_data_callback.h"
#include "locating_required_data.h"
#include "matching_wlan_info.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
using namespace OHOS::Location;
const int MIN_DATA_LEN = 2;
const int32_t LOCATING_DATA_COUNT_RANGE = 3;
const int32_t FIX_NUMBER_RANGE = 2;

void CheckCallbackHostRemoteDied(const sptr<LocatingRequiredDataCallbackHost>& callbackHost,
    MessageParcel& requestParcel, MessageParcel& reply, MessageOption& option)
{
    callbackHost->SetRemoteDied(true);
    callbackHost->OnRemoteRequest(
        static_cast<uint32_t>(ILocatingRequiredDataCallback::RECEIVE_INFO_EVENT),
        requestParcel, reply, option);
    callbackHost->GetRemoteDied();
    callbackHost->SetRemoteDied(false);
}

void CheckCallbackHostResult(const sptr<LocatingRequiredDataCallbackHost>& callbackHost, const uint8_t* data)
{
    callbackHost->IsRemoteDied();
    callbackHost->SetFixNumber(static_cast<int>(data[0] % FIX_NUMBER_RANGE));
    callbackHost->GetFixNumber();
    callbackHost->IsSingleLocationRequest();
    callbackHost->ClearSingleResult();
    std::vector<std::shared_ptr<LocatingRequiredData>> requiredDataList;
    auto requiredData = std::make_shared<LocatingRequiredData>();
    callbackHost->OnLocatingDataChange(requiredDataList);
    requiredDataList.push_back(requiredData);
    callbackHost->OnLocatingDataChange(requiredDataList);
    callbackHost->SetSingleResult(requiredDataList);
    callbackHost->GetSingleResult();
    std::vector<MatchingWlanInfo> matchingWlanInfos;
    callbackHost->OnMatchingWlanInfoChange(matchingWlanInfos);
}

bool LocatingRequiredDataCallbackHostFuzzerTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < MIN_DATA_LEN)) {
        return false;
    }
    auto callbackHost = sptr<LocatingRequiredDataCallbackHost>(
        new (std::nothrow) LocatingRequiredDataCallbackHost());
    if (callbackHost == nullptr) {
        return false;
    }
    uint32_t code = static_cast<uint32_t>(data[0] %
        (ILocatingRequiredDataCallback::MATCHING_WLAN_INFO_EVENT + 1));
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.ILocatingRequiredDataCallback");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    callbackHost->OnRemoteRequest(code, requestParcel, reply, option);

    MessageParcel structuredParcel;
    structuredParcel.WriteInterfaceToken(u"location.ILocatingRequiredDataCallback");
    structuredParcel.WriteInt32(static_cast<int32_t>(data[1] % LOCATING_DATA_COUNT_RANGE));
    structuredParcel.WriteBuffer(data, size);
    structuredParcel.RewindRead(0);
    callbackHost->OnRemoteRequest(
        static_cast<uint32_t>(ILocatingRequiredDataCallback::RECEIVE_INFO_EVENT),
        structuredParcel, reply, option);

    CheckCallbackHostRemoteDied(callbackHost, structuredParcel, reply, option);
    CheckCallbackHostResult(callbackHost, data);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::MIN_DATA_LEN) {
        return 0;
    }
    OHOS::LocatingRequiredDataCallbackHostFuzzerTest(data, size);
    return 0;
}
