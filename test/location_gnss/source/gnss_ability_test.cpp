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

#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_ability_test.h"

#include <cstdlib>

#include "accesstoken_kit.h"
#include "cell_information.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "gnss_event_callback.h"
#include "location_dumper.h"

#include "mock_i_cellular_data_manager.h"
#include "permission_manager.h"
#include "geofence_request.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Location {
using HDI::Location::Agnss::V2_0::IAGnssCallback;
using HDI::Location::Agnss::V2_0::AGnssRefInfo;
using HDI::Location::Gnss::V2_0::IGnssCallback;
using HDI::Location::Gnss::V2_0::LocationInfo;
using HDI::Location::Gnss::V2_0::ConstellationCategory;
using HDI::Location::Agnss::V2_0::AGnssRefInfoType;
const int32_t LOCATION_PERM_NUM = 4;
const std::string ARGS_HELP = "-h";
void GnssAbilityTest::SetUp()
{
    /*
     * @tc.setup: Get system ability's pointer and get sa proxy object.
     */
    MockNativePermission();
    ability_ = new (std::nothrow) GnssAbility();
    EXPECT_NE(nullptr, ability_);
    callbackStub_ = new (std::nothrow) GnssStatusCallbackHost();
    EXPECT_NE(nullptr, callbackStub_);
    nemaCallbackStub_ = new (std::nothrow) NmeaMessageCallbackHost();
    EXPECT_NE(nullptr, nemaCallbackStub_);
    cachedLocationCallbackStub_ = new (std::nothrow) CachedLocationsCallbackHost();
    EXPECT_NE(nullptr, cachedLocationCallbackStub_);
    proxy_ = new (std::nothrow) GnssAbilityProxy(ability_);
    EXPECT_NE(nullptr, proxy_);
}

void GnssAbilityTest::TearDown()
{
    /*
     * @tc.teardown: release memory.
     */
    proxy_ = nullptr;
    callbackStub_ = nullptr;
}

void GnssAbilityTest::MockNativePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "GnssAbilityTest",
        .aplStr = "system_basic",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

/*
 * @tc.name: SendLocationRequest001
 * @tc.desc: Build Request, marshall and unmarshall data Then Send it
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, SendLocationRequest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SendLocationRequest001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] SendLocationRequest001 begin");
    /*
     * @tc.steps: step1. build location request data.
     */
    std::unique_ptr<WorkRecord> workRecord = std::make_unique<WorkRecord>();
    int num = 2;
    for (int i = 0; i < num; i++) {
        std::shared_ptr<Request> request = std::make_shared<Request>();
        std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
        requestConfig->SetTimeInterval(i);
        request->SetUid(i + 1);
        request->SetPid(i + 2);
        request->SetPackageName("nameForTest");
        request->SetRequestConfig(*requestConfig);
        request->SetUuid(std::to_string(CommonUtils::IntRandom(MIN_INT_RANDOM, MAX_INT_RANDOM)));
        request->SetNlpRequestType(i + 1);
        workRecord->Add(request);
    }
    /*
     * @tc.steps: step2. send location request
     * @tc.expected: step2. no exception happens.
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SendLocationRequest(*workRecord));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] SendLocationRequest001 end");
}

/*
 * @tc.name: SetEnableAndDisable001
 * @tc.desc: Test disable and enable system ability
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, SetEnableAndDisable001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SetEnableAndDisable001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] SetEnableAndDisable001 begin");

    /*
     * @tc.steps: step1.remove SA
     * @tc.expected: step1. object1 is null.
     */
    MessageParcel data1;
    data1.WriteBool(false); // if the state is false
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SetEnable(data1.ReadBool()));

    /*
     * @tc.steps: step2. test enable SA
     * @tc.expected: step2. object2 is not null.
     */
    MessageParcel data2;
    data2.WriteBool(true); // if the state is true
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SetEnable(data2.ReadBool()));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] SetEnableAndDisable001 end");
}

/*
 * @tc.name: RefrashRequirements001
 * @tc.desc: Test refrash requirements
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RefrashRequirements001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RefrashRequirements001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RefrashRequirements001 begin");
    /*
     * @tc.steps: step1. test refrash requirements
     * @tc.expected: no exception happens.
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->RefrashRequirements());
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RefrashRequirements001 end");
}

/*
 * @tc.name: RegisterGnssStatusCallback001
 * @tc.desc: Test register gnss status callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterGnssStatusCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RegisterGnssStatusCallback001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterGnssStatusCallback001 begin");
    /*
     * @tc.steps: step1.the client is null.
     */
    pid_t lastCallingUid = 1;
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test register gnss status callback
     * @tc.expected: log info : "SendRegisterMsgToRemote callback is nullptr".
     */
    EXPECT_EQ(ERRCODE_INVALID_PARAM, proxy_->RegisterGnssStatusCallback(client, lastCallingUid));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterGnssStatusCallback001 end");
}

