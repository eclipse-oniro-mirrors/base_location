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
#include "country_code_manager.h"
#ifdef TEL_CELLULAR_DATA_ENABLE
#include "cellular_data_client.h"
#endif
#ifdef TEL_CORE_SERVICE_ENABLE
#include "core_service_client.h"
#endif
#ifdef I18N_ENABLE
#include "locale_config.h"
#endif
#include "parameter.h"

#include "common_event_manager.h"
#include "common_event_support.h"
#include "common_utils.h"
#include "constant_definition.h"
#include "country_code.h"
#include "location_log.h"
#include "locator_impl.h"

namespace OHOS {
namespace Location {
CountryCodeManager::CountryCodeManager()
{
    lastCountryByLocation_ = std::make_shared<CountryCode>();
    lastCountry_ = std::make_shared<CountryCode>();
    countryCodeCallback_ = std::make_unique<std::map<pid_t, sptr<ICountryCodeCallback>>>();
    simSubscriber_ = nullptr;
    networkSubscriber_ = nullptr;
    SubscribeLocaleConfigEvent();
}

CountryCodeManager::~CountryCodeManager()
{
}

void CountryCodeManager::NotifyAllListener()
{
    std::unique_lock lock(countryCodeCallbackMutex_);
    if (lastCountry_ == nullptr || countryCodeCallback_ == nullptr) {
        LBSLOGE(COUNTRY_CODE, "NotifyAllListener cancel, para is invalid");
        return;
    }
    auto country = std::make_shared<CountryCode>(*lastCountry_);
    for (auto iter = countryCodeCallback_->begin(); iter != countryCodeCallback_->end(); iter++) {
        sptr<ICountryCodeCallback> callback = (iter->second);
        if (callback) {
            callback->OnCountryCodeChange(country);
        }
    }
}

void CountryCodeManager::RegisterCountryCodeCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    std::unique_lock<std::mutex> lock(countryCodeCallbackMutex_, std::defer_lock);
    lock.lock();
    if (callback == nullptr || countryCodeCallback_ == nullptr) {
        LBSLOGE(COUNTRY_CODE, "callback is invalid");
        lock.unlock();
        return;
    }

