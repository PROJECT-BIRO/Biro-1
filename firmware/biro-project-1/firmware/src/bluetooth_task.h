#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEHIDDevice.h>
#include <HIDTypes.h>
#include <HIDKeyboardTypes.h>

#include "task.h"
#include "logger.h"
#include "display_task.h" // potentially displaying bluetooth status on the display
#include "serial/serial_protocol_plaintext.h"
#include "serial/serial_protocol_protobuf.h"
#include "serial/uart_stream.h"

class BluetoothTask: public Task<BluetoothTask>, public BLEServerCallbacks {
    friend class Task<BluetoothTask>; // Allow base Task to invoke protected run()

    public:
        BluetoothTask(const uint8_t task_core);
        ~BluetoothTask();

        void sendKeypress(uint8_t keycode);
        void sendConsumerAction(uint8_t action);

        

    protected:
        void run();

    private:

        BLEServer* pServer = nullptr;
        BLEHIDDevice* pHID = nullptr;
        BLECharacteristic* inputConsumer = nullptr;
        BLECharacteristic* keyboardInput = nullptr;

        bool deviceConnected = false;

        void setupBLE();
        void advertise();

        // Callbacks
        void onConnect(BLEServer* pServer) override;
        void onDisconnect(BLEServer* pServer) override;
};