/*
 * @tc.name: RegisterAndUnregisterGnssStatusCallback001
 * @tc.desc: Test register and unregister gnss status callback if client is not null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterAndUnregisterGnssStatusCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RegisterAndUnregisterGnssStatusCallback001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterAndUnregisterGnssStatusCallback001 begin");
    /*
     * @tc.steps: step1. give the last calling uid
     */
    pid_t lastCallingUid = 1;

    /*
     * @tc.steps: step2. test register gnss status callback
     * @tc.expected: no exception happens.
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->RegisterGnssStatusCallback(callbackStub_->AsObject(), lastCallingUid));

    /*
     * @tc.steps: step3. test unregister gnss status callback
     * @tc.expected: no exception happens
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->UnregisterGnssStatusCallback(callbackStub_->AsObject()));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterAndUnregisterGnssStatusCallback001 end");
}

/*
 * @tc.name: UnregisterGnssStatusCallback001
 * @tc.desc: Test unregister gnss status callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, UnregisterGnssStatusCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, UnregisterGnssStatusCallback001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] UnregisterGnssStatusCallback001 begin");
    /*
     * @tc.steps: step1.the client is null.
     */
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test unregister gnss status callback
     * @tc.expected: log info : "unregister an invalid gnssStatus callback".
     */
    EXPECT_EQ(ERRCODE_INVALID_PARAM, proxy_->UnregisterGnssStatusCallback(client));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] UnregisterGnssStatusCallback001 end");
}

/*
 * @tc.name: RegisterNmeaMessageCallback001
 * @tc.desc: Test register nmea message callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterNmeaMessageCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RegisterNmeaMessageCallback001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterNmeaMessageCallback001 begin");
    /*
     * @tc.steps: step1.the client is null.
     */
    pid_t uid = 1;
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test register nmea message callback
     * @tc.expected: log info : "register an invalid nmea callback".
     */
    EXPECT_EQ(ERRCODE_INVALID_PARAM, proxy_->RegisterNmeaMessageCallback(client, uid));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterNmeaMessageCallback001 end");
}

/*
 * @tc.name: RegisterAndUnregisterNmeaMessageCallback001
 * @tc.desc: Test register nmea message callback if client is not null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterAndUnregisterNmeaMessageCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RegisterAndUnregisterNmeaMessageCallback001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterAndUnregisterNmeaMessageCallback001 begin");
    /*
     * @tc.steps: step1.the client is not null.
     */
    pid_t uid = 1;

    /*
     * @tc.steps: step2. test register nmea message callback
     * @tc.expected: no exception happens
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->RegisterNmeaMessageCallback(nemaCallbackStub_->AsObject(), uid));

    /*
     * @tc.steps: step3. test unregister nmea message callback
     * @tc.expected: no exception happens.
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->UnregisterNmeaMessageCallback(nemaCallbackStub_->AsObject()));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterAndUnregisterNmeaMessageCallback001 end");
}

/*
 * @tc.name: UnregisterNmeaMessageCallback001
 * @tc.desc: Test unregister nmea message callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, UnregisterNmeaMessageCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, UnregisterNmeaMessageCallback001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] UnregisterNmeaMessageCallback001 begin");
    /*
     * @tc.steps: step1.the client is null.
     */
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test unregister nmea message callback
     * @tc.expected: log info : "unregister an invalid nmea callback".
     */
    EXPECT_EQ(ERRCODE_INVALID_PARAM, proxy_->UnregisterNmeaMessageCallback(client));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] UnregisterNmeaMessageCallback001 end");
}

/*
 * @tc.name: RegisterCachedCallback001
 * @tc.desc: Test register cache call back
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterCachedCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RegisterCachedCallback001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterCachedCallback001 begin");
    /*
     * @tc.steps: step1.prepare request config and the call back is null.
     */
    std::unique_ptr<CachedGnssLocationsRequest> requestConfig = std::make_unique<CachedGnssLocationsRequest>();
    requestConfig->reportingPeriodSec = 100;
    requestConfig->wakeUpCacheQueueFull = true;

    sptr<IRemoteObject> callback = nullptr;

    /*
     * @tc.steps: step2. test register cached call back if call back is null.
     * @tc.expected: log info : "register an invalid cached location callback"
     */
    EXPECT_EQ(ERRCODE_INVALID_PARAM, proxy_->RegisterCachedCallback(requestConfig, callback));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterCachedCallback001 end");
}

/*
 * @tc.name: RegisterCachedCallback002
 * @tc.desc: Test register cache call back and the queue is not full.
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterCachedCallback003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RegisterCachedCallback003, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterCachedCallback003 begin");
    /*
     * @tc.steps: step1.prepare request config and the call back is not null.
     */
    std::unique_ptr<CachedGnssLocationsRequest> requestConfig = std::make_unique<CachedGnssLocationsRequest>();
    requestConfig->reportingPeriodSec = 100;
    requestConfig->wakeUpCacheQueueFull = false;

    /*
     * @tc.steps: step2. test register cached call back if call back is not null.
     * @tc.expected: no exception happens
     */
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED,
        proxy_->RegisterCachedCallback(requestConfig, cachedLocationCallbackStub_->AsObject()));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterCachedCallback003 end");
}

/*
 * @tc.name: RegisterAndUnregisterCachedCallback001
 * @tc.desc: Test register and unregister cache call back and the queue is full.
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterAndUnregisterCachedCallback002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RegisterAndUnregisterCachedCallback002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterAndUnregisterCachedCallback002 begin");
    /*
     * @tc.steps: step1.prepare request config and the call back is not null.
     */
    std::unique_ptr<CachedGnssLocationsRequest> requestConfig = std::make_unique<CachedGnssLocationsRequest>();
    requestConfig->reportingPeriodSec = 100;
    requestConfig->wakeUpCacheQueueFull = true;

    /*
     * @tc.steps: step2. test register cached call back if call back is not null.
     * @tc.expected: no exception happens
     */
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED,
        proxy_->RegisterCachedCallback(requestConfig, cachedLocationCallbackStub_->AsObject()));

    /*
     * @tc.steps: step3. test unregister cached call back if call back is not null.
     * @tc.expected: no exception happens.
     */
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, proxy_->UnregisterCachedCallback(cachedLocationCallbackStub_->AsObject()));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterAndUnregisterCachedCallback002 end");
}

