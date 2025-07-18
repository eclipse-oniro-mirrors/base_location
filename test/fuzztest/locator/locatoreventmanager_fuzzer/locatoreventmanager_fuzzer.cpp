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

#include "locatoreventmanager_fuzzer.h"

#include "request.h"

namespace OHOS {
    using namespace OHOS::Location;
    bool LocatorEventManagerFuzzerTest(const uint8_t* data, size_t size)
    {
        if (size == 0) {
            return true;
        }
        auto locatorDftManager =
            LocatorDftManager::GetInstance();
        if (locatorDftManager == nullptr) {
            return false;
        }
        int index = 0;
        locatorDftManager->IpcCallingErr(data[index++]);
        std::shared_ptr<Request> request = std::make_shared<Request>();
        locatorDftManager->LocationSessionStart(request);
        locatorDftManager->DistributionSessionStart();
        locatorDftManager->DistributionDisconnect();
        locatorDftManager->SendDistributionDailyCount();
        locatorDftManager->SendRequestDailyCount();
        locatorDftManager->GetTopRequest();
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::LocatorEventManagerFuzzerTest(data, size);
    return 0;
}