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

#include "locationdumper_fuzzer.h"

#include "location_dumper.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace OHOS {
using namespace OHOS::Location;
const int MIN_DATA_LEN = 1;
const std::string ARGS_HELP = "-h";

bool LocationDumperFuzzerTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < MIN_DATA_LEN)) {
        return false;
    }
    LocationDumper dumper;
    auto saBasicDumpFunc = [](std::string& result) {
        result.append("fuzz basic dump");
    };
    std::vector<std::string> vecArgs;
    if ((data[0] & 1) == 0) {
        vecArgs.push_back(ARGS_HELP);
    }
    const size_t argLength = 4;
    for (size_t i = 1; i < size; i += argLength) {
        size_t len = std::min(argLength, size - i);
        vecArgs.push_back(std::string(reinterpret_cast<const char*>(&data[i]), len));
    }
    std::string result;
    dumper.GeocodeDump(saBasicDumpFunc, vecArgs, result);
    dumper.GnssDump(saBasicDumpFunc, vecArgs, result);
    dumper.LocatorDump(saBasicDumpFunc, vecArgs, result);
    dumper.NetWorkDump(saBasicDumpFunc, vecArgs, result);
    dumper.PassiveDump(saBasicDumpFunc, vecArgs, result);
    // cover the empty args branch
    std::vector<std::string> emptyArgs;
    dumper.GeocodeDump(saBasicDumpFunc, emptyArgs, result);
    dumper.GnssDump(saBasicDumpFunc, emptyArgs, result);
    dumper.LocatorDump(saBasicDumpFunc, emptyArgs, result);
    dumper.NetWorkDump(saBasicDumpFunc, emptyArgs, result);
    dumper.PassiveDump(saBasicDumpFunc, emptyArgs, result);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::MIN_DATA_LEN) {
        return 0;
    }
    OHOS::LocationDumperFuzzerTest(data, size);
    return 0;
}
