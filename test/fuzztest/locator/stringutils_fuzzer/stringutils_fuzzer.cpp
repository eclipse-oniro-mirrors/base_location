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

#include "stringutils_fuzzer.h"

#include "string_utils.h"

#include <cstdint>
#include <string>
#include <vector>

namespace OHOS {
using namespace OHOS::Location;
const int MIN_DATA_LEN = 1;

#ifdef FEATURE_GNSS_SUPPORT
bool StringUtilsFuzzerTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < MIN_DATA_LEN)) {
        return false;
    }
    std::string rawData(reinterpret_cast<const char*>(data), size);
    // hexStr contains only '0'-'9'/'A'-'F', safe for utf8/utf16 conversions
    std::string hexStr = StringUtils::StringToHex(rawData);

    StringUtils::StringToHex(rawData);
    StringUtils::StringToHex(reinterpret_cast<const char*>(data), static_cast<int>(size));
    std::vector<uint8_t> vecData(data, data + size);
    StringUtils::StringToHex(vecData);

    StringUtils::HexToString(rawData);
    StringUtils::HexToString(hexStr);
    StringUtils::HexToByteVector(rawData);
    StringUtils::HexToByteVector(hexStr);

    std::u16string u16Str(reinterpret_cast<const char16_t*>(hexStr.data()),
        hexStr.size() / sizeof(char16_t));
    StringUtils::ToUtf8(u16Str);
    StringUtils::ToUtf16(rawData);

    StringUtils::Utf8ToWstring(hexStr);
    StringUtils::Ucs2ToWstring(hexStr);
    std::wstring wStr(hexStr.begin(), hexStr.end());
    StringUtils::WstringToString(wStr);
    StringUtils::Gsm7Decode(rawData);
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
    OHOS::StringUtilsFuzzerTest(data, size);
#endif
    return 0;
}
