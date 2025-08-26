package main

import (
	// "encoding/json"
	"fmt"
	"log"
	"net/http"

	"github.com/gorilla/websocket"
)

// Upgrade HTTP to WebSocket
var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true // Allow all origins for testing
	},
}

// WebRTC signaling message types
type SignalingMessage struct {
	Type   string      `json:"type"`   // "offer", "answer", "ice-candidate", "join"
	From   string      `json:"from"`   // sender ID
	To     string      `json:"to"`     // receiver ID (empty for broadcast)
	Data   interface{} `json:"data"`   // SDP or ICE candidate data
}

// Connected client info
type Client struct {
	ID   string
	Conn *websocket.Conn
}

// Store all connected clients
var clients = make(map[string]*Client)
var clientList = make([]*Client, 0)

func main() {
	// Handle WebSocket connections for WebRTC signaling
	http.HandleFunc("/ws", handleWebRTCSignaling)
	
	// Simple web page to test
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		w.Write([]byte(`
<!DOCTYPE html>
<html>
<head><title>WebRTC Signaling Server</title></head>
<body>
	<h1>WebRTC Signaling Server</h1>
	<p>Server is running!</p>
	<p>WebRTC signaling endpoint: <code>ws://localhost:8080/ws</code></p>
	<p>Connect your libdatachannel C++ clients to this endpoint.</p>
	<div id="status">Waiting for clients...</div>
	<script>
		const ws = new WebSocket('ws://localhost:8080/ws');
		ws.onmessage = function(event) {
			document.getElementById('status').innerHTML += '<br>' + event.data;
		};
	</script>
</body>
</html>
		`))
	})
	
	fmt.Println("WebRTC Signaling Server starting on :8080")
	fmt.Println("WebRTC signaling: ws://localhost:8080/ws")
	fmt.Println("Test page: http://localhost:8080")
	
	// Start server
	log.Fatal(http.ListenAndServe(":8080", nil))
}

func handleWebRTCSignaling(w http.ResponseWriter, r *http.Request) {
	// Upgrade to WebSocket
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Printf("WebSocket upgrade failed: %v", err)
		return
	}
	defer conn.Close()
	
	var client *Client
	
	// Listen for messages from this client
	for {
		var msg SignalingMessage
		err := conn.ReadJSON(&msg)
		if err != nil {
			if client != nil {
				fmt.Printf("Client %s disconnected: %v\n", client.ID, err)
				delete(clients, client.ID)
				removeClientFromList(client)
				broadcastClientList()
			}
			break
		}
		
		fmt.Printf("Received %s from %s\n", msg.Type, msg.From)
		
		switch msg.Type {
		case "join":
			// Client wants to join
			clientID := msg.From
			if clientID == "" {
				clientID = fmt.Sprintf("client_%d", len(clients)+1)
			}
			
			client = &Client{ID: clientID, Conn: conn}
			clients[clientID] = client
			clientList = append(clientList, client)
			
			fmt.Printf("Client %s joined! Total clients: %d\n", clientID, len(clients))
			
			// Send client their ID
			response := SignalingMessage{
				Type: "joined",
				To:   clientID,
				Data: map[string]interface{}{"id": clientID},
			}
			conn.WriteJSON(response)
			
			// Broadcast updated client list
			broadcastClientList()
			
		case "connection-request":
			// Forward connection request to target client
			if msg.To != "" {
				if targetClient, exists := clients[msg.To]; exists {
					err := targetClient.Conn.WriteJSON(msg)
					if err != nil {
						fmt.Printf("Failed to send connection request to %s: %v\n", msg.To, err)
					} else {
						fmt.Printf("Forwarded connection request from %s to %s\n", msg.From, msg.To)
					}
				} else {
					fmt.Printf("Target client %s not found for connection request\n", msg.To)
				}
			}
			
		case "connection-response":
			// Forward connection response back to requesting client
			if msg.To != "" {
				if targetClient, exists := clients[msg.To]; exists {
					err := targetClient.Conn.WriteJSON(msg)
					if err != nil {
						fmt.Printf("Failed to send connection response to %s: %v\n", msg.To, err)
					} else {
						fmt.Printf("Forwarded connection response from %s to %s\n", msg.From, msg.To)
					}
				} else {
					fmt.Printf("Target client %s not found for connection response\n", msg.To)
				}
			}
			
		case "offer", "answer", "ice-candidate":
			// Forward signaling messages
			if msg.To != "" {
				// Send to specific client
				if targetClient, exists := clients[msg.To]; exists {
					err := targetClient.Conn.WriteJSON(msg)
					if err != nil {
						fmt.Printf("Failed to send %s to %s: %v\n", msg.Type, msg.To, err)
					} else {
						fmt.Printf("Forwarded %s from %s to %s\n", msg.Type, msg.From, msg.To)
					}
				} else {
					fmt.Printf("Target client %s not found\n", msg.To)
				}
			} else {
				// Broadcast to all other clients
				for id, c := range clients {
					if id != msg.From {
						err := c.Conn.WriteJSON(msg)
						if err != nil {
							fmt.Printf("Failed to broadcast to %s: %v\n", id, err)
						}
					}
				}
				fmt.Printf("Broadcasted %s from %s to all\n", msg.Type, msg.From)
			}
		}
	}
}

func removeClientFromList(client *Client) {
	for i, c := range clientList {
		if c.ID == client.ID {
			clientList = append(clientList[:i], clientList[i+1:]...)
			break
		}
	}
}

func broadcastClientList() {
	clientIDs := make([]string, len(clientList))
	for i, client := range clientList {
		clientIDs[i] = client.ID
	}
	
	message := SignalingMessage{
		Type: "client-list",
		Data: map[string]interface{}{"clients": clientIDs},
	}
	
	for _, client := range clientList {
		err := client.Conn.WriteJSON(message)
		if err != nil {
			fmt.Printf("Failed to send client list to %s: %v\n", client.ID, err)
		}
	}
	
	fmt.Printf("Broadcasted client list: %v\n", clientIDs)
}