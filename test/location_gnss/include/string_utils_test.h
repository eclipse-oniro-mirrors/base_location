/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef GNSS_EVENT_CALLBACK_TEST_H
#define GNSS_EVENT_CALLBACK_TEST_H
#ifdef FEATURE_GNSS_SUPPORT

#include <gtest/gtest.h>

namespace OHOS {
namespace Location {
class StringUtilsTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};
} // namespace Location
} // namespace OHOS
#endif // FEATURE_GNSS_SUPPORT
#endif // GNSS_EVENT_CALLBACK_TEST_H
