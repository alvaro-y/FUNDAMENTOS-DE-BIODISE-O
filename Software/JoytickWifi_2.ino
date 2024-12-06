#include <esp_now.h>
#include <WiFi.h>

// Dirección MAC del receptor
uint8_t broadcastAddress[] = {0xA0, 0xB7, 0x,65 0x2C, 0x08, 0x04}; // REEMPLAZAR CON LA DIRECCIÓN MAC DEL RECEPTOR

// Estructura para enviar datos
typedef struct struct_message {
  char direction[2]; // Para almacenar la dirección ("R", "L", "U", "D", "UR", "UL", "DR", "DL")
} struct_message;

struct_message myData;

// Callback cuando se envía el dato
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print(F("\r\n Paquete enviado:\t"));
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Éxito en la entrega" : "Fallo en la entrega");
}

void setup() {
  // Inicializar el monitor serial
  Serial.begin(115200);
  
  // Configurar el dispositivo como estación Wi-Fi
  WiFi.mode(WIFI_STA);
  
  // Inicializar ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println(F("Error al inicializar ESP-NOW"));
    return;
  }
  
  Serial.print(F("Transmisor inicializado : "));
  Serial.println(WiFi.macAddress());
  
  // Definir función de envío
  esp_now_register_send_cb(OnDataSent);
  
  // Registrar el peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Agregar el peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println(F("Fallo al agregar el peer"));
    return;
  }
}

void loop() {
  // Leer los valores del joystick
  int xValue = analogRead(39); // Pin analógico para eje X
  int yValue = analogRead(36); // Pin analógico para eje Y
  int deadZoneMin = 1800;
  int deadZoneMax = 2200;

  // Determinar dirección
    if (xValue < deadZoneMin && yValue < deadZoneMin) {
    strcpy(myData.direction, "UL"); // Arriba Izquierda
  } else if (xValue < deadZoneMin && yValue > deadZoneMax) {
    strcpy(myData.direction, "DL"); // Abajo Izquierda
  } else if (xValue > deadZoneMax && yValue < deadZoneMin) {
    strcpy(myData.direction, "UR"); // Arriba Derecha
  } else if (xValue > deadZoneMax && yValue > deadZoneMax) {
    strcpy(myData.direction, "DR"); // Abajo Derecha
  } else if (xValue < deadZoneMin) {
    strcpy(myData.direction, "L"); // Izquierda
  } else if (xValue > deadZoneMax) {
    strcpy(myData.direction, "R"); // Derecha
  } else if (yValue < deadZoneMin) {
    strcpy(myData.direction, "U"); // Arriba
  } else if (yValue > deadZoneMax) {
    strcpy(myData.direction, "D"); // Abajo
  } else {
    strcpy(myData.direction, "  "); // Sin movimiento (zona muerta)
  }

  // Enviar mensaje a través de ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.print(F("Sent with success: "));
    Serial.println(myData.direction);
  } else {
    Serial.println(F("Error sending the data"));
  }

  delay(100); // Ajustar el retardo según sea necesario
}
