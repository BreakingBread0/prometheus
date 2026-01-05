#include "state_replicator.h"
#include <ixwebsocket/IXNetSystem.h>
#include "serialization.h"

void state_replicator::network_init() {
	std::call_once(_network_init, []() {
		printf("init net system\n");
		ix::initNetSystem(); //Who needs uninitialization? lol
	});
}

void state_replicator::ws_client_callback(const ix::WebSocketMessagePtr& ws_msg) {
	if (ws_msg->type == ix::WebSocketMessageType::Open) {
		is_connected = true;
	}
	else if (ws_msg->type == ix::WebSocketMessageType::Message) {
		owassert(ws_msg->binary);
		nlohmann::json json = nlohmann::json::from_bson(ws_msg->str.data(), ws_msg->str.data() + ws_msg->str.size());
		auto msg = deserialize_message(json);
		if (msg->type == MsgBase::MessageType::None) {
			return; //invalid
		}
		if (msg->type == MsgBase::MessageType::AssignPlayerID) {
			local_playerid = dynamic_cast<AssignPlayerIDMessage*>(msg.get())->player_id;
			JoinInstanceMessage msg{};
			msg.player_id = local_playerid;
			msg.name = "Remote Player";
			_ws.sendBinary(serialize_message(&msg));
			if (_local_hero.heroid != 0) {
				emplace_hero_update(_local_hero);
			}
			return;
		}
		push_message(msg);
	}
	else if (ws_msg->type == ix::WebSocketMessageType::Close) {
		is_connected = false;
	}
}

void state_replicator::ws_server_callback_conn(std::weak_ptr<ix::WebSocket>, std::shared_ptr<ix::ConnectionState>) {
	//such empty, much wow
}

void state_replicator::ws_server_callback(std::shared_ptr<ix::ConnectionState> state, ix::WebSocket& ws, const ix::WebSocketMessagePtr& ws_msg) {
	if (ws_msg->type == ix::WebSocketMessageType::Open) {
		AssignPlayerIDMessage msg{};
		msg.player_id = state->getId();
		ws.sendBinary(serialize_message(&msg));
		if (_local_hero.heroid != 0) {
			emplace_hero_update(_local_hero);
		}
	}
	else if (ws_msg->type == ix::WebSocketMessageType::Message) {
		owassert(ws_msg->binary);
		nlohmann::json json = nlohmann::json::from_bson(ws_msg->str.data(), ws_msg->str.data() + ws_msg->str.size());
		auto msg = deserialize_message(json);
		if (msg->type == MsgBase::MessageType::None || msg->type == MsgBase::MessageType::AssignPlayerID) {
			return; //invalid
		}
		if (msg->type == MsgBase::MessageType::ChangeHero) {
			for (auto existing_client : _server_connected_clients) {
				ws.sendBinary(existing_client.second);
			}
			_server_connected_clients.emplace(dynamic_cast<ChangeHeroMessage*>(msg.get())->player_id, ws_msg->str);
		}
		for (auto conn : _wss.getClients()) {
			conn->sendBinary(ws_msg->str);
		}
		push_message(msg);
	}
	else if (ws_msg->type == ix::WebSocketMessageType::Close) {
		LeaveInstanceMessage msg{};
		msg.player_id = state->getId();
		_server_connected_clients.erase(state->getId());
		auto serialized_msg = serialize_message(&msg);
		for (auto conn : _wss.getClients()) {
			if (conn.get() != &ws) { //this is stupid
				conn->sendBinary(serialized_msg);
			}
		}
	}
}

void state_replicator::push_message(std::shared_ptr<MsgBase> msg) {
	std::unique_lock lock(_outstanding_messages_mut);
	_outstanding_messages.push_back(msg);
}

std::vector<std::uint8_t> state_replicator::serialize_message(MsgBase* msg) {
	nlohmann::json json;
	switch (msg->type) {
	case MsgBase::MessageType::None:
		owassert(false);
		break;
	case MsgBase::MessageType::ChangeHero:
		json = *dynamic_cast<ChangeHeroMessage*>(msg);
		break;
	case MsgBase::MessageType::JoinInstance:
		json = *dynamic_cast<JoinInstanceMessage*>(msg);
		break;
	case MsgBase::MessageType::LeaveInstance:
		json = *dynamic_cast<LeaveInstanceMessage*>(msg);
		break;
	case MsgBase::MessageType::PositionUpdate:
		json = *dynamic_cast<PositionUpdateMessage*>(msg);
		break;
	case MsgBase::MessageType::AssignPlayerID:
		json = *dynamic_cast<AssignPlayerIDMessage*>(msg);
		break;
	}
	return nlohmann::json::to_bson(json);
}

