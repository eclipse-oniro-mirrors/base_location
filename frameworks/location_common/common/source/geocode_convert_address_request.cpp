/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "geocode_convert_address_request.h"

namespace OHOS {
namespace Location {
GeocodeConvertAddressRequest::GeocodeConvertAddressRequest()
{
    locale_ = "";
    maxItems_ = 0;
    description_ = "";
    maxLatitude_ = 0.0;
    maxLongitude_ = 0.0;
    minLatitude_ = 0.0;
    minLongitude_ = 0.0;
    transId_ = "";
    country_ = "";
}

GeocodeConvertAddressRequest::~GeocodeConvertAddressRequest() {}

std::string GeocodeConvertAddressRequest::GetLocale()
{
    return locale_;
}

void GeocodeConvertAddressRequest::SetLocale(std::string locale)
{
    locale_ = locale;
}

int32_t GeocodeConvertAddressRequest::GetMaxItems()
{
    return maxItems_;
}

void GeocodeConvertAddressRequest::SetMaxItems(int32_t maxItems)
{
    maxItems_ = maxItems;
}

std::string GeocodeConvertAddressRequest::GetDescription()
{
    return description_;
}

void GeocodeConvertAddressRequest::SetDescription(std::string description)
{
    description_ = description;
}

double GeocodeConvertAddressRequest::GetMaxLatitude()
{
    return maxLatitude_;
}

void GeocodeConvertAddressRequest::SetMaxLatitude(double maxLatitude)
{
    maxLatitude_ = maxLatitude;
}

double GeocodeConvertAddressRequest::GetMaxLongitude()
{
    return maxLongitude_;
}

void GeocodeConvertAddressRequest::SetMaxLongitude(double maxLongitude)
{
    maxLongitude_ = maxLongitude;
}

double GeocodeConvertAddressRequest::GetMinLatitude()
{
    return minLatitude_;
}

void GeocodeConvertAddressRequest::SetMinLatitude(double minLatitude)
{
    minLatitude_ = minLatitude;
}

double GeocodeConvertAddressRequest::GetMinLongitude()
{
    return minLongitude_;
}

void GeocodeConvertAddressRequest::SetMinLongitude(double minLongitude)
{
    minLongitude_ = minLongitude;
}

std::string GeocodeConvertAddressRequest::GetTransId()
{
    return transId_;
}

void GeocodeConvertAddressRequest::SetTransId(std::string transId)
{
    transId_ = transId;
}

std::string GeocodeConvertAddressRequest::GetCountry()
{
    return country_;
}

void GeocodeConvertAddressRequest::SetCountry(std::string country)
{
    country_ = country;
}

bool GeocodeConvertAddressRequest::Marshalling(Parcel& parcel) const
{
    parcel.WriteString16(Str8ToStr16(locale_)); // locale
    parcel.WriteString16(Str8ToStr16(description_)); // description
    parcel.WriteInt32(maxItems_); // maxItems
    parcel.WriteDouble(minLatitude_); // minLatitude
    parcel.WriteDouble(minLongitude_); // minLongitude
    parcel.WriteDouble(maxLatitude_); // maxLatitude
    parcel.WriteDouble(maxLongitude_); // maxLongitude
    parcel.WriteString16(Str8ToStr16(transId_)); // transId
    parcel.WriteString16(Str8ToStr16(country_)); // country
    return true;
}

GeocodeConvertAddressRequest* GeocodeConvertAddressRequest::Unmarshalling(Parcel& parcel)
{
    auto geoConvertRequest = new GeocodeConvertAddressRequest();
    geoConvertRequest->ReadFromParcel(parcel);
    return geoConvertRequest;
}

void GeocodeConvertAddressRequest::ReadFromParcel(Parcel& parcel)
{
    locale_ = Str16ToStr8(parcel.ReadString16()); // locale
    description_ = Str16ToStr8(parcel.ReadString16()); // description
    maxItems_ = parcel.ReadInt32(); // maxItems
    minLatitude_ = parcel.ReadDouble(); // minLatitude
    minLongitude_ = parcel.ReadDouble(); // minLongitude
    maxLatitude_ = parcel.ReadDouble(); // maxLatitude
    maxLongitude_ = parcel.ReadDouble(); // maxLongitude
    transId_ = Str16ToStr8(parcel.ReadString16()); // transId
    country_ = Str16ToStr8(parcel.ReadString16()); // country
}

std::unique_ptr<GeocodeConvertAddressRequest> GeocodeConvertAddressRequest::UnmarshallingMessageParcel(
    MessageParcel& parcel)
{
    auto geoConvertRequest = std::make_unique<GeocodeConvertAddressRequest>();
    geoConvertRequest->ReadFromMessageParcel(parcel);
    return geoConvertRequest;
}

void GeocodeConvertAddressRequest::ReadFromMessageParcel(MessageParcel& parcel)
{
    locale_ = Str16ToStr8(parcel.ReadString16()); // locale
    description_ = Str16ToStr8(parcel.ReadString16()); // description
    maxItems_ = parcel.ReadInt32(); // maxItems
    minLatitude_ = parcel.ReadDouble(); // minLatitude
    minLongitude_ = parcel.ReadDouble(); // minLongitude
    maxLatitude_ = parcel.ReadDouble(); // maxLatitude
    maxLongitude_ = parcel.ReadDouble(); // maxLongitude
    transId_ = Str16ToStr8(parcel.ReadString16()); // transId
    country_ = Str16ToStr8(parcel.ReadString16()); // country
}
} // namespace Location
} // namespace OHOS
