#ifndef FIRM_DEFINITION_H
#define FIRM_DEFINITION_H

#define MAX_CURRENT            1500
#define MAX_MODULES            20
#define DEBUG                  0

/*** Pin Setups ***/
#define BUTTON_PIN             3
#define RELAY_PIN              5
#define LED_PIN                6
#define RST_PIN                12
#define ON_BOARD_LED_PIN       13
#define CURRENT_SENSOR_PIN     A3

/*** SERIAL ***/
#define CMD_FAIL               0x11
#define CMD_EOF                0x20
#define CMD_REQ_ADR            0x41 //'A'
#define CMD_LOAD_MODULE        0x42 //'B'
#define CMD_CONFIRM_RECEIVE    0x43 //'C'
#define CMD_DO_MODULE          0x44 //'D'
#define CMD_HI                 0x48 //'H'
#define CMD_INIT_MODULE        0x49 //'I'
#define CMD_LINK_MODULE        0x4C //'L'
#define CMD_UPDATE_MASTER      0x55 //'U'
#define CMD_START              0xFF

/*** Module Actions ***/
#define DO_TURN_ON             0x6E //'n'
#define DO_TURN_OFF            0x66 //'f'

/*** Characteristic Type ***/
#define MODULE_SWITCH_STATE    0x61 //'a' 
#define MODULE_CURRENT         0x62 //'b'

typedef enum CMD_STATE {
  RC_NONE,
  RC_HEADER,
  RC_PAYLOAD,
  RC_CHECK
};

#endif