std::shared_ptr<state_replicator::MsgBase> state_replicator::deserialize_message(nlohmann::json input) {
	std::shared_ptr<MsgBase> result;
	try {
		auto type = input["type"].get<MsgBase::MessageType>();
		switch (type) {
		case MsgBase::MessageType::None:
			owassert(false);
			break;
		case MsgBase::MessageType::ChangeHero:
			result = std::shared_ptr<MsgBase>(new ChangeHeroMessage(input.get<ChangeHeroMessage>()));
			break;
		case MsgBase::MessageType::JoinInstance:
			result = std::shared_ptr<MsgBase>(new JoinInstanceMessage(input.get<JoinInstanceMessage>()));
			break;
		case MsgBase::MessageType::LeaveInstance:
			result = std::shared_ptr<MsgBase>(new LeaveInstanceMessage(input.get<LeaveInstanceMessage>()));
			break;
		case MsgBase::MessageType::PositionUpdate:
			result = std::shared_ptr<MsgBase>(new PositionUpdateMessage(input.get<PositionUpdateMessage>()));
			break;
		case MsgBase::MessageType::AssignPlayerID:
			result = std::shared_ptr<MsgBase>(new AssignPlayerIDMessage(input.get<AssignPlayerIDMessage>()));
			break;
		}
	}
	catch (nlohmann::json::exception& ex) {
		printf("state_replicator: failed to deserialize message with error: %s\nmessage: %s\n", ex.what(), input.dump().c_str());
	}
	return result;
}

/*
using OnConnectionCallback =
			std::function<void(std::weak_ptr<WebSocket>, std::shared_ptr<ConnectionState>)>;

		using OnClientMessageCallback = std::function<void(
			std::shared_ptr<ConnectionState>, WebSocket&, const WebSocketMessagePtr&)>;
*/

void __declspec(noinline) state_replicator::start_server() {
	network_init();

	//_wss.setOnConnectionCallback(ws_server_callback_conn); //guess which error log gets written to stderr instead of stdout and which of those i havent redirected
	_wss.setOnClientMessageCallback(ws_server_callback);
	_wss.enablePong();
	auto result = _wss.listen();
	_wss.start();
	std::thread([](){
		_wss.wait();
	}).detach();
	
	_is_server = true;
	if (!(is_connected = result.first))
		printf("Failed to make server: %s\n", result.second.c_str());
	local_playerid = "Server";
}

void state_replicator::connect(std::string ip_addr) {
	network_init();

	_ws.setUrl("ws://" + ip_addr + ":6969/");
	_ws.setPingInterval(1);
	_ws.enablePerMessageDeflate();
	_ws.setOnMessageCallback(ws_client_callback);
	auto result = _ws.connect(5);
	if (!result.success)
		printf("Failed to connect: %s\n", result.errorStr.c_str());
	is_connected = result.success;
	_is_server = false;
}

std::shared_ptr<state_replicator::MsgBase> state_replicator::get_outstanding_message() {
	if (!_is_server)
		is_connected = _ws.getReadyState() == ix::ReadyState::Open;
	std::unique_lock lock(_outstanding_messages_mut);
	if (_outstanding_messages.size() > 0) {
		auto msg = *_outstanding_messages.begin();
		_outstanding_messages.erase(_outstanding_messages.begin());
		//printf("outstanding msg: %d\n", msg->type);
		return msg;
	}
	return std::shared_ptr<state_replicator::MsgBase>{ new NoneMessage };
}

void state_replicator::emplace_positon_update(MovementState* state) {
	PositionUpdateMessage msg{};
	msg.player_id = local_playerid;
	msg.data = *state;
	auto serialized_msg = serialize_message(&msg);
	if (_is_server) {
		for (auto conn : _wss.getClients()) {
			conn->sendBinary(serialized_msg);
		}
	}
	else if (is_connected) {
		_ws.sendBinary(serialized_msg);
	}
}

void state_replicator::emplace_hero_update(ChangeHeroMessage msg) {
	msg.player_id = local_playerid;
	auto serialized_msg = serialize_message(&msg);
	if (_is_server) {
		for (auto conn : _wss.getClients()) {
			conn->sendBinary(serialized_msg);
		}
	}
	else if (is_connected) {
		_ws.sendBinary(serialized_msg);
	}
}