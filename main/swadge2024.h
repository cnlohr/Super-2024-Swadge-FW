/*! \file swadge2024.h
 *
 * \section swadgeMode_design Design Philosophy
 *
 * A Swadge mode is struct of configuration variables and function pointers, which is the closest thing in C to an
 * object. These provide a common interface so that the system firmware can run each mode. The Swadge's system firmware
 * will initialize peripherals required by the mode and call the mode's function pointers when appropriate.
 *
 * If a mode does not need a particular function, for example it doesn't do audio handling, it is safe to set the
 * function pointer to \c NULL. The function won't be called. All fields must be initialized to something, since an
 * uninitialized field may lead to undefined behavior.
 *
 * \note The details of all configuration variables and function pointers can be found in ::swadgeMode_t.
 *
 * The top level menu will maintain a list of all available modes and the user can pick the mode to run. This approach
 * is similar to apps. Only one mode may run at a single time, and when it runs it will have full system resources.
 *
 * \section swadgeMode_usage Usage
 *
 * Each mode must have a single ::swadgeMode_t. How the Swadge mode works is flexible and left up to the mode's author
 * to determine. Maybe it uses a menu.h, maybe it has a custom UI. All Swadge mode source code should be in the \c
 * /main/modes folder. Each mode should have it's own folder to keep source organized.
 *
 * To build the firmware, the mode's source files must be added to \c /main/CMakeLists\.txt. The emulator's \c makefile
 * automatically finds files to compile recursively, so they do not need to be explicitly listed.
 *
 * It's best practice not to use 'generic' names for functions and variables, because they may collide with another
 * mode's functions or variables. When possible, functions and variables should be prefixed with something unique, like
 * \c demo in the example below.
 *
 * \section swadgeMode_example Example
 *
 * Adding a mode to the CMakeFile requires adding two separate lines in the idf_component_register section.
 *
 * \code{.c}
 * "modes/pong/pong.c"
 * \endcode
 *
 * under the SRCS section and
 *
 * \code{.c}
 * "modes/pong"
 * \endcode
 *
 * under the INCLUDES section.
 *
 *
 * Function prototypes must be declared before using them to initialize function pointers:
 * \code{.c}
 * // It's good practice to declare immutable strings as const so they get placed in ROM, not RAM
 * static const char demoName[]  = "Demo";
 *
 * static void demoEnterMode(void);
 * static void demoExitMode(void);
 * static void demoMainLoop(int64_t elapsedUs);
 * static void demoAudioCallback(uint16_t* samples, uint32_t sampleCnt);
 * static void demoBackgroundDrawCallback(int16_t x, int16_t y, int16_t w, int16_t h, int16_t up, int16_t upNum);
 * static void demoEspNowRecvCb(const esp_now_recv_info_t* esp_now_info, const uint8_t* data, uint8_t len, int8_t rssi);
 * static void demoEspNowSendCb(const uint8_t* mac_addr, esp_now_send_status_t status);
 * static int16_t demoAdvancedUSB(uint8_t* buffer, uint16_t length, uint8_t isGet);
 * static void demoDacCb(uint8_t *samples, int16_t len);
 * \endcode
 *
 * Then functions can be used to initialize a ::swadgeMode_t:
 * \code{.c}
 * swadgeMode_t demoMode = {
 *     .modeName                 = demoName,
 *     .wifiMode                 = ESP_NOW,
 *     .overrideUsb              = false,
 *     .usesAccelerometer        = true,
 *     .usesThermometer          = true,
 *     .overrideSelectBtn        = false,
 *     .fnEnterMode              = demoEnterMode,
 *     .fnExitMode               = demoExitMode,
 *     .fnMainLoop               = demoMainLoop,
 *     .fnAudioCallback          = demoAudioCallback,
 *     .fnBackgroundDrawCallback = demoBackgroundDrawCallback,
 *     .fnEspNowRecvCb           = demoEspNowRecvCb,
 *     .fnEspNowSendCb           = demoEspNowSendCb,
 *     .fnAdvancedUSB            = demoAdvancedUSB,
 *     .fnDacCb                  = demoDacCb,
 * };
 * \endcode
 *
 * The declared functions must actually exist somewhere:
 * \code{.c}
 * static void demoEnterMode(void)
 * {
 *     // Fill this in
 * }
 *
 * static void demoExitMode(void)
 * {
 *     // Fill this in
 * }
 *
 * static void demoMainLoop(int64_t elapsedUs)
 * {
 *     // Fill this in
 * }
 *
 * static void demoAudioCallback(uint16_t* samples, uint32_t sampleCnt)
 * {
 *     // Fill this in
 * }
 *
 * static void demoBackgroundDrawCallback(int16_t x, int16_t y, int16_t w, int16_t h, int16_t up, int16_t upNum)
 * {
 *     // Fill this in
 * }
 *
 * static void demoEspNowRecvCb(const esp_now_recv_info_t* esp_now_info, const uint8_t* data, uint8_t len, int8_t rssi)
 * {
 *     // Fill this in
 * }
 *
 * static void demoEspNowSendCb(const uint8_t* mac_addr, esp_now_send_status_t status)
 * {
 *     // Fill this in
 * }
 *
 * static int16_t demoAdvancedUSB(uint8_t* buffer, uint16_t length, uint8_t isGet)
 * {
 *     // Fill this in
 * 	   return 0;
 * }
 *
 * static void demoDacCb(uint8_t *samples, int16_t len)
 * {
 *     // Fill this in
 * }
 * \endcode
 *
 * The ::swadgeMode_t should be declared as \c extern in a header file so that it can be referenced in other modes.
 * \code{.c}
 * #ifndef _DEMO_MODE_H_
 * #define _DEMO_MODE_H_
 *
 * #include "swadge2024.h"
 *
 * extern swadgeMode_t demoMode;
 *
 * #endif
 * \endcode
 *
 * Add the   modeIncludeList.h:
 * \code{.c}
 * #include "demoMode.h"
 * \endcode
 *
 * Add the mode to the menu initializer and mode list in modeIncludeList.c:
 * \code{.c}
 * // add the following to the allSwadgeModes array, make sure to seperate by a comma
 * &demoMode
 *
 * // In modeListSetMenu(). Add to the appropriate section.
 * addSingleItemToMenu(mainMenu->menu, demoMode.modeName);
 * \endcode
 */

