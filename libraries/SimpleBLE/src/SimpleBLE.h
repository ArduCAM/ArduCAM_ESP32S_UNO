// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _SIMPLE_BLE_H_
#define _SIMPLE_BLE_H_


#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bt.h"

#include "Arduino.h"

struct ble_gap_adv_params_s;

class SimpleBLE {
    public:

        SimpleBLE(void);
        ~SimpleBLE(void);

        /**
         * Start BLE Advertising
         *
         * @param[in] localName  local name to advertise
         *
         * @return true on success
         *
         */
        bool begin(String localName=String());

        /**
         * Stop BLE Advertising
         *
         * @return none
         */
        void end(void);

    private:
        struct ble_gap_adv_params_s * _ble_adv_param;
        String local_name;
        void _ble_send_adv_param(void);
        void _ble_send_adv_data(void);
    private:

};

#endif
