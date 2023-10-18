#pragma once
#include "driver/gpio.h"
#include <type_traits>

/// @brief Pines entrada-salida de proposito general
///
/// @tparam[in] PIN : numero de pin tipo gpio_num_t 
template< gpio_num_t PIN >
class gpio{
        [[nodiscard]] static constexpr bool is_GPI() noexcept {
            if constexpr (  GPIO_NUM_34 == PIN ) return true;
            if constexpr (  GPIO_NUM_35 == PIN ) return true;
            if constexpr (  GPIO_NUM_36 == PIN ) return true;
            if constexpr (  GPIO_NUM_37 == PIN ) return true;
            if constexpr (  GPIO_NUM_38 == PIN ) return true;
            if constexpr (  GPIO_NUM_39 == PIN ) return true;
            else return false;
        }
        
        static constexpr gpio_config_t _cfg{
                .pin_bit_mask = static_cast<uint64_t>(1) << PIN,
                .mode         = GPIO_MODE_INPUT_OUTPUT,
                .pull_up_en   = GPIO_PULLUP_ENABLE,
                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                .intr_type    = GPIO_INTR_DISABLE
            };
        
        struct Input{
            [[nodiscard]] static inline constexpr bool get(){ return gpio_get_level(PIN); }
        };

        template< bool INV >
        struct Output: Input{ 
            [[nodiscard]] static inline constexpr esp_err_t set(const bool state){
                if constexpr (INV) return gpio_set_level(PIN, !state );
                else return gpio_set_level(PIN, state );
            };
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
            static_assert(  !( (MODE != GPIO_MODE_INPUT) && is_GPI() ) , "Ese pin no puede ser usado como salida" );

            [[nodiscard]] static esp_err_t init(){
                esp_err_t&& status { gpio_config( &_cfg ) };
                if constexpr (MODE == GPIO_MODE_OUTPUT){
                    status |= modo::set( INV );
                }
                return status ;
            };
            static constexpr auto& get = modo::get;

        };
    };