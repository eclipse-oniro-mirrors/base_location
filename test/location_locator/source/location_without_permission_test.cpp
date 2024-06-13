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

#include "location_without_permission_test.h"

#include "accesstoken_kit.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#ifdef FEATURE_GNSS_SUPPORT
#include "cached_locations_callback_host.h"
#endif
#include "constant_definition.h"
#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_address.h"
#endif
#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_status_callback_host.h"
#include "i_cached_locations_callback.h"
#endif
#include "i_locator_callback.h"
#include "location_switch_callback_host.h"
#include "locator.h"
#include "location_log.h"
#include "locator_callback_proxy.h"
#include "locator_impl.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "nmea_message_callback_host.h"
#endif
#include "request_config.h"
#include "permission_manager.h"
#include "geofence_request.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
const int32_t LOCATION_WITHOUT_PERM = 1;
void LocationWithoutPermissionTest::SetUp()
{
    MockNativePermission();
}

void LocationWithoutPermissionTest::TearDown()
{
}

void LocationWithoutPermissionTest::MockNativePermission()
{
    const char *perms[] = {
        ACCESS_BACKGROUND_LOCATION.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_WITHOUT_PERM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "LocatorPermissionTest",
        .aplStr = "system_basic",
    };
    tokenId_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(LocationWithoutPermissionTest, LocatorWithoutSettingsPermission001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationWithoutPermissionTest, LocatorWithoutSettingsPermission001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutSettingsPermission001 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    auto switchCallbackHost =
        sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
    EXPECT_NE(nullptr, switchCallbackHost);
    EXPECT_EQ(true, locatorImpl->RegisterSwitchCallback(switchCallbackHost->AsObject(), 1000));
    EXPECT_EQ(true, locatorImpl->UnregisterSwitchCallback(switchCallbackHost->AsObject()));
    locatorImpl->EnableAbility(true);
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutSettingsPermission001 end");
}

HWTEST_F(LocationWithoutPermissionTest, LocatorWithoutLocationPermission001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationWithoutPermissionTest, LocatorWithoutLocationPermission001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutLocationPermission001 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);

    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_ACCURACY);
    sptr<ILocatorCallback> callbackStub = new (std::nothrow) LocatorCallbackStub();
    locatorImpl->StartLocating(requestConfig, callbackStub);
    locatorImpl->StopLocating(callbackStub);

    EXPECT_EQ(nullptr, locatorImpl->GetCachedLocation());
#ifdef FEATURE_GEOCODE_SUPPORT
    locatorImpl->IsGeoServiceAvailable();
    MessageParcel request001;
    std::list<std::shared_ptr<GeoAddress>> geoAddressList001;
    locatorImpl->GetAddressByCoordinate(request001, geoAddressList001);
    MessageParcel request002;
    std::list<std::shared_ptr<GeoAddress>> geoAddressList002;
    locatorImpl->GetAddressByLocationName(request002, geoAddressList002);
#endif
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutLocationPermission001 end");
}

HWTEST_F(LocationWithoutPermissionTest, LocatorWithoutLocationPermission002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationWithoutPermissionTest, LocatorWithoutLocationPermission002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutLocationPermission002 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    bool state = false;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl->IsLocationEnabledV9(state));
#ifdef FEATURE_GNSS_SUPPORT
    auto gnssCallbackHost =
        sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
    EXPECT_NE(nullptr, gnssCallbackHost);
    EXPECT_EQ(true, locatorImpl->RegisterGnssStatusCallback(gnssCallbackHost->AsObject(), 1000));
    EXPECT_EQ(true, locatorImpl->UnregisterGnssStatusCallback(gnssCallbackHost->AsObject()));

    auto nmeaCallbackHost =
        sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
    EXPECT_NE(nullptr, nmeaCallbackHost);
    EXPECT_EQ(true, locatorImpl->RegisterNmeaMessageCallback(nmeaCallbackHost->AsObject(), 1000));
    EXPECT_EQ(true, locatorImpl->UnregisterNmeaMessageCallback(nmeaCallbackHost->AsObject()));
#endif
    EXPECT_EQ(false, locatorImpl->IsLocationPrivacyConfirmed(1));
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->SetLocationPrivacyConfirmStatus(1, true));
#ifdef FEATURE_GNSS_SUPPORT
    if (state) {
        EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->FlushCachedGnssLocations());
    } else {
        EXPECT_EQ(ERRCODE_SWITCH_OFF, locatorImpl->FlushCachedGnssLocations());
    }
