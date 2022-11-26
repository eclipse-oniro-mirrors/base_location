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

#include "locator_skeleton.h"

#include <file_ex.h>
#include "ipc_skeleton.h"
#include "common_utils.h"
#include "i_locator_callback.h"
#include "location.h"
#include "locator_ability.h"
#include "location_dumper.h"
#include "request_config.h"

namespace OHOS {
namespace Location {
void LocatorAbilityStub::InitLocatorHandleMap()
{
    if (locatorHandleMap_.size() != 0 || locatorHandleMap_.size() != 0) {
        return;
    }
    locatorHandleMap_[GET_SWITCH_STATE] = &LocatorAbilityStub::PreGetSwitchState;
    locatorHandleMap_[REG_SWITCH_CALLBACK] = &LocatorAbilityStub::PreRegisterSwitchCallback;
    locatorHandleMap_[START_LOCATING] = &LocatorAbilityStub::PreStartLocating;
    locatorHandleMap_[STOP_LOCATING] = &LocatorAbilityStub::PreStopLocating;
    locatorHandleMap_[GET_CACHE_LOCATION] = &LocatorAbilityStub::PreGetCacheLocation;
    locatorHandleMap_[ENABLE_ABILITY] = &LocatorAbilityStub::PreEnableAbility;
    locatorHandleMap_[UPDATE_SA_ABILITY] = &LocatorAbilityStub::PreUpdateSaAbility;
    locatorHandleMap_[GEO_IS_AVAILABLE] = &LocatorAbilityStub::PreIsGeoConvertAvailable;
    locatorHandleMap_[GET_FROM_COORDINATE] = &LocatorAbilityStub::PreGetAddressByCoordinate;
    locatorHandleMap_[GET_FROM_LOCATION_NAME] = &LocatorAbilityStub::PreGetAddressByLocationName;
    locatorHandleMap_[UNREG_SWITCH_CALLBACK] = &LocatorAbilityStub::PreUnregisterSwitchCallback;
    locatorHandleMap_[REG_GNSS_STATUS_CALLBACK] = &LocatorAbilityStub::PreRegisterGnssStatusCallback;
    locatorHandleMap_[UNREG_GNSS_STATUS_CALLBACK] = &LocatorAbilityStub::PreUnregisterGnssStatusCallback;
    locatorHandleMap_[REG_NMEA_CALLBACK] = &LocatorAbilityStub::PreRegisterNmeaMessageCallback;
    locatorHandleMap_[UNREG_NMEA_CALLBACK] = &LocatorAbilityStub::PreUnregisterNmeaMessageCallback;
    locatorHandleMap_[IS_PRIVACY_COMFIRMED] = &LocatorAbilityStub::PreIsLocationPrivacyConfirmed;
    locatorHandleMap_[SET_PRIVACY_COMFIRM_STATUS] = &LocatorAbilityStub::PreSetLocationPrivacyConfirmStatus;
    locatorHandleMap_[REG_CACHED_CALLBACK] = &LocatorAbilityStub::PreStartCacheLocating;
    locatorHandleMap_[UNREG_CACHED_CALLBACK] = &LocatorAbilityStub::PreStopCacheLocating;
    locatorHandleMap_[GET_CACHED_LOCATION_SIZE] = &LocatorAbilityStub::PreGetCachedGnssLocationsSize;
    locatorHandleMap_[FLUSH_CACHED_LOCATIONS] = &LocatorAbilityStub::PreFlushCachedGnssLocations;
    locatorHandleMap_[SEND_COMMAND] = &LocatorAbilityStub::PreSendCommand;
    locatorHandleMap_[ADD_FENCE] = &LocatorAbilityStub::PreAddFence;
    locatorHandleMap_[REMOVE_FENCE] = &LocatorAbilityStub::PreRemoveFence;
    locatorHandleMap_[GET_ISO_COUNTRY_CODE] = &LocatorAbilityStub::PreGetIsoCountryCode;
    locatorHandleMap_[ENABLE_LOCATION_MOCK] = &LocatorAbilityStub::PreEnableLocationMock;
    locatorHandleMap_[DISABLE_LOCATION_MOCK] = &LocatorAbilityStub::PreDisableLocationMock;
    locatorHandleMap_[SET_MOCKED_LOCATIONS] = &LocatorAbilityStub::PreSetMockedLocations;
    locatorHandleMap_[ENABLE_REVERSE_GEOCODE_MOCK] = &LocatorAbilityStub::PreEnableReverseGeocodingMock;
    locatorHandleMap_[DISABLE_REVERSE_GEOCODE_MOCK] = &LocatorAbilityStub::PreDisableReverseGeocodingMock;
    locatorHandleMap_[SET_REVERSE_GEOCODE_MOCKINFO] = &LocatorAbilityStub::PreSetReverseGeocodingMockInfo;
    locatorHandleMap_[REG_COUNTRY_CODE_CALLBACK] = &LocatorAbilityStub::PreRegisterCountryCodeCallback;
    locatorHandleMap_[UNREG_COUNTRY_CODE_CALLBACK] = &LocatorAbilityStub::PreUnregisterCountryCodeCallback;
    locatorHandleMap_[PROXY_UID_FOR_FREEZE] = &LocatorAbilityStub::PreProxyUidForFreeze;
    locatorHandleMap_[RESET_ALL_PROXY] = &LocatorAbilityStub::PreResetAllProxy;
    locatorHandleMap_[REG_NMEA_CALLBACK_v9] = &LocatorAbilityStub::PreRegisterNmeaMessageCallbackV9;
    locatorHandleMap_[UNREG_NMEA_CALLBACK_v9] = &LocatorAbilityStub::PreUnregisterNmeaMessageCallbackV9;
}


LocatorAbilityStub::LocatorAbilityStub()
{
    InitLocatorHandleMap();
}

int LocatorAbilityStub::PreGetSwitchState(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreGetSwitchState: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    reply.WriteInt32(locatorAbility.get()->GetSwitchState());
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreRegisterSwitchCallback(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreRegisterSwitchCallback: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    locatorAbility.get()->RegisterSwitchCallback(client, identity.GetUid());
    return REPLY_CODE_NO_EXCEPTION;
}


int LocatorAbilityStub::PreStartLocating(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreStartLocating: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    if (locatorAbility.get()->GetSwitchState() == DISABLED) {
        LBSLOGE(LOCATOR, "switch state is off.");
        return REPLY_CODE_SWITCH_OFF_EXCEPTION;
    }
    std::unique_ptr<RequestConfig> requestConfig = RequestConfig::Unmarshalling(data);
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    std::string bundleName = Str16ToStr8(data.ReadString16());
    if (remoteObject == nullptr) {
        LBSLOGE(LOCATOR, "StartLocating remote object nullptr");
        return REPLY_CODE_EXCEPTION;
    }
    if (bundleName.empty()) {
        LBSLOGE(LOCATOR, "StartLocating get empty bundle name");
        return REPLY_CODE_EXCEPTION;
    }

    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) LocatorCallbackDeathRecipient());
    remoteObject->AddDeathRecipient(death.GetRefPtr());
    sptr<ILocatorCallback> callback = iface_cast<ILocatorCallback>(remoteObject);
    locatorAbility.get()->StartLocating(requestConfig, callback, identity);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreStopLocating(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreStopLocating: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        LBSLOGE(LOCATOR, "LocatorAbility::StartLocating remote object nullptr");
        return REPLY_CODE_EXCEPTION;
    }
    sptr<ILocatorCallback> callback = iface_cast<ILocatorCallback>(remoteObject);
    locatorAbility.get()->StopLocating(callback);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreGetCacheLocation(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreGetCacheLocation: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    if (locatorAbility.get()->GetSwitchState() == DISABLED) {
        LBSLOGE(LOCATOR, "switch state is off.");
        return REPLY_CODE_SWITCH_OFF_EXCEPTION;
    }
    int ret = DelayedSingleton<LocatorAbility>::GetInstance().get()->GetCacheLocation(reply, identity);
    return ret;
}

int LocatorAbilityStub::PreEnableAbility(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetUid(), identity.GetTokenId())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    if (!CheckSettingsPermission(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreEnableAbility: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    bool isEnabled = data.ReadBool();
    locatorAbility.get()->EnableAbility(isEnabled);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreUpdateSaAbility(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetUid(), identity.GetTokenId())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreUpdateSaAbility: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    locatorAbility.get()->UpdateSaAbility();
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreIsGeoConvertAvailable(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreIsGeoConvertAvailable: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    int ret = locatorAbility.get()->IsGeoConvertAvailable(reply);
    return ret;
}

int LocatorAbilityStub::PreGetAddressByCoordinate(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreGetAddressByCoordinate: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    int ret = locatorAbility.get()->GetAddressByCoordinate(data, reply);
    return ret;
}

int LocatorAbilityStub::PreGetAddressByLocationName(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreGetAddressByLocationName: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    int ret = locatorAbility.get()->GetAddressByLocationName(data, reply);
    return ret;
}

int LocatorAbilityStub::PreUnregisterSwitchCallback(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreUnregisterSwitchCallback: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    locatorAbility.get()->UnregisterSwitchCallback(client);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreRegisterGnssStatusCallback(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreRegisterGnssStatusCallback: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    locatorAbility.get()->RegisterGnssStatusCallback(client, identity.GetUid());
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreUnregisterGnssStatusCallback(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreUnregisterGnssStatusCallback: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    locatorAbility.get()->UnregisterGnssStatusCallback(client);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreRegisterNmeaMessageCallback(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreRegisterNmeaMessageCallback: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    locatorAbility.get()->RegisterNmeaMessageCallback(client, identity.GetUid());
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreUnregisterNmeaMessageCallback(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreUnregisterNmeaMessageCallback: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    locatorAbility.get()->UnregisterNmeaMessageCallback(client);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreRegisterNmeaMessageCallbackV9(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckPreciseLocationPermissions(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    DelayedSingleton<LocatorAbility>::GetInstance().get()->RegisterNmeaMessageCallback(client, identity.GetUid());
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreUnregisterNmeaMessageCallbackV9(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckPreciseLocationPermissions(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    DelayedSingleton<LocatorAbility>::GetInstance().get()->UnregisterNmeaMessageCallback(client);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreIsLocationPrivacyConfirmed(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetUid(), identity.GetTokenId())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreIsLocationPrivacyConfirmed: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    int ret = locatorAbility.get()->IsLocationPrivacyConfirmed(data.ReadInt32());
    reply.WriteInt32(ret);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreSetLocationPrivacyConfirmStatus(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetUid(), identity.GetTokenId())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    if (!CheckSettingsPermission(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }

    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreSetLocationPrivacyConfirmStatus: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    int ret = locatorAbility.get()->SetLocationPrivacyConfirmStatus(data.ReadInt32(),
        data.ReadBool());
    reply.WriteInt32(ret);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreStartCacheLocating(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreStartCacheLocating: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    if (locatorAbility.get()->GetSwitchState() == DISABLED) {
        LBSLOGE(LOCATOR, "switch state is off.");
        return REPLY_CODE_SWITCH_OFF_EXCEPTION;
    }
    std::unique_ptr<CachedGnssLocationsRequest> requestConfig = std::make_unique<CachedGnssLocationsRequest>();
    requestConfig->reportingPeriodSec = data.ReadInt32();
    requestConfig->wakeUpCacheQueueFull = data.ReadBool();
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    std::string bundleName = Str16ToStr8(data.ReadString16());
    if (remoteObject == nullptr) {
        LBSLOGE(LOCATOR, "ParseDataAndStartCacheLocating remote object nullptr");
        return REPLY_CODE_EXCEPTION;
    }
    if (bundleName.empty()) {
        LBSLOGE(LOCATOR, "ParseDataAndStartCacheLocating get empty bundle name");
        return REPLY_CODE_EXCEPTION;
    }

    sptr<ICachedLocationsCallback> callback = iface_cast<ICachedLocationsCallback>(remoteObject);
    locatorAbility.get()->RegisterCachedLocationCallback(requestConfig,
        callback, bundleName);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreStopCacheLocating(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreStopCacheLocating: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        LBSLOGE(LOCATOR, "LocatorAbility::ParseDataAndStopCacheLocating remote object nullptr");
        return REPLY_CODE_EXCEPTION;
    }
    sptr<ICachedLocationsCallback> callback = iface_cast<ICachedLocationsCallback>(remoteObject);
    locatorAbility.get()->UnregisterCachedLocationCallback(callback);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreGetCachedGnssLocationsSize(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreGetCachedGnssLocationsSize: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    if (locatorAbility.get()->GetSwitchState() == DISABLED) {
        LBSLOGE(LOCATOR, "switch state is off.");
        return REPLY_CODE_SWITCH_OFF_EXCEPTION;
    }
    reply.WriteInt32(locatorAbility.get()->GetCachedGnssLocationsSize());
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreFlushCachedGnssLocations(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreFlushCachedGnssLocations: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    if (locatorAbility.get()->GetSwitchState() == DISABLED) {
        LBSLOGE(LOCATOR, "switch state is off.");
        return REPLY_CODE_SWITCH_OFF_EXCEPTION;
    }
    int ret = locatorAbility.get()->FlushCachedGnssLocations();
    return ret;
}

int LocatorAbilityStub::PreSendCommand(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreSendCommand: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    std::unique_ptr<LocationCommand> locationCommand = std::make_unique<LocationCommand>();
    locationCommand->scenario =  data.ReadInt32();
    locationCommand->command = data.ReadBool();
    locatorAbility.get()->SendCommand(locationCommand);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreAddFence(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreAddFence: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    if (locatorAbility.get()->GetSwitchState() == DISABLED) {
        LBSLOGE(LOCATOR, "switch state is off.");
        return REPLY_CODE_SWITCH_OFF_EXCEPTION;
    }
    std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
    request->scenario = data.ReadInt32();
    request->geofence.latitude = data.ReadDouble();
    request->geofence.longitude = data.ReadDouble();
    request->geofence.radius = data.ReadDouble();
    request->geofence.expiration = data.ReadDouble();
    locatorAbility.get()->AddFence(request);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreRemoveFence(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreRemoveFence: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    if (locatorAbility.get()->GetSwitchState() == DISABLED) {
        LBSLOGE(LOCATOR, "switch state is off.");
        return REPLY_CODE_SWITCH_OFF_EXCEPTION;
    }
    std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
    request->scenario = data.ReadInt32();
    request->geofence.latitude = data.ReadDouble();
    request->geofence.longitude = data.ReadDouble();
    request->geofence.radius = data.ReadDouble();
    request->geofence.expiration = data.ReadDouble();
    locatorAbility.get()->RemoveFence(request);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreGetIsoCountryCode(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreGetIsoCountryCode: LocatorAbility is nullptr.");
        reply.WriteString("");
        reply.WriteInt32(QUERY_COUNTRY_CODE_ERROR);
        reply.WriteInt32(QUERY_COUNTRY_CODE_ERROR);
        return REPLY_CODE_EXCEPTION;
    }
    auto country = locatorAbility.get()->GetIsoCountryCode();
    if (country) {
        reply.WriteString(country->GetCountryCodeStr());
        reply.WriteInt32(country->GetCountryCodeType());
        reply.WriteInt32(SUCCESS);
    } else {
        reply.WriteString("");
        reply.WriteInt32(QUERY_COUNTRY_CODE_ERROR);
        reply.WriteInt32(QUERY_COUNTRY_CODE_ERROR);
    }
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreEnableLocationMock(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetUid(), identity.GetTokenId())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreEnableLocationMock: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    bool result = locatorAbility.get()->EnableLocationMock();
    reply.WriteBool(result);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreDisableLocationMock(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetUid(), identity.GetTokenId())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreDisableLocationMock: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    bool result = locatorAbility.get()->DisableLocationMock();
    reply.WriteBool(result);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreSetMockedLocations(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetUid(), identity.GetTokenId())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreSetMockedLocations: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    int timeInterval = data.ReadInt32();
    int locationSize = data.ReadInt32();
    locationSize = locationSize > INPUT_ARRAY_LEN_MAX ? INPUT_ARRAY_LEN_MAX : locationSize;
    std::vector<std::shared_ptr<Location>> vcLoc;
    for (int i = 0; i < locationSize; i++) {
        vcLoc.push_back(Location::UnmarshallingShared(data));
    }
    bool result = locatorAbility.get()->SetMockedLocations(timeInterval, vcLoc);
    reply.WriteBool(result);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreEnableReverseGeocodingMock(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetUid(), identity.GetTokenId())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreEnableReverseGeocodingMock: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    bool result = locatorAbility.get()->EnableReverseGeocodingMock();
    reply.WriteBool(result);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreDisableReverseGeocodingMock(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetUid(), identity.GetTokenId())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreDisableReverseGeocodingMock: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    bool result = locatorAbility.get()->DisableReverseGeocodingMock();
    reply.WriteBool(result);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreSetReverseGeocodingMockInfo(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    if (!CommonUtils::CheckSystemPermission(identity.GetUid(), identity.GetTokenId())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]",
            identity.ToString().c_str());
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreSetReverseGeocodingMockInfo: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfo;
    int arraySize = data.ReadInt32();
    arraySize = arraySize > INPUT_ARRAY_LEN_MAX ? INPUT_ARRAY_LEN_MAX : arraySize;
    for (int i = 0; i < arraySize; i++) {
        std::shared_ptr<GeocodingMockInfo> info = std::make_shared<GeocodingMockInfo>();
        info->ReadFromParcel(data);
        mockInfo.push_back(info);
    }
    bool result = locatorAbility.get()->SetReverseGeocodingMockInfo(mockInfo);
    reply.WriteBool(result);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreRegisterCountryCodeCallback(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreRegisterCountryCodeCallback: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    locatorAbility.get()->RegisterCountryCodeCallback(client, identity.GetUid());
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreUnregisterCountryCodeCallback(MessageParcel &data,
    MessageParcel &reply, AppIdentity &identity)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreUnregisterCountryCodeCallback: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    locatorAbility.get()->UnregisterCountryCodeCallback(client);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreProxyUidForFreeze(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreProxyUidForFreeze: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    if (identity.GetUid() != ROOT_UID) {
        LBSLOGE(LOCATOR, "check root permission failed, [%{public}s]",
            identity.ToString().c_str());
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    int32_t uid = data.ReadInt32();
    bool isProxy = data.ReadBool();
    bool result = locatorAbility.get()->ProxyUidForFreeze(uid, isProxy);
    reply.WriteBool(result);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbilityStub::PreResetAllProxy(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!CheckLocationPermission(reply, identity)) {
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "PreResetAllProxy: LocatorAbility is nullptr.");
        return REPLY_CODE_EXCEPTION;
    }
    if (identity.GetUid() != ROOT_UID) {
        LBSLOGE(LOCATOR, "check root permission failed, [%{public}s]",
            identity.ToString().c_str());
        return REPLY_CODE_SECURITY_EXCEPTION;
    }
    bool result = locatorAbility.get()->ResetAllProxy();
    reply.WriteBool(result);
    return REPLY_CODE_NO_EXCEPTION;
}

bool LocatorAbilityStub::CheckLocationPermission(MessageParcel &reply, AppIdentity &identity)
{
    uint32_t callingTokenId = identity.GetTokenId();
    uint32_t callingFirstTokenid = identity.GetFirstTokenId();
    if (!CommonUtils::CheckLocationPermission(callingTokenId, callingFirstTokenid) &&
        !CommonUtils::CheckApproximatelyPermission(callingTokenId, callingFirstTokenid)) {
        LBSLOGE(LOCATOR, "CheckLocationPermission return false.");
        reply.WriteInt32(REPLY_CODE_SECURITY_EXCEPTION);
        reply.WriteString("should grant location permission");
        return false;
    } else {
        return true;
    }
}

bool LocatorAbilityStub::CheckPreciseLocationPermissions(MessageParcel &reply, AppIdentity &identity)
{
    uint32_t callingTokenId = identity.GetTokenId();
    uint32_t callingFirstTokenid = identity.GetFirstTokenId();
    if (!CommonUtils::CheckLocationPermission(callingTokenId, callingFirstTokenid) ||
        !CommonUtils::CheckApproximatelyPermission(callingTokenId, callingFirstTokenid)) {
        LBSLOGE(LOCATOR, "CheckPreciseLocationPermissions return false.");
        reply.WriteInt32(REPLY_CODE_SECURITY_EXCEPTION);
        reply.WriteString("should grant location permission");
        return false;
    } else {
        return true;
    }
}

bool LocatorAbilityStub::CheckSettingsPermission(MessageParcel &reply, AppIdentity &identity)
{
    uint32_t callingTokenId = identity.GetTokenId();
    uint32_t callingFirstTokenid = identity.GetFirstTokenId();
    if (!CommonUtils::CheckSecureSettings(callingTokenId, callingFirstTokenid)) {
        LBSLOGE(LOCATOR, "has no access permission, CheckSecureSettings return false");
        reply.WriteInt32(REPLY_CODE_SECURITY_EXCEPTION);
        reply.WriteString("should grant location permission");
        return false;
    } else {
        return true;
    }
}

int32_t LocatorAbilityStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    int ret = REPLY_CODE_NO_EXCEPTION;
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    uint32_t callingFirstTokenid = IPCSkeleton::GetFirstTokenID();

    AppIdentity identity;
    identity.SetPid(callingPid);
    identity.SetUid(callingUid);
    identity.SetTokenId(callingTokenId);
    identity.SetFirstTokenId(callingFirstTokenid);
    std::string bundleName = "";
    if (!CommonUtils::GetBundleNameByUid(callingUid, bundleName)) {
        LBSLOGD(LOCATOR, "Fail to Get bundle name: uid = %{public}d.", callingUid);
    }
    identity.SetBundleName(bundleName);
    LBSLOGI(LOCATOR, "OnReceived cmd = %{public}u, flags= %{public}d, identity= [%{public}s]",
        code, option.GetFlags(), identity.ToString().c_str());
    std::string callingIdentity = IPCSkeleton::ResetCallingIdentity();

    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(LOCATOR, "invalid token.");
        IPCSkeleton::SetCallingIdentity(callingIdentity);
        return REPLY_CODE_EXCEPTION;
    }

    auto handleFunc = locatorHandleMap_.find(code);
    if (handleFunc != locatorHandleMap_.end() && handleFunc->second != nullptr) {
        auto memberFunc = handleFunc->second;
        ret = (this->*memberFunc)(data, reply, identity);
    }
    IPCSkeleton::SetCallingIdentity(callingIdentity);
    return ret;
}

void LocatorAbilityStub::SaDumpInfo(std::string& result)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR, "SaDumpInfo: LocatorAbility is nullptr.");
        return;
    }
    int state = locatorAbility.get()->GetSwitchState();
    result += "Location switch state: ";
    std::string status = state ? "on" : "off";
    result += status + "\n";
}

int32_t LocatorAbilityStub::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::vector<std::string> vecArgs;
    std::transform(args.begin(), args.end(), std::back_inserter(vecArgs), [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });

    LocationDumper dumper;
    std::string result;
    dumper.LocatorDump(SaDumpInfo, vecArgs, result);
    if (!SaveStringToFd(fd, result)) {
        LBSLOGE(LOCATOR, "Gnss save string to fd failed!");
        return ERR_OK;
    }
    return ERR_OK;
}

LocatorCallbackDeathRecipient::LocatorCallbackDeathRecipient()
{
}

LocatorCallbackDeathRecipient::~LocatorCallbackDeathRecipient()
{
}

void LocatorCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    sptr<ILocatorCallback> callback = iface_cast<ILocatorCallback>(remote.promote());
    sptr<LocatorAbility> locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance().get();
    if (locatorAbility != nullptr) {
        locatorAbility->StopLocating(callback);
        LBSLOGI(LOCATOR, "locator callback OnRemoteDied");
    }
}

SwitchCallbackDeathRecipient::SwitchCallbackDeathRecipient()
{
}

SwitchCallbackDeathRecipient::~SwitchCallbackDeathRecipient()
{
}

void SwitchCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    sptr<LocatorAbility> locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance().get();
    if (locatorAbility != nullptr) {
        locatorAbility->UnregisterSwitchCallback(remote.promote());
        LBSLOGI(LOCATOR, "switch callback OnRemoteDied");
    }
}
} // namespace Location
} // namespace OHOS