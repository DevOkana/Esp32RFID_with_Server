#include "LectorRFID.h"

MFRC522 rfid(SS_PIN, RST_PIN);  // Crear instancia del lector RFID

// UID específico para borrar la EEPROM


// Inicializar variables
byte authorizedUIDs[10][7];
byte authorizedUIDSizes[10];
int authorizedUIDCount = 0; // Número inicial de UIDs autorizados
byte whiteCardUID[4] = {0xE3, 0x6A, 0x88, 0x29};
byte eraseEEPROMUID[4] = {0x77, 0x3B, 0xDF, 0x00};


// Funcion para iniciar el lector RFID
void startRFID() {
    SPI.begin();       // Iniciar SPI bus
    // Inicializar EEPROM
    EEPROM.begin(EEPROM_SIZE);

    // Cargar UIDs autorizados desde EEPROM
    loadAuthorizedUIDs();

    // Diagnóstico inicial del módulo RFID
    if (!diagnoseRFIDModule()) {
        Serial.println("RFID module initialization failed. Check connections and try again.");
        while (true); // Detener ejecución
    }

    rfid.PCD_Init();   // Iniciar el lector RC522
    Serial.println("Sistema de acceso RFID iniciado");
}

// Función para verificar si el UID coincide con un UID dado
bool isUIDMatching(byte *uid, byte uidSize, byte *targetUID, byte targetUIDSize) {
  if (uidSize != targetUIDSize) {
    return false;
  }
  for (byte i = 0; i < uidSize; i++) {
    if (uid[i] != targetUID[i]) {
      return false;
    }
  }
  return true;
}

// Función para imprimir el array de UIDs en hexadecimal
void printArray(byte arr[10][7], int count) {
  for (int i = 0; i < count; i++) {
    for (int j = 0; j < 7; j++) {
      Serial.print(arr[i][j], HEX); // Print each byte in hexadecimal format
      Serial.print(" "); // Space between bytes
    }
    Serial.println(); // New line after each row
  }
}

//Funcion para verificar si el uid esta autorizado
bool isAuthorized(byte *uid, byte uidSize) {
  for (int i = 0; i < authorizedUIDCount; i++) {
    if (uidSize == authorizedUIDSizes[i]) {
      bool match = true;
      for (byte j = 0; j < uidSize; j++) {
        if (uid[j] != authorizedUIDs[i][j]) {
          match = false;
          break;
        }
      }
      if (match) {
        return true;
      }
    }
  }
  return false;
}

// Funcion para comprobar si es una tarjeta blanca
bool isWhiteCard(byte *uid, byte uidSize) {
  if (uidSize != 4) { // Tamaño de la tarjeta blanca
    return false;
  }
  
  for (byte i = 0; i < uidSize; i++) {
    if (uid[i] != whiteCardUID[i]) {
      return false;
    }
  }
  return true;
}
// Funcion para agregar un nuevo UID
void addNewAuthorizedUID() {
  while (true) {
    // Revisar si una nueva tarjeta está presente
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      // Obtener el UID de la nueva tarjeta
      if (!isWhiteCard(rfid.uid.uidByte, rfid.uid.size)) { // Asegurarse de no agregar la tarjeta blanca
        // Verificar si el UID ya está en la lista de autorizados
        if (isAuthorized(rfid.uid.uidByte, rfid.uid.size)) {
          Serial.println("Esta tarjeta ya está autorizada.");
          rfid.PICC_HaltA();
          rfid.PCD_StopCrypto1();
          return;
        }

        // Agregar el nuevo UID al array
        if (authorizedUIDCount < EEPROM_UID_MAX_COUNT) {
          byte index = authorizedUIDCount;
          for (byte i = 0; i < rfid.uid.size; i++) {
            authorizedUIDs[index][i] = rfid.uid.uidByte[i];
          }
          authorizedUIDSizes[index] = rfid.uid.size;
          authorizedUIDCount++;

          // Guardar el array actualizado en EEPROM
          saveAuthorizedUIDs();

          Serial.println("Nueva tarjeta autorizada correctamente.");
          printArray(authorizedUIDs, authorizedUIDCount);
        } else {
          Serial.println("No se puede agregar la tarjeta. La lista de tarjetas autorizadas está llena.");
        }
        // Salir del modo de espera
        break;
      } else {
        delay(1000);
        Serial.println("Tarjeta blanca detectada de nuevo. Por favor, inserte una tarjeta diferente.");
        Serial.println("Espere unos 4 segundos para insertar la tarjeta nueva");
        delay(4000);
      }
    }
  } 
}

// Función para borrar toda la EEPROM
void clearEEPROM() {
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0xFF); // Escribir el valor 0xFF en cada byte
  }
  EEPROM.commit(); // Asegurarse de que los cambios se guarden
  ESP.restart(); //reiniciar el módulo
}

// Función para cargar los UIDs autorizados desde la EEPROM
void loadAuthorizedUIDs() {
  int count = 0;
  for (int i = 0; i < EEPROM_UID_MAX_COUNT; i++) {
    byte index = i * EEPROM_UID_SIZE;
    bool uidFound = false;
    for (byte j = 0; j < EEPROM_UID_SIZE; j++) {
      byte uidByte = EEPROM.read(EEPROM_UID_START + index + j);
      if (uidByte != 0xFF) {
        authorizedUIDs[i][j] = uidByte;
        uidFound = true;
      } else {
        uidFound = false;
        break;
      }
    }
    if (uidFound) {
      authorizedUIDSizes[i] = EEPROM.read(EEPROM_UID_START + index + EEPROM_UID_SIZE);
      count++;
    } else {
      break;
    }
  }
  authorizedUIDCount = count;
}
// Función para guardar los UIDs autorizados en la EEPROM
void saveAuthorizedUIDs() {
  for (int i = 0; i < authorizedUIDCount; i++) {
    byte index = i * EEPROM_UID_SIZE;
    for (byte j = 0; j < EEPROM_UID_SIZE; j++) {
      EEPROM.write(EEPROM_UID_START + index + j, authorizedUIDs[i][j]);
    }
    EEPROM.write(EEPROM_UID_START + index + EEPROM_UID_SIZE, authorizedUIDSizes[i]);
  }
  EEPROM.commit();
}

// Función para verificar si el módulo RFID está funcionando
bool diagnoseRFIDModule() {
  rfid.PCD_Init();
  byte version = rfid.PCD_ReadRegister(MFRC522::VersionReg);
  if (version == 0x00 || version == 0xFF) {
    Serial.println("RFID module not detected.");
    return false;
  } else {
    Serial.print("RFID module detected. Version: 0x");
    Serial.println(version, HEX);
    return true;
  }
}