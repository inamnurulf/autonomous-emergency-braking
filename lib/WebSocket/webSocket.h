#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <ArduinoWebsockets.h>
#include <WiFi.h>

using namespace websockets;
extern int distanceBack;

class WebSocket {
public:
    WebSocket(const char* ssid, const char* password, const char* serverAddress, int port);
    void begin();
    void loop();
    void send(const String &message);

private:
    const char* _ssid;
    const char* _password;
    const char* _serverAddress;
    int _port;
    bool _isConnected;

    WebsocketsClient client;

    void connectToWebSocket();
    void onMessage(WebsocketsMessage message);
    void onEvent(WebsocketsEvent event, String data);
};

#endif
