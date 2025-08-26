#include "Client.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

WebRTCClient::WebRTCClient(const std::string &id) : client_id(id)
{
	// Constructor now just stores the client ID
	// Peer connections will be created on-demand
}

void WebRTCClient::setupPeerConnection(const std::string &peer_id)
{
	// Create new peer connection
	rtc::Configuration config;
	config.iceServers.emplace_back("stun:stun.l.google.com:19302");

	PeerConnection &peer = peer_connections[peer_id];
	peer.pc = std::make_shared<rtc::PeerConnection>(config);

	// Handle connection state changes
	peer.pc->onStateChange([this, peer_id](rtc::PeerConnection::State state)
						   {
			// FLOW STEP 11: Monitor WebRTC connection state
			std::cout << "Connection to " << peer_id << " state: ";
			switch (state) {
				case rtc::PeerConnection::State::New:
					std::cout << "New" << std::endl;
					break;
				case rtc::PeerConnection::State::Connecting:
					std::cout << "Connecting..." << std::endl;
					break;
				case rtc::PeerConnection::State::Connected:
					// FLOW SUCCESS: Direct peer-to-peer connection established!
					std::cout << "Connected!" << std::endl;
					peer_connections[peer_id].connected = true;
					peer_connections[peer_id].negotiation_in_progress = false;
					// Now we can send messages directly without signaling server
					break;
				case rtc::PeerConnection::State::Disconnected:
					std::cout << "Disconnected!" << std::endl;
					peer_connections[peer_id].connected = false;
					break;
				case rtc::PeerConnection::State::Failed:
					std::cout << "Failed!" << std::endl;
					peer_connections[peer_id].connected = false;
					break;
				case rtc::PeerConnection::State::Closed:
					std::cout << "Closed!" << std::endl;
					peer_connections.erase(peer_id);
					break;
			} });

	// Add ICE connection state monitoring
	peer.pc->onGatheringStateChange([peer_id](rtc::PeerConnection::GatheringState state)
									{
			std::cout << "ICE gathering for " << peer_id << ": ";
			switch (state) {
				case rtc::PeerConnection::GatheringState::New:
					std::cout << "New" << std::endl;
					break;
				case rtc::PeerConnection::GatheringState::InProgress:
					std::cout << "In Progress" << std::endl;
					break;
				case rtc::PeerConnection::GatheringState::Complete:
					std::cout << "Complete" << std::endl;
					break;
			} });

	// Handle local description (offer/answer)
	peer.pc->onLocalDescription([this, peer_id](rtc::Description desc)
								{
			// FLOW STEP 6: WebRTC generates SDP offer/answer - send it via signaling server
			std::cout << "Sending " << desc.typeString() << " to " << peer_id << std::endl;
			
			// Send the SDP (offer or answer) to the other peer via websocket
			json message = {
				{"type", desc.typeString()}, // "offer" or "answer"
				{"from", client_id},
				{"to", peer_id},
				{"data", std::string(desc)}  // The actual SDP data
			};
			
			if (signaling_ws) {
				signaling_ws->send(message.dump());
			} });

	// Handle local ICE candidates
	peer.pc->onLocalCandidate([this, peer_id](rtc::Candidate candidate)
							  {
			std::cout << "Sending ICE candidate to " << peer_id << std::endl;
			
			json message = {
				{"type", "ice-candidate"},
				{"from", client_id},
				{"to", peer_id},
				{"data", std::string(candidate)}
			};
			
			if (signaling_ws) {
				signaling_ws->send(message.dump());
			} });

	// Handle incoming data channels
	peer.pc->onDataChannel([this, peer_id](std::shared_ptr<rtc::DataChannel> channel)
						   {
			std::cout << "Received data channel from " << peer_id << ": " << channel->label() << std::endl;
			setupDataChannel(peer_id, channel); });
}

void WebRTCClient::setupDataChannel(const std::string &peer_id, std::shared_ptr<rtc::DataChannel> channel)
{
	auto &peer = peer_connections[peer_id];
	peer.data_channel = channel;

	channel->onOpen([this, peer_id]()
					{ std::cout << "Data channel to " << peer_id << " opened! You can now chat!" << std::endl; });

	channel->onMessage([this, peer_id](rtc::message_variant message)
					   {
			if (std::holds_alternative<std::string>(message)) {
				std::string msg = std::get<std::string>(message);
				std::string formatted_msg = "[" + peer_id + "] " + msg;
				message_history.push_back(formatted_msg);
				std::cout << "received from " << peer_id << ": " << msg << std::endl;
			} });

	channel->onClosed([this, peer_id]()
					  { 
			std::cout << "Data channel to " << peer_id << " closed" << std::endl; 
			if (peer_connections.find(peer_id) != peer_connections.end()) {
				peer_connections[peer_id].connected = false;
			} });
}

