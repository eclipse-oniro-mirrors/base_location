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

#include "location_error_callback_napi.h"
#include "common_utils.h"
#include "ipc_skeleton.h"
#include "location_log.h"
#include "napi/native_common.h"
#include "napi_util.h"
#include "location_async_context.h"

namespace OHOS {
namespace Location {
LocationErrorCallbackNapi::LocationErrorCallbackNapi()
{
    env_ = nullptr;
    handlerCb_ = nullptr;
    callbackValid_ = false;
}

LocationErrorCallbackNapi::~LocationErrorCallbackNapi()
{
}

int LocationErrorCallbackNapi::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGI(LOCATION_ERR_CALLBACK, "LocatorCallbackHost::OnRemoteRequest! code = %{public}d", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(LOCATION_ERR_CALLBACK, "invalid token.");
        return -1;
    }

    switch (code) {
        case RECEIVE_ERROR_INFO_EVENT: {
            OnErrorReport(data.ReadInt32());
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

bool LocationErrorCallbackNapi::Send(int32_t errorCode)
{
    LBSLOGI(LOCATION_ERR_CALLBACK, "LocatorCallbackNapi::OnRemoteRequest! errorCode = %{public}d", errorCode);
    std::unique_lock<std::mutex> guard(mutex_);
    uv_loop_s *loop = nullptr;
    NAPI_CALL_BASE(env_, napi_get_uv_event_loop(env_, &loop), false);
    if (loop == nullptr) {
        LBSLOGE(LOCATION_ERR_CALLBACK, "loop == nullptr.");
        return false;
    }
    if (handlerCb_ == nullptr) {
        LBSLOGE(LOCATION_ERR_CALLBACK, "handler is nullptr.");
        return false;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LBSLOGE(LOCATION_ERR_CALLBACK, "work == nullptr.");
        return false;
    }
    LocationErrorAsyncContext *context = new (std::nothrow) LocationErrorAsyncContext(env_);
    if (context == nullptr) {
        LBSLOGE(LOCATION_ERR_CALLBACK, "context == nullptr.");
        delete work;
        return false;
    }
    if (!InitContext(context)) {
        LBSLOGE(LOCATION_ERR_CALLBACK, "InitContext fail");
        return false;
    }
    context->errCode = errorCode;
    work->data = context;
    UvQueueWork(loop, work);
    return true;
}

void LocationErrorCallbackNapi::UvQueueWork(uv_loop_s* loop, uv_work_t* work)
{
    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            LocationErrorAsyncContext *context = nullptr;
            napi_handle_scope scope = nullptr;
            if (work == nullptr) {
                LBSLOGE(LOCATION_ERR_CALLBACK, "work is nullptr!");
                return;
            }
            context = static_cast<LocationErrorAsyncContext *>(work->data);
            if (context == nullptr || context->env == nullptr || context->callbackValid == nullptr) {
                LBSLOGE(LOCATION_ERR_CALLBACK, "context is nullptr!");
                delete work;
                return;
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_open_handle_scope(context->env, &scope));
            napi_value jsEvent;
            CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_create_int32(context->env, context->errCode, &jsEvent),
                scope, context, work);
            if (scope == nullptr) {
                LBSLOGE(LOCATION_ERR_CALLBACK, "scope is nullptr");
                delete context;
                delete work;
                return;
            }
            if (context->callback[0] != nullptr && *(context->callbackValid)) {
                napi_value undefine;
                napi_value handler = nullptr;
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_get_undefined(context->env, &undefine),
                    scope, context, work);
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env,
                    napi_get_reference_value(context->env, context->callback[0], &handler), scope, context, work);
                if (napi_call_function(context->env, nullptr, handler, 1,
                    &jsEvent, &undefine) != napi_ok) {
                    LBSLOGE(LOCATION_ERR_CALLBACK, "Report event failed");
                }
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_close_handle_scope(context->env, scope));
            delete context;
            delete work;
    });
}

void LocationErrorCallbackNapi::OnLocationReport(const std::unique_ptr<Location>& location)
{
}

void LocationErrorCallbackNapi::OnLocatingStatusChange(const int status)
{
}

void LocationErrorCallbackNapi::OnErrorReport(const int errorCode)
{
    LBSLOGI(LOCATION_ERR_CALLBACK, "LocatorCallbackNapi::OnRemoteRequest! errorCode = %{public}d", errorCode);
    Send(errorCode);
}

void LocationErrorCallbackNapi::DeleteHandler()
{
    std::unique_lock<std::mutex> guard(mutex_);
    if (handlerCb_ == nullptr || env_ == nullptr) {
        LBSLOGE(LOCATION_ERR_CALLBACK, "handler or env is nullptr.");
        return;
    }
    NAPI_CALL_RETURN_VOID(env_, napi_delete_reference(env_, handlerCb_));
    handlerCb_ = nullptr;
    callbackValid_ = false;
}
}  // namespace Location
}  // namespace OHOS