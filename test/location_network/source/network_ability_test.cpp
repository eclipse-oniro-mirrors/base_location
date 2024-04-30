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

#ifdef FEATURE_NETWORK_SUPPORT
#include "network_ability_test.h"

#include <cstdlib>
#include "accesstoken_kit.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "location.h"
#include "location_dumper.h"
#include "location_log.h"
#include "network_ability_skeleton.h"

#include "network_callback_host.h"
#include "permission_manager.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 4;
const std::string ARGS_HELP = "-h";
void NetworkAbilityTest::SetUp()
{
    /*
     * @tc.setup: Get system ability's pointer and get sa proxy object.
     */
    MockNativePermission();
    ability_ = new (std::nothrow) NetworkAbility();
    EXPECT_NE(nullptr, ability_);
    proxy_ = new (std::nothrow) NetworkAbilityProxy(ability_);
    EXPECT_NE(nullptr, proxy_);
}

void NetworkAbilityTest::TearDown()
{
    /*
     * @tc.teardown: release memory.
     */
    proxy_ = nullptr;
}

void NetworkAbilityTest::MockNativePermission()
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
        .processName = "NetworkAbilityTest",
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
HWTEST_F(NetworkAbilityTest, SendLocationRequest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, SendLocationRequest001, TestSize.Level1";
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] SendLocationRequest001 begin");
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
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] SendLocationRequest001 end");
}

/*
 * @tc.name: SetEnableAndDisable001
 * @tc.desc: Test disable and enable system ability
 * @tc.type: FUNC
 */
HWTEST_F(NetworkAbilityTest, SetEnableAndDisable001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, SetEnableAndDisable001, TestSize.Level1";
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] SetEnableAndDisable001 begin");
    /*
     * @tc.steps: step1.remove SA
     * @tc.expected: step1. object1 is null.
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SetEnable(false)); // after mock, sa obj is nullptr

    /*
     * @tc.steps: step2. test enable SA
     * @tc.expected: step2. object2 is not null.
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SetEnable(true)); // after mock, sa obj is nullptr
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] SetEnableAndDisable001 end");
}

/*
 * @tc.name: SelfRequest001
 * @tc.desc: test self request function
 * @tc.type: FUNC
 */
HWTEST_F(NetworkAbilityTest, SelfRequest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, SelfRequest001, TestSize.Level1";
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] SelfRequest001 begin");
    /*
     * @tc.steps: step1. send location request
     * @tc.expected: step1. no exception happens.
     */
    EXPECT_EQ(ERRCODE_SWITCH_OFF, proxy_->SelfRequest(true));
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] SelfRequest001 end");
}

/*
 * @tc.name: SelfRequest002
 * @tc.desc: test self request function
 * @tc.type: FUNC
 */
HWTEST_F(NetworkAbilityTest, SelfRequest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, SelfRequest002, TestSize.Level1";
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] SelfRequest002 begin");
    /*
     * @tc.steps: step1. send location request
     * @tc.expected: step1. no exception happens.
     */
    EXPECT_EQ(ERRCODE_SWITCH_OFF, proxy_->SelfRequest(false));
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] SelfRequest002 end");
}

HWTEST_F(NetworkAbilityTest, NetworkLocationMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, NetworkLocationMock001, TestSize.Level1";
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] NetworkLocationMock001 begin");
    int timeInterval = 1;
    std::vector<std::shared_ptr<Location>> locations;
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->EnableMock());
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SetMocked(timeInterval, locations));
    
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->DisableMock());
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, proxy_->SetMocked(timeInterval, locations));
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] NetworkLocationMock001 end");
}

HWTEST_F(NetworkAbilityTest, NetworkOnStartAndOnStop001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, NetworkOnStartAndOnStop001, TestSize.Level1";
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] NetworkOnStartAndOnStop001 begin");
    ability_->OnStart(); // start ability
    EXPECT_EQ(ServiceRunningState::STATE_NOT_START,
        (ServiceRunningState)ability_->QueryServiceState()); // after mock
    ability_->OnStart(); // start ability again
    EXPECT_EQ(ServiceRunningState::STATE_NOT_START,
        (ServiceRunningState)ability_->QueryServiceState()); // after mock

    ability_->OnStop(); // stop ability
    EXPECT_EQ(ServiceRunningState::STATE_NOT_START,
        (ServiceRunningState)ability_->QueryServiceState()); // after mock
    ability_->OnStart(); // restart ability
    EXPECT_EQ(ServiceRunningState::STATE_NOT_START,
        (ServiceRunningState)ability_->QueryServiceState()); // after mock
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] NetworkOnStartAndOnStop001 end");
}


