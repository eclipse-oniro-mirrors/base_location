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

#include "subability_common.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"

#include "common_hisysevent.h"
#include "common_utils.h"
#include "ilocator_service.h"
#include "locationhub_ipc_interface_code.h"
#include "location_log_event_ids.h"
#include "app_identity.h"

namespace OHOS {
namespace Location {
SubAbility::SubAbility()
{
    label_ = { LOG_CORE, LOCATION_LOG_DOMAIN, "unknown" };
    newRecord_ = std::make_unique<WorkRecord>();
    lastRecord_ = std::make_unique<WorkRecord>();
}

SubAbility::~SubAbility()
{
    newRecord_ = nullptr;
    lastRecord_ = nullptr;
}

void SubAbility::SetAbility(std::string name)
{
    name_ = name;
    label_ = CommonUtils::GetLabel(name);
    capability_ = CommonUtils::GetCapability(name);
}

void SubAbility::StopAllLocationRequests()
{
    // When the switch is turned off, all current requests are stopped
    std::unique_ptr<WorkRecord> emptyRecord = std::make_unique<WorkRecord>();
    HandleLocalRequest(*emptyRecord);
    lastRecord_->Clear();
}

void SubAbility::RestartAllLocationRequests()
{
    // When the switch is turned on, all SA requests will be refreshed
    lastRecord_->Clear();
    HandleRefrashRequirements();
}

void SubAbility::LocationRequest(WorkRecord &workRecord)
{
    interval_ = workRecord.GetTimeInterval(0);
    newRecord_->Clear();
    newRecord_->Set(workRecord);
    HandleRefrashRequirements();
}

void SubAbility::HandleRefrashRequirements()
{
    LBSLOGD(label_, "refrash requirements");

    // send local request
    HandleLocalRequest(*newRecord_);
    lastRecord_->Clear();
    lastRecord_->Set(*newRecord_);
}

int SubAbility::GetRequestNum()
{
    if (newRecord_ == nullptr) {
        return 0;
    }
    return newRecord_->Size();
}

void SubAbility::HandleLocalRequest(WorkRecord &record)
{
    HandleRemoveRecord(record);
    HandleAddRecord(record);
}

void SubAbility::HandleRemoveRecord(WorkRecord &newRecord)
{
    for (int i = 0; i < lastRecord_->Size(); i++) {
        int uid = lastRecord_->GetUid(i);
        bool isFind = newRecord.Find(uid, lastRecord_->GetName(i), lastRecord_->GetUuid(i));
        LBSLOGD(label_, "remove record isFind:%{public}d, uid:%{public}d, lastRecord:%{public}s, newRecord:%{public}s",
            isFind, uid, lastRecord_->ToString().c_str(), newRecord.ToString().c_str());
        if (!isFind) {
            std::unique_ptr<WorkRecord> workRecord = std::make_unique<WorkRecord>();
            workRecord->Add(*lastRecord_, i);
            workRecord->SetDeviceId(newRecord.GetDeviceId());
            RequestRecord(*workRecord, false);
        }
    }
}

void SubAbility::HandleAddRecord(WorkRecord &newRecord)
{
    for (int i = 0; i < newRecord.Size(); i++) {
        int uid = newRecord.GetUid(i);
        bool isFind = lastRecord_->Find(uid, newRecord.GetName(i), newRecord.GetUuid(i));
        LBSLOGD(label_, "add record isFind:%{public}d, uid:%{public}d, lastRecord:%{public}s, newRecord:%{public}s",
            isFind, uid, lastRecord_->ToString().c_str(), newRecord.ToString().c_str());
        if (!isFind) {
            std::unique_ptr<WorkRecord> workRecord = std::make_unique<WorkRecord>();
            workRecord->Add(newRecord, i);
            workRecord->SetDeviceId(newRecord.GetDeviceId());
            RequestRecord(*workRecord, true);
        }
    }
}

void SubAbility::Enable(bool state, const sptr<IRemoteObject> ability)
{
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LBSLOGE(label_, "Enable can not get SystemAbilityManager");
        return;
    }

