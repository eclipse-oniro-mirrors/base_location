/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include "location_mock_ipc.h"
#include <map>
#define private public
#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_ability.h"
#include "gnss_ability_proxy.h"
#include "gnss_ability_skeleton.h"
#endif
#ifdef FEATURE_NETWORK_SUPPORT
#include "network_ability.h"
#include "network_ability_proxy.h"
#include "network_ability_skeleton.h"
#endif
#ifdef FEATURE_PASSIVE_SUPPORT
#include "passive_ability.h"
#include "passive_ability_proxy.h"
#include "passive_ability_skeleton.h"
#endif
#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_convert_service.h"
#include "geo_convert_proxy.h"
#include "geo_convert_skeleton.h"
#endif
#undef private
#include "locator_ability.h"
#include "locator_proxy.h"
#include "locationhub_ipc_interface_code.h"
#include "location_log.h"
#include "constant_definition.h"
#include "subability_common.h"

#include "iremote_broker.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Location {

std::string g_gnssIpcCode = "GNSS";
std::string g_networkIpcCode = "NETWORK";
std::string g_passiveIpcCode = "PASSIVE";
std::string g_geoIpcCode = "GEO_CONVERT";
const int32_t WAIT_RESPONSE_SEC = 2;

void LocationMockIpcTest::SetUp()
{
#ifdef FEATURE_GNSS_SUPPORT
    ipcMap_[static_cast<uint32_t>(GnssInterfaceCode::SEND_LOCATION_REQUEST)] = g_gnssIpcCode;
    ipcMap_[static_cast<uint32_t>(GnssInterfaceCode::SET_MOCKED_LOCATIONS)] = g_gnssIpcCode;
    ipcMap_[static_cast<uint32_t>(GnssInterfaceCode::SET_ENABLE)] = g_gnssIpcCode;
    ipcMap_[static_cast<uint32_t>(GnssInterfaceCode::REFRESH_REQUESTS)] = g_gnssIpcCode;
    ipcMap_[static_cast<uint32_t>(GnssInterfaceCode::REG_GNSS_STATUS)] = g_gnssIpcCode;
    ipcMap_[static_cast<uint32_t>(GnssInterfaceCode::UNREG_GNSS_STATUS)] = g_gnssIpcCode;
    ipcMap_[static_cast<uint32_t>(GnssInterfaceCode::REG_NMEA)] = g_gnssIpcCode;
    ipcMap_[static_cast<uint32_t>(GnssInterfaceCode::UNREG_NMEA)] = g_gnssIpcCode;
    ipcMap_[static_cast<uint32_t>(GnssInterfaceCode::REG_CACHED)] = g_gnssIpcCode;
    ipcMap_[static_cast<uint32_t>(GnssInterfaceCode::UNREG_CACHED)] = g_gnssIpcCode;
    ipcMap_[static_cast<uint32_t>(GnssInterfaceCode::GET_CACHED_SIZE)] = g_gnssIpcCode;
    ipcMap_[static_cast<uint32_t>(GnssInterfaceCode::FLUSH_CACHED)] = g_gnssIpcCode;
    ipcMap_[static_cast<uint32_t>(GnssInterfaceCode::SEND_COMMANDS)] = g_gnssIpcCode;
    ipcMap_[static_cast<uint32_t>(GnssInterfaceCode::ENABLE_LOCATION_MOCK)] = g_gnssIpcCode;
    ipcMap_[static_cast<uint32_t>(GnssInterfaceCode::DISABLE_LOCATION_MOCK)] = g_gnssIpcCode;
    ipcMap_[static_cast<uint32_t>(GnssInterfaceCode::ADD_FENCE_INFO)] = g_gnssIpcCode;
    ipcMap_[static_cast<uint32_t>(GnssInterfaceCode::REMOVE_FENCE_INFO)] = g_gnssIpcCode;
#endif
#ifdef FEATURE_NETWORK_SUPPORT
    ipcMap_[static_cast<uint32_t>(NetworkInterfaceCode::SEND_LOCATION_REQUEST)] = g_networkIpcCode;
    ipcMap_[static_cast<uint32_t>(NetworkInterfaceCode::SET_MOCKED_LOCATIONS)] = g_networkIpcCode;
    ipcMap_[static_cast<uint32_t>(NetworkInterfaceCode::SELF_REQUEST)] = g_networkIpcCode;
    ipcMap_[static_cast<uint32_t>(NetworkInterfaceCode::SET_ENABLE)] = g_networkIpcCode;
    ipcMap_[static_cast<uint32_t>(NetworkInterfaceCode::ENABLE_LOCATION_MOCK)] = g_networkIpcCode;
    ipcMap_[static_cast<uint32_t>(NetworkInterfaceCode::DISABLE_LOCATION_MOCK)] = g_networkIpcCode;
#endif
#ifdef FEATURE_GEOCODE_SUPPORT
    ipcMap_[static_cast<uint32_t>(GeoConvertInterfaceCode::IS_AVAILABLE)] = g_geoIpcCode;
    ipcMap_[static_cast<uint32_t>(GeoConvertInterfaceCode::GET_FROM_COORDINATE)] = g_geoIpcCode;
    ipcMap_[static_cast<uint32_t>(GeoConvertInterfaceCode::GET_FROM_LOCATION_NAME_BY_BOUNDARY)] = g_geoIpcCode;
    ipcMap_[static_cast<uint32_t>(GeoConvertInterfaceCode::ENABLE_REVERSE_GEOCODE_MOCK)] = g_geoIpcCode;
    ipcMap_[static_cast<uint32_t>(GeoConvertInterfaceCode::DISABLE_REVERSE_GEOCODE_MOCK)] = g_geoIpcCode;
    ipcMap_[static_cast<uint32_t>(GeoConvertInterfaceCode::SET_REVERSE_GEOCODE_MOCKINFO)] = g_geoIpcCode;
#endif
#ifdef FEATURE_PASSIVE_SUPPORT
    ipcMap_[static_cast<uint32_t>(PassiveInterfaceCode::SEND_LOCATION_REQUEST)] = g_passiveIpcCode;
    ipcMap_[static_cast<uint32_t>(PassiveInterfaceCode::SET_ENABLE)] = g_passiveIpcCode;
    ipcMap_[static_cast<uint32_t>(PassiveInterfaceCode::ENABLE_LOCATION_MOCK)] = g_passiveIpcCode;
    ipcMap_[static_cast<uint32_t>(PassiveInterfaceCode::DISABLE_LOCATION_MOCK)] = g_passiveIpcCode;
    ipcMap_[static_cast<uint32_t>(PassiveInterfaceCode::SET_MOCKED_LOCATIONS)] = g_passiveIpcCode;
#endif
}