/*
 * @tc.name: UnregisterCachedCallback001
 * @tc.desc: Test unregister cache call back and call back is null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, UnregisterCachedCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, UnregisterCachedCallback001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] UnregisterCachedCallback001 begin");
    /*
     * @tc.steps: step1. the call back is null.
     */
    sptr<IRemoteObject> callback = nullptr;

    /*
     * @tc.steps: step2. test unregister cached call back if call back is null.
     * @tc.expected: log info : "register an invalid cached location callback"
     */
    EXPECT_EQ(ERRCODE_INVALID_PARAM, proxy_->UnregisterCachedCallback(callback));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] UnregisterCachedCallback001 end");
}

/*
 * @tc.name: GetCachedGnssLocationsSize001
 * @tc.desc: Test get cached gnss locations size
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, GetCachedGnssLocationsSize001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCachedGnssLocationsSize001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GetCachedGnssLocationsSize001 begin");
    /*
     * @tc.steps: step1. test get cached gnss locations size.
     * @tc.expected: size equals -1.
     */
    int size = -1;
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, proxy_->GetCachedGnssLocationsSize(size));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GetCachedGnssLocationsSize001 end");
}

/*
 * @tc.name: FlushCachedGnssLocations001
 * @tc.desc: Test unregister country code call back
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, FlushCachedGnssLocations001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, FlushCachedGnssLocations001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] FlushCachedGnssLocations001 begin");
    /*
     * @tc.steps: step1. test flush cached gnss locations.
     * @tc.expected: reply code unsupport
     */
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, proxy_->FlushCachedGnssLocations());
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] FlushCachedGnssLocations001 end");
}

/*
 * @tc.name: SendCommand001
 * @tc.desc: Test send
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, SendCommand001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SendCommand001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] SendCommand001 begin");
    /*
     * @tc.steps: step1. build location command
     */
    std::unique_ptr<LocationCommand> locationCommand = std::make_unique<LocationCommand>();
    locationCommand->scenario = 1;
    locationCommand->command = true;

    /*
     * @tc.steps: step2. test send command.
     * @tc.expected: current function is empty, nothing happens
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SendCommand(locationCommand));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] SendCommand001 end");
}

/*
 * @tc.name: AddFence001
 * @tc.desc: Test add fence
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, AddFence001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AddFence001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AddFence001 begin");
    /*
     * @tc.steps: step1. build geo fence request
     */
    GeoFence geofence;
    geofence.latitude = 35.1;
    geofence.longitude = 40.2;
    geofence.radius = 2.2;
    geofence.expiration = 12.2;
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    request->SetGeofence(geofence);

    /*
     * @tc.steps: step2. test add fence
     * @tc.expected: no exception happens
     */
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, proxy_->AddFence(request));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AddFence001 end");
}

/*
 * @tc.name: RemoveFence001
 * @tc.desc: Test add fence
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RemoveFence001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RemoveFence001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RemoveFence001 begin");
    /*
     * @tc.steps: step1. build geo fence request
     */
    GeoFence geofence;
    geofence.latitude = 35.1;
    geofence.longitude = 40.2;
    geofence.radius = 2.2;
    geofence.expiration = 12.2;
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    request->SetGeofence(geofence);

    /*
     * @tc.steps: step2. test remove fence
     * @tc.expected: no exception happens
     */
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, proxy_->RemoveFence(request));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RemoveFence001 end");
}

HWTEST_F(GnssAbilityTest, GnssLocationMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssLocationMock001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssLocationMock001 begin");
    int timeInterval = 0;
    std::vector<std::shared_ptr<Location>> locations;
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->EnableMock());
    EXPECT_EQ(true, ability_->IsMockEnabled());
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SetMocked(timeInterval, locations));
    
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->DisableMock());
    EXPECT_EQ(false, ability_->IsMockEnabled());
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, proxy_->SetMocked(timeInterval, locations));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssLocationMock001 begin");
}

HWTEST_F(GnssAbilityTest, GnssOnStartAndOnStop001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssOnStartAndOnStop001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssOnStartAndOnStop001 begin");
    ability_->OnStart(); // start ability
    EXPECT_EQ(ServiceRunningState::STATE_NOT_START, ability_->QueryServiceState()); // mock will return nullptr

    ability_->OnStop(); // stop ability
    EXPECT_EQ(ServiceRunningState::STATE_NOT_START, ability_->QueryServiceState()); // mock will return nullptr
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssOnStartAndOnStop001 end");
}