    int saId = CommonUtils::AbilityConvertToId(name_);
    if (state) {
        if (sam->CheckSystemAbility(saId) == nullptr) {
            sam->AddSystemAbility(saId, ability, ISystemAbilityManager::SAExtraProp(false, 1, capability_, u""));
            LBSLOGI(label_, "enable %{public}s ability", name_.c_str());
        }
    } else {
        if (sam->CheckSystemAbility(saId) != nullptr) {
            sam->RemoveSystemAbility(saId);
            LBSLOGI(label_, "disable %{public}s ability", name_.c_str());
        }
    }
}

bool SubAbility::EnableLocationMock()
{
    LBSLOGI(label_, "EnableLocationMock current state is %{public}d", mockEnabled_);
    mockEnabled_ = true;
    return true;
}

bool SubAbility::DisableLocationMock()
{
    LBSLOGI(label_, "DisableLocationMock current state is %{public}d", mockEnabled_);
    mockEnabled_ = false;
    return true;
}

bool SubAbility::SetMockedLocations(const int timeInterval, const std::vector<std::shared_ptr<Location>> &location)
{
    if (!mockEnabled_) {
        LBSLOGE(label_, "SetMockedLocations current state is %{public}d, need enbale it", mockEnabled_);
        return false;
    }
    CacheLocationMock(location);
    mockTimeInterval_ = timeInterval;
    SendReportMockLocationEvent();
    return true;
}

void SubAbility::CacheLocationMock(const std::vector<std::shared_ptr<Location>> &location)
{
    int locationSize = static_cast<int>(location.size());
    ClearLocationMock();
    std::unique_lock lock(mutex_);
    for (int i = 0; i < locationSize; i++) {
        mockLoc_.push_back(std::make_shared<Location>(*location.at(i)));
    }
}

bool SubAbility::IsLocationMocked()
{
    return mockEnabled_;
}

int SubAbility::GetTimeIntervalMock()
{
    return mockTimeInterval_;
}

std::vector<std::shared_ptr<Location>> SubAbility::GetLocationMock()
{
    std::unique_lock lock(mutex_);
    return mockLoc_;
}

void SubAbility::ClearLocationMock()
{
    std::unique_lock lock(mutex_);
    mockLoc_.clear();
}

void SubAbility::ReportLocationInfo(const std::string& systemAbility, const std::shared_ptr<Location> location)
{
    sptr<IRemoteObject> objectLocator =
        CommonUtils::GetRemoteObject(LOCATION_LOCATOR_SA_ID, CommonUtils::InitDeviceId());
    if (objectLocator == nullptr) {
        LBSLOGE(label_, "%{public}s get locator sa failed", __func__);
        return;
    }
    sptr<ILocatorService> client = iface_cast<ILocatorService>(objectLocator);
    if (!client || !client->AsObject()) {
        LBSLOGE(label_, "%{public}s: get locator service failed.", __func__);
        return;
    }
    client->ReportLocation(systemAbility, *location);
}

void SubAbility::ReportLocationError(int32_t errCode, std::string errMsg, std::string uuid)
{
    sptr<IRemoteObject> objectLocator =
        CommonUtils::GetRemoteObject(LOCATION_LOCATOR_SA_ID, CommonUtils::InitDeviceId());
    if (objectLocator == nullptr) {
        LBSLOGE(label_, "%{public}s get locator sa failed", __func__);
        return;
    }
    sptr<ILocatorService> client = iface_cast<ILocatorService>(objectLocator);
    if (!client || !client->AsObject()) {
        LBSLOGE(label_, "%{public}s: get locator service failed.", __func__);
        return;
    }
    client->ReportLocationError(LOCATING_FAILED_INTERNET_ACCESS_FAILURE, errMsg, uuid);
}
} // namespace Location
} // namespace OHOS
