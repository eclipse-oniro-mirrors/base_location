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

#include "location_config_manager_test.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "location_config_manager.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
const int STATE_UNKNOWN = -1;
const int PRIVACY_TYPE_INVALID_LEFT = -1;
const int PRIVACY_TYPE_INVALID_RIGHT = 3;
void LocationConfigManagerTest::SetUp()
{
}

void LocationConfigManagerTest::TearDown()
{
}

HWTEST_F(LocationConfigManagerTest, LocationConfigManagerInitTest001, TestSize.Level1)
{
    EXPECT_EQ(0, LocationConfigManager::GetInstance().Init());
}

HWTEST_F(LocationConfigManagerTest, LocationConfigManagerSwitchStateTest001, TestSize.Level1)
{
    EXPECT_EQ(0,
        LocationConfigManager::GetInstance().SetLocationSwitchState(STATE_CLOSE));
    EXPECT_EQ(STATE_CLOSE,
        LocationConfigManager::GetInstance().GetLocationSwitchState());

    EXPECT_EQ(0,
        LocationConfigManager::GetInstance().SetLocationSwitchState(STATE_OPEN));
    EXPECT_EQ(STATE_OPEN,
        LocationConfigManager::GetInstance().GetLocationSwitchState());

    EXPECT_EQ(-1,
        LocationConfigManager::GetInstance().SetLocationSwitchState(STATE_UNKNOWN));
    EXPECT_EQ(STATE_OPEN,
        LocationConfigManager::GetInstance().GetLocationSwitchState());
}

HWTEST_F(LocationConfigManagerTest, LocationConfigManagerPrivacyTypeStateTest001, TestSize.Level1)
{
    EXPECT_EQ(REPLY_CODE_EXCEPTION,
        LocationConfigManager::GetInstance().SetPrivacyTypeState(PRIVACY_TYPE_INVALID_LEFT, true));
    EXPECT_EQ(false,
        LocationConfigManager::GetInstance().GetPrivacyTypeState(PRIVACY_TYPE_INVALID_LEFT));
    
    EXPECT_EQ(REPLY_CODE_EXCEPTION,
        LocationConfigManager::GetInstance().SetPrivacyTypeState(PRIVACY_TYPE_INVALID_RIGHT, true));
    EXPECT_EQ(false,
        LocationConfigManager::GetInstance().GetPrivacyTypeState(PRIVACY_TYPE_INVALID_RIGHT));
    
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        LocationConfigManager::GetInstance().SetPrivacyTypeState(PRIVACY_TYPE_STARTUP, true));
    EXPECT_EQ(true,
        LocationConfigManager::GetInstance().GetPrivacyTypeState(PRIVACY_TYPE_STARTUP));
    
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        LocationConfigManager::GetInstance().SetPrivacyTypeState(PRIVACY_TYPE_CORE_LOCATION, false));
    EXPECT_EQ(false,
        LocationConfigManager::GetInstance().GetPrivacyTypeState(PRIVACY_TYPE_CORE_LOCATION));
}

HWTEST_F(LocationConfigManagerTest, LocationConfigManagerIsExistFileTest001, TestSize.Level1)
{
    EXPECT_EQ(false,
        LocationConfigManager::GetInstance().IsExistFile("invalid_path"));
    int userId = 0;
    CommonUtils::GetCurrentUserId(userId);
    std::string configPath = LOCATION_DIR + SWITCH_CONFIG_NAME + "_" + std::to_string(userId) + ".conf";
    EXPECT_EQ(false,
        LocationConfigManager::GetInstance().IsExistFile("/wrongpath" + configPath));
    EXPECT_EQ(true,
        LocationConfigManager::GetInstance().IsExistFile(configPath));
}

HWTEST_F(LocationConfigManagerTest, LocationConfigManagerCreateFileTest001, TestSize.Level1)
{
    EXPECT_EQ(true, LocationConfigManager::GetInstance().CreateFile("filename", "filedata"));
}

HWTEST_F(LocationConfigManagerTest, LocationConfigManagerPrivacyTypeConfigTest001, TestSize.Level1)
{
    EXPECT_NE("", LocationConfigManager::GetInstance().GetPrivacyTypeConfigPath(PRIVACY_TYPE_INVALID_LEFT));
}
}  // namespace Location
}  // namespace OHOS