HWTEST_F(NetworkAbilityTest, NetworkDump001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, NetworkDump001, TestSize.Level1";
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] NetworkDump001 begin");
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
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] NetworkDump001 end");
}

HWTEST_F(NetworkAbilityTest, NetworkSendReportMockLocationEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, NetworkSendReportMockLocationEvent001, TestSize.Level1";
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] NetworkSendReportMockLocationEvent001 begin");
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
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] NetworkSendReportMockLocationEvent001 end");
}

HWTEST_F(NetworkAbilityTest, NetworkSendReportMockLocationEvent002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, NetworkSendReportMockLocationEvent002, TestSize.Level1";
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] NetworkSendReportMockLocationEvent002 begin");
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
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] NetworkSendReportMockLocationEvent002 end");
}

/*
 * @tc.name: NetworkConnectNlpService001
 * @tc.desc: connect nlp service
 * @tc.type: FUNC
 */
HWTEST_F(NetworkAbilityTest, NetworkConnectNlpService001, TestSize.Level1)
{
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] NetworkConnectNlpService001 begin");
    EXPECT_EQ(false, ability_->ReConnectNlpService());
    LBSLOGI(NETWORK_TEST, "[NetworkAbilityTest] NetworkConnectNlpService001 end");
}

HWTEST_F(NetworkAbilityTest, ReConnectNlpService001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, ReConnectNlpService001, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityTest] ReConnectNlpService001 begin");
    auto ability = sptr<NetworkAbility>(new (std::nothrow) NetworkAbility());
    EXPECT_EQ(false, ability->ReConnectNlpService()); // Connect success
    LBSLOGI(NETWORK, "[NetworkAbilityTest] ReConnectNlpService001 end");
}

HWTEST_F(NetworkAbilityTest, NotifyConnected001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, NotifyConnected001, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NotifyConnected001 begin");
    auto ability = sptr<NetworkAbility>(new (std::nothrow) NetworkAbility());
    sptr<IRemoteObject> obj = nullptr;
    ability->NotifyConnected(obj);
    EXPECT_EQ(nullptr, ability->nlpServiceProxy_);

    ability->NotifyDisConnected();
    EXPECT_EQ(nullptr, ability->nlpServiceProxy_);
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NotifyConnected001 end");
}

HWTEST_F(NetworkAbilityTest, SendLocationRequest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, SendLocationRequest002, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityTest] SendLocationRequest002 begin");
    auto ability = sptr<NetworkAbility>(new (std::nothrow) NetworkAbility());
    WorkRecord workRecord;
    ability->SendLocationRequest(workRecord);
    LBSLOGI(NETWORK, "[NetworkAbilityTest] SendLocationRequest002 end");
}

HWTEST_F(NetworkAbilityTest, RequestRecord001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, RequestRecord001, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityTest] RequestRecord001 begin");
    auto ability = sptr<NetworkAbility>(new (std::nothrow) NetworkAbility());
    WorkRecord workRecord;
    ability->RequestRecord(workRecord, true);

    ability->RequestRecord(workRecord, false);
    LBSLOGI(NETWORK, "[NetworkAbilityTest] RequestRecord001 end");
}

HWTEST_F(NetworkAbilityTest, NetworkCallbackHostOnRemoteRequest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, NetworkCallbackHostOnRemoteRequest001, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NetworkCallbackHostOnRemoteRequest001 begin");
    sptr<NetworkCallbackHost> callback = new (std::nothrow) NetworkCallbackHost();
    uint32_t code = static_cast<uint32_t>(ILocatorCallback::RECEIVE_LOCATION_INFO_EVENT);
    MessageParcel data;
    data.WriteInterfaceToken(u"location.ILocatorCallback");
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(0, callback->OnRemoteRequest(code, data, reply, option));
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NetworkCallbackHostOnRemoteRequest001 end");
}