bool WebRTCClient::connectToSignalingServer(const std::string &url)
{
	try
	{
		signaling_ws = std::make_shared<rtc::WebSocket>();

		signaling_ws->onOpen([this]()
							 {
                std::cout << "Connected to signaling server" << std::endl;
                
                // Join the server
                json join_message = {
                    {"type", "join"},
                    {"from", client_id}
                };
                signaling_ws->send(join_message.dump()); });

		signaling_ws->onMessage([this](rtc::message_variant message)
								{
                if (std::holds_alternative<std::string>(message)) {
                    handleSignalingMessage(std::get<std::string>(message));
                } });

		signaling_ws->onClosed([this]()
							   { std::cout << "Signaling server connection closed" << std::endl; });

		signaling_ws->onError([this](std::string error)
							  { std::cout << "Signaling error: " << error << std::endl; });

		signaling_ws->open(url);

		// Wait for connection
		std::this_thread::sleep_for(std::chrono::seconds(1));
		return true;
	}
	catch (const std::exception &e)
	{
		std::cout << "Failed to connect to signaling server: " << e.what() << std::endl;
		return false;
	}
}

void WebRTCClient::handleSignalingMessage(const std::string &message)
{
	std::cout << "Signaling message received: " << message << std::endl;

	try
	{
		json msg = json::parse(message);
		std::string type = msg["type"];

		if (type == "offer")
		{
			// FLOW STEP 7: Receive WebRTC offer from the requester
			std::string from_peer_id = msg["from"];
			std::string sdp = msg["data"];

			std::cout << "Received offer from " << from_peer_id << ", creating answer..." << std::endl;

			// Check if we already have a connection in progress or established
			auto it = peer_connections.find(from_peer_id);
			if (it != peer_connections.end() && (it->second.connected || it->second.negotiation_in_progress))
			{
				std::cout << "Ignoring offer from " << from_peer_id << " - connection already exists or in progress" << std::endl;
				return;
			}

			// Set up peer connection for this peer if not exists
			if (it == peer_connections.end())
			{
				setupPeerConnection(from_peer_id); // Sets up callbacks
			}

			auto &peer = peer_connections[from_peer_id];
			peer.negotiation_in_progress = true;

			// FLOW STEP 8: Set their offer as remote description, create our answer
			peer.pc->setRemoteDescription(rtc::Description(sdp, "offer"));
			peer.pc->setLocalDescription(); // Triggers onLocalDescription with "answer"
		}
		else if (type == "answer")
		{
			// FLOW STEP 9: Original requester receives the answer
			std::string from_peer_id = msg["from"];
			std::string sdp = msg["data"];

			std::cout << "Received answer from " << from_peer_id << std::endl;

			auto it = peer_connections.find(from_peer_id);
			if (it != peer_connections.end())
			{
				// Set their answer as remote description - now both sides have SDP
				it->second.pc->setRemoteDescription(rtc::Description(sdp, "answer"));
				// WebRTC will now start ICE candidate exchange automatically
			}
		}
		else if (type == "ice-candidate")
		{
			// FLOW STEP 10: Exchange ICE candidates (happens multiple times)
			std::string from_peer_id = msg["from"];
			std::string candidate_str = msg["data"];

			std::cout << "Received ICE candidate from " << from_peer_id << std::endl;

			auto it = peer_connections.find(from_peer_id);
			if (it != peer_connections.end())
			{
				try
				{
					// Add their network path info so we can connect directly
					it->second.pc->addRemoteCandidate(rtc::Candidate(candidate_str));
				}
				catch (const std::exception &e)
				{
					std::cout << "Failed to add ICE candidate from " << from_peer_id << ": " << e.what() << std::endl;
				}
			}
		}
		else if (type == "client-list")
		{
			std::cout << "Updated client list received" << std::endl;

			connected_clients.clear();

			if (msg["data"].contains("clients") && msg["data"]["clients"].is_array())
			{
				for (const auto &client : msg["data"]["clients"])
				{
					std::string client_str = client;
					if (!client_str.empty() && client_str != client_id)
					{ // Don't include self
						connected_clients.push_back(client_str);
					}
				}
			}

			std::cout << "Active clients: " << connected_clients.size() << std::endl;
		}
		else if (type == "connection-request")
		{
			// FLOW STEP 2 (Receiving): Someone wants to connect to us
			std::cout << "Received connection request" << std::endl;

			std::string from_client_id = msg["from"];

			// Trigger the callback that shows the "Accept/Reject" popup in App.cpp
			if (onConnectionRequest)
			{
				onConnectionRequest(from_client_id, from_client_id);
			}
		}
		else if (type == "connection-response")
		{
			// FLOW STEP 4: We get response to our connection request
			std::string from_client_id = msg["from"];
			bool accepted = msg["data"]["accepted"];

			if (accepted)
			{
				std::cout << "Connection accepted by " << from_client_id << std::endl;

				// FLOW STEP 5: Since WE made the request, WE create the WebRTC offer
				// This starts the actual peer-to-peer connection process
				std::cout << "We initiated the request, so we create the offer to " << from_client_id << std::endl;
				createOffer(from_client_id); // Creates data channel + WebRTC offer
			}
			else
			{
				std::cout << "Connection rejected by " << from_client_id << std::endl;
			}

			if (onConnectionResponse)
			{
				onConnectionResponse(from_client_id, accepted);
			}
		}
	}
	catch (const json::exception &e)
	{
		std::cout << "JSON parsing error: " << e.what() << std::endl;
	}
}