#endif
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutLocationPermission002 end");
}

HWTEST_F(LocationWithoutPermissionTest, LocatorWithoutLocationPermission003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationWithoutPermissionTest, LocatorWithoutLocationPermission003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutLocationPermission003 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
#ifdef FEATURE_GNSS_SUPPORT
    locatorImpl->GetCachedGnssLocationsSize();

    auto cachedLocationsCallbackHost =
        sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
    EXPECT_NE(nullptr, cachedLocationsCallbackHost);
    auto cachedCallback = sptr<ICachedLocationsCallback>(cachedLocationsCallbackHost);
    EXPECT_NE(nullptr, cachedCallback);
    auto request = std::make_unique<CachedGnssLocationsRequest>();
    EXPECT_NE(nullptr, request);
    request->reportingPeriodSec = 10;
    request->wakeUpCacheQueueFull = true;
    locatorImpl->RegisterCachedLocationCallback(request, cachedCallback);
    locatorImpl->UnregisterCachedLocationCallback(cachedCallback);
#endif
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutLocationPermission003 end");
}

HWTEST_F(LocationWithoutPermissionTest, LocatorWithoutSettingsPermissionV9001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationWithoutPermissionTest, LocatorWithoutSettingsPermissionV9001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutSettingsPermissionV9001 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->EnableAbilityV9(true));
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->SetLocationPrivacyConfirmStatusV9(1, true));
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutSettingsPermissionV9001 end");
}

HWTEST_F(LocationWithoutPermissionTest, LocatorWithoutLocationPermissionV9001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationWithoutPermissionTest, LocatorWithoutLocationPermissionV9001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutLocationPermissionV9001 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);

    bool state = false;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl->IsLocationEnabledV9(state));

    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_ACCURACY);
    sptr<ILocatorCallback> callbackStub = new (std::nothrow) LocatorCallbackStub();
    if (state) {
        EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->StartLocatingV9(requestConfig, callbackStub));
    } else {
        EXPECT_EQ(ERRCODE_SWITCH_OFF, locatorImpl->StartLocatingV9(requestConfig, callbackStub));
    }
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->StopLocatingV9(callbackStub));

    std::unique_ptr loc = std::make_unique<Location>();
    if (state) {
        EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->GetCachedLocationV9(loc));
    } else {
        EXPECT_EQ(ERRCODE_SWITCH_OFF, locatorImpl->GetCachedLocationV9(loc));
    }
    EXPECT_EQ(nullptr, loc);
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutLocationPermissionV9001 end");
}

HWTEST_F(LocationWithoutPermissionTest, LocatorWithoutLocationPermissionV9002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationWithoutPermissionTest, LocatorWithoutLocationPermissionV9002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutLocationPermissionV9002 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);

    bool state = false;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl->IsLocationEnabledV9(state));
#ifdef FEATURE_GNSS_SUPPORT
    auto gnssCallbackHost =
        sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
    EXPECT_NE(nullptr, gnssCallbackHost);
    if (state) {
        EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->RegisterGnssStatusCallbackV9(gnssCallbackHost->AsObject()));
    } else {
        EXPECT_EQ(ERRCODE_SWITCH_OFF, locatorImpl->RegisterGnssStatusCallbackV9(gnssCallbackHost->AsObject()));
    }
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->UnregisterGnssStatusCallbackV9(gnssCallbackHost->AsObject()));

    auto nmeaCallbackHost =
        sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
    EXPECT_NE(nullptr, nmeaCallbackHost);
    if (state) {
        EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->RegisterNmeaMessageCallbackV9(nmeaCallbackHost->AsObject()));
    } else {
        EXPECT_EQ(ERRCODE_SWITCH_OFF, locatorImpl->RegisterNmeaMessageCallbackV9(nmeaCallbackHost->AsObject()));
    }
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->UnregisterNmeaMessageCallbackV9(nmeaCallbackHost->AsObject()));
    if (state) {
        EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->FlushCachedGnssLocationsV9());
    } else {
        EXPECT_EQ(ERRCODE_SWITCH_OFF, locatorImpl->FlushCachedGnssLocationsV9());
    }
#endif
    std::set<int> pidList;
    pidList.insert(1000);
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->ProxyForFreeze(pidList, false));
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->ResetAllProxy());
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutLocationPermissionV9002 end");
}
}  // namespace Location
}  // namespace OHOS