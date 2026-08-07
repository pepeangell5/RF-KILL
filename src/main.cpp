#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>
#include <esp_system.h>

namespace {

// Unicos cambios de hardware respecto al ESP32 DevKit: pinout y bus SPI C3.
constexpr uint8_t RADIO_SCK = 4;
constexpr uint8_t RADIO_MISO = 5;
constexpr uint8_t RADIO_MOSI = 6;

constexpr uint8_t RADIO_A_CE = 3;
constexpr uint8_t RADIO_A_CSN = 7;
constexpr uint8_t RADIO_B_CE = 1;
constexpr uint8_t RADIO_B_CSN = 10;

// RF24 interpreta el numero de canal como un desplazamiento en MHz:
// frecuencia = 2400 MHz + canal. Por ello 2..80 cubre 2402..2480 MHz.
constexpr uint8_t MIN_RF_CHANNEL = 2;
constexpr uint8_t MAX_RF_CHANNEL = 80;
constexpr uint8_t RF_CHANNEL_COUNT =
    MAX_RF_CHANNEL - MIN_RF_CHANNEL + 1;

// Como 37 y 79 son coprimos, sumar 37 modulo 79 genera una permutacion:
// se visitan los 79 canales exactamente una vez antes de repetir el ciclo.
// El segundo radio conserva un desplazamiento no nulo de 39 posiciones, por
// lo que nunca coincide con el primero durante el barrido.
constexpr uint8_t SWEEP_STEP = 37;
constexpr uint8_t RADIO_OFFSET = 39;

// Tiempo de permanencia por pareja de canales. random() usa limite superior
// exclusivo, por eso improvedSweep() suma 1 al maximo. El promedio es 150 us
// y un ciclo ideal tarda 79 * 150 us = 11.85 ms, mas las operaciones SPI.
constexpr uint16_t MIN_DWELL_US = 130;
constexpr uint16_t MAX_DWELL_US = 170;

uint8_t sweepIndex = 0;
uint8_t radioAChannel = MIN_RF_CHANNEL;
uint8_t radioBChannel = MIN_RF_CHANNEL + RADIO_OFFSET;

// Velocidad SPI experimental usada por el firmware C3 de GitHub.
constexpr uint32_t RADIO_SPI_SPEED = 19909090;
SPIClass radioSpi(FSPI);
RF24 radioA(RADIO_A_CE, RADIO_A_CSN, RADIO_SPI_SPEED);
RF24 radioB(RADIO_B_CE, RADIO_B_CSN, RADIO_SPI_SPEED);

bool radioAReady = false;
bool radioBReady = false;

void improvedSweep() {
  sweepIndex = (sweepIndex + SWEEP_STEP) % RF_CHANNEL_COUNT;

  radioAChannel = MIN_RF_CHANNEL + sweepIndex;
  radioBChannel =
      MIN_RF_CHANNEL + ((sweepIndex + RADIO_OFFSET) % RF_CHANNEL_COUNT);

  radioA.setChannel(radioAChannel);
  radioB.setChannel(radioBChannel);

  // La variacion temporal evita que el barrido se sincronice con el periodo
  // de muestreo de un analizador de espectro externo.
  delayMicroseconds(random(MIN_DWELL_US, MAX_DWELL_US + 1));
}

void prepareRadioPins() {
  // En un bus SPI compartido, todos los CSN deben estar inactivos antes de
  // inicializar el primer dispositivo para evitar contencion sobre MISO.
  pinMode(RADIO_A_CE, OUTPUT);
  pinMode(RADIO_A_CSN, OUTPUT);
  pinMode(RADIO_B_CE, OUTPUT);
  pinMode(RADIO_B_CSN, OUTPUT);

  digitalWrite(RADIO_A_CE, LOW);
  digitalWrite(RADIO_B_CE, LOW);
  digitalWrite(RADIO_A_CSN, HIGH);
  digitalWrite(RADIO_B_CSN, HIGH);
}

void printRadioStatus(const char* name, bool ready) {
  Serial.print(name);
  Serial.println(ready ? ": OK" : ": FALLO");
}

}  // namespace

void setup() {
  Serial.begin(115200);
  randomSeed(esp_random());

  // El USB CDC del C3 necesita mas tiempo que el UART del DevKit.
  delay(1500);
  Serial.println("\nRF-KILL: inicio automatico");

  prepareRadioPins();
  radioSpi.begin(RADIO_SCK, RADIO_MISO, RADIO_MOSI, -1);
  delay(100);

  // Primera inicializacion equivalente a la realizada durante el arranque
  // del firmware de referencia.
  radioA.begin(&radioSpi);
  radioB.begin(&radioSpi);

  // Da tiempo a que la alimentacion y los modulos PA/LNA se estabilicen.
  delay(500);

  // Segunda inicializacion: primero ambos begin() y despues la configuracion,
  // conservando el orden validado experimentalmente.
  radioA.begin(&radioSpi);
  radioB.begin(&radioSpi);

  radioA.setAutoAck(false);
  radioA.stopListening();
  radioA.setRetries(0, 0);
  radioA.setPayloadSize(32);
  radioA.setAddressWidth(5);
  radioA.setPALevel(RF24_PA_MAX, true);
  radioA.setDataRate(RF24_2MBPS);
  radioA.setCRCLength(RF24_CRC_DISABLED);

  radioB.setAutoAck(false);
  radioB.stopListening();
  radioB.setRetries(0, 0);
  radioB.setPayloadSize(32);
  radioB.setAddressWidth(5);
  radioB.setPALevel(RF24_PA_MAX, true);
  radioB.setDataRate(RF24_2MBPS);
  radioB.setCRCLength(RF24_CRC_DISABLED);

  radioAReady = radioA.isChipConnected();
  radioBReady = radioB.isChipConnected();

  printRadioStatus("nRF24 #1", radioAReady);
  printRadioStatus("nRF24 #2", radioBReady);

  radioA.startConstCarrier(RF24_PA_MAX, radioAChannel);
  radioB.startConstCarrier(RF24_PA_MAX, radioBChannel);

  // Misma espera aplicada por la referencia antes de comenzar el hopping.
  delay(400);

  if (!radioAReady && !radioBReady) {
    Serial.println("Sin radios disponibles; revisa cableado y alimentacion.");
  } else {
    Serial.println("Barrido automatico activo.");
  }
}

void loop() {
  // Dos canales distintos por paso; cobertura completa antes de repetir.
  while (radioAReady || radioBReady) {
    improvedSweep();
  }

  delay(1000);
}
