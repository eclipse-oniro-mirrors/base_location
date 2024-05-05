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

#ifndef GNSS_ABILITY_H
#define GNSS_ABILITY_H
#ifdef FEATURE_GNSS_SUPPORT

#include <mutex>
#include <singleton.h>
#include <v2_0/ignss_interface.h>
#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
#include <v2_0/igeofence_interface.h>
#endif
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
#include <v2_0/ia_gnss_interface.h>
#endif

#include "event_handler.h"
#include "system_ability.h"
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
#include "agnss_event_callback.h"
#endif
#include "common_utils.h"
#include "constant_definition.h"
#include "gnss_ability_skeleton.h"
#include "i_gnss_status_callback.h"
#include "i_nmea_message_callback.h"
#include "subability_common.h"
#include "i_gnss_geofence_callback.h"
#include "geofence_request.h"
#include "locationhub_ipc_interface_code.h"
#include "geofence_event_callback.h"
#include "ipc_skeleton.h"
#ifdef NOTIFICATION_ENABLE
#include "notification_request.h"
#include "notification_helper.h"
#endif

namespace OHOS {
namespace Location {
#ifdef __aarch64__
#define VENDOR_GNSS_ADAPTER_SO_PATH "/system/lib64/vendorGnssAdapter.so"
#else
#define VENDOR_GNSS_ADAPTER_SO_PATH "/system/lib/vendorGnssAdapter.so"
#endif

using HDI::Location::Gnss::V2_0::IGnssInterface;
using HDI::Location::Gnss::V2_0::IGnssCallback;
using HDI::Location::Gnss::V2_0::GNSS_START_TYPE_NORMAL;
using HDI::Location::Gnss::V2_0::GNSS_WORKING_STATUS_NONE;
using HDI::Location::Gnss::V2_0::GNSS_WORKING_STATUS_SESSION_BEGIN;
using HDI::Location::Gnss::V2_0::GNSS_WORKING_STATUS_SESSION_END;
using HDI::Location::Gnss::V2_0::GNSS_WORKING_STATUS_ENGINE_ON;
using HDI::Location::Gnss::V2_0::GNSS_WORKING_STATUS_ENGINE_OFF;
using HDI::Location::Gnss::V2_0::GnssAuxiliaryDataType;
using HDI::Location::Gnss::V2_0::GnssWorkingMode;
using HDI::Location::Gnss::V2_0::GnssConfigPara;
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
using HDI::Location::Agnss::V2_0::IAGnssInterface;
using HDI::Location::Agnss::V2_0::IAGnssCallback;
using HDI::Location::Agnss::V2_0::AGNSS_TYPE_SUPL;
using HDI::Location::Agnss::V2_0::AGnssServerInfo;
#endif
#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
using HDI::Location::Geofence::V2_0::IGeofenceInterface;
using HDI::Location::Geofence::V2_0::IGeofenceCallback;
using HDI::Location::Geofence::V2_0::GeofenceEvent;
using HDI::Location::Geofence::V2_0::GeofenceInfo;
#endif

typedef struct {
    std::shared_ptr<GeofenceRequest> request;
#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
    sptr<IGeofenceCallback> callback;
#endif
    int requestCode;
    int retCode;
    std::vector<CoordinateSystemType> coordinateSystemTypes;
} FenceStruct;

class GnssHandler : public AppExecFwk::EventHandler {
public:
    explicit GnssHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~GnssHandler() override;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;

private:
    void InitGnssEventProcessMap();
    void HandleReportMockLocation(const AppExecFwk::InnerEvent::Pointer& event);
    void HandleSendLocationRequest(const AppExecFwk::InnerEvent::Pointer& event);
    void HandleSetMockedLocations(const AppExecFwk::InnerEvent::Pointer& event);
    void HandleSendCommands(const AppExecFwk::InnerEvent::Pointer& event);
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    void HandleSetSubscriberSetId(const AppExecFwk::InnerEvent::Pointer& event);
    void HandleSetAgnssRefInfo(const AppExecFwk::InnerEvent::Pointer& event);
#endif
    void HandleReconnectHdi(const AppExecFwk::InnerEvent::Pointer& event);
    void HandleSetEnable(const AppExecFwk::InnerEvent::Pointer& event);
    void HandleInitHdi(const AppExecFwk::InnerEvent::Pointer& event);

