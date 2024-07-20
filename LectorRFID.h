#ifndef RFID_MODULE_H
#define RFID_MODULE_H

#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>  // Biblioteca para manejo de EEPROM

// Definiciones para el RC522
#define SS_PIN 5    // Selección de esclavo (SDA) del RC522
#define RST_PIN 22  // Reset del RC522

// Constantes para EEPROM
#define EEPROM_SIZE 512
#define EEPROM_UID_START 0
#define EEPROM_UID_MAX_COUNT 10
#define EEPROM_UID_SIZE 7

// Variables globales
extern MFRC522 rfid;
extern byte authorizedUIDs[10][7];
extern byte authorizedUIDSizes[10];
extern int authorizedUIDCount;
extern byte eraseEEPROMUID[4];
extern byte whiteCardUID[4];

// Funciones
void printArray(byte arr[10][7], int count);
bool isUIDMatching(byte *uid, byte uidSize, byte *targetUID, byte targetUIDSize);
bool isAuthorized(byte *uid, byte uidSize);
bool isWhiteCard(byte *uid, byte uidSize);
void addNewAuthorizedUID();
void clearEEPROM();
void loadAuthorizedUIDs();
void saveAuthorizedUIDs();
bool diagnoseRFIDModule();
void startRFID();

#endif // RFID_MODULE_H
