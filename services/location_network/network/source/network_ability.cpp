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

#include "network_ability.h"
#include <file_ex.h>
#include "ability_connect_callback_interface.h"
#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "want_agent_helper.h"

#include "common_utils.h"
#include "location_log.h"
#include "location_dumper.h"
#include "locator_ability.h"

namespace OHOS {
namespace Location {
const uint32_t EVENT_REPORT_LOCATION = 0x0001;
const uint32_t EVENT_INTERVAL_UNITE = 1000;
const bool REGISTER_RESULT = NetworkAbility::MakeAndRegisterAbility(
    DelayedSingleton<NetworkAbility>::GetInstance().get());

NetworkAbility::NetworkAbility() : SystemAbility(LOCATION_NETWORK_LOCATING_SA_ID, true)
{
    SetAbility(NETWORK_ABILITY);
    networkHandler_ = std::make_shared<NetworkHandler>(AppExecFwk::EventRunner::Create(true));
    LBSLOGI(NETWORK, "ability constructed.");
}

NetworkAbility::~NetworkAbility() {}

void NetworkAbility::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        LBSLOGI(NETWORK, "ability has already started.");
        return;
    }
    if (!Init()) {
        LBSLOGE(NETWORK, "failed to init ability");
        OnStop();
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    LBSLOGI(NETWORK, "OnStart start ability success.");
}

void NetworkAbility::OnStop()
{
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToAbility_ = false;
    LBSLOGI(NETWORK, "OnStop ability stopped.");
}

bool NetworkAbility::Init()
{
    if (!registerToAbility_) {
        if (!Publish(AsObject())) {
            LBSLOGE(NETWORK, "Init Publish failed!");
            return false;
        }
        registerToAbility_ = true;
    }
    uuid_ = std::to_string(CommonUtils::IntRandom(MIN_INT_RANDOM, MAX_INT_RANDOM));
    callback_ = new (std::nothrow) NetworkCallbackHost();
    if (callback_ == nullptr) {
        LBSLOGE(NETWORK, "create NetworkCallbackHost failed.");
        return false;
    }
    LBSLOGI(NETWORK, "Init() success.");
    return true;
}

class AbilityConnection : public AAFwk::AbilityConnectionStub {
public:
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override
    {
        std::string uri = element.GetURI();
        LBSLOGI(NETWORK, "Connected uri is %{public}s, result is %{public}d.", uri.c_str(), resultCode);
        if (resultCode != ERR_OK) {
            return;
        }
        DelayedSingleton<NetworkAbility>::GetInstance().get()->NotifyConnected(remoteObject);
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int) override
    {
        std::string uri = element.GetURI();
        LBSLOGI(NETWORK, "Disconnected uri is %{public}s.", uri.c_str());
        DelayedSingleton<NetworkAbility>::GetInstance().get()->NotifyDisConnected();
    }
};

bool NetworkAbility::ConnectNetworkService()
{
    LBSLOGI(NETWORK, "start ConnectNetworkService");
    std::unique_lock<std::mutex> uniqueLock(connectMutex_);
    if (!networkServiceReady_) {
        AAFwk::Want connectionWant;
        connectionWant.SetElementName(SERVICE_NAME, ABILITY_NAME);
        sptr<AAFwk::IAbilityConnection> conn = new AbilityConnection();
        int32_t ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(connectionWant, conn, -1);
        if (ret != ERR_OK) {
            LBSLOGE(NETWORK, "Connect cloud service failed!");
            return false;
        }

        auto waitStatus = connectCondition_.wait_for(
            uniqueLock, std::chrono::seconds(CONNECT_TIME_OUT), [this]() { return networkServiceReady_; });
        if (!waitStatus) {
            LBSLOGE(NETWORK, "Connect cloudService timeout!");
            return false;
        }
    }
    return true;
}

void NetworkAbility::NotifyConnected(const sptr<IRemoteObject>& remoteObject)
{
    std::unique_lock<std::mutex> uniqueLock(connectMutex_);
    networkServiceReady_ = true;
    networkServiceProxy_ = remoteObject;
    connectCondition_.notify_all();
}

void NetworkAbility::NotifyDisConnected()
{
    std::unique_lock<std::mutex> uniqueLock(connectMutex_);
    networkServiceReady_ = false;
    networkServiceProxy_ = nullptr;
    connectCondition_.notify_all();
}

void NetworkAbility::SendLocationRequest(WorkRecord &workrecord)
{
    LocationRequest(workrecord);
}

void NetworkAbility::SetEnable(bool state)
{
    Enable(state, AsObject());
}

void NetworkAbility::SelfRequest(bool state)
{
    LBSLOGI(NETWORK, "SelfRequest %{public}d", state);
    HandleSelfRequest(IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(), state);
}

void NetworkAbility::RequestRecord(WorkRecord &workRecord, bool isAdded)
{
    if (!networkServiceReady_ && !ConnectNetworkService()) {
        LBSLOGE(NETWORK, "network service is not ready.");
        return;
    }
    if (networkServiceProxy_ == nullptr) {
        LBSLOGE(NETWORK, "networkProxy is nullptr.");
        return;
    }
    MessageParcel data, reply;
    MessageOption option;
    if (isAdded) {
        LBSLOGD(NETWORK, "start network location, uuid=%{public}s", uuid_.c_str());
        if (GetRequestNum() == 0) {
            LBSLOGE(NETWORK, "no valid request.");
            return;
        }
        data.WriteString16(Str8ToStr16(uuid_));
        data.WriteInt64(workRecord.GetTimeInterval(0) * SEC_TO_MILLI_SEC);
        data.WriteInt32(LocationRequestType::PRIORITY_TYPE_BALANCED_POWER_ACCURACY);
        data.WriteRemoteObject(callback_->AsObject());
        int error = networkServiceProxy_->SendRequest(REQUEST_NETWORK_LOCATION, data, reply, option);
        if (error != ERR_OK) {
            LBSLOGE(NETWORK, "SendRequest to cloud service failed.");
            return;
        }
    } else {
        LBSLOGD(NETWORK, "stop network location, uuid=%{public}s", uuid_.c_str());
        if (GetRequestNum() != 0) {
            LBSLOGE(NETWORK, "exist valid request.");
            return;
        }
        data.WriteString16(Str8ToStr16(uuid_));
        int error = networkServiceProxy_->SendRequest(REMOVE_NETWORK_LOCATION, data, reply, option);
        if (error != ERR_OK) {
            LBSLOGE(NETWORK, "SendRequest to cloud service failed.");
            return;
        }
    }
}

bool NetworkAbility::EnableMock()
{
    return EnableLocationMock();
}

bool NetworkAbility::DisableMock()
{
    return DisableLocationMock();
}

bool NetworkAbility::IsMockEnabled()
{
    return IsLocationMocked();
}

bool NetworkAbility::SetMocked(const int timeInterval,
    const std::vector<std::shared_ptr<Location>> &location)
{
    return SetMockedLocations(timeInterval, location);
}

void NetworkAbility::ProcessReportLocationMock()
{
    std::vector<std::shared_ptr<Location>> mockLocationArray = GetLocationMock();
    if (mockLocationIndex_ < mockLocationArray.size()) {
        ReportMockedLocation(mockLocationArray[mockLocationIndex_++]);
        if (networkHandler_ != nullptr) {
            networkHandler_->SendHighPriorityEvent(EVENT_REPORT_LOCATION,
                0, GetTimeIntervalMock() * EVENT_INTERVAL_UNITE);
        }
    } else {
        ClearLocationMock();
        mockLocationIndex_ = 0;
    }
}

void NetworkAbility::SendReportMockLocationEvent()
{
    if (networkHandler_ == nullptr) {
        return;
    }
    networkHandler_->SendHighPriorityEvent(EVENT_REPORT_LOCATION, 0, 0);
}

int32_t NetworkAbility::ReportMockedLocation(const std::shared_ptr<Location> location)
{
    std::unique_ptr<Location> locationNew = std::make_unique<Location>();
    locationNew->SetLatitude(location->GetLatitude());
    locationNew->SetLongitude(location->GetLongitude());
    locationNew->SetAltitude(location->GetAltitude());
    locationNew->SetAccuracy(location->GetAccuracy());
    locationNew->SetSpeed(location->GetSpeed());
    locationNew->SetDirection(location->GetDirection());
    locationNew->SetTimeStamp(location->GetTimeStamp());
    locationNew->SetTimeSinceBoot(location->GetTimeSinceBoot());
    locationNew->SetAdditions(location->GetAdditions());
    locationNew->SetAdditionSize(location->GetAdditionSize());
    locationNew->SetIsFromMock(location->GetIsFromMock());
    if ((IsLocationMocked() && !location->GetIsFromMock()) ||
        (!IsLocationMocked() && location->GetIsFromMock())) {
        LBSLOGE(NETWORK, "location mock is enabled, do not report gnss location!");
        return ERR_OK;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility != nullptr) {
        locatorAbility.get()->ReportLocation(locationNew, NETWORK_ABILITY);
        locatorAbility.get()->ReportLocation(locationNew, PASSIVE_ABILITY);
    }
    return ERR_OK;
}

void NetworkAbility::SaDumpInfo(std::string& result)
{
    result += "Network Location enable status: false";
    result += "\n";
}

int32_t NetworkAbility::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::vector<std::string> vecArgs;
    std::transform(args.begin(), args.end(), std::back_inserter(vecArgs), [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });

