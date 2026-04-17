#include "state_replicator.h"
// #include <ixwebsocket/IXNetSystem.h>
#include "serialization.h"


/*
using OnConnectionCallback =
			std::function<void(std::weak_ptr<WebSocket>, std::shared_ptr<ConnectionState>)>;

		using OnClientMessageCallback = std::function<void(
			std::shared_ptr<ConnectionState>, WebSocket&, const WebSocketMessagePtr&)>;
*/

void __declspec(noinline) state_replicator::start_server() {

	//_wss.setOnConnectionCallback(ws_server_callback_conn); //guess which error log gets written to stderr instead of stdout and which of those i havent redirected
	//_wss.setOnClientMessageCallback(ws_server_callback);
	//_wss.enablePong();
	//auto result = _wss.listen();
	//_wss.start();
	//std::thread([](){
	//	_wss.wait();
	//}).detach();
	//
	//_is_server = true;
	//if (!(is_connected = result.first))
	//	printf("Failed to make server: %s\n", result.second.c_str());
	//local_playerid = "Server";
}

void state_replicator::connect(std::string ip_addr) {

	//_ws.setUrl("ws://" + ip_addr + ":6969/");
	//_ws.setPingInterval(1);
	//_ws.enablePerMessageDeflate();
	//_ws.setOnMessageCallback(ws_client_callback);
	//auto result = _ws.connect(5);
	//if (!result.success)
	//	printf("Failed to connect: %s\n", result.errorStr.c_str());
	//is_connected = result.success;
	//_is_server = false;
}

std::shared_ptr<state_replicator::MsgBase> state_replicator::get_outstanding_message() {
	//if (!_is_server)
	//	is_connected = _ws.getReadyState() == ix::ReadyState::Open;
	//std::unique_lock lock(_outstanding_messages_mut);
	//if (_outstanding_messages.size() > 0) {
	//	auto msg = *_outstanding_messages.begin();
	//	_outstanding_messages.erase(_outstanding_messages.begin());
	//	//printf("outstanding msg: %d\n", msg->type);
	//	return msg;
	//}
	//return std::shared_ptr<state_replicator::MsgBase>{ new NoneMessage };
}

void state_replicator::emplace_positon_update(MovementState* state) {
	//PositionUpdateMessage msg{};
	//msg.player_id = local_playerid;
	//msg.data = *state;
	//auto serialized_msg = serialize_message(&msg);
	//if (_is_server) {
	//	for (auto conn : _wss.getClients()) {
	//		conn->sendBinary(serialized_msg);
	//	}
	//}
	//else if (is_connected) {
	//	_ws.sendBinary(serialized_msg);
	//}
}

void state_replicator::emplace_hero_update(ChangeHeroMessage msg) {
	//msg.player_id = local_playerid;
	//auto serialized_msg = serialize_message(&msg);
	//if (_is_server) {
	//	for (auto conn : _wss.getClients()) {
	//		conn->sendBinary(serialized_msg);
	//	}
	//}
	//else if (is_connected) {
	//	_ws.sendBinary(serialized_msg);
	//}
}