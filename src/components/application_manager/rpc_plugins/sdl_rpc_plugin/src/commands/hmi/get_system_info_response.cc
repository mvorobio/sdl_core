/*
 * Copyright (c) 2018, Ford Motor Company
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of the Ford Motor Company nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "sdl_rpc_plugin/commands/hmi/get_system_info_response.h"
#include "application_manager/message_helper.h"
#include "application_manager/policies/policy_handler_interface.h"

namespace sdl_rpc_plugin {
using namespace application_manager;
namespace commands {

GetSystemInfoResponse::GetSystemInfoResponse(
    const application_manager::commands::MessageSharedPtr& message,
    ApplicationManager& application_manager,
    rpc_service::RPCService& rpc_service,
    HMICapabilities& hmi_capabilities,
    policy::PolicyHandlerInterface& policy_handle)
    : ResponseFromHMI(message,
                      application_manager,
                      rpc_service,
                      hmi_capabilities,
                      policy_handle) {}

GetSystemInfoResponse::~GetSystemInfoResponse() {}

void GetSystemInfoResponse::Run() {
  LOG4CXX_AUTO_TRACE(logger_);
  const hmi_apis::Common_Result::eType code =
      static_cast<hmi_apis::Common_Result::eType>(
          (*message_)[strings::params][hmi_response::code].asInt());

  const SystemInfo& info = GetSystemInfo(code);

  // We have to set preloaded flag as false in policy table on any response
  // of GetSystemInfo (SDLAQ-CRS-2365)
  policy_handler_.OnGetSystemInfo(
      info.ccpu_version, info.wers_country_code, info.language);
}

const SystemInfo GetSystemInfoResponse::GetSystemInfo(
    const hmi_apis::Common_Result::eType code) const {
  SystemInfo info;

  if (hmi_apis::Common_Result::SUCCESS != code) {
    LOG4CXX_WARN(logger_, "GetSystemError returns an error code " << code);
    return info;
  }
  info.ccpu_version =
      (*message_)[strings::msg_params]["ccpu_version"].asString();

  info.wers_country_code =
      (*message_)[strings::msg_params]["wersCountryCode"].asString();

  const uint32_t lang_code =
      (*message_)[strings::msg_params]["language"].asUInt();
  info.language = application_manager::MessageHelper::CommonLanguageToString(
      static_cast<hmi_apis::Common_Language::eType>(lang_code));

  hmi_capabilities_.set_ccpu_version(info.ccpu_version);

  return info;
}

}  // namespace commands

}  // namespace sdl_rpc_plugin
