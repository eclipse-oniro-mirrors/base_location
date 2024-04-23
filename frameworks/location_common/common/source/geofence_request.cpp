/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "geofence_request.h"
#include <parcel.h>
#include "common_utils.h"
#include "notification_request.h"
#include "iremote_object.h"

namespace OHOS {
namespace Location {
void GeofenceRequest::ReadFromParcel(Parcel& data)
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    geofence_->latitude = data.ReadDouble();
    geofence_->longitude = data.ReadDouble();
    geofence_->radius = data.ReadDouble();
    geofence_->expiration = data.ReadDouble();
    geofence_->coordinateSystemType = static_cast<CoordinateSystemType>(data.ReadInt32());
    int monitorGeofenceTransitionSize = data.ReadInt32();
    if (monitorGeofenceTransitionSize > MAX_TRANSITION_SIZE) {
        LBSLOGE(LOCATOR, "fence transition list size should not be greater than 3");
        return;
    }
    for (int i = 0; i < monitorGeofenceTransitionSize; i++) {
        transitionStatusList_.push_back(static_cast<GeofenceTransitionEvent>(data.ReadInt32()));
    }
    int requestSize = data.ReadInt32();
    if (requestSize > MAX_NOTIFICATION_REQUEST_LIST_SIZE) {
        LBSLOGE(LOCATOR, "request size should not be greater than 3");
        return;
    }
    for (int i = 0; i < requestSize; i++) {
        auto request = Notification::NotificationRequest::Unmarshalling(data);
        notificationRequestList_.push_back(std::make_shared<Notification::NotificationRequest>(*request));
    }
    callback_ = data.ReadObject<IRemoteObject>();
    bundleName_ = data.ReadString();
}

bool GeofenceRequest::Marshalling(Parcel& parcel) const
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    parcel.WriteDouble(geofence_->latitude);
    parcel.WriteDouble(geofence_->longitude);
    parcel.WriteDouble(geofence_->radius);
    parcel.WriteDouble(geofence_->expiration);
    parcel.WriteInt32(static_cast<int>(geofence_->coordinateSystemType));
    if (transitionStatusList_.size() > MAX_TRANSITION_SIZE) {
        LBSLOGE(LOCATOR, "fence transition list size should not be greater than 3");
        return false;
    }
    parcel.WriteInt32(transitionStatusList_.size());
    for (int i = 0; i < transitionStatusList_.size(); i++) {
        parcel.WriteInt32(static_cast<int>(transitionStatusList_[i]));
    }
    if (notificationRequestList_.size() > MAX_NOTIFICATION_REQUEST_LIST_SIZE) {
        LBSLOGE(LOCATOR, "request size should not be greater than 3");
        return false;
    }
    parcel.WriteInt32(notificationRequestList_.size());
    for (int i = 0; i < notificationRequestList_.size(); i++) {
        notificationRequestList_[i]->Marshalling(parcel);
    }
    parcel.WriteRemoteObject(callback_);
    parcel.WriteString(bundleName_);
    return true;
}

std::shared_ptr<GeofenceRequest> GeofenceRequest::Unmarshalling(Parcel& parcel)
{
    std::shared_ptr<GeofenceRequest> geofenceRequest = std::make_shared<GeofenceRequest>();
    geofenceRequest->ReadFromParcel(parcel);
    return geofenceRequest;
}
} // namespace Location
} // namespace OHOS