#ifndef _SWADGE_MODE_H_
#define _SWADGE_MODE_H_

// Standard C includes
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Useful ESP things
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <esp_random.h>

// Hardware interfaces
#include "crashwrap.h"
#include "hdw-imu.h"
#include "hdw-battmon.h"
#include "hdw-btn.h"
#include "hdw-dac.h"
#include "hdw-esp-now.h"
#include "hdw-led.h"
#include "hdw-mic.h"
#include "hdw-nvs.h"
#include "hdw-temperature.h"
#include "hdw-tft.h"
#include "hdw-usb.h"

// Drawing interfaces
#include "palette.h"
#include "color_utils.h"
#include "font.h"
#include "wsg.h"
#include "shapes.h"
#include "fill.h"
#include "menu.h"
#include "menuManiaRenderer.h"
#include "menuMegaRenderer.h"

// Asset loaders
#include "cnfs.h"
#include "fs_wsg.h"
#include "fs_font.h"
#include "fs_txt.h"
#include "fs_json.h"

// Connection interface
#include "p2pConnection.h"

// General utilities
#include "linked_list.h"
#include "macros.h"
#include "trigonometry.h"
#include "vector2d.h"
#include "geometry.h"
#include "settingsManager.h"
#include "touchUtils.h"
#include "vectorFl2d.h"
#include "geometryFl.h"
#include "imu_utils.h"
#include "swadgePass.h"
#include "trophy.h"

// Sound utilities
#include "soundFuncs.h"
#include "swSynth.h"
#include "midiPlayer.h"

#define EXIT_TIME_US 1000000
/// @brief the default time between drawn frames, in microseconds (40FPS)
#define DEFAULT_FRAME_RATE_US (1000000 / 40)

// Forward declaration
struct swadgePassPacket;

/**
 * @struct swadgeMode_t
 * @brief A struct of all the function pointers necessary for a swadge mode. If a mode does not need a particular
 * function, for example it doesn't do audio handling, it is safe to set the pointer to NULL. It just won't be called.
 */
