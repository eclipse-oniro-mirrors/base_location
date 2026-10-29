/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "fusionfenceability_fuzzer.h"

#include "fusion_fence_ability.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "fusion_fence_request.h"
#include "geofence.h"
#include "geofence_definition.h"
#include "constant_definition.h"

namespace OHOS {
using namespace OHOS::Location;
const int MIN_DATA_LEN = 8;
const int32_t FUSION_FENCE_SCENE_TYPE_COUNT = 4;
const int32_t GEOFENCE_TRANSITION_BIT_COUNT = 6;
const size_t BUNDLE_NAME_OFFSET = 2;

#ifdef FEATURE_GNSS_SUPPORT
std::shared_ptr<FusionFenceRequest> BuildFusionFenceRequest(const uint8_t* data, size_t size)
{
    int index = 0;
    std::string identifier(reinterpret_cast<const char*>(data), size);
    std::string bundleName(reinterpret_cast<const char*>(&data[BUNDLE_NAME_OFFSET]),
        size - BUNDLE_NAME_OFFSET);
    auto request = std::make_shared<FusionFenceRequest>();
    request->SetIdentifier(identifier);
    request->SetBundleName(bundleName);
    request->SetScene(static_cast<FusionFenceScene>(
        data[index++] % FUSION_FENCE_SCENE_TYPE_COUNT + 1));
    request->SetFenceType(static_cast<int32_t>(data[index++]));
    request->SetPoiType(bundleName);
    request->SetMonitorTransitionEvents(static_cast<int32_t>(data[index++]));
    request->SetLoiterTimeMs(static_cast<int64_t>(data[index++]));
    request->SetExpirationMs(static_cast<int64_t>(data[index++]));
    request->SetUid(data[index++]);
    request->SetPid(data[index]);
    auto gnssFence = std::make_shared<FusionFenceGnss>();
    gnssFence->gnssFenceType = GNSS_FENCE_CIRCULAR;
    gnssFence->circularFence = std::make_shared<Geofence>();
    std::vector<std::shared_ptr<FusionFenceGnss>> gnssFences;
    if ((data[index] & 1) == 1) {
        gnssFences.push_back(gnssFence);
    }
    request->SetGnssFences(gnssFences);
    return request;
}

bool FusionFenceAbilityFuzzerTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < MIN_DATA_LEN)) {
        return false;
    }
    FusionFenceAbility* fusionFenceAbility = FusionFenceAbility::GetInstance();
    if (fusionFenceAbility == nullptr) {
        return false;
    }
    auto request = BuildFusionFenceRequest(data, size);
    std::string identifier = request->GetIdentifier();
    std::string bundleName = request->GetBundleName();
    fusionFenceAbility->IsFusionFenceSupported();
    fusionFenceAbility->HasDuplicateAddFusionFenceRequest(request);
    fusionFenceAbility->IsFusionFenceExists(identifier);
    fusionFenceAbility->CheckFenceLimit(request);
    fusionFenceAbility->AddFusionFence(request);
    fusionFenceAbility->HasDuplicateAddFusionFenceRequest(request);
    fusionFenceAbility->IsFusionFenceExists(identifier);
    fusionFenceAbility->GetGnssFenceCount();
    fusionFenceAbility->GetGnssFenceCountForOneApp(bundleName);
    fusionFenceAbility->GetFusionFenceRequest(identifier);

    FusionFenceTransition transition;
    transition.identifier = identifier;
    transition.scene = static_cast<FusionFenceScene>(
        data[0] % FUSION_FENCE_SCENE_TYPE_COUNT + 1);
    transition.transitionEvent = static_cast<GeofenceTransitionEvent>(
        1 << (data[1] % GEOFENCE_TRANSITION_BIT_COUNT));
    fusionFenceAbility->ReportFusionFenceEvent(transition);
    fusionFenceAbility->ReportOperateResult(
        request, GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD, ERRCODE_SUCCESS);
    fusionFenceAbility->RemoveFusionFence(request);
    fusionFenceAbility->RemoveFusionFenceByCallbackWhenAppDie(nullptr);
    std::shared_ptr<FusionFenceRequest> nullRequest = nullptr;
    fusionFenceAbility->AddFusionFence(nullRequest);
    fusionFenceAbility->RemoveFusionFence(nullRequest);
    fusionFenceAbility->HasDuplicateAddFusionFenceRequest(nullRequest);
    fusionFenceAbility->CheckFenceLimit(nullRequest);
    fusionFenceAbility->GetFusionFenceRequest(identifier);
    return true;
}
#endif
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::MIN_DATA_LEN) {
        return 0;
    }
#ifdef FEATURE_GNSS_SUPPORT
    OHOS::FusionFenceAbilityFuzzerTest(data, size);
#endif
    return 0;
}