void WebRTCClient::createOffer(const std::string &peer_id)
{
	// FLOW STEP 5: Create WebRTC offer (called by the requester)
	std::cout << "Creating offer for " << peer_id << "..." << std::endl;

	// Set up peer connection if not exists
	if (peer_connections.find(peer_id) == peer_connections.end())
	{
		setupPeerConnection(peer_id); // Sets up WebRTC peer connection + callbacks
	}

	auto &peer = peer_connections[peer_id];
	peer.is_initiator = true;
	peer.negotiation_in_progress = true;

	// Create data channel for chat messages (this will trigger offer creation)
	peer.data_channel = peer.pc->createDataChannel("chat");
	setupDataChannel(peer_id, peer.data_channel);

	// FLOW STEP 6: Generate SDP offer - this triggers onLocalDescription callback
	peer.pc->setLocalDescription(); // Async - callback sends offer to other peer
}

void WebRTCClient::sendMessage(const std::string &msg, const std::string &peer_id)
{
	std::string full_message = client_id + ": " + msg;

	if (peer_id.empty())
	{
		// Broadcast to all connected peers
		int sent_count = 0;
		for (auto &[id, peer] : peer_connections)
		{
			if (peer.connected && peer.data_channel)
			{
				peer.data_channel->send(full_message);
				sent_count++;
			}
		}
		if (sent_count > 0)
		{
			message_history.push_back("[You] " + msg);
			std::cout << "Broadcast sent to " << sent_count << " peers: " << msg << std::endl;
		}
		else
		{
			std::cout << "No connected peers to send message to!" << std::endl;
		}
	}
	else
	{
		// Send to specific peer
		auto it = peer_connections.find(peer_id);
		if (it != peer_connections.end() && it->second.connected && it->second.data_channel)
		{
			it->second.data_channel->send(full_message);
			message_history.push_back("[You -> " + peer_id + "] " + msg);
			std::cout << "Sent to " << peer_id << ": " << msg << std::endl;
		}
		else
		{
			std::cout << "Not connected to " << peer_id << "!" << std::endl;
		}
	}
}

const std::vector<std::string> &WebRTCClient::getMessageHistory() const
{
	return message_history;
}

const std::vector<std::string> &WebRTCClient::getConnectedClients() const
{
	return connected_clients;
}

std::vector<std::string> WebRTCClient::getConnectedPeerIds() const
{
	std::vector<std::string> connected_peers;
	for (const auto &[peer_id, peer] : peer_connections)
	{
		if (peer.connected)
		{
			connected_peers.push_back(peer_id);
		}
	}
	return connected_peers;
}

bool WebRTCClient::isConnectedToPeer(const std::string &peer_id) const
{
	auto it = peer_connections.find(peer_id);
	return it != peer_connections.end() && it->second.connected;
}

void WebRTCClient::sendConnectionRequest(const std::string &targetClientId)
{
	if (signaling_ws)
	{
		json request_message = {
			{"type", "connection-request"},
			{"from", client_id},
			{"to", targetClientId},
			{"data", json::object()}};
		signaling_ws->send(request_message.dump());
		std::cout << "Sent connection request to " << targetClientId << std::endl;
	}
}

void WebRTCClient::sendConnectionResponse(const std::string &targetClientId, bool accepted)
{
	if (signaling_ws)
	{
		json response_message = {
			{"type", "connection-response"},
			{"from", client_id},
			{"to", targetClientId},
			{"data", {{"accepted", accepted}}}};
		signaling_ws->send(response_message.dump());
		std::cout << "Sent connection " << (accepted ? "acceptance" : "rejection") << " to " << targetClientId << std::endl;
	}
}

void WebRTCClient::disconnectFromPeer(const std::string &peer_id)
{
	auto it = peer_connections.find(peer_id);
	if (it != peer_connections.end())
	{
		std::cout << "Disconnecting from " << peer_id << std::endl;

		// Close data channel first
		if (it->second.data_channel)
		{
			it->second.data_channel->close();
		}

		// Close peer connection
		if (it->second.pc)
		{
			it->second.pc->close();
		}

		// Remove from our connections map
		peer_connections.erase(it);

		std::cout << "Disconnected from " << peer_id << std::endl;
	}
}
