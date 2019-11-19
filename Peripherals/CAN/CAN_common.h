
#ifndef CAN_COMMON_H
#define CAN_COMMON_H

/*
 * List of CAN filters for motor controller 
 * Information on the messages listed below 
 *
 *   ------------------------------------------------
 *   | Name        | Decltype | Units | Period (ms) |
 *   ------------------------------------------------
 *   ------------------------------------------------
 *   | Current     | float    | V     | 200         |
 *   ------------------------------------------------
 *   | Speed       | float    | m/s   | 200         |
 *   ------------------------------------------------
 *   | Temperature | float    | C     | 1000        |
 *   ------------------------------------------------
 */
#define CAN_ID_MOTOR_BASE                        0x00000200
#define CAN_ID_MOTOR_CURRENT_OFFSET              0x00000002
#define CAN_ID_MOTOR_SPEED_OFFSET                0x00000003
#define CAN_ID_MOTOR_TEMPERATURE_OFFSET          0x00000011

/*
 * List of CAN filters for battery controller 
 * Information on the messages listed below 
 *
 *   ------------------------------------------------
 *   | Name        | Decltype | Units | Period (ms) |
 *   ------------------------------------------------
 *   ------------------------------------------------
 *   | Error       | N/A      | N/A   | N/A         |
 *   ------------------------------------------------
 *   | Voltage     | uint16_t | V     | 1000        |
 *   ------------------------------------------------
 *   | Current     | int16_t  | A     | 1000        |
 *   ------------------------------------------------
 *   | Charge      | uint8_t  | N/A   | 1000        |
 *   ------------------------------------------------
 *   | Temperature | int8_t   | C     | 1000        |
 *   ------------------------------------------------
 */
#define CAN_ID_BATTERY_BASE                      0x00000620
#define CAN_ID_BATTERY_ERROR_OFFSET              0x00000002
#define CAN_ID_BATTERY_VOLTAGE_OFFSET            0x00000003
#define CAN_ID_BATTERY_CURRENT_OFFSET            0x00000004
#define CAN_ID_BATTERY_CHARGE_OFFSET             0x00000006
#define CAN_ID_BATTERY_TEMPERATURE_OFFSET        0x00000007

#define CAN_ID_ARRAY_BASE                        0x00000700

#endif //CAN_COMMON_H
