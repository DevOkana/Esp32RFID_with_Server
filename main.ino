#include "LectorRFID.h"
#include "ServerWeb.h"


void setup() {
  Serial.begin(115200);
  startRFID();
  setupServer();
}

void loop() {
  // Revisar si una nueva tarjeta está presente
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }
  // Mostrar el UID de la tarjeta detectada en hexadecimal y decimal
  //Serial.println("A new card has been detected.");
  //Serial.println("The NUID tag is:");
  
  Serial.print("In hex: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Depurar el UID en decimal
  /*Serial.print("In dec: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i]);
    if (i < rfid.uid.size - 1) {
      Serial.print(" ");
    }
  }
  Serial.println();*/

  // Obtener y mostrar el tipo de PICC (tarjeta)
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.print("PICC type: ");
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Verificar si la tarjeta es del tipo MIFARE Classic
  if (piccType == MFRC522::PICC_TYPE_MIFARE_MINI || 
      piccType == MFRC522::PICC_TYPE_MIFARE_1K || 
      piccType == MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println("Tu tarjeta es de tipo MIFARE Classic.");
  } else {
    Serial.println("Tu tarjeta no es de tipo MIFARE Classic.");
  }

  // Verificar si el UID de la tarjeta leída coincide con el UID para borrar EEPROM
  if (isUIDMatching(rfid.uid.uidByte, rfid.uid.size, eraseEEPROMUID, sizeof(eraseEEPROMUID))) {
    Serial.println("UID de borrado detectado. Limpiando EEPROM...");
    clearEEPROM();
    Serial.println("EEPROM limpiada.");
  } 
  // Verificar si el UID de la tarjeta leída coincide con la tarjeta blanca para agregar un nuevo UID
  else if (isWhiteCard(rfid.uid.uidByte, rfid.uid.size)) {
    Serial.println("Tarjeta blanca detectada. Por favor, inserte la tarjeta para agregar.");
    addNewAuthorizedUID();
  } else {
    // Verificar si el UID de la tarjeta leída coincide con alguno de los UIDs autorizados
    if (isAuthorized(rfid.uid.uidByte, rfid.uid.size)) {
      Serial.println("Acceso concedido.");
      // Aquí puedes añadir cualquier acción a realizar cuando la tarjeta esté autorizada
    } else {
      Serial.println("Acceso denegado.");
    }
  }
  // Poner en espera a la tarjeta
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}




