enum
{
	DRIVE_FORWAD_MOTOR_1 = 0, // DRIVE Forward Motor 1
	DRIVE_FORWAD_MOTOR_2 = 4, // DRIVE Forward Motor 2
		// @, cmd, byteValue, CRC_16
		// [ 0 ; 127 ]
		// 0 : stop
		// 64 : midle speed
	DRIVE_BACKWARD_MOTOR_1 = 1, // DRIVE Backwards Motor 1
	DRIVE_BACKWARD_MOTOR_2 = 5, // DRIVE Backwards Motor 2
		// @, cmd, byteValue, CRC_16
		// [ 0 ; 127 ]
		// 0 : stop
		// 64 : midle speed
	SET_MINIMUM_MAIN_VOLTAGE = 2, // Set Main Voltage Minimum
		// @, 2, byteValue, CRC_16
		// (Command 57 Preferred)
		// If the battery voltages drops below the set voltage level RoboClaw will stop driving the motors.
		// The voltage is set in .2 volt increments. A value of 0 sets the minimum value allowed which is 6V.
		// The valid data range is 0 - 140 (6V - 34V). The formula for calculating the voltage is: 
		// \f$ (Desired Volts - 6) x 5 = Value \f$
	SET_MAXIMUM_MAIN_VOLTAGE = 3, // Set Main Voltage Maximum
		// @, 3, byteValue, CRC_16
		// (Command 57 Preferred)
		// Sets main battery (B- / B+) maximum voltage level. The valid data range is 30 - 175 (6V - 34V).
		// During regenerative breaking a back voltage is applied to charge the battery. 
		// When using a power supply, by setting the maximum voltage level, RoboClaw will, before exceeding it,
		// go into hard braking mode until the voltage drops below the maximum value set. This will prevent
		// overvoltage conditions when using power supplies. The formula for calculating the voltage is:
		// \f$ Desired Volts x 5.12 = Value \f$
	DRIVE_MOTOR_1= 6, // DRIVE Motor 1 (7 Bit)
	DRIVE_MOTOR_2= 7, // DRIVE Motor 2 (7 Bit)
		// @, cmd, byteValue, CRC_16
		// [ 0 ; 127 ]
		// 0 : full reverse
		// 64 : stop
		// 127 : full forward

	DRIVE_FORWARD= 8, // DRIVE Forward Mixed Mode
	DRIVE_BACKWARDS= 9, // DRIVE Backwards Mixed Mode
	TURN_RIGHT_MIXED = 10, // Turn Right Mixed Mode
	TURN_LEFT_MIXED = 11, // Turn Left Mixed Mode
	DRIVE_FORWARD_OR_BACKWARDS = 12, // Drive Forward or Backward (7 bit)
	TURN_LEFT_RIGHT = 13, // Turn Left or Right (7 Bit)

	READ_ENCODER_COUNT_1 = 16, // Read Encoder Count/Value for M1.
	READ_ENCODER_COUNT_2 = 17, // Read Encoder Count/Value for M2.
		// @, cmd
		// [ 7 ] = { uint32_t, uint8_t, CRC_16 }
		// status : 0 = counter underflow
		// direction : 1 = 0 forward / 1 backward
		// Counter overflow : 1 = 1 counter overflow
	READ_ENCODER_SPEED_1 = 18, // Read M1 Speed in Encoder Counts Per Second.
	READ_ENCODER_SPEED_2 = 19, // Read M2 Speed in Encoder Counts Per Second.
		// @, cmd
		// [ 7 ] = { uint32_t, uint8_t, CRC_16 }
		// status = 0 forward / 1 backward
	RESET_ENCODER_1_2 = 20, // Resets Encoder Registers for M1 and M2(Quadrature only).
		// @, 20, CRC_16

