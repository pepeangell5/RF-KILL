# RF-KILL ESP32-C3 SuperMini V2

![Version](https://img.shields.io/badge/Version-2.0.0-35C759)
![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32--C3-orange)
![Framework](https://img.shields.io/badge/Framework-Arduino-00979D)
![Board](https://img.shields.io/badge/Board-ESP32--C3%20SuperMini-2F80ED)
![Radio](https://img.shields.io/badge/Radio-2x%20nRF24L01%2B-FF9500)
![License](https://img.shields.io/badge/License-MIT-black)

Firmware headless para ESP32-C3 SuperMini y dos modulos nRF24L01+ en un bus SPI compartido. V2 adopta el flujo RF validado en el proyecto ESP32 DevKit: doble inicializacion, estabilizacion de los modulos y un barrido determinista sin colisiones entre radios.

> Uso previsto: investigacion academica, banco de laboratorio controlado y pruebas RF autorizadas. Respeta la normativa local aplicable.

## Versiones

| Version | Ubicacion | Descripcion |
| --- | --- | --- |
| V2 actual | Raiz del repositorio | Barrido uniforme de 79 canales y Web Flasher principal. |
| V1 respaldo | [`RF-KILL-V1/`](RF-KILL-V1/) | Copia completa del firmware anteriormente publicado en GitHub. |

La carpeta V1 conserva su propio codigo fuente, documentacion, imagenes, manifiesto y binarios. Consulta tambien [`CHANGELOG.md`](CHANGELOG.md).

## Caracteristicas de V2

- Arranque automatico sin pantalla ni botones.
- Dos nRF24L01+ sobre `FSPI` compartido.
- Preparacion previa de ambos CSN para evitar contencion sobre MISO.
- Doble inicializacion de las radios con 500 ms de estabilizacion.
- Portadoras iniciadas antes de comenzar el barrido, con espera adicional de 400 ms.
- Cobertura exacta de canales RF 2 a 80, equivalentes a 2402–2480 MHz.
- Separacion fija entre radios para evitar transmitir sobre el mismo canal.
- Diagnostico por monitor serie a 115200 baudios.
- Binario unificado listo para Web Flasher.

## Algoritmo de barrido

La frecuencia del nRF24 se obtiene mediante:

```text
frecuencia_MHz = 2400 + canal_RF
```

V2 utiliza 79 canales y actualiza el indice con:

```cpp
sweepIndex = (sweepIndex + 37) % 79;
```

Como 37 y 79 son coprimos, cada radio visita los 79 canales exactamente una vez antes de repetir. La segunda radio mantiene un desplazamiento de 39 posiciones, evitando colisiones durante todo el ciclo.

| Parametro | Valor |
| --- | ---: |
| Canal minimo | 2 |
| Canal maximo | 80 |
| Paso | 37 |
| Offset radio B | 39 |
| Dwell minimo | 130 us |
| Dwell maximo | 170 us |
| SPI experimental | 19,909,090 Hz |
| Potencia RF24 | `RF24_PA_MAX` |
| Velocidad RF | `RF24_2MBPS` |
| CRC | Deshabilitado |
| RF24 | 1.6.0 |

La frecuencia SPI es un valor experimental validado para este montaje. Si aparecen fallos de deteccion o escritura de registros, utiliza el checkpoint estable de 16 MHz o reduce la velocidad del bus.

## Hardware y conexiones

Los dos radios comparten alimentación, tierra y las tres señales SPI. Cada uno tiene CE y CSN independientes.

| Señal | ESP32-C3 SuperMini |
| --- | --- |
| SCK compartido | GPIO4 |
| MISO compartido | GPIO5 |
| MOSI compartido | GPIO6 |
| CE nRF24 #1 | GPIO3 |
| CSN nRF24 #1 | GPIO7 |
| CE nRF24 #2 | GPIO1 |
| CSN nRF24 #2 | GPIO10 |
| VCC nRF24 | 3.3 V regulados |
| GND | Tierra comun |

Coloca un capacitor de 10–100 uF cerca de cada nRF24. Los modulos no deben alimentarse con 5 V.

Consulta [`PINOUT_ESP32C3_NRF24.md`](PINOUT_ESP32C3_NRF24.md) para la tabla completa.

## Estructura

```text
.
|-- RF-KILL-V1/              # respaldo completo de V1
|-- binarios/
|   |-- README.md
|   |-- boot_app0.bin
|   |-- bootloader.bin
|   |-- firmware.bin
|   |-- firmware_unificado.bin
|   `-- partitions.bin
|-- img/
|-- src/
|   `-- main.cpp
|-- CHANGELOG.md
|-- index.html
|-- manifest.json
|-- PINOUT_ESP32C3_NRF24.md
|-- platformio.ini
`-- README.md
```

## Compilacion con PlatformIO

Versiones fijadas:

- Espressif32 6.4.0
- Arduino ESP32 2.0.11
- RF24 1.6.0
- Placa `esp32-c3-devkitm-1`

```powershell
Set-Location -LiteralPath "C:\PEPEANGELL\AA_VSCODE_PROYECTOS\ESP32-C3-SUPERMINI"
& "C:\Users\PepeAngell\.platformio\penv\Scripts\platformio.exe" run
```

Subir por USB:

```powershell
& "C:\Users\PepeAngell\.platformio\penv\Scripts\platformio.exe" run --target upload --upload-port COM6
```

Monitor serie:

```powershell
& "C:\Users\PepeAngell\.platformio\penv\Scripts\platformio.exe" device monitor --port COM6 --baud 115200
```

Salida esperada:

```text
RF-KILL: inicio automatico
nRF24 #1: OK
nRF24 #2: OK
Barrido automatico activo.
```

## Metodos de flasheo

### Web Flasher

El instalador principal publica V2:

[https://pepeangell5.github.io/RF-KILL/](https://pepeangell5.github.io/RF-KILL/)

`manifest.json` escribe `binarios/firmware_unificado.bin` en el offset `0x0`.

### Binario unificado con esptool

```bash
esptool.py --chip esp32c3 --baud 460800 write_flash 0x0 binarios/firmware_unificado.bin
```

### Segmentos individuales

```bash
esptool.py --chip esp32c3 --baud 460800 write_flash -z \
  0x0000 binarios/bootloader.bin \
  0x8000 binarios/partitions.bin \
  0xe000 binarios/boot_app0.bin \
  0x10000 binarios/firmware.bin
```

Los tamaños y SHA-256 se encuentran en [`binarios/README.md`](binarios/README.md).

## Web Flasher de V1

El respaldo conserva su instalador en:

```text
RF-KILL-V1/index.html
```

Cuando GitHub Pages publique la rama actual, estará disponible en:

[https://pepeangell5.github.io/RF-KILL/RF-KILL-V1/](https://pepeangell5.github.io/RF-KILL/RF-KILL-V1/)

## Solucion de problemas

| Problema | Revision recomendada |
| --- | --- |
| No aparece el puerto COM | Usa un cable USB de datos y cierra otros monitores seriales. |
| `nRF24 #1: FALLO` | Comprueba MISO compartido y que ambos CSN estén en HIGH antes de inicializar. |
| Alguna radio muestra `FALLO` | Revisa 3.3 V, GND, CE, CSN, MISO, MOSI y SCK. |
| Reinicios al transmitir | Revisa el regulador externo y capacitores cercanos a cada radio. |
| Fallos intermitentes SPI | Reduce `RADIO_SPI_SPEED` a 16000000 o menos. |
| Web Flasher no abre | Usa Chrome o Edge de escritorio mediante HTTPS. |

## Galeria

<p align="center">
  <img src="img/BT-KILL.JPG" width="31%" alt="Montaje RF-KILL">
  <img src="img/BT-KILL-RUIDO.JPG" width="31%" alt="Prueba RF V2">
  <img src="img/BT-KILL-RUIDO2.JPG" width="31%" alt="Prueba RF V2 alternativa">
</p>

## Licencia

Este proyecto se distribuye bajo licencia MIT. Consulta [`LICENSE`](LICENSE).
