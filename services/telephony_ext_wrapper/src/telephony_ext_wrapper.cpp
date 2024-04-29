/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dlfcn.h>
#include "telephony_ext_wrapper.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
namespace {
const std::string TELEPHONY_EXT_WRAPPER_PATH = "libtel_ext_symbol.z.so";
} // namespace

TelephonyExtWrapper::TelephonyExtWrapper() {}
TelephonyExtWrapper::~TelephonyExtWrapper()
{
    TELEPHONY_LOGD("TelephonyExtWrapper::~TelephonyExtWrapper() start");
    dlclose(telephonyExtWrapperHandle_);
    telephonyExtWrapperHandle_ = nullptr;
}

void TelephonyExtWrapper::InitTelephonyExtWrapper()
{
    TELEPHONY_LOGD("TelephonyExtWrapper::InitTelephonyExtWrapper() start");
    telephonyExtWrapperHandle_ = dlopen(TELEPHONY_EXT_WRAPPER_PATH.c_str(), RTLD_NOW);
    if (telephonyExtWrapperHandle_ == nullptr) {
        TELEPHONY_LOGE("libtel_ext_symbol.z.so was not loaded, error: %{public}s", dlerror());
        return;
    }

    onNetworkStateUpdated_ = (ON_NETWORK_STATE_UPDATE)dlsym(telephonyExtWrapperHandle_, "OnNetworkStateUpdatedExt");
    onSignalInfoUpdated_ = (ON_SIGNAL_INFO_UPDATE)dlsym(telephonyExtWrapperHandle_, "OnSignalInfoUpdatedExt");
    onCellInfoUpdated_ = (ON_CELL_INFO_UPDATE)dlsym(telephonyExtWrapperHandle_, "OnCellInfoUpdatedExt");
    onCellularDataConnectStateUpdated_ = (ON_CELLULAR_DATA_CONNECT_STATE_UPDATE)
        dlsym(telephonyExtWrapperHandle_, "OnCellularDataConnectStateUpdatedExt");

    sendNetworkStateChanged_ = (SEND_NETWORK_STATE_CHANGED)dlsym(telephonyExtWrapperHandle_,
        "SendNetworkStateChangedExt");
    sendSignalInfoChanged_ = (SEND_SIGNAL_INFO_CHANGED)dlsym(telephonyExtWrapperHandle_, "SendSignalInfoChangedExt");
    // Check whether all function pointers are empty.
    if (onNetworkStateUpdated_ == nullptr || onSignalInfoUpdated_ == nullptr || onCellInfoUpdated_ == nullptr
        || onCellularDataConnectStateUpdated_ == nullptr || sendNetworkStateChanged_ == nullptr
        || sendSignalInfoChanged_ == nullptr) {
        TELEPHONY_LOGE("telephony ext wrapper symbol failed, error: %{public}s", dlerror());
        return;
    }

    TELEPHONY_LOGI("telephony ext wrapper init success");
}
} // namespace Telephony
} // namespace OHOS