    using GnssEventProcessHandle = void (GnssHandler::*)(const AppExecFwk::InnerEvent::Pointer& event);
    using GnssEventProcessMap = std::map<uint32_t, GnssEventProcessHandle>;
    GnssEventProcessMap gnssEventProcessMap_;
};

class GnssAbility : public SystemAbility, public GnssAbilityStub, public SubAbility, DelayedSingleton<GnssAbility> {
DECLEAR_SYSTEM_ABILITY(GnssAbility);

public:
    DISALLOW_COPY_AND_MOVE(GnssAbility);
    GnssAbility();
    ~GnssAbility() override;
    void OnStart() override;
    void OnStop() override;
    ServiceRunningState QueryServiceState() const
    {
        return state_;
    }
    LocationErrCode SendLocationRequest(WorkRecord &workrecord) override;
    LocationErrCode SetEnable(bool state) override;
    LocationErrCode RefrashRequirements() override;
    LocationErrCode RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    LocationErrCode UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback) override;
    LocationErrCode RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    LocationErrCode UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback) override;
    LocationErrCode RegisterCachedCallback(const std::unique_ptr<CachedGnssLocationsRequest>& request,
        const sptr<IRemoteObject>& callback) override;
    LocationErrCode UnregisterCachedCallback(const sptr<IRemoteObject>& callback) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    LocationErrCode GetCachedGnssLocationsSize(int &size) override;
    LocationErrCode FlushCachedGnssLocations() override;
    LocationErrCode SendCommand(std::unique_ptr<LocationCommand>& commands) override;
    LocationErrCode AddFence(std::shared_ptr<GeofenceRequest>& request) override;
    LocationErrCode RemoveFence(std::shared_ptr<GeofenceRequest>& request) override;
    LocationErrCode AddGnssGeofence(std::shared_ptr<GeofenceRequest>& request) override;
    LocationErrCode RemoveGnssGeofence(std::shared_ptr<GeofenceRequest>& request) override;
    void ReportGnssSessionStatus(int status);
    void ReportNmea(int64_t timestamp, const std::string &nmea);
    void ReportSv(const std::unique_ptr<SatelliteStatus> &sv);
    LocationErrCode EnableMock() override;
    LocationErrCode DisableMock() override;
    LocationErrCode SetMocked(const int timeInterval, const std::vector<std::shared_ptr<Location>> &location) override;
    void RequestRecord(WorkRecord &workRecord, bool isAdded) override;
    void SendReportMockLocationEvent() override;
    void SendMessage(uint32_t code, MessageParcel &data, MessageParcel &reply) override;
    void UnloadGnssSystemAbility() override;
    void StartGnss();
    void StopGnss();
    bool EnableGnss();
    void DisableGnss();
    bool ConnectHdi();
    bool RemoveHdi();
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    void SetAgnssServer();
    void SetAgnssCallback();
    void SetSetId(const SubscriberSetId& id);
    void SetSetIdImpl(const SubscriberSetId &id);
    void SetRefInfo(const AGnssRefInfo& refInfo);
    void SetRefInfoImpl(const AGnssRefInfo &refInfo);
#endif
#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
    bool SetGeofenceCallback();
#endif
    void ReConnectHdiImpl();
    bool IsMockEnabled();
    void ProcessReportLocationMock();
    void ReConnectHdi();
    bool CheckIfHdiConnected();
    void RestGnssWorkStatus();
    bool RegisterGnssGeofenceCallback(std::shared_ptr<GeofenceRequest> &request,
        const sptr<IRemoteObject>& callback);
    bool UnregisterGnssGeofenceCallback(int fenceId);
    std::shared_ptr<GeofenceRequest> GetGeofenceRequestByFenceId(int fenceId);
#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
    void ReportGeofenceEvent(int fenceId, GeofenceEvent event);
    void ReportGeofenceOperationResult(
        int fenceId, GeofenceOperateType type, GeofenceOperateResult result);
#endif
    bool RemoveGnssGeofenceRequestByCallback(sptr<IRemoteObject> callbackObj);
    LocationErrCode QuerySupportCoordinateSystemType(
        std::vector<CoordinateSystemType>& coordinateSystemTypes) override;

private:
    bool Init();
    static void SaDumpInfo(std::string& result);
    bool IsGnssEnabled();
    int32_t ReportMockedLocation(const std::shared_ptr<Location> location);
    bool CheckIfGnssConnecting();
    bool IsMockProcessing();
    void RegisterLocationHdiDeathRecipient();
    bool GetCommandFlags(std::unique_ptr<LocationCommand>& commands, GnssAuxiliaryDataType& flags);
    LocationErrCode SetPositionMode();
    void SendEvent(AppExecFwk::InnerEvent::Pointer& event, MessageParcel &reply);
    bool ExecuteFenceProcess(
        GnssInterfaceCode code, std::shared_ptr<GeofenceRequest>& request);
    int32_t GenerateFenceId();
    bool IsGnssfenceRequestMapExist();
    bool CheckBundleNameInGnssGeofenceRequestMap(std::string bundleName, int fenceId);

    size_t mockLocationIndex_ = 0;
    bool registerToAbility_ = false;
    int gnssWorkingStatus_ = 0;
    std::shared_ptr<GnssHandler> gnssHandler_;
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
    std::mutex gnssMutex_;
    std::mutex nmeaMutex_;
    std::mutex hdiMutex_;
    std::mutex statusMutex_;
    std::vector<sptr<IGnssStatusCallback>> gnssStatusCallback_;
    std::vector<sptr<INmeaMessageCallback>> nmeaCallback_;
    sptr<IGnssInterface> gnssInterface_;
    sptr<IGnssCallback> gnssCallback_;
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    sptr<IAGnssCallback> agnssCallback_;
    sptr<IAGnssInterface> agnssInterface_;
#endif
#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
    sptr<IGeofenceInterface> geofenceInterface_;
    sptr<IGeofenceCallback> geofenceCallback_;
#endif
    int32_t fenceId_;
    std::mutex fenceIdMutex_;
    std::mutex gnssGeofenceRequestMapMutex_;
    std::map<std::shared_ptr<GeofenceRequest>, sptr<IRemoteObject>> gnssGeofenceRequestMap_;
};

class LocationHdiDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    LocationHdiDeathRecipient();
    ~LocationHdiDeathRecipient() override;
};

#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
class AgnssRefInfoMessage {
public:
    void SetAgnssRefInfo(const AGnssRefInfo &refInfo);
    AGnssRefInfo GetAgnssRefInfo();

private:
    AGnssRefInfo agnssRefInfo_;
};
#endif
} // namespace Location
} // namespace OHOS
#endif // FEATURE_GNSS_SUPPORT
#endif // GNSS_ABILITY_H
