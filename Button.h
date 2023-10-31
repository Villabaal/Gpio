#pragma once
#include "Gpio.h"
#include <chrono>

/// @brief boton metodo polling
///
/// @tparam[in] PIN : numero de pin tipo gpio_mode_t
/// @tparam[in] DB_DELAY : rezago antirebote (default 50ms) 
template< gpio_num_t PIN, int64_t DB_DELAY = 50 >
class Button {
    using ms = std::chrono::milliseconds;
    using time_point = std::chrono::_V2::system_clock::time_point;
    static constexpr time_point (&now)() {std::chrono::system_clock::now};
    using pin = typename gpio< PIN >::direction< GPIO_MODE_INPUT >;
public:
    /// @brief inicializa el botón
    static constexpr auto& init { pin::init };
    /// @brief handler del evento persionar boton
    static void(*onPress)();
    /// @brief handler del evento de mantener presionado
    static void(*onHold)();
    /// @brief handler del evento soltar boton
    static void(*onRelease)();
	/// @brief Debe estar en un ciclo
	static void polling(){ 
        debounce( onPress );
        while(!_state){ 
            if( onHold ) onHold(); 
            debounce( onRelease,true ); 
        } 
    }		
private:
	static bool _state,_lastState;
    static time_point _lastDebounceTime;
 	// filtra el rebote y llama la funcion si se detecta un flanco en el boton
	// especificado por _trigger.
	static void debounce( void(*onEdge)() ,bool edgeDirection = false ) {
		const bool&& reading = pin::get();
		// checa si el boton acaba de cambiar de estado
		if (reading != _lastState) _lastDebounceTime = now();
		//debounce y detecta los flancos
        int64_t&& interval {std::chrono::duration_cast< ms >( now() - _lastDebounceTime ).count()};
		if ( ( interval > DB_DELAY ) && (reading != _state)) {
		    _state = reading; 
            if( (reading == edgeDirection) && onEdge ) onEdge(); 
            return;
		 }
		_lastState = reading;
	}
};

template< gpio_num_t PIN, int64_t DB_DELAY > 
Button< PIN, DB_DELAY >::time_point Button< PIN, DB_DELAY >::_lastDebounceTime {};
template< gpio_num_t PIN, int64_t DB_DELAY > 
bool Button< PIN, DB_DELAY >::_state {true};
template< gpio_num_t PIN, int64_t DB_DELAY > 
bool Button< PIN, DB_DELAY >::_lastState {true};
template< gpio_num_t PIN, int64_t DB_DELAY > void(*Button< PIN, DB_DELAY >::onPress)();
template< gpio_num_t PIN, int64_t DB_DELAY > void(*Button< PIN, DB_DELAY >::onHold)();
template< gpio_num_t PIN, int64_t DB_DELAY > void(*Button< PIN, DB_DELAY >::onRelease)();