HWTEST_F(GnssAbilityTest, GnssDump001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssDump001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssDump001 begin");
    int32_t fd = 0;
    std::vector<std::u16string> args;
    std::u16string arg1 = Str8ToStr16("arg1");
    args.emplace_back(arg1);
    std::u16string arg2 = Str8ToStr16("arg2");
    args.emplace_back(arg2);
    std::u16string arg3 = Str8ToStr16("arg3");
    args.emplace_back(arg3);
    std::u16string arg4 = Str8ToStr16("arg4");
    args.emplace_back(arg4);
    EXPECT_EQ(ERR_OK, ability_->Dump(fd, args));

    std::vector<std::u16string> emptyArgs;
    EXPECT_EQ(ERR_OK, ability_->Dump(fd, emptyArgs));

    std::vector<std::u16string> helpArgs;
    std::u16string helpArg1 = Str8ToStr16(ARGS_HELP);
    helpArgs.emplace_back(helpArg1);
    EXPECT_EQ(ERR_OK, ability_->Dump(fd, helpArgs));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssDump001 end");
}

HWTEST_F(GnssAbilityTest, GnssSendReportMockLocationEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssSendReportMockLocationEvent001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendReportMockLocationEvent001 begin");
    ability_->SendReportMockLocationEvent(); // clear location mock

    int timeInterval = 0;
    std::vector<std::shared_ptr<Location>> locations;
    Parcel parcel;
    parcel.WriteDouble(10.6); // latitude
    parcel.WriteDouble(10.5); // longitude
    parcel.WriteDouble(10.4); // altitude
    parcel.WriteDouble(1.0); // accuracy
    parcel.WriteDouble(5.0); // speed
    parcel.WriteDouble(10); // direction
    parcel.WriteInt64(1611000000); // timestamp
    parcel.WriteInt64(1611000000); // time since boot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteInt32(1); // isFromMock is true
    locations.push_back(Location::UnmarshallingShared(parcel));
    EXPECT_EQ(ERRCODE_SUCCESS, ability_->EnableMock());
    EXPECT_EQ(ERRCODE_SUCCESS, ability_->SetMocked(timeInterval, locations));
    sleep(2);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendReportMockLocationEvent001 end");
}

HWTEST_F(GnssAbilityTest, GnssSendReportMockLocationEvent002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssSendReportMockLocationEvent002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendReportMockLocationEvent002 begin");
    ability_->SendReportMockLocationEvent(); // clear location mock

    int timeInterval = 0;
    std::vector<std::shared_ptr<Location>> locations;
    Parcel parcel;
    parcel.WriteDouble(10.6); // latitude
    parcel.WriteDouble(10.5); // longitude
    parcel.WriteDouble(10.4); // altitude
    parcel.WriteDouble(1.0); // accuracy
    parcel.WriteDouble(5.0); // speed
    parcel.WriteDouble(10); // direction
    parcel.WriteInt64(1611000000); // timestamp
    parcel.WriteInt64(1611000000); // time since boot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteInt32(0); // isFromMock is false
    locations.push_back(Location::UnmarshallingShared(parcel));
    EXPECT_EQ(ERRCODE_SUCCESS, ability_->EnableMock());
    EXPECT_EQ(ERRCODE_SUCCESS, ability_->SetMocked(timeInterval, locations));
    sleep(2);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendReportMockLocationEvent002 end");
}

HWTEST_F(GnssAbilityTest, AddFenceAndRemoveFenceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AddFenceAndRemoveFenceTest001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AddFenceAndRemoveFenceTest001 begin");
    std::shared_ptr<GeofenceRequest> fence = std::make_shared<GeofenceRequest>();
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, ability_->AddFence(fence));
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, ability_->RemoveFence(fence));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AddFenceAndRemoveFenceTest001 end");
}

HWTEST_F(GnssAbilityTest, GnssAbilityReportSv001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssAbilityReportSv001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssAbilityReportSv001 begin");
    std::unique_ptr<SatelliteStatus> status = std::make_unique<SatelliteStatus>();
    MessageParcel parcel;
    int sateNum = 2;
    parcel.WriteInt64(2); // satellitesNumber
    for (int i = 0; i < sateNum; i++) {
        parcel.WriteInt64(i); // satelliteId
        parcel.WriteDouble(i + 1.0); // carrierToNoiseDensity
        parcel.WriteDouble(i + 2.0); // altitude
        parcel.WriteDouble(i + 3.0); // azimuth
        parcel.WriteDouble(i + 4.0); // carrierFrequency
        parcel.WriteInt64(i + 5.0); // constellation type
    }
    ASSERT_TRUE(status != nullptr);
    status->ReadFromParcel(parcel);
    ASSERT_TRUE(ability_ != nullptr);
    ability_->ReportSv(status);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssAbilityReportSv001 end");
}

