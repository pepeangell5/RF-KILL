# Historial de versiones

## V2.0.0

- Sustituye el barrido aleatorio de V1 por una permutación uniforme de 79 canales.
- Añade separación fija de 39 posiciones entre radios para evitar colisiones.
- Usa dwell aleatorio de 130–170 us.
- Añade doble inicialización y pausas de estabilización de 500 y 400 ms.
- Prepara ambos CSN antes de inicializar el bus SPI compartido.
- Fija RF24 en 1.6.0.
- Añade diagnóstico serial de ambos nRF24.
- Reduce el binario al eliminar las pilas Wi-Fi/Bluetooth no utilizadas.
- Regenera el binario unificado y el Web Flasher para V2.

## V1.0.0

- Firmware original publicado en `main` hasta el commit `2a47a95bf6f5bc466be488b7d57b0cce2a4a1119`.
- Barrido incremental ±2/±4 combinado con canales aleatorios.
- Copia completa conservada en [`RF-KILL-V1/`](RF-KILL-V1/).
