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

#include "request.h"
#include "common_utils.h"
#include "constant_definition.h"

namespace OHOS {
namespace Location {
Request::Request()
{
    this->pid_ = -1;
    this->uid_ = -1;
    this->tokenId_ = 0;
    this->tokenIdEx_ = 0;
    this->firstTokenId_ = 0;
    this->packageName_ = "";
    this->isRequesting_ = false;
    requestConfig_ = new (std::nothrow) RequestConfig();
    lastLocation_ = new (std::nothrow) Location();
    isUsingLocationPerm_ = false;
    isUsingBackgroundPerm_ = false;
    isUsingApproximatelyPerm_ = false;
}

Request::Request(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<ILocatorCallback>& callback, AppIdentity &identity)
{
    this->pid_ = -1;
    this->uid_ = -1;
    this->tokenId_ = 0;
    this->firstTokenId_ = 0;
    this->packageName_ = "";
    this->isRequesting_ = false;
    requestConfig_ = new (std::nothrow) RequestConfig();
    lastLocation_ = new (std::nothrow) Location();
    isUsingLocationPerm_ = false;
    isUsingBackgroundPerm_ = false;
    isUsingApproximatelyPerm_ = false;
    SetUid(identity.GetUid());
    SetPid(identity.GetPid());
    SetTokenId(identity.GetTokenId());
    SetTokenIdEx(identity.GetTokenIdEx());
    SetFirstTokenId(identity.GetFirstTokenId());
    SetPackageName(identity.GetBundleName());
    SetRequestConfig(*requestConfig);
    requestConfig_->SetTimeStamp(CommonUtils::GetCurrentTime());
    SetLocatorCallBack(callback);
    SetUuid(CommonUtils::GenerateUuid());
}


Request::~Request() {}

void Request::SetRequestConfig(RequestConfig& requestConfig)
{
    if (this->requestConfig_ == nullptr) {
        return;
    }
    this->requestConfig_->Set(requestConfig);
}

void Request::SetLocatorCallBack(const sptr<ILocatorCallback>& callback)
{
    this->callBack_ = callback;
}

sptr<RequestConfig> Request::GetRequestConfig()
{
    return requestConfig_;
}

sptr<ILocatorCallback> Request::GetLocatorCallBack()
{
    return callBack_;
}

void Request::SetUid(pid_t uid)
{
    this->uid_ = uid;
}

pid_t Request::GetUid()
{
    return uid_;
}

void Request::SetPid(pid_t pid)
{
    this->pid_ = pid;
}

pid_t Request::GetPid()
{
    return pid_;
}

void Request::SetTokenId(uint32_t tokenId)
{
    this->tokenId_ = tokenId;
}

uint32_t Request::GetTokenId()
{
    return tokenId_;
}

void Request::SetTokenIdEx(uint64_t tokenIdEx)
{
    this->tokenIdEx_ = tokenIdEx;
}

uint64_t Request::GetTokenIdEx()
{
    return tokenIdEx_;
}

void Request::SetFirstTokenId(uint32_t firstTokenId)
{
    this->firstTokenId_ = firstTokenId;
}

uint32_t Request::GetFirstTokenId()
{
    return firstTokenId_;
}

void Request::SetPackageName(std::string packageName)
{
    this->packageName_ = packageName;
}

std::string Request::GetPackageName()
{
    return packageName_;
}

bool Request::GetIsRequesting()
{
    return isRequesting_;
}

void Request::SetRequesting(bool state)
{
    this->isRequesting_ = state;
}

sptr<Location> Request::GetLastLocation()
{
    return lastLocation_;
}

std::string Request::GetUuid()
{
    return uuid_;
}

void Request::SetUuid(std::string uuid)
{
    this->uuid_ = uuid;
}

void Request::SetLastLocation(const std::unique_ptr<Location>& location)
{
    if (this->lastLocation_ == nullptr) {
        return;
    }
    this->lastLocation_->SetLatitude(location->GetLatitude());
    this->lastLocation_->SetLongitude(location->GetLongitude());
    this->lastLocation_->SetAltitude(location->GetAltitude());
    this->lastLocation_->SetAccuracy(location->GetAccuracy());
    this->lastLocation_->SetSpeed(location->GetSpeed());
    this->lastLocation_->SetDirection(location->GetDirection());
    this->lastLocation_->SetTimeStamp(location->GetTimeStamp());
    this->lastLocation_->SetTimeSinceBoot(location->GetTimeSinceBoot());
    this->lastLocation_->SetLocationSourceType(location->GetLocationSourceType());
}

void Request::GetProxyName(std::shared_ptr<std::list<std::string>> proxys)
{
    if (requestConfig_ == nullptr || proxys == nullptr) {
        return;
    }
#ifdef EMULATOR_ENABLED
    proxys->push_back(GNSS_ABILITY);
#else
    switch (requestConfig_->GetScenario()) {
        case LOCATION_SCENE_NAVIGATION:
        case LOCATION_SCENE_SPORT:
        case LOCATION_SCENE_TRANSPORT:
        case LOCATION_SCENE_HIGH_POWER_CONSUMPTION:
        case SCENE_NAVIGATION:
        case SCENE_TRAJECTORY_TRACKING:
        case SCENE_CAR_HAILING: {
            proxys->push_back(GNSS_ABILITY);
            proxys->push_back(NETWORK_ABILITY);
            break;
        }
        case LOCATION_SCENE_LOW_POWER_CONSUMPTION:
        case LOCATION_SCENE_DAILY_LIFE_SERVICE:
        case SCENE_DAILY_LIFE_SERVICE: {
            proxys->push_back(NETWORK_ABILITY);
            break;
        }
        case LOCATION_SCENE_NO_POWER_CONSUMPTION:
        case SCENE_NO_POWER: {
            proxys->push_back(PASSIVE_ABILITY);
            break;
        }
        case SCENE_UNSET: {
            GetProxyNameByPriority(proxys);
            break;
        }
        default:
            break;
    }
#endif
}

void Request::GetProxyNameByPriority(std::shared_ptr<std::list<std::string>> proxys)
{
    if (requestConfig_ == nullptr || proxys == nullptr) {
        return;
    }
#ifdef EMULATOR_ENABLED
    proxys->push_back(GNSS_ABILITY);
#else
    switch (requestConfig_->GetPriority()) {
        case PRIORITY_LOW_POWER:
            proxys->push_back(NETWORK_ABILITY);
            break;
        case LOCATION_PRIORITY_ACCURACY:
        case LOCATION_PRIORITY_LOCATING_SPEED:
        case PRIORITY_ACCURACY:
        case PRIORITY_FAST_FIRST_FIX:
            proxys->push_back(GNSS_ABILITY);
            proxys->push_back(NETWORK_ABILITY);
            break;
        default:
            break;
    }
#endif
}

bool Request::GetLocationPermState()
{
    return isUsingLocationPerm_;
}

bool Request::GetBackgroundPermState()
{
    return isUsingBackgroundPerm_;
}

bool Request::GetApproximatelyPermState()
{
    return isUsingApproximatelyPerm_;
}

void Request::SetLocationPermState(bool state)
{
    isUsingLocationPerm_ = state;
}

void Request::SetBackgroundPermState(bool state)
{
    isUsingBackgroundPerm_ = state;
}

void Request::SetApproximatelyPermState(bool state)
{
    isUsingApproximatelyPerm_ = state;
}

void Request::SetNlpRequestType(int nlpRequestType)
{
    nlpRequestType_ = nlpRequestType;
}

int Request::GetNlpRequestType()
{
    return nlpRequestType_;
}

void Request::SetNlpRequestType()
{
    if (requestConfig_->GetScenario() == SCENE_NAVIGATION ||
        requestConfig_->GetScenario() == SCENE_TRAJECTORY_TRACKING ||
        requestConfig_->GetScenario() == SCENE_CAR_HAILING ||
        requestConfig_->GetScenario() == LOCATION_SCENE_NAVIGATION ||
        requestConfig_->GetScenario() == LOCATION_SCENE_SPORT ||
        requestConfig_->GetScenario() == LOCATION_SCENE_TRANSPORT ||
        requestConfig_->GetPriority() == PRIORITY_ACCURACY ||
        requestConfig_->GetPriority() == PRIORITY_FAST_FIRST_FIX ||
        requestConfig_->GetPriority() == LOCATION_SCENE_HIGH_POWER_CONSUMPTION ||
        requestConfig_->GetPriority() == LOCATION_PRIORITY_ACCURACY) {
        nlpRequestType_ = NlpRequestType::PRIORITY_TYPE_INDOOR;
    } else {
        nlpRequestType_ = NlpRequestType::PRIORITY_TYPE_BALANCED_POWER_ACCURACY;
    }
}

std::string Request::ToString() const
{
    if (requestConfig_ == nullptr) {
        return "";
    }
    std::string str = "[request config: " + requestConfig_->ToString() +
        "] from pid:" + std::to_string(pid_) +
        ", uid:" + std::to_string(uid_) +
        ", tokenId:" + std::to_string(tokenId_) +
        ", tokenIdEx:" + std::to_string(tokenIdEx_) +
        ", firstTokenId:" + std::to_string(firstTokenId_) +
        ", uuid:" + uuid_ + ", packageName:" + packageName_;
    return str;
}

void Request::SetLocationErrorCallBack(const sptr<ILocatorCallback>& callback)
{
    this->locationErrorcallBack_ = callback;
}

sptr<ILocatorCallback> Request::GetLocationErrorCallBack()
{
    return locationErrorcallBack_;
}

} // namespace Location
} // namespace OHOS