typedef struct swadgeMode
{
    /**
     * @brief This swadge mode's name, used in menus. This is not a function pointer.
     */
    const char* modeName;

    /**
     * @brief This is a setting, not a function pointer. Set it to ::NO_WIFI to save power by not using WiFi at all. Set
     * it to ::ESP_NOW to send and receive packets to and from all Swadges in range. ::ESP_NOW_IMMEDIATE is the same as
     * ::ESP_NOW but does not use a queue for incoming packets.
     */
    wifiMode_t wifiMode;

    /**
     * @brief If this is false, then the default TinyUSB driver will be installed (HID gamepad). If this is true, then
     * the swadge mode can do whatever it wants with USB.
     */
    bool overrideUsb;

    /**
     * @brief If this is false, the accelerometer will not be initialized and accelGetAccelVec() will not work. If
     * this is true, then the swadge will be initialized.
     */
    bool usesAccelerometer;

    /**
     * @brief If this is false, the thermometer will not be initialized and readTemperatureSensor() will not work.
     * If this is true, then the swadge will be initialized.
     */
    bool usesThermometer;

    /**
     * @brief If this is false, then ::PB_SELECT events will only be used to return to the main menu or open the quick
     * settings menu. If this is true then ::PB_SELECT events will be passed to the Swadge mode and ::PB_SELECT will not
     * return to the main menu or open the quick settings menu.
     */
    bool overrideSelectBtn;

    /**
     * @brief This function is called when this mode is started. It should initialize variables and start the mode.
     */
    void (*fnEnterMode)(void);

    /**
     * @brief This function is called when the mode is exited. It should free any allocated memory.
     */
    void (*fnExitMode)(void);

    /**
     * @brief This function is called from the main loop. It's pretty quick, but the timing may be inconsistent.
     *
     * @param elapsedUs The time elapsed since the last time this function was called. Use this value to determine when
     * it's time to do things
     */
    void (*fnMainLoop)(int64_t elapsedUs);

    /**
     * @brief This function is called whenever audio samples are read from the microphone (ADC) and are ready for
     * processing. Samples are read at 8KHz. If this function is not NULL, then readBattmon() will not work
     *
     * @param samples A pointer to 12 bit audio samples
     * @param sampleCnt The number of samples read
     */
    void (*fnAudioCallback)(uint16_t* samples, uint32_t sampleCnt);

    /**
     * @brief This function is called when the display driver wishes to update a section of the display.
     *
     * @param x the x coordinate that should be updated
     * @param y the x coordinate that should be updated
     * @param w the width of the rectangle to be updated
     * @param h the height of the rectangle to be updated
     * @param up update number
     * @param upNum update number denominator
     */
    void (*fnBackgroundDrawCallback)(int16_t x, int16_t y, int16_t w, int16_t h, int16_t up, int16_t upNum);

    /**
     * @brief This function is called whenever an ESP-NOW packet is received.
     *
     * @param esp_now_info Information about the transmission, including The MAC addresses
     * @param data     A pointer to the data received
     * @param len      The length of the data received
     * @param rssi     The RSSI for this packet, from 1 (weak) to ~90 (touching)
     */
    void (*fnEspNowRecvCb)(const esp_now_recv_info_t* esp_now_info, const uint8_t* data, uint8_t len, int8_t rssi);

    /**
     * @brief This function is called whenever an ESP-NOW packet is sent. It is just a status callback whether or not
     * the packet was actually sent. This will be called after calling espNowSend().
     *
     * @param mac_addr The MAC address which the data was sent to
     * @param status   The status of the transmission
     */
    void (*fnEspNowSendCb)(const uint8_t* mac_addr, esp_now_send_status_t status);

    /**
     * @brief Advanced USB Functionality, for hooking existing advanced_usb interface.
     * - if \c isGet == 1, that is a "get" or an "IN" endpoint, where the Swadge sends data to the Host.
     * - if \c isGet == 0, that is a "set" or an "OUT" endpoint, where the Host sends data to the Swadge.
     *
     * @param buffer Pointer to full command
     * @param length Total length of the buffer (command ID included)
     * @param isGet 0 if this is a \c SET_REPORT, 1 if this is a \c GET_REPORT
     * @return The number of bytes returned to the host
     */
    int16_t (*fnAdvancedUSB)(uint8_t* buffer, uint16_t length, uint8_t isGet);

    /**
     * @brief This function is called to fill sample buffers for the DAC. If this is NULL, then
     * globalMidiPlayerFillBuffer() will be used instead to fill sample buffers
     */
    fnDacCallback_t fnDacCb;

    /**
     * @brief This function is called to fill in a SwadgePass packet with mode-specific data. The Swadge mode should
     * only fill in it's relevant data and not touch other mode's data.
     *
     * @warning This function will be called when the mode is not initialized or running, so it MUST NOT rely on memory
     * allocated or data loaded in the mode's initializer.
     *
     * @param packet The packet to fill in
     */
    void (*fnAddToSwadgePassPacket)(struct swadgePassPacket* packet);

    /**
     * @brief A struct with the settings and data required for trophy behavior. Set to NULL for no trophies
     */
    trophyDataList_t* trophyData;
} swadgeMode_t;

bool checkButtonQueueWrapper(buttonEvt_t* evt);

void switchToSwadgeMode(const swadgeMode_t* mode);
void softSwitchToPendingSwadge(void);

void deinitSystem(void);

void openQuickSettings(void);
void setFrameRateUs(uint32_t newFrameRateUs);
uint32_t getFrameRateUs(void);

void switchToSpeaker(void);
void switchToMicrophone(void);

void powerDownPeripherals(void);
void powerUpPeripherals(void);

// Getters
font_t* getSysFont(void);
midiFile_t* getSysSound(void);

#endif