HWTEST_F(NetworkAbilityTest, NetworkCallbackHostOnRemoteRequest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, NetworkCallbackHostOnRemoteRequest002, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NetworkCallbackHostOnRemoteRequest002 begin");
    sptr<NetworkCallbackHost> callback = new (std::nothrow) NetworkCallbackHost();
    uint32_t code = 0;
    MessageParcel data;
    data.WriteInterfaceToken(u"location.ILocatorCallback");
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(0, callback->OnRemoteRequest(code, data, reply, option));
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NetworkCallbackHostOnRemoteRequest002 end");
}

HWTEST_F(NetworkAbilityTest, NetworkCallbackHostOnRemoteRequest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, NetworkCallbackHostOnRemoteRequest003, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NetworkCallbackHostOnRemoteRequest003 begin");
    sptr<NetworkCallbackHost> callback = new (std::nothrow) NetworkCallbackHost();
    uint32_t code = 0;
    MessageParcel data;
    data.WriteInterfaceToken(u"wrong.interface.token");
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(-1, callback->OnRemoteRequest(code, data, reply, option));
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NetworkCallbackHostOnRemoteRequest003 end");
}

HWTEST_F(NetworkAbilityTest, NetworkAbilityInit001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, NetworkAbilityInit001, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NetworkAbilityInit001 begin");
    ability_->registerToAbility_ = false;
    bool ret = ability_->Init(); // after mock, sa obj is nullptr
    EXPECT_EQ(false, ret);

    ability_->registerToAbility_ = true;
    ret = ability_->Init();
    EXPECT_EQ(true, ret);
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NetworkAbilityInit001 end");
}

HWTEST_F(NetworkAbilityTest, NetworkAbilityConnectNlpService001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, NetworkAbilityConnectNlpService001, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NetworkAbilityConnectNlpService001 begin");
    bool ret = ability_->ConnectNlpService();
    EXPECT_EQ(false, ret);
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NetworkAbilityConnectNlpService001 end");
}

HWTEST_F(NetworkAbilityTest, NetworkAbilityReConnectNlpService001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, NetworkAbilityReConnectNlpService001, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NetworkAbilityReConnectNlpService001 begin");
    bool ret = ability_->ReConnectNlpService();
    EXPECT_EQ(false, ret);
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NetworkAbilityReConnectNlpService001 end");
}

HWTEST_F(NetworkAbilityTest, NetworkAbilityProcessReportLocationMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, NetworkAbilityProcessReportLocationMock001, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NetworkAbilityProcessReportLocationMock001 begin");
    ability_->mockLocationIndex_ = -1;
    ability_->networkHandler_ = nullptr;
    ability_->ProcessReportLocationMock();
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NetworkAbilityProcessReportLocationMock001 end");
}

HWTEST_F(NetworkAbilityTest, NetworkAbilitySendReportMockLocationEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, NetworkAbilitySendReportMockLocationEvent001, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NetworkAbilitySendReportMockLocationEvent001 begin");
    ability_->networkHandler_ = nullptr;
    ability_->SendReportMockLocationEvent();

    ability_->networkHandler_ = std::make_shared<NetworkHandler>(AppExecFwk::EventRunner::Create(true));;
    ability_->SendReportMockLocationEvent();
    LBSLOGI(NETWORK, "[NetworkAbilityTest] NetworkAbilitySendReportMockLocationEvent001 end");
}

HWTEST_F(NetworkAbilityTest, NetworkAbilitySendMessage001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityStubTest, NetworkAbilitySendMessage001, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilitySendMessage001 begin");
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.INetworkAbility");
    requestParcel.WriteBuffer("data", 4);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    ability_->networkHandler_ = nullptr;
    ability_->SendMessage(0, requestParcel, reply);

    ability_->networkHandler_ = std::make_shared<NetworkHandler>(AppExecFwk::EventRunner::Create(true));;
    ability_->SendMessage(0, requestParcel, reply);
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilitySendMessage001 end");
}

HWTEST_F(NetworkAbilityTest, ResetServiceProxy001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityTest, ResetServiceProxy001, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityTest] ResetServiceProxy001 begin");
    auto ability = sptr<NetworkAbility>(new (std::nothrow) NetworkAbility());
    EXPECT_EQ(true, ability->ResetServiceProxy()); // Connect success
    LBSLOGI(NETWORK, "[NetworkAbilityTest] ResetServiceProxy001 end");
}
} // namespace Location
} // namespace OHOS
#endif // FEATURE_NETWORK_SUPPORT