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

#include "countrycodemanager_fuzzer.h"

namespace OHOS {
    using namespace OHOS::Location;

    bool CountrycodeManagerFuzzTest(const uint8_t* data, size_t size)
    {
        std::shared_ptr<CountryCodeManager> countryCodeManager =
            std::make_shared<CountryCodeManager>();
        countryCodeManager->GetIsoCountryCode();
        pid_t uid = *reinterpret_cast<const pid_t*>(data);
        countryCodeManager->UnregisterCountryCodeCallback(nullptr);
        countryCodeManager->RegisterCountryCodeCallback(nullptr, uid);
        countryCodeManager->ReSubscribeEvent();
        countryCodeManager->ReUnsubscribeEvent();
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::CountrycodeManagerFuzzTest(data, size);
    return 0;
}