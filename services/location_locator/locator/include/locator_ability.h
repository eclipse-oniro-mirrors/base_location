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

#ifndef LOCATOR_ABILITY_H
#define LOCATOR_ABILITY_H

#include <map>
#include <mutex>
#include <singleton.h>

#include "event_handler.h"
#include "system_ability.h"

#include "app_identity.h"
#include "common_utils.h"
#include "geo_coding_mock_info.h"
#include "i_switch_callback.h"
#include "i_cached_locations_callback.h"
#include "locator_event_subscriber.h"
#include "locator_skeleton.h"
#include "permission_status_change_cb.h"
#include "request.h"
#include "request_manager.h"
#include "report_manager.h"

namespace OHOS {
namespace Location {
class LocatorHandler : public AppExecFwk::EventHandler {
public:
    using LocatorEventHandle = void (LocatorHandler::*)(
        const AppExecFwk::InnerEvent::Pointer& event);
    using LocatorEventHandleMap = std::map<int, LocatorEventHandle>;
    explicit LocatorHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~LocatorHandler() override;
    void InitLocatorHandlerEventMap();
private:
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;
    void UpdateSaEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void InitRequestManagerEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void ApplyRequirementsEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void RetryRegisterActionEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void ReportLocationMessageEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void SendSwitchStateToHifenceEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void UnloadSaEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void StartLocatingEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void StopLocatingEvent(const AppExecFwk::InnerEvent::Pointer& event);
    LocatorEventHandleMap locatorHandlerEventMap_;
};

class LocatorAbility : public SystemAbility, public LocatorAbilityStub, DelayedSingleton<LocatorAbility> {
DECLEAR_SYSTEM_ABILITY(LocatorAbility);

public:
    DISALLOW_COPY_AND_MOVE(LocatorAbility);
    LocatorAbility();
    ~LocatorAbility() override;
    void OnStart() override;
    void OnStop() override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    ServiceRunningState QueryServiceState() const
    {
        return state_;
    }
    void InitSaAbility();
    void InitRequestManagerMap();

    LocationErrCode UpdateSaAbility();
    LocationErrCode GetSwitchState(int& state);
    LocationErrCode EnableAbility(bool isEnabled);
    LocationErrCode RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid);
    LocationErrCode UnregisterSwitchCallback(const sptr<IRemoteObject>& callback);
#ifdef FEATURE_GNSS_SUPPORT
    LocationErrCode RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid);
    LocationErrCode UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback);
    LocationErrCode RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid);
    LocationErrCode UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback);
    LocationErrCode RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback, std::string bundleName);
    LocationErrCode UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback);
    LocationErrCode GetCachedGnssLocationsSize(int& size);
    LocationErrCode FlushCachedGnssLocations();
    LocationErrCode SendCommand(std::unique_ptr<LocationCommand>& commands);
    LocationErrCode AddFence(std::unique_ptr<GeofenceRequest>& request);
    LocationErrCode RemoveFence(std::unique_ptr<GeofenceRequest>& request);
#endif
    LocationErrCode StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback, AppIdentity &identity);
    LocationErrCode StopLocating(sptr<ILocatorCallback>& callback);
    LocationErrCode GetCacheLocation(std::unique_ptr<Location>& loc, AppIdentity &identity);
#ifdef FEATURE_GEOCODE_SUPPORT
    LocationErrCode IsGeoConvertAvailable(bool &isAvailable);
    void GetAddressByCoordinate(MessageParcel &data, MessageParcel &reply, std::string bundleName);
    void GetAddressByLocationName(MessageParcel &data, MessageParcel &reply, std::string bundleName);
    LocationErrCode EnableReverseGeocodingMock();
    LocationErrCode DisableReverseGeocodingMock();
    LocationErrCode SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo);
