#pragma once
#include "Gpio.h"
#include <chrono>


template< gpio_num_t PIN, int64_t DB_DELAY = 50 >
class Button {
    using ms = std::chrono::milliseconds;
    using system_clock = std::chrono::system_clock;
    using time_point = std::chrono::_V2::system_clock::time_point;
    using pin = typename gpio< PIN >::direction< GPIO_MODE_INPUT >;
public:
    static constexpr auto& init { pin::init };
	//callbakc de cada evento, (funciona dentro de un ciclo infinito)
	static void polling(  void(*onPress)(), void(*onHold)(), void(*onRelease)() ){ 
        debounce( onPress );
        while(!_state){ 
            if( onHold ) onHold(); 
            debounce( onRelease,true ); 
        } 
    }		
	static void polling( void(*onPress)(), void(*onHold)() ) {
		polling( onPress, onHold, nullptr ); 
	}
	static void polling( void(*onPress)() ) { debounce( onPress ); }

private:
	static bool _state,_lastState;
    static time_point _lastDebounceTime;
 	// filtra el rebote y llama la funcion si se detecta un flanco en el boton
	// especificado por _trigger.
	static void debounce( void(*onEdge)() ,bool edgeDirection = false ) {
		const bool&& reading = pin::get();
		// checa si el boton acaba de cambiar de estado
		if (reading != _lastState) _lastDebounceTime = system_clock::now();
		//debounce y detecta los flancos
        int64_t&& interval {std::chrono::duration_cast< ms >( system_clock::now() - _lastDebounceTime).count()};
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