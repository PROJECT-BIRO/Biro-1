#include "bluetooth_task.h"

BluetoothTask::BluetoothTask(const uint8_t task_core)
    : Task<BluetoothTask>("BluetoothTask", 4096, 1, task_core) {
}

BluetoothTask::~BluetoothTask() {
    if (pHID) {
        delete pHID;
    }
}

void BluetoothTask::run() {
    setupBLE();
    log("Done setup\n");
    advertise();
    log("Advertised\n");

    while (true) {
        // use queue or event bits to wait for input signals
        sendConsumerAction(0xCD); // Play/Pause
        sendKeypress(0x04); // 'a' key
        delay(2000);              // Wait 2 seconds before repeating
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        log("WHILE LOOP\n");
    }
}


void BluetoothTask::setupBLE() {
    BLEDevice::init("Biro V1");

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(this);

    pHID = new BLEHIDDevice(pServer);

    keyboardInput = pHID->inputReport(1); // Report ID 1
    keyboardInput->setAccessPermissions(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE);
    keyboardInput->setWriteProperty(true);
    keyboardInput->setNotifyProperty(true);
    
    inputConsumer = pHID->inputReport(2); // Report ID 2
    inputConsumer->setAccessPermissions(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE);
    inputConsumer->setWriteProperty(true);
    inputConsumer->setNotifyProperty(true);

    const uint8_t reportMap[] = {
        // -------- Consumer Control (Report ID 1) --------
        USAGE_PAGE(1),      0x0C,         // Consumer
        USAGE(1),           0x01,         // Consumer Control
        COLLECTION(1),      0x01,         // Application
        REPORT_ID(1),       0x01,
        USAGE_MINIMUM(1),   0x00,
        USAGE_MAXIMUM(1),   0xFF,
        LOGICAL_MINIMUM(1), 0x00,
        LOGICAL_MAXIMUM(1), 0xFF,
        REPORT_SIZE(1),     0x08,
        REPORT_COUNT(1),    0x01,
        INPUT,              0x00,         // Data, Array, Absolute
        END_COLLECTION(0),
    
        // -------- Keyboard (Report ID 2) --------
        USAGE_PAGE(1),      0x01,         // Generic Desktop
        USAGE(1),           0x06,         // Keyboard
        COLLECTION(1),      0x01,         // Application
        REPORT_ID(1),       0x02,
        USAGE_PAGE(1),      0x07,         // Keyboard/Keypad
        USAGE_MINIMUM(1),   0xE0,         // Modifier keys
        USAGE_MAXIMUM(1),   0xE7,
        LOGICAL_MINIMUM(1), 0x00,
        LOGICAL_MAXIMUM(1), 0x01,
        REPORT_SIZE(1),     0x01,
        REPORT_COUNT(1),    0x08,
        INPUT,              0x02,         // Data, Variable, Absolute
    
        REPORT_COUNT(1),    0x01,         // Reserved byte
        REPORT_SIZE(1),     0x08,
        INPUT,              0x01,         // Constant
    
        REPORT_COUNT(1),    0x06,         // Up to 6 keys at once
        REPORT_SIZE(1),     0x08,
        LOGICAL_MINIMUM(1), 0x00,
        LOGICAL_MAXIMUM(1), 0x65,
        USAGE_MINIMUM(1),   0x00,
        USAGE_MAXIMUM(1),   0x65,
        INPUT,              0x00,         // Data, Array
        END_COLLECTION(0)
    };
    

    pHID->reportMap((uint8_t*)reportMap, sizeof(reportMap));
    pHID->startServices();

    BLESecurity* pSecurity = new BLESecurity();
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);
    pSecurity->setCapability(ESP_IO_CAP_NONE);

    // pHID->manufacturer("WDBA");

}

void BluetoothTask::advertise() {
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->setAppearance(HID_KEYBOARD);
    pAdvertising->addServiceUUID(pHID->hidService()->getUUID());
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x06);  // Minimum preferred connection interval
    pAdvertising->setMaxPreferred(0x12);  // Maximum preferred connection interval

    BLEDevice::startAdvertising();
    Serial.println("BLE advertising started");
}

void BluetoothTask::sendConsumerAction(uint8_t action) {
    if (inputConsumer) {
        uint8_t buffer[2] = { 0x01, (uint8_t)action }; // example payload
        inputConsumer->setValue(buffer, sizeof(buffer));
        inputConsumer->notify();
        log("SENT CONSUMER ACTION\n");
    }
}

void BluetoothTask::sendKeypress(uint8_t keyCode) {
    if (keyboardInput) {
        uint8_t buffer[2] = { 0x02, keyCode }; // example payload
        keyboardInput->setValue(buffer, sizeof(buffer));
        keyboardInput->notify();
        log("SENT KEY PRESS");
    }
}

// void BluetoothTask::sendKeypress(uint8_t keycode) {
//     // if (!deviceConnected || !keyboardInput) return;

//     uint8_t report[8] = { 0 }; // HID keyboard report: [mods, reserved, key1, key2, ..., key6]
//     report[2] = keycode;

//     keyboardInput->setValue(report, sizeof(report));
//     keyboardInput->notify();

//     // Send key release (all 0s)
//     delay(10);  // Small delay ensures host processes key press
//     memset(report, 0, sizeof(report));
//     keyboardInput->setValue(report, sizeof(report));
//     keyboardInput->notify();
// }

// void BluetoothTask::sendConsumerAction(uint8_t actionCode) {
//     // if (!deviceConnected || !inputConsumer) return;

//     uint8_t report[2] = { actionCode & 0xFF, (actionCode >> 8) & 0xFF };

//     inputConsumer->setValue(report, sizeof(report));
//     inputConsumer->notify();

//     // Send "release"
//     delay(10);
//     uint8_t release[2] = { 0x00, 0x00 };
//     inputConsumer->setValue(release, sizeof(release));
//     inputConsumer->notify();
// }

// ---- Callback Definitions ----
void BluetoothTask::onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("BLE device connected");
}

void BluetoothTask::onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("BLE device disconnected");
    BLEDevice::startAdvertising(); // Re-advertise after disconnection
}

void BluetoothTask::setLogger(Logger* logger) {
    logger_ = logger;
}

void BluetoothTask::log(const char* msg) {
    if (logger_ != nullptr) {
        logger_->log(msg);
    }
}