#include "rtc/rtc.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

// Structure to hold each peer's connection data
struct PeerConnection
{
	std::shared_ptr<rtc::PeerConnection> pc;
	std::shared_ptr<rtc::DataChannel> data_channel;
	bool connected = false;
	bool is_initiator = false; // true if we initiated the connection
	bool negotiation_in_progress = false; // prevent simultaneous negotiations
};

// Simple WebSocket client using libdatachannel's built-in WebSocket
class WebRTCClient
{
private:
	std::shared_ptr<rtc::WebSocket> signaling_ws;
	std::string client_id;
	std::vector<std::string> message_history;
	std::vector<std::string> connected_clients;
	
	// Map of peer_id -> PeerConnection
	std::unordered_map<std::string, PeerConnection> peer_connections;
	

public:
	WebRTCClient(const std::string &id);

	void setupPeerConnection(const std::string& peer_id);

	void setupDataChannel(const std::string& peer_id, std::shared_ptr<rtc::DataChannel> channel);

	bool connectToSignalingServer(const std::string &url);

	void handleSignalingMessage(const std::string &message);
	void createOffer(const std::string& peer_id);
	void sendMessage(const std::string &msg, const std::string& peer_id = ""); // empty = broadcast to all
	const std::vector<std::string>& getMessageHistory() const;
	const std::vector<std::string>& getConnectedClients() const;
	std::vector<std::string> getConnectedPeerIds() const;
	bool isConnectedToPeer(const std::string& peer_id) const;
	
	// Connection request methods
	void sendConnectionRequest(const std::string& targetClientId);
	void sendConnectionResponse(const std::string& targetClientId, bool accepted);
	void disconnectFromPeer(const std::string& peer_id);
	
	// Callback for connection requests - to be set by App
	std::function<void(const std::string& fromClientId, const std::string& fromClientName)> onConnectionRequest;
	std::function<void(const std::string& fromClientId, bool accepted)> onConnectionResponse;
};