#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestSetUpAgnssDataLink001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestSetUpAgnssDataLink001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestSetUpAgnssDataLink001 begin");
    sptr<IAGnssCallback> agnssCallback = new (std::nothrow) AGnssEventCallback();
    EXPECT_NE(nullptr, agnssCallback);
    AGnssDataLinkRequest request;
    request.agnssType = HDI::Location::Agnss::V2_0::AGNSS_TYPE_SUPL;
    request.setUpType = HDI::Location::Agnss::V2_0::ESTABLISH_DATA_CONNECTION;
    EXPECT_EQ(ERR_OK, agnssCallback->RequestSetUpAgnssDataLink(request));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestSetUpAgnssDataLink001 end");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestSubscriberSetId001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestSubscriberSetId001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestSubscriberSetId001 begin");
    sptr<IAGnssCallback> agnssCallback = new (std::nothrow) AGnssEventCallback();
    EXPECT_NE(nullptr, agnssCallback);
    SubscriberSetIdType type = HDI::Location::Agnss::V2_0::AGNSS_SETID_TYPE_IMSI;
    EXPECT_EQ(ERR_OK, agnssCallback->RequestSubscriberSetId(type));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestSubscriberSetId001 end");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo001 begin");
    sptr<IAGnssCallback> agnssCallback = new (std::nothrow) AGnssEventCallback();
    EXPECT_CALL(MockICellularDataManager::GetInstance(), GetDefaultCellularDataSlotId).WillRepeatedly(Return(-1));
    EXPECT_NE(nullptr, agnssCallback);
    AGnssRefInfoType type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    EXPECT_EQ(ERR_OK, agnssCallback->RequestAgnssRefInfo(type));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo001 end");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo002 begin");
    sptr<AGnssEventCallback> agnssCallback = new (std::nothrow) AGnssEventCallback();
    EXPECT_NE(nullptr, agnssCallback);
    AGnssRefInfo refInfo;
    refInfo.type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    sptr<Telephony::GsmCellInformation> gsmCellInformation = new Telephony::GsmCellInformation();
    gsmCellInformation->Init(0, 0, 0);
    agnssCallback->JudgmentDataGsm(refInfo, gsmCellInformation);
    EXPECT_EQ(HDI::Location::Agnss::V2_0::CELLID_TYPE_GSM, refInfo.cellId.type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo002 end");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo003, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo003 begin");
    sptr<AGnssEventCallback> agnssCallback = new (std::nothrow) AGnssEventCallback();
    EXPECT_NE(nullptr, agnssCallback);
    AGnssRefInfo refInfo;
    refInfo.type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    sptr<Telephony::GsmCellInformation> gsmCellInformation = nullptr;
    agnssCallback->JudgmentDataGsm(refInfo, gsmCellInformation);
    EXPECT_NE(HDI::Location::Agnss::V2_0::CELLID_TYPE_GSM, refInfo.cellId.type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo003 begin");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo004, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo004 begin");
    sptr<AGnssEventCallback> agnssCallback = new (std::nothrow) AGnssEventCallback();
    EXPECT_NE(nullptr, agnssCallback);
    AGnssRefInfo refInfo;
    refInfo.type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    sptr<Telephony::LteCellInformation> lteCellInformation = new Telephony::LteCellInformation();
    lteCellInformation->Init(0, 0, 0);
    agnssCallback->JudgmentDataLte(refInfo, lteCellInformation);
    EXPECT_EQ(HDI::Location::Agnss::V2_0::CELLID_TYPE_LTE, refInfo.cellId.type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo004 begin");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo005, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo005 begin");
    sptr<AGnssEventCallback> agnssCallback = new (std::nothrow) AGnssEventCallback();
    EXPECT_NE(nullptr, agnssCallback);
    AGnssRefInfo refInfo;
    refInfo.type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    sptr<Telephony::LteCellInformation> lteCellInformation = nullptr;
    agnssCallback->JudgmentDataLte(refInfo, lteCellInformation);
    EXPECT_NE(HDI::Location::Agnss::V2_0::CELLID_TYPE_LTE, refInfo.cellId.type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo005 begin");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo006, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo006, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo006 begin");
    sptr<AGnssEventCallback> agnssCallback = new (std::nothrow) AGnssEventCallback();
    EXPECT_NE(nullptr, agnssCallback);
    AGnssRefInfo refInfo;
    refInfo.type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    sptr<Telephony::WcdmaCellInformation> umtsCellInformation = new Telephony::WcdmaCellInformation();
    umtsCellInformation->Init(0, 0, 0);
    agnssCallback->JudgmentDataUmts(refInfo, umtsCellInformation);
    EXPECT_EQ(HDI::Location::Agnss::V2_0::CELLID_TYPE_UMTS, refInfo.cellId.type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo006 begin");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo007, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo007, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo007 begin");
    sptr<AGnssEventCallback> agnssCallback = new (std::nothrow) AGnssEventCallback();
    EXPECT_NE(nullptr, agnssCallback);
    AGnssRefInfo refInfo;
    refInfo.type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    sptr<Telephony::WcdmaCellInformation> umtsCellInformation = nullptr;
    agnssCallback->JudgmentDataUmts(refInfo, umtsCellInformation);
    EXPECT_NE(HDI::Location::Agnss::V2_0::CELLID_TYPE_UMTS, refInfo.cellId.type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo007 begin");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo008, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo008, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo008 begin");
    sptr<AGnssEventCallback> agnssCallback = new (std::nothrow) AGnssEventCallback();
    EXPECT_NE(nullptr, agnssCallback);
    AGnssRefInfo refInfo;
    refInfo.type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    sptr<Telephony::NrCellInformation> nrCellInformation = new Telephony::NrCellInformation();
    nrCellInformation->Init(0, 0, 0);
    agnssCallback->JudgmentDataNr(refInfo, nrCellInformation);
    EXPECT_EQ(HDI::Location::Agnss::V2_0::CELLID_TYPE_NR, refInfo.cellId.type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo008 begin");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo009, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo009, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo009 begin");
    sptr<AGnssEventCallback> agnssCallback = new (std::nothrow) AGnssEventCallback();
    EXPECT_NE(nullptr, agnssCallback);
    AGnssRefInfo refInfo;
    refInfo.type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    sptr<Telephony::NrCellInformation> nrCellInformation = nullptr;
    agnssCallback->JudgmentDataNr(refInfo, nrCellInformation);
    EXPECT_NE(HDI::Location::Agnss::V2_0::CELLID_TYPE_NR, refInfo.cellId.type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo009 begin");
}
#endif

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportLocation001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportLocation001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    LocationInfo locationInfo;
    locationInfo.latitude = 1.0;
    locationInfo.longitude = 2.0;
    locationInfo.altitude = 1.0;
    locationInfo.horizontalAccuracy = 1.0;
    locationInfo.speed = 1.0;
    locationInfo.bearing= 1.0;
    locationInfo.timeForFix = 1000000000;
    locationInfo.timeSinceBoot = 1000000000;
    gnssCallback->ReportLocation(locationInfo);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportLocation001 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportLocation002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportLocation002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportLocation002 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    LocationInfo locationInfo;
    locationInfo.latitude = 1.0;
    locationInfo.longitude = 2.0;
    locationInfo.altitude = 1.0;
    locationInfo.horizontalAccuracy = 1.0;
    locationInfo.speed = 1.0;
    locationInfo.bearing= 1.0;
    locationInfo.timeForFix = 1000000000;
    locationInfo.timeSinceBoot = 1000000000;
    proxy_->EnableMock();
    auto ret = gnssCallback->ReportLocation(locationInfo);
    EXPECT_EQ(ERR_OK, ret);
    proxy_->DisableMock();
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportLocation002 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportGnssWorkingStatus001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportGnssWorkingStatus001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportGnssWorkingStatus001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssWorkingStatus status = HDI::Location::Gnss::V2_0::GNSS_WORKING_STATUS_NONE;
    gnssCallback->ReportGnssWorkingStatus(status);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportGnssWorkingStatus001 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportNmea001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportNmea001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportNmea001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    gnssCallback->ReportNmea(0, "nmea", 0);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportNmea001 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportGnssCapabilities001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportGnssCapabilities001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportGnssCapabilities001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssCapabilities capabilities = HDI::Location::Gnss::V2_0::GNSS_CAP_SUPPORT_MSB;
    EXPECT_EQ(ERR_OK, gnssCallback->ReportGnssCapabilities(capabilities));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportGnssCapabilities001 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportSatelliteStatusInfo002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportSatelliteStatusInfo002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportSatelliteStatusInfo002 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    SatelliteStatusInfo statusInfo;
    statusInfo.satellitesNumber = 0;
    EXPECT_EQ(ERR_OK, gnssCallback->ReportSatelliteStatusInfo(statusInfo));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportSatelliteStatusInfo002 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportSatelliteStatusInfo003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportSatelliteStatusInfo003, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportSatelliteStatusInfo003 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    SatelliteStatusInfo statusInfo;
    statusInfo.satellitesNumber = 1;
    statusInfo.elevation.push_back(12);
    statusInfo.azimuths.push_back(30);
    statusInfo.carrierFrequencies.push_back(40);
    statusInfo.carrierToNoiseDensitys.push_back(40);
    statusInfo.satelliteIds.push_back(1);
    statusInfo.constellation.push_back(static_cast<ConstellationCategory>(1));
    statusInfo.additionalInfo.push_back(
        HDI::Location::Gnss::V2_0::SATELLITES_ADDITIONAL_INFO_EPHEMERIS_DATA_EXIST);
    EXPECT_EQ(ERR_OK, gnssCallback->ReportSatelliteStatusInfo(statusInfo));
    LocationInfo locationInfo;
    locationInfo.latitude = 1.0;
    locationInfo.longitude = 2.0;
    locationInfo.altitude = 1.0;
    locationInfo.horizontalAccuracy = 1.0;
    locationInfo.speed = 1.0;
    locationInfo.bearing= 1.0;
    locationInfo.timeForFix = 1000000000;
    locationInfo.timeSinceBoot = 1000000000;
    EXPECT_EQ(ERR_OK, gnssCallback->ReportLocation(locationInfo));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportSatelliteStatusInfo003 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackRequestGnssReferenceInfo001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackRequestGnssReferenceInfo001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackRequestGnssReferenceInfo001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssRefInfoType type = HDI::Location::Gnss::V2_0::GNSS_REF_INFO_TIME;
    gnssCallback->RequestGnssReferenceInfo(type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackRequestGnssReferenceInfo001 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackRequestPredictGnssData001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackRequestPredictGnssData001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackRequestPredictGnssData001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    gnssCallback->RequestPredictGnssData();
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackRequestPredictGnssData001 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportCachedLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportCachedLocation001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportCachedLocation001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    std::vector<LocationInfo> gnssLocations;
    LocationInfo locationInfo;
    locationInfo.latitude = 1.0;
    locationInfo.longitude = 2.0;
    locationInfo.altitude = 1.0;
    locationInfo.horizontalAccuracy = 1.0;
    locationInfo.speed = 1.0;
    locationInfo.bearing= 1.0;
    locationInfo.timeForFix = 1000000000;
    locationInfo.timeSinceBoot = 1000000000;
    gnssLocations.push_back(locationInfo);
    gnssCallback->ReportCachedLocation(gnssLocations);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportCachedLocation001 end");
}

