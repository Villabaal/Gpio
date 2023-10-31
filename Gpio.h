#pragma once
#include "driver/gpio.h"
#include <type_traits>

/// @brief Pines entrada-salida de proposito general
///
/// @tparam[in] PIN : numero de pin tipo gpio_num_t 
template< gpio_num_t PIN >
class gpio{
        [[nodiscard]] static consteval bool is_GPI() noexcept {
            if constexpr (  GPIO_NUM_34 == PIN ) return true;
            else if constexpr (  GPIO_NUM_35 == PIN ) return true;
            else if constexpr (  GPIO_NUM_36 == PIN ) return true;
            else if constexpr (  GPIO_NUM_37 == PIN ) return true;
            else if constexpr (  GPIO_NUM_38 == PIN ) return true;
            else if constexpr (  GPIO_NUM_39 == PIN ) return true;
            else return false;
        }
        template< gpio_mode_t MODE >
        [[nodiscard]] static consteval gpio_mode_t mode_select() noexcept{
            if constexpr ( GPIO_MODE_INPUT == MODE ) return GPIO_MODE_INPUT;
            else return GPIO_MODE_INPUT_OUTPUT;
        }
        template< bool GPIO, gpio_mode_t MODE >
        static constexpr gpio_config_t _cfg{
            .pin_bit_mask = static_cast<uint64_t>(1) << PIN,
            .mode         = mode_select< MODE >(),
            .pull_up_en   = static_cast<gpio_pullup_t>( GPIO ),
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type    = GPIO_INTR_DISABLE
        };
        struct Input{
            [[nodiscard]] static inline constexpr bool get(){ return gpio_get_level(PIN); }
        };

        template< bool INV >
        struct Output: Input{ 
            /// @brief Modo de uso del pin (entrada ó salida)
            ///
            /// @param[in] state : Imprime el estado en la salida.          
            [[nodiscard]] static inline constexpr esp_err_t set(const bool state){
                if constexpr (INV) return gpio_set_level(PIN, !state );
                else return gpio_set_level(PIN, state );
            };
            /// @brief Invierte el estado de salida.              
            [[nodiscard]] static inline constexpr esp_err_t set()
                { return gpio_set_level(PIN, !Input::get() ); };  
        };
        template < gpio_mode_t MODE,bool INV = false > 
        using mode = std::conditional_t< (MODE == GPIO_MODE_OUTPUT)&&(!is_GPI()) , Output<INV> , Input >;
    public:
        /// @brief Modo de uso del pin (entrada ó salida)
        ///
        /// @tparam[in] MODE : numero de pin tipo gpio_mode_t
        /// @tparam[in] INV : salida invertida (no invertida por defecto) tipo bool     
        template < gpio_mode_t MODE,bool INV = false >
        class direction: public mode< MODE,INV > {
            using modo = mode< MODE,INV >;
        public:
            static_assert(  (MODE == GPIO_MODE_INPUT) || (MODE == GPIO_MODE_OUTPUT)  , "Modo invalido" );
            static_assert(  !( (MODE != GPIO_MODE_INPUT) && is_GPI() ) , "Ese pin no puede ser usado como salida" );
            /// @brief Inicializa el pin 
            [[nodiscard]] static esp_err_t init(){
                esp_err_t&& status { gpio_config( &_cfg< !is_GPI(), MODE > ) };
                if constexpr (MODE == GPIO_MODE_OUTPUT){
                    if ( ESP_OK != status ) return status;
                    status = modo::set( INV );
                }
                return status ;
            };
            static constexpr auto& get = modo::get;

        };
};