	READ_FIRMWARE_VERSION = 21, // Read Firmware Version
		// @, 21
		// max length 48 byte termnated by "\n\0"
	SET_ENCODER_1 = 22, // Set Encoder 1 Register(Quadrature only).
	SET_ENCODER_2 = 23, // Set Encoder 2 Register(Quadrature only).
	READ_MAIN_BATTERY = 24, // Read Main Battery Voltage
		// @, 24
		// [ 4 ] = { uint16_t, CRC_16 }
	READ_LOGIC_BATTERY = 25, // Read Logic Battery Voltage
	SET_MINIMUM_LOGIC_VOLTAGE_LEVEL = 26, // Set Minimum Logic Voltage Level
	SET_MAXIMUM_LOGIC_VOLTAGE_LEVEL = 27, // Set Maximum Logic Voltage Level
	SET_SPEED_PID_1 = 28, // Set Velocity PID Constants for M1.
	SET_SPEED_PID_2 = 29, // Set Velocity PID Constants for M2.
		// @, cmd, D(4 bytes), P(4 bytes), I(4 bytes), QPPS(4 byte), CRC_16
		// default PID values will need to be tuned for the systems being driven. 
		// This gives greater flexibility in what motor and encoder combinations
		// can be used. 
		// The RoboClaw PID system consist of four constants starting with QPPS,
		//  - P = Proportional
		//  - I= Integral
		//  - D= Derivative.
		// The defaults values are:
		// QPPS = 44000
		// P = 0x00010000
		// I = 0x00008000
		// D = 0x00004000
		// QPPS is the speed of the encoder when the motor is at 100% power.
		// P, I, D are the default values used after a reset.
	READ_CURRENT_RAW_SPEED = 30, // Read Current M1 Raw Speed
	READ_CURRENT_RAW_SPEED = 31, // Read Current M2 Raw Speed
	SET_SIGNED_DUTY_CYCLE_1 = 32, // Drive M1 With Signed Duty Cycle. (Encoders not required)
	SET_SIGNED_DUTY_CYCLE_2 = 33, // Drive M2 With Signed Duty Cycle. (Encoders not required)
	SET_SIGNED_DUTY_CYCLE_1_2 = 34, // Drive M1 / M2 With Signed Duty Cycle. (Encoders not required)
	SET_SIGNED_SPEED_1 = 35, // Drive M1 With Signed Speed.
	SET_SIGNED_SPEED_2 = 36, // Drive M2 With Signed Speed.
	SET_SIGNED_SPEED_1_2 = 37, // Drive M1 / M2 With Signed Speed.
	SET_SIGNED_SPEED_ACC_1 = 38, // Drive M1 With Signed Speed And Acceleration.
	SET_SIGNED_SPEED_ACC_2 = 39, // Drive M2 With Signed Speed And Acceleration.
		// @, cmd, acc(4B), speed(4B), CRC_16
	SET_SIGNED_SPEED_ACC_1_2 = 40, // Drive M1 / M2 With Signed Speed And Acceleration.
	SET_SIGNED_SPEED_DISTANCE_BUFFERED_1 = 41, // Drive M1 With Signed Speed And Distance. Buffered.
	SET_SIGNED_SPEED_DISTANCE_BUFFERED_2 = 42, // Drive M2 With Signed Speed And Distance. Buffered.
	SET_SIGNED_SPEED_DISTANCE_BUFFERED_1_2 = 43, // Drive M1 / M2 With Signed Speed And Distance. Buffered.
	SET_SIGNED_SPEED_ACC_DISTANCE_1 = 44, // Drive M1 With Signed Speed, Acceleration and Distance. Buffered.
	SET_SIGNED_SPEED_ACC_DISTANCE_2 = 45, // Drive M2 With Signed Speed, Acceleration and Distance. Buffered.
	SET_SIGNED_SPEED_ACC_DISTANCE_1_2 = 46, // Drive M1 / M2 With Signed Speed, Acceleration And Distance. Buffered.
	READ_BUFFER_LENGTH = 47, // Read Buffer Length.
	READ_PWMS = 48, // Read Motor PWMs
	READ_CURRENTS = 49, // Read Motor Currents
	SET_INDIVIDUAL_SIGNED_SPEDD_ACC_1_2 = 50, // Drive M1 / M2 With Individual Signed Speed and Acceleration
	SET_INDIVIDUAL_SIGNED_SPEDD_ACC_DISTANCE_1_2 = 51, // Drive M1 / M2 With Individual Signed Speed, Accel and Distance
	SET_SIGNED_DUTY_ACC_1 = 52, // Drive M1 With Signed Duty and Accel. (Encoders not required)
	SET_SIGNED_DUTY_ACC_2 = 53, // Drive M2 With Signed Duty and Accel. (Encoders not required)
	SET_SIGNED_DUTY_ACC_1_2 = 54, // Drive M1 / M2 With Signed Duty and Accel. (Encoders not required)
	READ_MOTOR_PID_1 = 55, // Read Motor 1 Velocity PID Constants
	READ_MOTOR_PID_2 = 56, // Read Motor 2 Velocity PID Constants
	SET_MAIN_BATTERY_VOLTAGE = 57, // Set Main Battery Voltages
	SET_LOGIC_BATTERY_VOLTAGE = 58, // Set Logic Battery Voltages
	READ_MAIN_BATTERY_VOLTAGE= 59, // Read Main Battery Voltage Settings
	READ_LOGIC_BATTERY_VOLTAGE= 60, // Read Logic Battery Voltage Settings
	SET_POSITION_PID_1 = 61, // Set Position PID Constants for M1.
	SET_POSITION_PID_2 = 62, // Set Position PID Constants for M2
	READ_POSITION_1 = 63, // Read Motor 1 Position PID Constants
	READ_POSITION_2 = 64, // Read Motor 2 Position PID Constants
	SET_SADP_1 = 65, // Drive M1 with Speed, Accel, Deccel and Position
	SET_SADP_2 = 66, // Drive M2 with Speed, Accel, Deccel and Position
	SET_SADP_1_2 =67, // Drive M1 / M2 with Speed, Accel, Deccel and Position
	SET_DEFAULT_DUTY_CYLE_ACCELERATION_FOR_MOTOR_1 = 68, // Set default duty cycle acceleration for M1
	SET_DEFAULT_DUTY_CYLE_ACCELERATION_FOR_MOTOR_2 = 69, // Set default duty cycle acceleration for M2
	SET_S3_S4_S5 MODES= 74, // Set S3,S4 and S5 Modes
	READ_S3_S4_S5 MODES= 75, // Read S3,S4 and S5 Modes
	SET_DEAD_BAND_FOR_RC_CMDS = 76, // Set DeadBand for RC/Analog controls
	READ_DEAD_BAND_FOR_RC_CMDS = 77, // Read DeadBand for RC/Analog controls
	READ_ENCODER_COUNT = 78, // Read Encoders Counts
	READ_MOTOR_SPEED = 79, // Read Motor Speeds
	RESTORT_DEFAULTS = 80, // Restore Defaults
	READ_DEAFULT_DUTY_CYCLE_ACCLELERATION = 81, // Read Default Duty Cycle Accelerations
	READ_TEMP= 82, // Read Temperature
	READ_TEMP_2= 83, // Read Temperature 2
	READ_STATUS = 90, // Read Status
	READ_ENCODERS_MODES= 91, // Read Encoder Modes
	SET_MOTOR_1_ENCODER_MODE = 92, // Set Motor 1 Encoder Mode
	SET_MOTOR_2_ENCODER_MODE = 93, // Set Motor 2 Encoder Mode
	WRITE_SETTINGS_TO_EEPROM = 94, // Write Settings to EEPROM
	READ_SETTINGS_TO_EEPROM = 95, // Read Settings from EEPROM
	WRITE_STANDARD_CONFIG_SETTINGS = 98, // Set Standard Config Settings
	READ_STANDARD_CONFIG_SETTINGS = 99, // Read Standard Config Settings
	SET_CTRL_MODE = 100, // Set CTRL Modes
	READ_CTRL_MODE = 101, // Read CTRL Modes
	SET_CTRL_1= 102, // Set CTRL1
	SET_CTRL_2= 103, // Set CTRL2
	READ_CTRLS = 104, // Read CTRLs
	SET_MOTOR_1_CURRENT_MAX = 133, // Set M1 Maximum Current
	SET_MOTOR_2_CURRENT_MAX = 134, // Set M2 Maximum Current
	READ_MOTOR_1_CURRENT_MAX = 135, // Read M1 Maximum Current
	READ_MOTOR_2_CURRENT_MAX = 136, // Read M2 Maximum Current
	SET_PWM_MODE = 148, // Set PWM Mode
	READ_PWM_MODE = 149, // Read PWM Mode
}
robotClawCmds;

int openRoboClaw ( const char busName[], int * const roboClaw )
{
	if ( roboClaw )
	{
		return ( __LINE__ );
	}

	*roboClaw = open ( busName, O_RDWR );
	if ( *roboClaw < 0 )
	{
		return ( __LINE__ );
	}

	return ( 0 );
}