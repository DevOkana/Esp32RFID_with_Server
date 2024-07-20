#ifndef SERVER_WEB_H
#define SERVER_WEB_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Variables externas
extern const char* ssid;
extern const char* password;
extern const int port;

// Funciones del servidor
void setupServer();
void handleRoot(AsyncWebServerRequest *request);
void handleNotFound(AsyncWebServerRequest *request);
void handleUpload(AsyncWebServerRequest *request);
void handleDelete(AsyncWebServerRequest *request);

#endif // SERVER_WEB_H
