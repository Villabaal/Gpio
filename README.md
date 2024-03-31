# Clases para Control de GPIO y Botones en ESP32

## Clase GPIO para Pines de Entrada-Salida de Propósito General

Esta plantilla de clase en C++ proporciona una interfaz para controlar pines de entrada-salida (GPIO) de propósito general en un ESP32 utilizando el marco ESP-IDF.

### Uso

Para utilizar esta plantilla, sigue estos pasos:

1. Incluye el archivo de encabezado en tu proyecto.
2. Crea una instancia de la clase `gpio` con el número deseado de pin GPIO (`gpio_num_t`).
3. Elige el modo del pin (`gpio_mode_t`) y opcionalmente especifica si la salida debe estar invertida con el parametro de plantilla `INV`.
4. Inicializa el pin con método `init`.
5. Opcionalmente, puedes leer el estado del pin utilizando el método `get` y establecer el estado de salida utilizando el método `set`.

### Ejemplo de Código

```cpp
#include "Gpio.h"

struct Main{
    [[nodiscard]] static esp_err_t setup();
    static void loop();
};

// Define el número del pin y el modo de operación
using pin = gpio< GPIO_NUM_34 >::direction< GPIO_MODE_OUTPUT >;

esp_err_t Main::setup() {
    // Inicializa el pin GPIO como una salida (no invertida)
    return pin::init();
}

void Main::loop() {
    // Establece el estado del pin GPIO en ALTO (asumiendo que está configurado como salida)
    ESP_ERROR_CHECK( pin::set(true) );
    // invierte el estado del pin
    ESP_ERROR_CHECK( pin::set() );
    // lee el estado del pin
    bool&& state {pin::get()};
}

extern "C" void app_main(){  
    ESP_ERROR_CHECK( Main::setup() ); 
    while(true) Main::loop(); 
} 
```
## Clase Button para Botones con Método Polling

Esta clase en C++ proporciona una interfaz para leer un botón utilizando el método de polling en un ESP32.

### Uso

Para utilizar esta plantilla, sigue estos pasos:

1. Incluye el archivo de encabezado en tu proyecto.
2. Crea una instancia de la clase `Button` con el número deseado de pin GPIO (`gpio_num_t`) y opcionalmente el tiempo de antirrebote (`DB_DELAY`).
3. Puedes configurar las funciones a llamar con el método (`setCallback`) para los eventos del botón (`on_press`, `on_hold`, `on_release`).
4. Inicializa el pin con el método `init`.
5. Llama al operador de llamada `operator()` en un ciclo infinito (o muy largo) para leer el botón utilizando el método de polling.

### Ejemplo de Código

En el archivo main.hpp
```cpp
#pragma once
#include "Gpio.h"
#include "Button.h"

#include "esp_task_wdt.h"

class Main{
    using time_point = std::chrono::_V2::system_clock::time_point;
    using led = gpio<GPIO_NUM_2>::direction<GPIO_MODE_OUTPUT>;
    static constexpr auto &now {std::chrono::system_clock::now};
    static inline Button< GPIO_NUM_4 > button {};    
    static inline time_point lastTimer {};
    static void whilePushing();
    template< bool edge > static void on_edge();
public:
    [[nodiscard]] static esp_err_t setup();
    static void loop();
};
```
En el archivo main.cpp
```cpp
#include "main.h"
#define LOG_LEVEL_LOCAL ESP_LOG_VERBOSE
using namespace std::literals::chrono_literals;

//inicia o termina el parpadeo en cada uno de los flancos del botón
template< bool edge > void Main::on_edge(){
    ESP_ERROR_CHECK( led::set(!edge) );
    if constexpr (!edge) lastTimer = now();
}

//invierte el estado del led cada 0.5s
void Main::whilePushing(){
    time_point &&ahora {now()};
    if( ahora - lastTimer >= 500ms ) 
        { ESP_ERROR_CHECK( led::set() ); lastTimer = ahora; }
}

esp_err_t Main::setup(){
    esp_task_wdt_deinit();
    esp_err_t &&status {led::init()};  if (ESP_OK != status) return status;
    status =  button.init();  if (ESP_OK != status) return status;
    //enciende el led al presionar
    button.setCallback( on_release, on_edge< false > );
    //apaga el led al dejar de presionar
    button.setCallback( on_release, on_edge< true > );
    //parpadea el led mientras se mantiene el botón presionado
    button.setCallback( on_hold, whilePushing );
    return ESP_OK;
}

void Main::loop(){
    button();
}

extern "C" void app_main(){  
    ESP_ERROR_CHECK( Main::setup() ); 
    while(true) Main::loop(); 
} 
```
