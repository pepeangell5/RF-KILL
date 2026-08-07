# Pinout RF-KILL V2 — ESP32-C3 SuperMini

Los dos nRF24L01+ comparten el bus `FSPI`. CE y CSN son independientes para cada radio.

## Bus compartido

| nRF24L01+ | ESP32-C3 SuperMini |
| --- | --- |
| VCC | 3.3 V regulados |
| GND | GND comun |
| SCK | GPIO4 |
| MISO | GPIO5 |
| MOSI | GPIO6 |

## Radio A — nRF24 #1

| Señal | ESP32-C3 SuperMini |
| --- | --- |
| CE | GPIO3 |
| CSN | GPIO7 |

## Radio B — nRF24 #2

| Señal | ESP32-C3 SuperMini |
| --- | --- |
| CE | GPIO1 |
| CSN | GPIO10 |

## Recomendaciones

- No alimentes los nRF24 desde 5 V.
- Usa tierra común entre ESP32-C3, regulador y radios.
- Coloca un capacitor de 10–100 uF cerca de cada radio.
- Mantén cortas las conexiones SPI.
- Ambos CSN deben estar en HIGH antes de inicializar el primer dispositivo.

## Monitor serie esperado

```text
RF-KILL: inicio automatico
nRF24 #1: OK
nRF24 #2: OK
Barrido automatico activo.
```