HWTEST_F(GnssAbilityTest, GnssOnStart001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssOnStart001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssOnStart001 begin");
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility != nullptr);
    gnssAbility->state_ = ServiceRunningState::STATE_RUNNING;
    gnssAbility->OnStart();
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssOnStart001 end");
}

HWTEST_F(GnssAbilityTest, GnssSendLocationRequest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssSendLocationRequest001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendLocationRequest001 begin");
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility != nullptr);
    WorkRecord workrecord;
    gnssAbility->SendLocationRequest(workrecord);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendLocationRequest001 end");
}

HWTEST_F(GnssAbilityTest, GnssDisableGnss001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssDisableGnss001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssDisableGnss001 begin");
    sptr<GnssAbility> gnssAbility1 = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility1 != nullptr);
    gnssAbility1->gnssWorkingStatus_ = GNSS_WORKING_STATUS_SESSION_BEGIN;
    gnssAbility1->DisableGnss();

    sptr<GnssAbility> gnssAbility2 = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility2 != nullptr);
    gnssAbility2->gnssWorkingStatus_ = GNSS_WORKING_STATUS_ENGINE_OFF;
    gnssAbility2->DisableGnss();
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssDisableGnss001 end");
}

HWTEST_F(GnssAbilityTest, GnssStartGnss001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssStartGnss001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssStartGnss001 begin");
    sptr<GnssAbility> gnssAbility1 = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility1 != nullptr);
    gnssAbility1->gnssWorkingStatus_ = GNSS_WORKING_STATUS_SESSION_BEGIN;
    gnssAbility1->StartGnss();

    sptr<GnssAbility> gnssAbility2 = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility2 != nullptr);
    gnssAbility2->gnssWorkingStatus_ = GNSS_WORKING_STATUS_ENGINE_OFF;
    gnssAbility2->StartGnss();
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssStartGnss001 end");
}

