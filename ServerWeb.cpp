#include "ServerWeb.h"
#include "LectorRFID.h"

// Variables globales (debes definir estas variables en otro archivo o en el mismo archivo principal)

// Credenciales de Wi-Fi
const char* ssid = "(((Mariola)))";
const char* password = "Resiliencia1*";
const int port = 80;

AsyncWebServer server(port);

// Configuración del servidor web
void setupServer() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado a: ");
  Serial.println(ssid);
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  // Configuración de rutas
  server.on("/", HTTP_GET, handleRoot);
  server.on("/add", HTTP_POST, handleUpload);
  server.on("/delete", HTTP_POST, handleDelete);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Servidor web iniciado");
}

void handleUpload(AsyncWebServerRequest *request) {
  if (request->hasParam("uid", true)) {
    String uidParam = request->getParam("uid", true)->value();
    Serial.println("Nueva tarjeta recibida: " + uidParam);
    byte newUID[7];
    int index = 0;
    char* token = strtok((char*)uidParam.c_str(), " ");
    while (token != NULL && index < 7) {
      newUID[index++] = strtol(token, NULL, 16);
      token = strtok(NULL, " ");
    }
    if (index > 0) {
      if (authorizedUIDCount < EEPROM_UID_MAX_COUNT) {
        for (byte i = 0; i < index; i++) {
          authorizedUIDs[authorizedUIDCount][i] = newUID[i];
        }
        authorizedUIDSizes[authorizedUIDCount] = index;
        authorizedUIDCount++;
        // Aquí deberías guardar los UIDs en EEPROM, si es necesario
        saveAuthorizedUIDs();
        request->send(200, "text/html", "<html><body><h2>Tarjeta Agregada Exitosamente</h2><a href=\"/\">Volver</a></body></html>");
      } else {
        request->send(200, "text/html", "<html><body><h2>Error: Lista de tarjetas llena</h2><a href=\"/\">Volver</a></body></html>");
      }
    } else {
      request->send(200, "text/html", "<html><body><h2>Error: UID inválido</h2><a href=\"/\">Volver</a></body></html>");
    }
  } else {
    request->send(200, "text/html", "<html><body><h2>Error: UID no recibido</h2><a href=\"/\">Volver</a></body></html>");
  }
}

void handleDelete(AsyncWebServerRequest *request) {
  if (request->hasParam("uid", true)) {
    String uidParam = request->getParam("uid", true)->value();
    Serial.println("Eliminar tarjeta recibida: " + uidParam);
    byte uidToDelete[7];
    int index = 0;
    char* token = strtok((char*)uidParam.c_str(), " ");
    while (token != NULL && index < 7) {
      uidToDelete[index++] = strtol(token, NULL, 16);
      token = strtok(NULL, " ");
    }
    if (index > 0) {
      bool found = false;
      for (int i = 0; i < authorizedUIDCount; i++) {
        if (memcmp(authorizedUIDs[i], uidToDelete, sizeof(uidToDelete)) == 0) {
          for (int j = i; j < authorizedUIDCount - 1; j++) {
            memcpy(authorizedUIDs[j], authorizedUIDs[j + 1], sizeof(authorizedUIDs[j]));
            authorizedUIDSizes[j] = authorizedUIDSizes[j + 1];
          }
          authorizedUIDCount--;
          // Aquí deberías guardar los UIDs en EEPROM, si es necesario
          saveAuthorizedUIDs();
          found = true;
          break;
        }
      }
      if (found) {
        request->send(200, "text/html", "<html><body><h2>Tarjeta Eliminada Exitosamente</h2><a href=\"/\">Volver</a></body></html>");
      } else {
        request->send(200, "text/html", "<html><body><h2>Error: Tarjeta no encontrada</h2><a href=\"/\">Volver</a></body></html>");
      }
    } else {
      request->send(200, "text/html", "<html><body><h2>Error: UID inválido</h2><a href=\"/\">Volver</a></body></html>");
    }
  } else {
    request->send(200, "text/html", "<html><body><h2>Error: UID no recibido</h2><a href=\"/\">Volver</a></body></html>");
  }
}

void handleNotFound(AsyncWebServerRequest *request) {
  String message = "Archivo no encontrado\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMétodo: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArgumentos: ";
  message += request->args();
  message += "\n";
  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
  request->send(404, "text/plain", message);
}

void handleRoot(AsyncWebServerRequest *request) {
  String html = "<html><body>";
  html += "<h1>Tarjetas RFID Autorizadas</h1>";
  html += "<ul>";
  for (int i = 0; i < authorizedUIDCount; i++) {
    html += "<li>";
    for (int j = 0; j < authorizedUIDSizes[i]; j++) {
      html += String(authorizedUIDs[i][j], HEX) + " ";
    }
    html += "</li>";
  }
  html += "</ul>";
  html += "<h2>Agregar Nueva Tarjeta</h2>";
  html += "<form action=\"/add\" method=\"POST\">";
  html += "<label for=\"uid\">UID (hex, separado por espacios):</label>";
  html += "<input type=\"text\" id=\"uid\" name=\"uid\">";
  html += "<input type=\"submit\" value=\"Agregar\">";
  html += "</form>";
  html += "<h2>Eliminar Tarjeta</h2>";
  html += "<form action=\"/delete\" method=\"POST\">";
  html += "<label for=\"uid\">UID (hex, separado por espacios):</label>";
  html += "<input type=\"text\" id=\"uid\" name=\"uid\">";
  html += "<input type=\"submit\" value=\"Eliminar\">";
  html += "</form>";
  html += "</body></html>";
}
