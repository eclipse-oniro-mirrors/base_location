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

#ifndef ASYNC_CONTEXT_H
#define ASYNC_CONTEXT_H

#include <chrono>
#include <condition_variable>
#include <ctime>
#include <list>
#include <mutex>
#include <string>
#include <array>
#include "constant_definition.h"
#include "geo_address.h"
#include "location.h"
#include "location_log.h"
#include "locator_proxy.h"
#include "message_parcel.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "request_config.h"
#include "satellite_status.h"
#include "uv.h"
#include "constant_definition.h"
#include "location.h"
#include "napi_util.h"
#include "message_parcel.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "request_config.h"
#include "common_utils.h"
#include "ipc_skeleton.h"
#include "location_log.h"
#include "locator.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Location {
class AsyncContext {
public:
    napi_env env;
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callback[3] = { 0 };
    std::function<void(void*)> executeFunc;
    std::function<void(void*)> completeFunc;
    napi_value resourceName;
    napi_value result[RESULT_SIZE];
    int errCode;

    AsyncContext(napi_env e, napi_async_work w = nullptr, napi_deferred d = nullptr)
    {
        env = e;
        work = w;
        deferred = d;
        executeFunc = nullptr;
        completeFunc = nullptr;
        resourceName = nullptr;
        result[PARAM0] = 0;
        result[PARAM1] = 0;
        errCode = 0;
    }

    AsyncContext() = delete;

    virtual ~AsyncContext()
    {
    }
};

class LocationRequestAsyncContext : public AsyncContext {
public:
    std::unique_ptr<Location> loc;
    std::unique_ptr<RequestConfig> request;

    LocationRequestAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), loc(nullptr), request(nullptr) {}

    LocationRequestAsyncContext() = delete;

    virtual ~LocationRequestAsyncContext() {}
};

class LocationAsyncContext : public AsyncContext {
public:
    std::unique_ptr<Location> loc;

    LocationAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), loc(nullptr) {}

    LocationAsyncContext() = delete;

    virtual ~LocationAsyncContext() {}
};

class LocatorAsyncContext : public AsyncContext {
public:
    std::shared_ptr<Location> loc;

    LocatorAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), loc(nullptr) {}

    LocatorAsyncContext() = delete;

    virtual ~LocatorAsyncContext() {}
};

class SwitchAsyncContext : public AsyncContext {
public:
    bool enable;

    SwitchAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), enable(false) {}

    SwitchAsyncContext() = delete;

    virtual ~SwitchAsyncContext() {}
};

class NmeaAsyncContext : public AsyncContext {
public:
    std::string msg;

    NmeaAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), msg("") {}

    NmeaAsyncContext() = delete;

    virtual ~NmeaAsyncContext() {}
};

class GnssStatusAsyncContext : public AsyncContext {
public:
    std::shared_ptr<SatelliteStatus> statusInfo;

    GnssStatusAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), statusInfo(nullptr) {}

    GnssStatusAsyncContext() = delete;

    virtual ~GnssStatusAsyncContext() {}
};

class CachedLocationAsyncContext : public AsyncContext {
public:
    std::vector<std::shared_ptr<Location>> locationList;

    CachedLocationAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred) {}

    CachedLocationAsyncContext() = delete;

    virtual ~CachedLocationAsyncContext() {}
};

class PrivacyAsyncContext : public AsyncContext {
public:
    int type;
    bool isConfirmed;

    PrivacyAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), type(PRIVACY_TYPE_OTHERS), isConfirmed(false) {}

    PrivacyAsyncContext() = delete;

    virtual ~PrivacyAsyncContext() {}
};

class CachedAsyncContext : public AsyncContext {
public:
    bool enable;
    int locationSize;

    CachedAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), enable(false), locationSize(0) {}

    CachedAsyncContext() = delete;

    virtual ~CachedAsyncContext() {}
};

class CommandAsyncContext : public AsyncContext {
public:
    bool enable;
    std::unique_ptr<LocationCommand> command;

    CommandAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), enable(false), command(nullptr) {}

    CommandAsyncContext() = delete;

    virtual ~CommandAsyncContext() {}
};

class ReverseGeoCodeAsyncContext : public AsyncContext {
public:
    MessageParcel reverseGeoCodeRequest;
    std::list<std::shared_ptr<GeoAddress>> replyList;

    ReverseGeoCodeAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred) {}

    ReverseGeoCodeAsyncContext() = delete;

    virtual ~ReverseGeoCodeAsyncContext() {}
};

class GeoCodeAsyncContext : public AsyncContext {
public:
    MessageParcel geoCodeRequest;
    std::list<std::shared_ptr<GeoAddress>> replyList;

    GeoCodeAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred) {}

    GeoCodeAsyncContext() = delete;

    virtual ~GeoCodeAsyncContext() {}
};

class CurrentLocationAsyncContext : public AsyncContext {
public:
    int timeout;

    CurrentLocationAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), timeout(0) {}

    CurrentLocationAsyncContext() = delete;

    virtual ~CurrentLocationAsyncContext() {}
};
}  // namespace Location
}  // namespace OHOS
#endif // NAPI_UTIL_H