HWTEST_F(GnssAbilityTest, GnssEnableGnss001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEnableGnss001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEnableGnss001 begin");
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility != nullptr);
    gnssAbility->gnssWorkingStatus_ = GNSS_WORKING_STATUS_NONE;
    bool ret = gnssAbility->EnableGnss();
    EXPECT_EQ(false, ret);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEnableGnss001 end");
}

HWTEST_F(GnssAbilityTest, GnssEnableGnss002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEnableGnss002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEnableGnss002 begin");
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility != nullptr);
    gnssAbility->gnssWorkingStatus_ = GNSS_WORKING_STATUS_ENGINE_ON;
    bool ret = gnssAbility->EnableGnss();
    EXPECT_EQ(false, ret);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEnableGnss002 end");
}

HWTEST_F(GnssAbilityTest, GnssInit001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssInit001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssInit001 begin");
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility != nullptr);
    gnssAbility->registerToAbility_ = true;
    EXPECT_EQ(true, gnssAbility->Init());
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssInit001 end");
}

HWTEST_F(GnssAbilityTest, GnssRequestRecord001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssRequestRecord001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssRequestRecord001 begin");
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility != nullptr);
    std::unique_ptr<WorkRecord> workRecord = std::make_unique<WorkRecord>();
    gnssAbility->RequestRecord(*workRecord, false);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssRequestRecord001 end");
}

HWTEST_F(GnssAbilityTest, GnssReConnectHdi001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssReConnectHdi001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssReConnectHdi001 begin");
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility != nullptr);
    gnssAbility->ReConnectHdi();
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssReConnectHdi001 end");
}

#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
HWTEST_F(GnssAbilityTest, GnssSetRefInfo001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssSetRefInfo001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSetRefInfo001 begin");
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility != nullptr);
    AGnssRefInfo refInfo;
    gnssAbility->SetRefInfo(refInfo);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSetRefInfo001 end");
}
#endif

HWTEST_F(GnssAbilityTest, GnssSendReportMockLocationEvent003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssSendReportMockLocationEvent003, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendReportMockLocationEvent003 begin");
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility != nullptr);
    gnssAbility->gnssHandler_  = nullptr;
    gnssAbility->SendReportMockLocationEvent();
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendReportMockLocationEvent003 end");
}

HWTEST_F(GnssAbilityTest, GnssSendMessage001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssSendMessage001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendMessage001 begin");
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility != nullptr);
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.INetworkAbility");
    requestParcel.WriteBuffer("data", 4);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    gnssAbility->gnssHandler_  = nullptr;
    gnssAbility->SendMessage(0, requestParcel, reply);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendMessage001 end");
}


HWTEST_F(GnssAbilityTest, GnssRegisterLocationHdiDeathRecipient001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssRegisterLocationHdiDeathRecipient001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssRegisterLocationHdiDeathRecipient001 begin");
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility != nullptr);
    gnssAbility->gnssHandler_  = nullptr;
    gnssAbility->RegisterLocationHdiDeathRecipient();
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssRegisterLocationHdiDeathRecipient001 end");
}

