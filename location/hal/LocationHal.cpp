/*
 * Copyright (C) 2026 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "LocationHal.h"

#include <android-base/logging.h>

#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

namespace aidl::vendor::intel::location {

namespace {

bool ParseDoubleNoExcept(const std::string& text, double* out) {
    errno = 0;
    char* end = nullptr;
    const double value = std::strtod(text.c_str(), &end);
    if (end == text.c_str()) {
        return false;
    }
    while (*end == ' ' || *end == '\t') {
        ++end;
    }
    if (*end != '\0' || errno == ERANGE) {
        return false;
    }
    *out = value;
    return true;
}

}  // namespace

LocationHal::LocationHal(const std::string& configPath) : mConfigPath(configPath) {
    readConfig();
}

ndk::ScopedAStatus LocationHal::getLatitude(double* _aidl_return) {
    std::lock_guard<std::mutex> lock(mMutex);
    readConfig();
    *_aidl_return = mLatitude;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus LocationHal::getLongitude(double* _aidl_return) {
    std::lock_guard<std::mutex> lock(mMutex);
    readConfig();
    *_aidl_return = mLongitude;
    return ndk::ScopedAStatus::ok();
}

void LocationHal::readConfig() {
    std::ifstream file(mConfigPath);
    if (!file.is_open()) {
        LOG(WARNING) << "LocationHal: cannot open " << mConfigPath;
        return;
    }

    std::string line;
    int lineNum = 0;
    while (std::getline(file, line)) {
        lineNum++;
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        // Parse key=value
        size_t pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);

        // Trim whitespace
        while (!key.empty() && (key.back() == ' ' || key.back() == '\t')) {
            key.pop_back();
        }
        while (!val.empty() && (val.front() == ' ' || val.front() == '\t')) {
            val.erase(0, 1);
        }

        // Parse values
        if (key == "lat") {
            double parsed = 0.0;
            if (ParseDoubleNoExcept(val, &parsed)) {
                mLatitude = parsed;
            } else {
                LOG(WARNING) << "LocationHal: invalid latitude value: " << val;
            }
        } else if (key == "lon") {
            double parsed = 0.0;
            if (ParseDoubleNoExcept(val, &parsed)) {
                mLongitude = parsed;
            } else {
                LOG(WARNING) << "LocationHal: invalid longitude value: " << val;
            }
        }
    }
}

}  // namespace aidl::vendor::intel::location
