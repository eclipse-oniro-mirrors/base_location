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

#include "reportmanager_fuzzer.h"

#include "i_locator_callback.h"
#include "location.h"
#include "locator_callback_napi.h"
#include "report_manager.h"
#include "request.h"

namespace OHOS {
    using namespace OHOS::Location;
    const int MIN_DATA_LEN = 6;
    bool ReportManagerFuzzerTest(const uint8_t* data, size_t size)
    {
        if (size < MIN_DATA_LEN) {
            return true;
        }
        auto reportManager =
            ReportManager::GetInstance();
        if (reportManager == nullptr) {
            return false;
        }
        std::unique_ptr<OHOS::Location::Location> location =
            std::make_unique<OHOS::Location::Location>();
        auto locatorCallbackHostForTest =
            sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
        sptr<ILocatorCallback> locatorCallback =
            sptr<ILocatorCallback>(locatorCallbackHostForTest);
        int index = 0;
        int type = data[index++];
        int result = data[index++];
        reportManager->ReportRemoteCallback(locatorCallback, type, result);
        std::shared_ptr<Request> request = std::make_shared<Request>();
        reportManager->ResultCheck(location, request);
        reportManager->UpdateCacheLocation(location, "gps");
        reportManager->GetLastLocation();
        request->SetUid(data[index++]);
        request->SetTokenId(data[index++]);
        request->SetFirstTokenId(data[index++]);
        request->SetTokenIdEx(data[index++]);
        reportManager->GetPermittedLocation(request, location);
        reportManager->UpdateRandom();
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::ReportManagerFuzzerTest(data, size);
    return 0;
}