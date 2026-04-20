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

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include "LocationHal.h"

using aidl::vendor::intel::location::LocationHal;

int main() {
    android::base::InitLogging(nullptr, android::base::LogdLogger());

    ABinderProcess_setThreadPoolMaxThreadCount(1);

    ABinderProcess_startThreadPool();

    auto service = ndk::SharedRefBase::make<LocationHal>(
            "/data/vendor/location/location.conf");

    const std::string instance =
            std::string(LocationHal::descriptor) + "/default";

    const binder_status_t status =
            AServiceManager_addService(service->asBinder().get(), instance.c_str());
    if (status != STATUS_OK) {
        LOG(FATAL) << "LocationHal: failed to register " << instance
                   << " status=" << status;
        return EXIT_FAILURE;
    }

    ABinderProcess_joinThreadPool();
    LOG(ERROR) << "LocationHal: Thread pool ended unexpectedly";
    return EXIT_FAILURE;
}
