#pragma once
#include "driver/gpio.h"
#include <type_traits>

class gpio{
        template<gpio_num_t PIN>
        static constexpr gpio_config_t _cfg{
                .pin_bit_mask = static_cast<uint64_t>(1) << PIN,
                .mode         = GPIO_MODE_INPUT_OUTPUT,
                .pull_up_en   = GPIO_PULLUP_ENABLE,
                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                .intr_type    = GPIO_INTR_DISABLE
            };

        template<gpio_num_t PIN>
        [[nodiscard]] static constexpr bool get(){ return gpio_get_level(PIN); }

        template<gpio_num_t PIN,bool INV>
        struct Output{ 
            [[nodiscard]] static constexpr esp_err_t set(const bool state){
                if constexpr (INV) return gpio_set_level(PIN, !state );
                else return gpio_set_level(PIN, state );
            };
            static constexpr esp_err_t set(){ return gpio_set_level(PIN, !get<PIN>() ); };  
        };

    public:
        template <gpio_num_t PIN,gpio_mode_t MODE,bool INV = false>
        struct direction: std::conditional_t< MODE == GPIO_MODE_OUTPUT,Output<PIN,INV>, void> {
            [[nodiscard]] static esp_err_t init(){
                 esp_err_t&& status { ESP_OK|gpio_config( &_cfg<PIN> ) };
                if constexpr (MODE == GPIO_MODE_OUTPUT){
                    status |= output::set( INV );
                }
                return status ;
            };
            static constexpr auto& get = gpio::get<PIN>;
        private:
            using output = Output<PIN,INV>;
        };

    };

    