    LocationDumper dumper;
    std::string result;
    dumper.NetWorkDump(SaDumpInfo, vecArgs, result);
    if (!SaveStringToFd(fd, result)) {
        LBSLOGE(NETWORK, "Network save string to fd failed!");
        return ERR_OK;
    }
    return ERR_OK;
}

void NetworkAbility::SendMessage(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    if (networkHandler_ == nullptr) {
        return;
    }
    switch (code) {
        case SET_MOCKED_LOCATIONS: {
            if (!IsMockEnabled()) {
                reply.WriteBool(false);
                break;
            }
            int timeInterval = data.ReadInt32();
            int locationSize = data.ReadInt32();
            locationSize = locationSize > INPUT_ARRAY_LEN_MAX ? INPUT_ARRAY_LEN_MAX :
                locationSize;
            std::shared_ptr<std::vector<std::shared_ptr<Location>>> vcLoc =
                std::make_shared<std::vector<std::shared_ptr<Location>>>();
            for (int i = 0; i < locationSize; i++) {
                vcLoc->push_back(Location::UnmarshallingShared(data));
            }
            AppExecFwk::InnerEvent::Pointer event =
                AppExecFwk::InnerEvent::Get(code, vcLoc, timeInterval);
            bool result = networkHandler_->SendEvent(event);
            reply.WriteBool(result);
            break;
        }
        default:
            break;
    }
}

NetworkHandler::NetworkHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner) {}

NetworkHandler::~NetworkHandler() {}

void NetworkHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto networkAbility = DelayedSingleton<NetworkAbility>::GetInstance();
    if (networkAbility == nullptr) {
        LBSLOGE(NETWORK, "ProcessEvent: NetworkAbility is nullptr");
        return;
    }
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGI(NETWORK, "ProcessEvent event:%{public}d", eventId);
    switch (eventId) {
        case EVENT_REPORT_LOCATION: {
            networkAbility->ProcessReportLocationMock();
            break;
        }
        case ISubAbility::SET_MOCKED_LOCATIONS: {
            int timeInterval = event->GetParam();
            auto vcLoc = event->GetSharedObject<std::vector<std::shared_ptr<Location>>>();
            if (vcLoc != nullptr) {
                std::vector<std::shared_ptr<Location>> mockLocations;
                for (auto it = vcLoc->begin(); it != vcLoc->end(); ++it) {
                    mockLocations.push_back(*it);
                }
                networkAbility->SetMocked(timeInterval, mockLocations);
            }
            break;
        }
        default:
            break;
    }
}
} // namespace Location
} // namespace OHOS