    sptr<ICountryCodeCallback> countryCodeCallback = iface_cast<ICountryCodeCallback>(callback);
    if (countryCodeCallback == nullptr) {
        LBSLOGE(COUNTRY_CODE, "iface_cast ICountryCodeCallback failed!");
        lock.unlock();
        return;
    }
    countryCodeCallback_->erase(uid);
    countryCodeCallback_->insert(std::make_pair(uid, countryCodeCallback));
    LBSLOGD(COUNTRY_CODE, "after uid:%{public}d register, countryCodeCallback_ size:%{public}s",
        uid, std::to_string(countryCodeCallback_->size()).c_str());
    if (countryCodeCallback_->size() != 1) {
        lock.unlock();
        return;
    }
    lock.unlock();
    SubscribeSimEvent();
    SubscribeNetworkStatusEvent();
}

void CountryCodeManager::UnregisterCountryCodeCallback(const sptr<IRemoteObject>& callback)
{
    std::unique_lock<std::mutex> lock(countryCodeCallbackMutex_, std::defer_lock);
    lock.lock();
    if (callback == nullptr || countryCodeCallback_ == nullptr) {
        LBSLOGE(COUNTRY_CODE, "unregister an invalid callback");
        lock.unlock();
        return;
    }
    sptr<ICountryCodeCallback> countryCodeCallback = iface_cast<ICountryCodeCallback>(callback);
    if (countryCodeCallback == nullptr) {
        LBSLOGE(COUNTRY_CODE, "iface_cast ICountryCodeCallback failed!");
        lock.unlock();
        return;
    }

    pid_t uid = -1;
    for (auto iter = countryCodeCallback_->begin(); iter != countryCodeCallback_->end(); iter++) {
        sptr<IRemoteObject> remoteObject = (iter->second)->AsObject();
        if (remoteObject == callback) {
            uid = iter->first;
            break;
        }
    }
    countryCodeCallback_->erase(uid);
    LBSLOGD(COUNTRY_CODE, "after uid:%{public}d unregister, countryCodeCallback_ size:%{public}s",
        uid, std::to_string(countryCodeCallback_->size()).c_str());
    if (countryCodeCallback_->size() != 0) {
        lock.unlock();
        return;
    }
    lock.unlock();
    UnsubscribeSimEvent();
    UnsubscribeNetworkStatusEvent();
}

bool CountryCodeManager::IsCountryCodeRegistered()
{
    std::unique_lock lock(countryCodeCallbackMutex_);
    return countryCodeCallback_->size() != 0;
}

std::string CountryCodeManager::GetCountryCodeByLastLocation()
{
    std::string code = "";
    if (lastCountryByLocation_ == nullptr) {
        LBSLOGE(COUNTRY_CODE, "lastCountryByLocation_ is nullptr");
        return code;
    }
    if (lastCountryByLocation_->GetCountryCodeStr().empty()) {
        auto locatorImpl = LocatorImpl::GetInstance();
        if (locatorImpl) {
            std::unique_ptr<Location> lastLocation = std::make_unique<Location>();
            LocationErrCode errorCode = locatorImpl->GetCachedLocationV9(lastLocation);
            if (errorCode != ERRCODE_SUCCESS) {
                return code;
            }
            code = GetCountryCodeByLocation(lastLocation);
            lastCountryByLocation_->SetCountryCodeStr(code);
        }
    }
    return lastCountryByLocation_->GetCountryCodeStr();
}

void CountryCodeManager::UpdateCountryCode(std::string countryCode, int type)
{
    if (lastCountry_ == nullptr) {
        LBSLOGE(COUNTRY_CODE, "lastCountry_ is nullptr");
        return;
    }
    if (lastCountry_->IsMoreReliable(type)) {
        LBSLOGI(COUNTRY_CODE, "lastCountry_ is more reliable,there is no need to update the data");
        return;
    }
    lastCountry_->SetCountryCodeStr(countryCode);
    lastCountry_->SetCountryCodeType(type);
}

bool CountryCodeManager::UpdateCountryCodeByLocation(std::string countryCode, int type)
{
    if (lastCountryByLocation_ == nullptr) {
        LBSLOGE(COUNTRY_CODE, "lastCountryByLocation_ is nullptr");
        return false;
    }
    if (lastCountryByLocation_->GetCountryCodeStr() == countryCode) {
        LBSLOGE(COUNTRY_CODE, "countryCode is same");
        return false;
    }

    lastCountryByLocation_->SetCountryCodeStr(countryCode);
    lastCountryByLocation_->SetCountryCodeType(type);
    return true;
}

std::string CountryCodeManager::GetCountryCodeByLocation(const std::unique_ptr<Location>& location)
{
    if (location == nullptr) {
        LBSLOGE(COUNTRY_CODE, "GetCountryCodeByLocation location is nullptr");
        return "";
    }
    auto locatorImpl = LocatorImpl::GetInstance();
    if (locatorImpl == nullptr) {
        LBSLOGE(COUNTRY_CODE, "locatorImpl is nullptr");
        return "";
    }
    MessageParcel dataParcel;
    std::list<std::shared_ptr<GeoAddress>> replyList;
    if (!dataParcel.WriteInterfaceToken(LocatorProxy::GetDescriptor())) {
        LBSLOGE(COUNTRY_CODE, "write interfaceToken fail!");
        return "";
    }
    dataParcel.WriteString16(Str8ToStr16("en")); // locale
    dataParcel.WriteDouble(location->GetLatitude()); // latitude
    dataParcel.WriteDouble(location->GetLongitude()); // longitude
    dataParcel.WriteInt32(1); // maxItems

    bool isAvailable = false;
    LocationErrCode errorCode = locatorImpl->IsGeoServiceAvailableV9(isAvailable);
    if (errorCode != ERRCODE_SUCCESS || !isAvailable) {
        LBSLOGE(COUNTRY_CODE, "geocode service is not available.");
        return "";
    }
    errorCode = locatorImpl->GetAddressByCoordinateV9(dataParcel, replyList);
    if (replyList.empty() || errorCode != ERRCODE_SUCCESS) {
        LBSLOGE(COUNTRY_CODE, "geocode fail.");
        return "";
    }
    return replyList.front()->countryCode_;
}

std::shared_ptr<CountryCode> CountryCodeManager::GetIsoCountryCode()
{
    LBSLOGD(COUNTRY_CODE, "CountryCodeManager::GetIsoCountryCode");
    int type = COUNTRY_CODE_FROM_LOCALE;
    std::string countryCodeStr8;
#if defined(TEL_CORE_SERVICE_ENABLE) && defined(TEL_CELLULAR_DATA_ENABLE)
    int slotId = Telephony::CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    std::u16string countryCodeForNetwork;
    DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetIsoCountryCodeForNetwork(
        slotId, countryCodeForNetwork);
    countryCodeStr8 = Str16ToStr8(countryCodeForNetwork);
    type = COUNTRY_CODE_FROM_NETWORK;
#endif
    if (countryCodeStr8.empty()) {
        countryCodeStr8 = GetCountryCodeByLastLocation();
        type = COUNTRY_CODE_FROM_LOCATION;
    }
#if defined(TEL_CORE_SERVICE_ENABLE) && defined(TEL_CELLULAR_DATA_ENABLE)
    if (countryCodeStr8.empty()) {
        std::u16string countryCodeForSim;
        DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetISOCountryCodeForSim(
            slotId, countryCodeForSim);
        countryCodeStr8 = Str16ToStr8(countryCodeForSim);
        type = COUNTRY_CODE_FROM_SIM;
    }
#endif
#ifdef I18N_ENABLE
    if (countryCodeStr8.empty()) {
        countryCodeStr8 = Global::I18n::LocaleConfig::GetSystemRegion();
        type = COUNTRY_CODE_FROM_LOCALE;
    }
#endif
    // transfer to uppercase
    transform(countryCodeStr8.begin(), countryCodeStr8.end(), countryCodeStr8.begin(), ::toupper);
    CountryCode country;
    country.SetCountryCodeStr(countryCodeStr8);
    country.SetCountryCodeType(type);
    if (lastCountry_ && !country.IsSame(*lastCountry_) && !lastCountry_->IsMoreReliable(type)) {
        UpdateCountryCode(countryCodeStr8, type);
        NotifyAllListener();
    }
    return lastCountry_;
}

bool CountryCodeManager::SubscribeSimEvent()
{
    LBSLOGD(COUNTRY_CODE, "SubscribeSimEvent");
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(SIM_STATE_CHANGE_ACTION);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::unique_lock<std::mutex> lock(simSubscriberMutex_, std::defer_lock);
    lock.lock();
    if (simSubscriber_ == nullptr) {
        simSubscriber_ = std::make_shared<SimSubscriber>(subscriberInfo);
    }
    lock.unlock();
    bool result = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(simSubscriber_);
    if (!result) {
        LBSLOGE(COUNTRY_CODE, "SubscribeSimEvent failed.");
    }
    return result;
}

bool CountryCodeManager::SubscribeNetworkStatusEvent()
{
    LBSLOGD(COUNTRY_CODE, "SubscribeNetworkStatusEvent");
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(SEARCH_NET_WORK_STATE_CHANGE_ACTION);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::unique_lock<std::mutex> lock(networkSubscriberMutex_, std::defer_lock);
    lock.lock();
    if (networkSubscriber_ == nullptr) {
        networkSubscriber_ = std::make_shared<NetworkSubscriber>(subscriberInfo);
    }
    lock.unlock();
    bool result = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(networkSubscriber_);
    if (!result) {
        LBSLOGE(COUNTRY_CODE, "SubscribeNetworkStatusEvent failed.");
    }
    return result;
}

bool CountryCodeManager::SubscribeLocaleConfigEvent()
{
    auto eventCallback = [](const char *key, const char *value, void *context) {
        LBSLOGD(COUNTRY_CODE, "LOCALE_KEY changed");
        auto manager = DelayedSingleton<CountryCodeManager>::GetInstance();
        if (manager == nullptr) {
            LBSLOGE(COUNTRY_CODE, "SubscribeLocaleConfigEvent CountryCodeManager is nullptr");
            return;
        }
        manager->GetIsoCountryCode();
    };

    int ret = WatchParameter(LOCALE_KEY.c_str(), eventCallback, nullptr);
    if (ret != SUCCESS) {
        LBSLOGD(COUNTRY_CODE, "WatchParameter fail");
        return false;
    }
    return true;
}

bool CountryCodeManager::UnsubscribeSimEvent()
{
    LBSLOGD(COUNTRY_CODE, "UnsubscribeSimEvent");
    if (simSubscriber_) {
        return OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(simSubscriber_);
    }
    return false;
}

bool CountryCodeManager::UnsubscribeNetworkStatusEvent()
{
    LBSLOGD(COUNTRY_CODE, "UnsubscribeNetworkStatusEvent");
    if (networkSubscriber_) {
        OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(networkSubscriber_);
    }
    return false;
}

void CountryCodeManager::LocatorCallback::OnLocationReport(const std::unique_ptr<Location>& location)
{
    auto manager = DelayedSingleton<CountryCodeManager>::GetInstance();
    if (manager == nullptr) {
        LBSLOGE(COUNTRY_CODE, "OnLocationReport CountryCodeManager is nullptr");
        return;
    }
    if (location == nullptr) {
        LBSLOGE(COUNTRY_CODE, "OnLocationReport location is nullptr");
        return;
    }
    std::string code = manager->GetCountryCodeByLocation(location);
    CountryCode country;
    country.SetCountryCodeStr(code);
    country.SetCountryCodeType(COUNTRY_CODE_FROM_LOCATION);
    LBSLOGI(COUNTRY_CODE, "OnLocationReport");
    if (manager->UpdateCountryCodeByLocation(code, COUNTRY_CODE_FROM_LOCATION)) {
        LBSLOGI(COUNTRY_CODE, "OnLocationReport,countryCode is change");
        manager->GetIsoCountryCode();
    }
}

void CountryCodeManager::LocatorCallback::OnLocatingStatusChange(const int status)
{
}

void CountryCodeManager::LocatorCallback::OnErrorReport(const int errorCode)
{
}

CountryCodeManager::NetworkSubscriber::NetworkSubscriber(
    const OHOS::EventFwk::CommonEventSubscribeInfo &info)
    : CommonEventSubscriber(info)
{
    LBSLOGD(COUNTRY_CODE, "create NetworkSubscriber");
}

void CountryCodeManager::NetworkSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData& event)
{
    auto manager = DelayedSingleton<CountryCodeManager>::GetInstance();
    if (manager == nullptr) {
        LBSLOGE(COUNTRY_CODE, "CountryCodeManager is nullptr");
        return;
    }
    LBSLOGI(COUNTRY_CODE, "NetworkSubscriber::OnReceiveEvent");
    manager->GetIsoCountryCode();
}