#endif
    LocationErrCode IsLocationPrivacyConfirmed(const int type, bool& isConfirmed);
    LocationErrCode SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed);
    LocationErrCode EnableLocationMock();
    LocationErrCode DisableLocationMock();
    LocationErrCode SetMockedLocations(
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location);
    LocationErrCode ReportLocation(const std::unique_ptr<Location>& location, std::string abilityName);
    LocationErrCode ReportLocationStatus(sptr<ILocatorCallback>& callback, int result);
    LocationErrCode ReportErrorStatus(sptr<ILocatorCallback>& callback, int result);
    LocationErrCode ProcessLocationMockMsg(
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location, int msgId);
#ifdef FEATURE_GNSS_SUPPORT
    LocationErrCode SendLocationMockMsgToGnssSa(const sptr<IRemoteObject> obj,
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location, int msgId);
#endif
#ifdef FEATURE_NETWORK_SUPPORT
    LocationErrCode SendLocationMockMsgToNetworkSa(const sptr<IRemoteObject> obj,
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location, int msgId);
#endif
#ifdef FEATURE_PASSIVE_SUPPORT
    LocationErrCode SendLocationMockMsgToPassiveSa(const sptr<IRemoteObject> obj,
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location, int msgId);
#endif
    LocationErrCode RegisterWifiScanInfoCallback(const sptr<IRemoteObject>& callback, pid_t uid);
    LocationErrCode UnregisterWifiScanInfoCallback(const sptr<IRemoteObject>& callback);
    LocationErrCode RegisterBluetoothScanInfoCallback(const sptr<IRemoteObject>& callback, pid_t uid);
    LocationErrCode UnregisterBluetoothScanInfoCallback(const sptr<IRemoteObject>& callback);
    LocationErrCode RegisterBleScanInfoCallback(const sptr<IRemoteObject>& callback, pid_t uid);
    LocationErrCode UnregisterBleScanInfoCallback(const sptr<IRemoteObject>& callback);

    std::shared_ptr<std::map<std::string, std::list<std::shared_ptr<Request>>>> GetRequests();
    std::shared_ptr<std::map<sptr<IRemoteObject>, std::list<std::shared_ptr<Request>>>> GetReceivers();
    std::shared_ptr<std::map<std::string, sptr<IRemoteObject>>> GetProxyMap();
    void UpdateSaAbilityHandler();
    void ApplyRequests(int delay);
    void RegisterAction();
    LocationErrCode ProxyUidForFreeze(int32_t uid, bool isProxy);
    LocationErrCode ResetAllProxy();
    bool IsProxyUid(int32_t uid);
    int GetActiveRequestNum();
    void RegisterPermissionCallback(const uint32_t callingTokenId, const std::vector<std::string>& permissionNameList);
    void UnregisterPermissionCallback(const uint32_t callingTokenId);
    void RemoveUnloadTask(uint32_t code);
    void PostUnloadTask(uint32_t code);
    std::set<int32_t> GetProxyUid();
    void UpdatePermissionUsedRecord(uint32_t tokenId, std::string permissionName, int succCnt, int failCnt);

private:
    bool Init();
    bool CheckSaValid();
#ifdef FEATURE_GEOCODE_SUPPORT
    LocationErrCode SendGeoRequest(int type, MessageParcel &data, MessageParcel &reply);
#endif
#ifdef FEATURE_GNSS_SUPPORT
    LocationErrCode SendGnssRequest(int type, MessageParcel &data, MessageParcel &reply);
#endif
    void UpdateProxyMap();
    bool CheckIfLocatorConnecting();
    void UpdateLoadedSaMap();
    bool NeedReportCacheLocation(const std::shared_ptr<Request>& request, sptr<ILocatorCallback>& callback);
    void HandleStartLocating(const std::shared_ptr<Request>& request, sptr<ILocatorCallback>& callback);
    bool IsCacheVaildScenario(const sptr<RequestConfig>& requestConfig);
    void SendSwitchState(const int state);
    std::shared_ptr<Request> InitRequest(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback, AppIdentity &identity);

    bool registerToAbility_ = false;
    bool isActionRegistered = false;
    std::string deviceId_;
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
    std::shared_ptr<LocatorEventSubscriber> locatorEventSubscriber_;
    std::mutex switchMutex_;
    std::mutex requestsMutex_;
    std::mutex receiversMutex_;
    std::mutex proxyMapMutex_;
    std::mutex permissionMapMutex_;
    std::mutex loadedSaMapMutex_;
    std::unique_ptr<std::map<pid_t, sptr<ISwitchCallback>>> switchCallbacks_;
    std::shared_ptr<std::map<std::string, std::list<std::shared_ptr<Request>>>> requests_;
    std::shared_ptr<std::map<sptr<IRemoteObject>, std::list<std::shared_ptr<Request>>>> receivers_;
    std::shared_ptr<std::map<std::string, sptr<IRemoteObject>>> proxyMap_;
    std::shared_ptr<std::map<std::string, sptr<IRemoteObject>>> loadedSaMap_;
    std::shared_ptr<std::map<uint32_t, std::shared_ptr<PermissionStatusChangeCb>>> permissionMap_;
    std::shared_ptr<LocatorHandler> locatorHandler_;
    std::shared_ptr<RequestManager> requestManager_;
    std::shared_ptr<ReportManager> reportManager_;
    std::mutex proxyUidsMutex_;
    std::set<int32_t> proxyUids_;
};

class LocationMessage {
public:
    void SetAbilityName(std::string abilityName);
    std::string GetAbilityName();
    void SetLocation(const std::unique_ptr<Location>& location);
    std::unique_ptr<Location> GetLocation();
private:
    std::string abilityName_;
    std::unique_ptr<Location> location_;
};

class LocatorCallbackMessage {
public:
    void SetCallback(const sptr<ILocatorCallback>& callback);
    sptr<ILocatorCallback> GetCallback();
private:
    std::string abilityName_;
    sptr<ILocatorCallback> callback_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_ABILITY_H
