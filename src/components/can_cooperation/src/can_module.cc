#include "can_cooperation/can_module.h"
#include "can_cooperation/mobile_command_factory.h"
#include "can_cooperation/can_module_event.h"
#include "can_cooperation/event_engine/event_dispatcher.h"
#include "application_manager/application.h"
#include "./can_tcp_connection.h"
#include "utils/logger.h"
#include "utils/threads/thread.h"


namespace can_cooperation {

using functional_modules::ProcessResult;
using functional_modules::GenericModule;
using functional_modules::PluginInfo;
using functional_modules::MobileFunctionID;
using event_engine::EventDispatcher;
namespace hmi_api = functional_modules::hmi_api;

EXPORT_FUNCTION_IMPL(CANModule);
CREATE_LOGGERPTR_GLOBAL(logger_, "CanModule");

class TCPClientDelegate : public threads::ThreadDelegate {
public:
  explicit TCPClientDelegate(CANModule* can_module);
  ~TCPClientDelegate();
  void threadMain();
  bool exitThreadMain();
private:
  CANModule* can_module_;
  bool stop_flag_;
};

TCPClientDelegate::TCPClientDelegate(CANModule* can_module)
: can_module_(can_module)
, stop_flag_(false) {
  DCHECK(can_module);
}

TCPClientDelegate::~TCPClientDelegate() {
  can_module_ = NULL;
  stop_flag_ = true;
}

void TCPClientDelegate::threadMain() {
  while(!stop_flag_) {
    while(ConnectionState::OPENED == 
        can_module_->can_connection->GetData()) {
      can_module_->from_can_.PostMessage(
        static_cast<CANTCPConnection*>(can_module_->can_connection.get())
          ->ReadData());
    }
  }
}

bool TCPClientDelegate::exitThreadMain() {
  stop_flag_ = true;
  return false;
}

CANModule::CANModule()
  : GenericModule(kCANModuleID)
  , can_connection()
  , from_can_("FromCan To Mobile", this)
  , from_mobile_("FromMobile To Can", this)
  , thread_(NULL)
  , is_scan_started_(false) {
  	can_connection = new CANTCPConnection;
  	if (ConnectionState::OPENED != can_connection->OpenConnection()) {
  		LOG4CXX_ERROR(logger_, "Failed to connect to CAN");
  	} else {
      thread_ = new threads::Thread("CANClientListener", new TCPClientDelegate(this));
      const size_t kStackSize = 16384;
      thread_->startWithOptions(threads::ThreadOptions(kStackSize));
    }
  plugin_info_.name = "ReverseSDLPlugin";
  plugin_info_.version = 1;
  SubscribeOnFunctions();
}

void CANModule::SubscribeOnFunctions() {
  plugin_info_.mobile_function_list.push_back(MobileFunctionID::TUNE_RADIO);
  plugin_info_.mobile_function_list.push_back(MobileFunctionID::TUNE_UP);
  plugin_info_.mobile_function_list.push_back(MobileFunctionID::TUNE_DOWN);
  plugin_info_.mobile_function_list.push_back(MobileFunctionID::GRANT_ACCESS);
  plugin_info_.mobile_function_list.push_back(MobileFunctionID::CANCEL_ACCESS);
  plugin_info_.mobile_function_list.push_back(MobileFunctionID::START_SCAN);
  plugin_info_.mobile_function_list.push_back(MobileFunctionID::STOP_SCAN);
  plugin_info_.mobile_function_list.push_back(
      MobileFunctionID::ON_CONTROL_CHANGED);
  plugin_info_.mobile_function_list.push_back(
      MobileFunctionID::ON_RADIO_DETAILS);
  plugin_info_.mobile_function_list.push_back(
      MobileFunctionID::ON_PRESET_CHANGED);

  plugin_info_.hmi_function_list.push_back(hmi_api::grant_access);
  plugin_info_.hmi_function_list.push_back(hmi_api::cancel_access);
  plugin_info_.hmi_function_list.push_back(hmi_api::on_control_changed);
}

CANModule::~CANModule() {
  if (can_connection) {
    can_connection->CloseConnection();
  }
  if (thread_ ) {
    thread_->stop();
    delete thread_;
  }
  RemoveAppExtensions();
}

functional_modules::PluginInfo CANModule::GetPluginInfo() const {
  return plugin_info_;
}

ProcessResult CANModule::ProcessMessage(application_manager::MessagePtr msg) {
  DCHECK(msg);
  if (!msg) {
    LOG4CXX_ERROR(logger_, "Null pointer message received.");
    return ProcessResult::FAILED;
  }

  commands::Command* command = MobileCommandFactory::CreateCommand(msg);
  if (command) {
    request_controller_.AddRequest(msg->correlation_id(), command);
    command->Run();
  }

  return ProcessResult::PROCESSED;
}

void CANModule::SendMessageToCan(const std::string& msg) {
  from_mobile_.PostMessage(msg);
}

ProcessResult CANModule::ProcessHMIMessage(application_manager::MessagePtr msg) {
  return HandleMessage(msg);
}

void CANModule::ProcessCANMessage(const MessageFromCAN& can_msg) {
  DCHECK(Json::ValueType::objectValue == can_msg.type());
  from_can_.PostMessage(can_msg);
}

void CANModule::Handle(const std::string message) {
  static_cast<CANTCPConnection*>(can_connection.get())->WriteData(
      message);

  if (ConnectionState::OPENED != can_connection->Flash()) {
    LOG4CXX_ERROR(logger_, "Failed to send message to CAN");
  }
}
  
void CANModule::Handle(const MessageFromCAN can_msg) {
  application_manager::MessagePtr msg(new application_manager::Message(
      protocol_handler::MessagePriority::kDefault));

  Json::FastWriter writer;
  std::string msg_to_send = writer.write(can_msg);
  msg->set_json_message(msg_to_send);

  if (ProcessResult::PROCESSED != HandleMessage(msg)) {
    LOG4CXX_ERROR(logger_, "Failed process CAN message!");
  }
}

functional_modules::ProcessResult CANModule::HandleMessage(
    application_manager::MessagePtr msg) {

  Json::Value value;
  Json::Reader reader;
  reader.parse(msg->json_message(), value);

  std::string function_name;

  // Request or notification
  if (value.isMember("method")) {
    function_name = value["method"].asCString();

    if (value.isMember("id")) {
      msg->set_message_type(application_manager::MessageType::kRequest);
    } else {
      msg->set_message_type(application_manager::MessageType::kNotification);
    }
  // Response
  } else if (value.isMember("result") && value["result"].isMember("method")) {
    function_name = value["result"]["method"].asCString();
    msg->set_message_type(application_manager::MessageType::kResponse);
  // Error response
  }  else if (value.isMember("error") && value["error"].isMember("data") &&
              value["error"]["data"].isMember("method")) {
    function_name = value["error"]["data"]["method"].asCString();
    msg->set_message_type(application_manager::MessageType::kErrorResponse);
  } else {
    DCHECK(false);
    return ProcessResult::FAILED;
  }


  if (value.isMember("id")) {
    msg->set_correlation_id(value["id"].asInt());
  } else if (application_manager::MessageType::kNotification !=  msg->type()) {
    DCHECK(false);
    return ProcessResult::FAILED;
  }

  msg->set_protocol_version(application_manager::ProtocolVersion::kV3);

  switch (msg->type()) {
    case application_manager::MessageType::kResponse:
    case application_manager::MessageType::kErrorResponse: {
      CanModuleEvent event(msg, function_name);
      EventDispatcher<application_manager::MessagePtr, std::string>::instance()->
          raise_event(event);
      break;
    }
    case application_manager::MessageType::kNotification: {
      break;
    }
    case application_manager::MessageType::kRequest:
    default: {
      return  ProcessResult::FAILED;;
    }
  }

  return ProcessResult::PROCESSED;
}

void CANModule::SendResponseToMobile(application_manager::MessagePtr msg) {
  service_->SendMessageToMobile(msg);
  request_controller_.DeleteRequest(msg->correlation_id());
}

bool CANModule::IsScanStarted() const {
  return is_scan_started_;
}

void CANModule::SetScanStarted(bool is_scan_started) {
  is_scan_started_ = is_scan_started;
}

void CANModule::RemoveAppExtensions() {

}

void CANModule::RemoveAppExtension(uint32_t app_id) {
  application_manager::ApplicationSharedPtr app =
      service_->GetApplication(app_id);

  if (app.valid()) {
    application_manager::AppExtensionPtr extension =
        app->QueryInterface(kCANModuleID);

   if (extension.valid()) {
      app->RemoveExtension(kCANModuleID);
    }
  }
}

application_manager::ServicePtr CANModule::GetServiceHandler() {
  return service_;
}

}  //  namespace can_cooperation

