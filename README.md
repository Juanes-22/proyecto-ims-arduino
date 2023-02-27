# Repositorio proyecto IMS - Arduino

Código en Arduino para la IMS BLE gauge board. Con este código es posible transmitir lecturas de un conversor analógico ADS1232 mediante notificaciones de un servicio BLE personalizado. 

## Definiciones BLE
```c++
#define BLE_DEVICE_NAME           "BLE IMS gauge board"
#define BLE_SERVICE_UUID          "68D2E014-B38D-11EC-B909-0242AC120002"
#define BLE_CHARACTERISTIC_UUID   "68D2E015-B38D-11EC-B909-0242AC120002"
```

## Funcionamiento de la tarjeta

La tarjeta IMS BLE gauge board se encarga de recoger las lecturas del ADS1232 (que se encuentra conectado a una galga extensiométrica) y funcionando como un periférico BLE notifica las lecturas analógicas a un dispositivo central BLE. Para comenzar el proceso de transmisión de datos BLE se presiona el botón de START en la tarjeta. Para detener la transmisión de datos, se vuelve a presionar el botón de START.