void LocationMockIpcTest::TearDown()
{
    ipcMap_.clear();
}

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocationMockIpcTest, MockGnssStubCallingPermission001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationMockIpcTest, MockGnssStubCallingPermission001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationMockIpcTest] MockGnssStubCallingPermission001 begin");

    auto gnssAbilityStub = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    for (auto iter = ipcMap_.begin(); iter != ipcMap_.end(); iter++) {
        if (iter->second != g_gnssIpcCode) {
            continue;
        }
        MessageParcel parcel;
        parcel.WriteInterfaceToken(u"location.IGnssAbility");
        MessageParcel reply;
        MessageOption option;
        EXPECT_EQ(LOCATION_ERRCODE_PERMISSION_DENIED,
            gnssAbilityStub->OnRemoteRequest(iter->first, parcel, reply, option));
    }
    sleep(WAIT_RESPONSE_SEC);
    gnssAbilityStub = nullptr;
    LBSLOGI(LOCATOR, "[LocationMockIpcTest] MockGnssStubCallingPermission001 end");
}
#endif

#ifdef FEATURE_NETWORK_SUPPORT
HWTEST_F(LocationMockIpcTest, MockNetworkStubCallingPermission001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationMockIpcTest, MockNetworkStubCallingPermission001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationMockIpcTest] MockNetworkStubCallingPermission001 begin");

    auto networkAbilityStub = sptr<NetworkAbility>(new (std::nothrow) NetworkAbility());
    networkAbilityStub->networkHandler_ =
        std::make_shared<NetworkHandler>(AppExecFwk::EventRunner::Create(true, AppExecFwk::ThreadMode::FFRT));
    for (auto iter = ipcMap_.begin(); iter != ipcMap_.end(); iter++) {
        if (iter->second != g_networkIpcCode) {
            continue;
        }
        MessageParcel parcel;
        parcel.WriteInterfaceToken(u"location.INetworkAbility");
        MessageParcel reply;
        MessageOption option;
        networkAbilityStub->OnRemoteRequest(iter->first, parcel, reply, option);
    }
    sleep(WAIT_RESPONSE_SEC);
    networkAbilityStub->networkHandler_->TaskCancelAndWait();
    LBSLOGI(LOCATOR, "[LocationMockIpcTest] MockNetworkStubCallingPermission001 end");
}
#endif

