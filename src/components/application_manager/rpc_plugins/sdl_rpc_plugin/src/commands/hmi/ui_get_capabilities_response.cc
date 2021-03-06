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
#include "sdl_rpc_plugin/commands/hmi/ui_get_capabilities_response.h"

namespace sdl_rpc_plugin {
using namespace application_manager;

namespace commands {

UIGetCapabilitiesResponse::UIGetCapabilitiesResponse(
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

UIGetCapabilitiesResponse::~UIGetCapabilitiesResponse() {}

void UIGetCapabilitiesResponse::Run() {
  LOG4CXX_AUTO_TRACE(logger_);

  HMICapabilities& hmi_capabilities = hmi_capabilities_;

  const smart_objects::SmartObject& msg_params =
      (*message_)[strings::msg_params];

  if (msg_params.keyExists(hmi_response::display_capabilities)) {
    hmi_capabilities.set_display_capabilities(
        msg_params[hmi_response::display_capabilities]);
  }

  if (msg_params.keyExists(hmi_response::hmi_zone_capabilities)) {
    hmi_capabilities.set_hmi_zone_capabilities(
        msg_params[hmi_response::hmi_zone_capabilities]);
  }

  if (msg_params.keyExists(hmi_response::soft_button_capabilities)) {
    hmi_capabilities.set_soft_button_capabilities(
        msg_params[hmi_response::soft_button_capabilities]);
  }

  // use newer parameter "audioPassThruCapabilitiesList" when available
  if (msg_params.keyExists(strings::audio_pass_thru_capabilities_list)) {
    hmi_capabilities.set_audio_pass_thru_capabilities(
        msg_params[strings::audio_pass_thru_capabilities_list]);
  } else if (msg_params.keyExists(strings::audio_pass_thru_capabilities)) {
    smart_objects::SmartObject audio_pass_thru_capabilities_list(
        smart_objects::SmartType_Array);
    audio_pass_thru_capabilities_list[0] =
        msg_params[strings::audio_pass_thru_capabilities];
    hmi_capabilities.set_audio_pass_thru_capabilities(
        audio_pass_thru_capabilities_list);
  }

  if (msg_params.keyExists(strings::hmi_capabilities)) {
    if (msg_params[strings::hmi_capabilities].keyExists(strings::navigation)) {
      hmi_capabilities.set_navigation_supported(
          msg_params[strings::hmi_capabilities][strings::navigation].asBool());
    }
    if (msg_params[strings::hmi_capabilities].keyExists(strings::phone_call)) {
      hmi_capabilities.set_phone_call_supported(
          msg_params[strings::hmi_capabilities][strings::phone_call].asBool());
    }
    if (msg_params[strings::hmi_capabilities].keyExists(
            strings::video_streaming)) {
      hmi_capabilities.set_video_streaming_supported(
          msg_params[strings::hmi_capabilities][strings::video_streaming]
              .asBool());
    }
  }

  if (msg_params.keyExists(strings::system_capabilities)) {
    if (msg_params[strings::system_capabilities].keyExists(
            strings::navigation_capability)) {
      hmi_capabilities.set_navigation_capability(
          msg_params[strings::system_capabilities]
                    [strings::navigation_capability]);
    }
    if (msg_params[strings::system_capabilities].keyExists(
            strings::phone_capability)) {
      hmi_capabilities.set_phone_capability(
          msg_params[strings::system_capabilities][strings::phone_capability]);
    }
    if (msg_params[strings::system_capabilities].keyExists(
            strings::video_streaming_capability)) {
      hmi_capabilities.set_video_streaming_capability(
          msg_params[strings::system_capabilities]
                    [strings::video_streaming_capability]);
    }
    if (msg_params[strings::system_capabilities].keyExists(
            strings::display_capabilities)) {
      hmi_capabilities.set_system_display_capabilities(
          msg_params[strings::system_capabilities]
                    [strings::display_capabilities]);
    }
  }
}

}  // namespace commands

}  // namespace sdl_rpc_plugin
