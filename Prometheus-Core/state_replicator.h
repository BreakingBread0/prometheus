#pragma once
#include <nlohmann/json.hpp>
#include "entity_admin.h"
#include <mutex>
#include "MovementState.h"
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketMessage.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <map>

class state_replicator {
public:
	typedef std::string playerid;
	class MsgBase {
	public:
		enum class MessageType {
			None = 0,
			ChangeHero,
			PositionUpdate,
			JoinInstance,
			LeaveInstance,
			AssignPlayerID
		} type;

	protected:
		MsgBase(MessageType typ) : type(typ) {}
		virtual void please_be_polymorphic() {}
	};

	class NoneMessage : public MsgBase {
	public:
		NoneMessage() : MsgBase(MessageType::None) {}
	};

	class ChangeHeroMessage : public MsgBase {
	public:
		ChangeHeroMessage() : MsgBase(MessageType::ChangeHero) {}

		playerid player_id;
		__int64 heroid;
		__int64 skinid;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(ChangeHeroMessage, type, player_id, heroid, skinid);
	};

	class PositionUpdateMessage : public MsgBase {
	public:
		PositionUpdateMessage() : MsgBase(MessageType::PositionUpdate) {}

		playerid player_id;
		nlohmann::json data; //serialized MovementState

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(PositionUpdateMessage, type, player_id, data);
	};

	class JoinInstanceMessage : public MsgBase {
	public:
		JoinInstanceMessage() : MsgBase(MessageType::JoinInstance) {}

		playerid player_id;
		std::string name;
		
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(JoinInstanceMessage, type, player_id, name);
	};

	class LeaveInstanceMessage : public MsgBase {
	public:
		LeaveInstanceMessage() : MsgBase(MessageType::LeaveInstance) {}

		playerid player_id;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(LeaveInstanceMessage, type, player_id);
	};

	class AssignPlayerIDMessage : public MsgBase {
	public:
		AssignPlayerIDMessage() : MsgBase(MessageType::AssignPlayerID) {}

		playerid player_id;

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(AssignPlayerIDMessage, type, player_id);
	};

	static inline bool is_connected = false;
	static inline playerid local_playerid{};

	static void start_server();
	static void connect(std::string ip_addr);
	static std::shared_ptr<MsgBase> get_outstanding_message(); //returns NoneMessage if empty
	static void emplace_hero_update(ChangeHeroMessage msg);
	static void emplace_positon_update(MovementState* state);
private:
	static void network_init();
	static inline bool _is_server = false;
	static inline ix::WebSocket _ws{};
	static inline ix::WebSocketServer _wss{6969, "0.0.0.0"}; //secure? never heard of 'er!
	static inline std::once_flag _network_init{};
	static void ws_client_callback(const ix::WebSocketMessagePtr&);
	static void ws_server_callback_conn(std::weak_ptr<ix::WebSocket>, std::shared_ptr<ix::ConnectionState>);
	static void ws_server_callback(std::shared_ptr<ix::ConnectionState>, ix::WebSocket&, const ix::WebSocketMessagePtr&);
	static void push_message(std::shared_ptr<MsgBase> msg);
	static std::vector<std::uint8_t> serialize_message(MsgBase* msg);
	static std::shared_ptr<MsgBase> deserialize_message(nlohmann::json input);

	static inline std::mutex _outstanding_messages_mut{};
	static inline std::mutex _local_position_updates_mut{};
	static inline std::vector<std::shared_ptr<MsgBase>> _outstanding_messages{}; //OUT: Updates from other clients / servers... At this point we dont care if its a server or client really
	//static inline std::vector<nlohmann::json> _local_position_updates{}; //IN: Local Position updates to be sent
	static inline ChangeHeroMessage _local_hero{};
	static inline JoinInstanceMessage _server_join{};
	static inline std::map<playerid, std::string> _server_connected_clients; //value: change hero message
};