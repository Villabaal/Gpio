#pragma once
#include "Gpio.h"
#include <chrono>
#include <concepts>
#include <array>

enum class button_event_t {
    on_press,  /*Cuando el botón es presionado*/
    on_hold,   /*Mientras el botón esta presionado*/
    on_release /*Cuando el botón es liberado*/
};
using enum button_event_t;

/// @brief boton metodo polling
///
/// @tparam[in] PIN : numero de pin tipo gpio_mode_t
/// @tparam[in] DB_DELAY : rezago antirebote (default 50ms) 
template< gpio_num_t PIN, int64_t DB_DELAY = 50 >
class Button {
    using ms = std::chrono::milliseconds;
    using time_point = std::chrono::_V2::system_clock::time_point;
    using pin = typename gpio< PIN >::direction< GPIO_MODE_INPUT >;

    std::array<std::function<void()>,3> callbacks {};
    /// @brief handler del evento persionar boton
    std::function<void()> &onPress {callbacks[0]};
    /// @brief handler del evento de mantener presionado
    std::function<void()> &onHold {callbacks[1]};
    /// @brief handler del evento soltar boton
    std::function<void()> &onRelease {callbacks[2]};
public:
    /// @brief inicializa el botón
    static constexpr esp_err_t (&init)() { pin::init };
    /// @brief Configura la función llamada para el evento seleccionado
    ///
    /// @param[in] event : evento del botón
    /// @param[in] callback : función callback del botón
    inline void setCallback(button_event_t const &event, std::invocable auto const &callback){
        callbacks[ uint8_t(event) ] = callback;
    }
	/// @brief lee el botón por método polling. Debe estar en un ciclo infinito. 
    void operator()(){ 
        debounce( onPress );
        while(!_state)
            { if( onHold ) onHold(); debounce( onRelease,true );  } 
    }	
private:
    static inline ms delayTime { ms(DB_DELAY) };
	bool _state {true},_lastState {true};
    time_point _lastDebounceTime {};
    static constexpr time_point (&now)() {std::chrono::system_clock::now};
    /// @brief filtra el rebote y llama la funcion si se detecta un flanco en el boton
	///especificado por edgeDirection.
    ///
    /// @param[in] onEdge : funcion a llamar cuando el flanco es detectado
    /// @param[in] edgeDirection : flanco gatillo de onEdge (por defecto "flanco de bajada")   
	void debounce( std::function<void()> const& onEdge , bool const& edgeDirection = false ) {
		const bool&& reading = pin::get();
		// checa si el boton acaba de cambiar de estado
		if (reading != _lastState) _lastDebounceTime = now();
		//debounce y detecta los flancos
        ms&& interval {std::chrono::duration_cast< ms >( now() - _lastDebounceTime )};
        if ( ( interval > delayTime ) && (reading != _state)) {
		    _state = reading; 
            if( (reading == edgeDirection) && onEdge ) onEdge(); 
            return;
		 }
		_lastState = reading;
	}
};