HWTEST_F(GnssAbilityTest, SubAbilityCommonGetRequestNum001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "SubAbilityCommonTest, GetRequestNum001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[SubAbilityCommonTest] GetRequestNum001 begin");
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    gnssAbility->newRecord_ = nullptr;
    gnssAbility->GetRequestNum();

    gnssAbility->newRecord_ = std::make_unique<WorkRecord>();
    std::shared_ptr<WorkRecord> workRecord = std::make_shared<WorkRecord>();
    int num = 2;
    std::shared_ptr<Request> request = std::make_shared<Request>();
    for (int i = 0; i < num; i++) {
        std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
        requestConfig->SetTimeInterval(i);
        request->SetUid(i + 1);
        request->SetPid(i + 2);
        request->SetPackageName("nameForTest");
        request->SetRequestConfig(*requestConfig);
        request->SetUuid(std::to_string(CommonUtils::IntRandom(MIN_INT_RANDOM, MAX_INT_RANDOM)));
        request->SetNlpRequestType(0);
        workRecord->Add(request);
    }
    gnssAbility->newRecord_->Set(*workRecord);
    gnssAbility->GetRequestNum();

    gnssAbility->newRecord_ = nullptr;
    gnssAbility->GetRequestNum();

    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetTimeInterval(0);
    request->SetUid(0);
    request->SetPid(0);
    request->SetRequestConfig(*requestConfig);
    request->SetUuid(std::to_string(CommonUtils::IntRandom(MIN_INT_RANDOM, MAX_INT_RANDOM)));
    request->SetNlpRequestType(0);
    gnssAbility->lastRecord_->Add(request);
    gnssAbility->HandleRemoveRecord(*workRecord);
    gnssAbility->lastRecord_->Clear();
    gnssAbility->lastRecord_->Set(*workRecord);
    gnssAbility->HandleAddRecord(*workRecord);
    LBSLOGI(LOCATOR, "[SubAbilityCommonTest] GetRequestNum001 end");
}

HWTEST_F(GnssAbilityTest, SubAbilityCommonHandleSelfRequest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "SubAbilityCommonTest, HandleSelfRequest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[SubAbilityCommonTest] HandleSelfRequest001 begin");
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    gnssAbility->HandleSelfRequest(0, 0, false);

    gnssAbility->GetTimeIntervalMock();
    LBSLOGI(LOCATOR, "[SubAbilityCommonTest] HandleSelfRequest001 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags001 begin");
    GnssAuxiliaryDataType flags;
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    EXPECT_NE(nullptr, gnssAbility);
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_ephemeris";
    bool result = gnssAbility->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_EPHEMERIS, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags001 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags002 begin");
    GnssAuxiliaryDataType flags;
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    EXPECT_NE(nullptr, gnssAbility);
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_almanac";
    bool result = gnssAbility->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_ALMANAC, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags002 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags003 begin");
    GnssAuxiliaryDataType flags;
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    EXPECT_NE(nullptr, gnssAbility);
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_position";
    bool result = gnssAbility->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_POSITION, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags003 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags004, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags004 begin");
    GnssAuxiliaryDataType flags;
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    EXPECT_NE(nullptr, gnssAbility);
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_time";
    bool result = gnssAbility->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_TIME, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags004 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags005, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags005 begin");
    GnssAuxiliaryDataType flags;
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    EXPECT_NE(nullptr, gnssAbility);
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_iono";
    bool result = gnssAbility->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_IONO, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags005 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags006, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags006, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags006 begin");
    GnssAuxiliaryDataType flags;
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    EXPECT_NE(nullptr, gnssAbility);
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_utc";
    bool result = gnssAbility->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_UTC, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags006 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags007, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags007, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags007 begin");
    GnssAuxiliaryDataType flags;
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    EXPECT_NE(nullptr, gnssAbility);
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_health";
    bool result = gnssAbility->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_HEALTH, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags007 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags008, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags008, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags008 begin");
    GnssAuxiliaryDataType flags;
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    EXPECT_NE(nullptr, gnssAbility);
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_svdir";
    bool result = gnssAbility->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_SVDIR, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags008 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags009, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags009, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags009 begin");
    GnssAuxiliaryDataType flags;
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    EXPECT_NE(nullptr, gnssAbility);
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_svsteer";
    bool result = gnssAbility->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_SVSTEER, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags009 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags010, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags010, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags010 begin");
    GnssAuxiliaryDataType flags;
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    EXPECT_NE(nullptr, gnssAbility);
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_sadata";
    bool result = gnssAbility->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_SADATA, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags010 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags011, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags011, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags011 begin");
    GnssAuxiliaryDataType flags;
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    EXPECT_NE(nullptr, gnssAbility);
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_rti";
    bool result = gnssAbility->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_RTI, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags011 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags012, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags012, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags012 begin");
    GnssAuxiliaryDataType flags;
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    EXPECT_NE(nullptr, gnssAbility);
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_celldb_info";
    bool result = gnssAbility->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_CELLDB_INFO, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags012 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags013, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags013, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags013 begin");
    GnssAuxiliaryDataType flags;
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    EXPECT_NE(nullptr, gnssAbility);
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_all";
    bool result = gnssAbility->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_ALL, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags013 end");
}

HWTEST_F(GnssAbilityTest, ReConnectHdiImpl001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, ReConnectHdiImpl001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ReConnectHdiImpl001 begin");
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    EXPECT_NE(nullptr, gnssAbility);
    gnssAbility->ReConnectHdiImpl();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ReConnectHdiImpl001 end");
}
}  // namespace Location
}  // namespace OHOS
#endif // FEATURE_GNSS_SUPPORT
