/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "telephony_state_registry_stub.h"

#include "ipc_skeleton.h"
#include "string_ex.h"

#include "telephony_errors.h"
#include "sim_state_type.h"

namespace OHOS {
namespace Telephony {
TelephonyStateRegistryStub::TelephonyStateRegistryStub()
{
    memberFuncMap_[StateNotifyCode::CELL_INFO] = &TelephonyStateRegistryStub::OnUpdateCellInfo;
    memberFuncMap_[StateNotifyCode::SIM_STATE] = &TelephonyStateRegistryStub::OnUpdateSimState;
    memberFuncMap_[StateNotifyCode::SIGNAL_INFO] = &TelephonyStateRegistryStub::OnUpdateSignalInfo;
    memberFuncMap_[StateNotifyCode::NET_WORK_STATE] = &TelephonyStateRegistryStub::OnUpdateNetworkState;
    memberFuncMap_[StateNotifyCode::CALL_STATE] = &TelephonyStateRegistryStub::OnUpdateCallState;
    memberFuncMap_[StateNotifyCode::CALL_STATE_FOR_ID] = &TelephonyStateRegistryStub::OnUpdateCallStateForSimId;
    memberFuncMap_[StateNotifyCode::ADD_OBSERVER] = &TelephonyStateRegistryStub::OnRegisterStateChange;
    memberFuncMap_[StateNotifyCode::REMOVE_OBSERVER] = &TelephonyStateRegistryStub::OnUnregisterStateChange;
}

TelephonyStateRegistryStub::~TelephonyStateRegistryStub()
{
    memberFuncMap_.clear();
}

int32_t TelephonyStateRegistryStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnRemoteRequest start##code = %{public}u\n", code);
    std::u16string myToken = TelephonyStateRegistryStub::GetDescriptor();
    std::u16string remoteToken = data.ReadInterfaceToken();
    if (myToken != remoteToken) {
        TELEPHONY_LOGE("TelephonyStateRegistryStub::OnRemoteRequest end##descriptor checked fail");
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    auto itFunc = memberFuncMap_.find(static_cast<StateNotifyCode>(code));
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    int ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnRemoteRequest end##ret=%{public}d\n", ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnUpdateCallState(MessageParcel &data, MessageParcel &reply)
{
    int32_t callState = data.ReadInt32();
    std::u16string phoneNumber = data.ReadString16();
    int32_t ret = UpdateCallState(callState, phoneNumber);
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnUpdateCallState end##ret=%{public}d\n", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnUpdateSimState(MessageParcel &data, MessageParcel &reply)
{
    int32_t simId = data.ReadInt32();
    SimState state = static_cast<SimState>(data.ReadInt32());
    LockReason reason = static_cast<LockReason>(data.ReadInt32());
    int32_t ret = UpdateSimState(simId, state, reason);
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnUpdateSimState end##ret=%{public}d\n", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnUpdateCallStateForSimId(MessageParcel &data, MessageParcel &reply)
{
    int32_t simId = data.ReadInt32();
    int32_t callId = data.ReadInt32();
    int32_t callState = data.ReadInt32();
    std::u16string incomingNumber = data.ReadString16();
    int32_t ret = UpdateCallStateForSimId(simId, callId, callState, incomingNumber);
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnUpdateCallStateForSimId end##ret=%{public}d\n", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnUpdateSignalInfo(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = TELEPHONY_SUCCESS;
    int32_t simId = data.ReadInt32();
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnUpdateSignalInfo size=%{public}d\n", size);
    size = ((size > SignalInformation::MAX_SIGNAL_NUM) ? 0 : size);
    if (size <= 0) {
        ret = TELEPHONY_ERR_FAIL;
        TELEPHONY_LOGE("TelephonyStateRegistryStub::OnUpdateSignalInfo size <= 0\n");
        return ret;
    }
    std::vector<sptr<SignalInformation>> result;
    parseSignalInfos(data, size, result);
    ret = UpdateSignalInfo(simId, result);
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnUpdateSignalInfo end##ret=%{public}d\n", ret);
    reply.WriteInt32(ret);
    return ret;
}

void TelephonyStateRegistryStub::parseSignalInfos(
    MessageParcel &data, const int32_t size, std::vector<sptr<SignalInformation>> &result)
{
    SignalInformation::NetworkType type;
    for (int i = 0; i < size; ++i) {
        type = static_cast<SignalInformation::NetworkType>(data.ReadInt32());
        switch (type) {
            case SignalInformation::NetworkType::GSM: {
                TELEPHONY_LOGI("TelephonyStateRegistryStub::parseSignalInfos NetworkType::GSM\n");
                std::unique_ptr<GsmSignalInformation> signal = std::make_unique<GsmSignalInformation>();
                if (signal != nullptr) {
                    signal->ReadFromParcel(data);
                    result.emplace_back(signal.release());
                }
                break;
            }
            case SignalInformation::NetworkType::CDMA: {
                TELEPHONY_LOGI("TelephonyStateRegistryStub::parseSignalInfos NetworkType::CDMA\n");
                std::unique_ptr<CdmaSignalInformation> signal = std::make_unique<CdmaSignalInformation>();
                if (signal != nullptr) {
                    signal->ReadFromParcel(data);
                    result.emplace_back(signal.release());
                }
                break;
            }
            case SignalInformation::NetworkType::LTE: {
                TELEPHONY_LOGI("TelephonyStateRegistryStub::parseSignalInfos NetworkType::LTE\n");
                std::unique_ptr<LteSignalInformation> signal = std::make_unique<LteSignalInformation>();
                if (signal != nullptr) {
                    signal->ReadFromParcel(data);
                    result.emplace_back(signal.release());
                }
                break;
            }
            case SignalInformation::NetworkType::WCDMA: {
                TELEPHONY_LOGI("TelephonyStateRegistryStub::parseSignalInfos NetworkType::Wcdma\n");
                std::unique_ptr<WcdmaSignalInformation> signal = std::make_unique<WcdmaSignalInformation>();
                if (signal != nullptr) {
                    signal->ReadFromParcel(data);
                    result.emplace_back(signal.release());
                }
                break;
            }
            default:
                break;
        }
    }
}

int32_t TelephonyStateRegistryStub::OnUpdateCellInfo(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = TELEPHONY_SUCCESS;
    int32_t simId = data.ReadInt32();
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGI("OnRemoteRequest OnUpdateCellInfo:size=%{public}d\n", size);
    size = ((size > CellInformation::MAX_CELL_NUM) ? 0 : size);
    if (size <= 0) {
        ret = TELEPHONY_ERR_FAIL;
        return ret;
    }
    std::vector<sptr<CellInformation>> cells;
    CellInformation::CellType type;
    for (int i = 0; i < size; ++i) {
        type = static_cast<CellInformation::CellType>(data.ReadInt32());
        switch (type) {
            case CellInformation::CellType::CELL_TYPE_GSM: {
                std::unique_ptr<GsmCellInformation> cell = std::make_unique<GsmCellInformation>();
                if (cell != nullptr) {
                    cell->ReadFromParcel(data);
                    cells.emplace_back(cell.release());
                }
                break;
            }
            case CellInformation::CellType::CELL_TYPE_LTE: {
                std::unique_ptr<LteCellInformation> cell = std::make_unique<LteCellInformation>();
                if (cell != nullptr) {
                    cell->ReadFromParcel(data);
                    cells.emplace_back(cell.release());
                }
                break;
            }
            default:
                break;
        }
    }
    ret = UpdateCellInfo(simId, cells);
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnUpdateCellInfo end##ret=%{public}d\n", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnUpdateNetworkState(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = TELEPHONY_SUCCESS;
    int32_t simId = data.ReadInt32();
    sptr<NetworkState> result = NetworkState::Unmarshalling(data);
    if (result == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryStub::OnUpdateNetworkState GetNetworkStatus  is null\n");
        ret = TELEPHONY_ERR_FAIL;
        return ret;
    }
    ret = UpdateNetworkState(simId, result);
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnUpdateNetworkState end##ret=%{public}d\n", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnRegisterStateChange(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = TELEPHONY_SUCCESS;
    int32_t simId = data.ReadInt32();
    int32_t mask = data.ReadInt32();
    bool notifyNow = data.ReadBool();
    std::u16string callingPackage = data.ReadString16();
    sptr<TelephonyObserverBroker> callback = nullptr;
    ret = ReadData(data, reply, callback);
    if (ret != TELEPHONY_SUCCESS) {
        reply.WriteInt32(ret);
        return ret;
    }
    ret = RegisterStateChange(callback, simId, mask, callingPackage, notifyNow);
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnRegisterStateChange end##ret=%{public}d\n", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnUnregisterStateChange(MessageParcel &data, MessageParcel &reply)
{
    int32_t simId = data.ReadInt32();
    int32_t mask = data.ReadInt32();
    int32_t ret = UnregisterStateChange(simId, mask);
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnUnregisterStateChange end##ret=%{public}d\n", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::ReadData(
    MessageParcel &data, MessageParcel &reply, sptr<TelephonyObserverBroker> &callback)
{
    int32_t result = TELEPHONY_SUCCESS;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryStub::ReadData  remote is nullptr.\n");
        result = TELEPHONY_ERR_FAIL;
        reply.WriteInt32(result);
        return result;
    }
    callback = iface_cast<TelephonyObserverBroker>(remote);
    if (callback == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryStub::ReadData callback is nullptr.\n");
        result = TELEPHONY_ERR_FAIL;
        reply.WriteInt32(result);
        return result;
    }
    return result;
}

int32_t TelephonyStateRegistryStub::RegisterStateChange(const sptr<TelephonyObserverBroker> &telephonyObserver,
    int32_t simId, uint32_t mask, const std::u16string &package, bool isUpdate)
{
    return RegisterStateChange(telephonyObserver, simId, mask, package, isUpdate, IPCSkeleton::GetCallingPid());
}

int32_t TelephonyStateRegistryStub::UnregisterStateChange(int32_t simId, uint32_t mask)
{
    return UnregisterStateChange(simId, mask, IPCSkeleton::GetCallingPid());
}
} // namespace Telephony
} // namespace OHOS