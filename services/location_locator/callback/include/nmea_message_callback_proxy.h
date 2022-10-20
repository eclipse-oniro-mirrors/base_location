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

#ifndef NMEA_MESSAGE_CALLBACK_PROXY_H
#define NMEA_MESSAGE_CALLBACK_PROXY_H

#include <string>

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"

#include "i_nmea_message_callback.h"

namespace OHOS {
namespace Location {
class NmeaMessageCallbackProxy : public IRemoteProxy<INmeaMessageCallback> {
public:
    explicit NmeaMessageCallbackProxy(const sptr<IRemoteObject> &impl);
    ~NmeaMessageCallbackProxy() = default;
    void OnMessageChange(const std::string msg) override;
private:
    static inline BrokerDelegator<NmeaMessageCallbackProxy> delegator_;
};
} // namespace Location
} // namespace OHOS
#endif // NMEA_MESSAGE_CALLBACK_PROXY_H