#ifdef FEATURE_PASSIVE_SUPPORT
HWTEST_F(LocationMockIpcTest, MockPassiveStubCallingPermission001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "LocationMockIpcTest, MockPassiveStubCallingPermission001, TestSize.Level0";
    LBSLOGI(LOCATOR, "[LocationMockIpcTest] MockPassiveStubCallingPermission001 begin");

    auto passiveAbilityStub = sptr<PassiveAbility>(new (std::nothrow) PassiveAbility());
    for (auto iter = ipcMap_.begin(); iter != ipcMap_.end(); iter++) {
        if (iter->second != g_passiveIpcCode) {
            continue;
        }
        MessageParcel parcel;
        parcel.WriteInterfaceToken(u"location.IPassiveAbility");
        MessageParcel reply;
        MessageOption option;
        EXPECT_EQ(LOCATION_ERRCODE_PERMISSION_DENIED,
            passiveAbilityStub->OnRemoteRequest(iter->first, parcel, reply, option));
    }
    sleep(WAIT_RESPONSE_SEC);
    passiveAbilityStub->passiveHandler_->TaskCancelAndWait();
    LBSLOGI(LOCATOR, "[LocationMockIpcTest] MockPassiveStubCallingPermission001 end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocationMockIpcTest, MockGeoCodeStubCallingPermission001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationMockIpcTest, MockGeoCodeStubCallingPermission001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationMockIpcTest] MockGeoCodeStubCallingPermission001 begin");

    auto geoConvertServiceStub = sptr<GeoConvertService>(new (std::nothrow) GeoConvertService());
    for (auto iter = ipcMap_.begin(); iter != ipcMap_.end(); iter++) {
        if (iter->second != g_geoIpcCode) {
            continue;
        }
        MessageParcel parcel;
        parcel.WriteInterfaceToken(u"location.IGeoConvert");
        MessageParcel reply;
        MessageOption option;
        EXPECT_EQ(LOCATION_ERRCODE_PERMISSION_DENIED,
            geoConvertServiceStub->OnRemoteRequest(iter->first, parcel, reply, option));
    }
    sleep(WAIT_RESPONSE_SEC);
    geoConvertServiceStub = nullptr;
    LBSLOGI(LOCATOR, "[LocationMockIpcTest] MockGeoCodeStubCallingPermission001 end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocationMockIpcTest, MockGeoConvertServiceStub001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationMockIpcTest, MockGeoConvertServiceStub001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationMockIpcTest] MockGeoConvertServiceStub001 begin");
    auto geoConvertServiceStub = sptr<GeoConvertService>(new (std::nothrow) GeoConvertService());
    geoConvertServiceStub->InitGeoConvertHandleMap();
    geoConvertServiceStub->InitGeoConvertHandleMap();
    LBSLOGI(LOCATOR, "[LocationMockIpcTest] MockGeoConvertServiceStub001 end");
}
#endif
 
#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocationMockIpcTest, MockGeoConvertServiceStub002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationMockIpcTest, MockGeoConvertServiceStub002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationMockIpcTest] MockGeoConvertServiceStub002 begin");
    MessageParcel data;
    auto geoConvertServiceStub = sptr<GeoConvertService>(new (std::nothrow) GeoConvertService());
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfo = geoConvertServiceStub->ParseGeocodingMockInfos(data);
    data.WriteInt32(2);
    mockInfo = geoConvertServiceStub->ParseGeocodingMockInfos(data);
    LBSLOGI(LOCATOR, "[LocationMockIpcTest] MockGeoConvertServiceStub002 end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocationMockIpcTest, MockGeoConvertServiceStub003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationMockIpcTest, MockGeoConvertServiceStub003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationMockIpcTest] MockGeoConvertServiceStub003 begin");
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    auto geoConvertServiceStub = sptr<GeoConvertService>(new (std::nothrow) GeoConvertService());
    geoConvertServiceStub->IsGeoConvertAvailableInner(data, reply, identity);
    geoConvertServiceStub->GetAddressByCoordinateInner(data, reply, identity);
    geoConvertServiceStub->GetAddressByLocationNameInner(data, reply, identity);
    geoConvertServiceStub->EnableReverseGeocodingMockInner(data, reply, identity);
    geoConvertServiceStub->DisableReverseGeocodingMockInner(data, reply, identity);
    geoConvertServiceStub->SetGeocodingMockInfoInner(data, reply, identity);
    identity.uid_ = 1021;
    identity.pid_ = 1494;
    geoConvertServiceStub->IsGeoConvertAvailableInner(data, reply, identity);
    geoConvertServiceStub->GetAddressByCoordinateInner(data, reply, identity);
    geoConvertServiceStub->GetAddressByLocationNameInner(data, reply, identity);
    geoConvertServiceStub->EnableReverseGeocodingMockInner(data, reply, identity);
    geoConvertServiceStub->DisableReverseGeocodingMockInner(data, reply, identity);
    geoConvertServiceStub->SetGeocodingMockInfoInner(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocationMockIpcTest] MockGeoConvertServiceStub003 end");
}
#endif
}  // namespace Location
}  // namespace OHOS