CountryCodeManager::SimSubscriber::SimSubscriber(
    const OHOS::EventFwk::CommonEventSubscribeInfo &info)
    : CommonEventSubscriber(info)
{
    LBSLOGD(COUNTRY_CODE, "create SimSubscriber");
}

void CountryCodeManager::SimSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData& event)
{
    auto manager = DelayedSingleton<CountryCodeManager>::GetInstance();
    if (manager == nullptr) {
        LBSLOGE(COUNTRY_CODE, "CountryCodeManager is nullptr");
        return;
    }
    LBSLOGI(COUNTRY_CODE, "SimSubscriber::OnReceiveEvent");
    manager->GetIsoCountryCode();
}

void CountryCodeManager::ReSubscribeEvent()
{
    std::unique_lock<std::mutex> lock(countryCodeCallbackMutex_, std::defer_lock);
    lock.lock();
    if (countryCodeCallback_->size() <= 0) {
        LBSLOGD(COUNTRY_CODE, "no valid callback registed, no need to subscribe");
        lock.unlock();
        return;
    }
    lock.unlock();
    SubscribeSimEvent();
    SubscribeNetworkStatusEvent();
}

void CountryCodeManager::ReUnsubscribeEvent()
{
    std::unique_lock<std::mutex> lock(countryCodeCallbackMutex_, std::defer_lock);
    lock.lock();
    if (countryCodeCallback_->size() <= 0) {
        LBSLOGE(COUNTRY_CODE, "no valid callback registed, no need to unsubscribe");
        lock.unlock();
        return;
    }
    lock.unlock();
    UnsubscribeSimEvent();
    UnsubscribeNetworkStatusEvent();
}
} // namespace Location
} // namespace OHOS
