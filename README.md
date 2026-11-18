# Smart Home Security System

An Arduino UNO-based home security system designed to provide password-based access control, door locking, alarm activation, and password recovery.

## Project Overview

This project implements a smart home security system using an Arduino UNO. Users can enter a password through a 4×4 keypad to unlock the door. A 16×2 I2C LCD displays instructions and system status.

The system also detects repeated incorrect password attempts and activates an alarm for additional security.

## Features

- Password-based authentication using a 4×4 keypad.
- Automatic door locking and unlocking using an SG90 servo motor.
- LCD display for system instructions and status.
- Red and green LED indicators.
- Buzzer for audio alerts.
- Alarm activation after three consecutive incorrect password attempts.
- Recovery password verification.
- Factory reset button for setting a new password.
- Password storage using EEPROM.

## Hardware Components

- Arduino UNO
- 4×4 Keypad
- 16×2 I2C LCD
- SG90 Servo Motor
- Buzzer
- Red LED
- Green LED
- Factory Reset Button
- Breadboard
- Jumper Wires

## Software and Libraries

- Arduino IDE
- Keypad.h
- Servo.h
- Wire.h
- LiquidCrystal_I2C.h
- EEPROM.h

## How It Works

1. The system starts and initializes all connected components.
2. The LCD asks the user to enter a password.
3. The Arduino checks the entered password against the stored password.
4. If the password is correct, the servo unlocks the door and the green LED indicates successful authentication.
5. If the password is incorrect, the door remains locked and the system indicates an error.
6. After three consecutive incorrect attempts, the security alarm is activated.
7. The user must enter the recovery password twice to complete the recovery process.
8. The factory reset button allows a new password to be configured.

## Testing

The system was tested under different conditions, including:

- Correct password authentication.
- Incorrect password detection.
- Three consecutive incorrect attempts.
- Alarm activation.
- Recovery password verification.
- Factory password reset operation.

The testing results confirmed that the system performed the required security operations successfully.

## Limitations

- Security mainly depends on password protection.
- The keypad may be vulnerable to physical damage.
- The SG90 servo motor is suitable for demonstration purposes but may not be strong enough for a real residential door.
- The system requires a continuous power supply.

## Future Improvements

- Fingerprint authentication.
- RFID-based access control.
- Mobile application control.
- Wi-Fi or Bluetooth connectivity.
- Remote security notifications.
- Stronger electronic locking mechanism.
- Backup power supply.

## Project Files

- [Smart_Home_Security_System.ino](./Smart_Home_Security_System.ino) – Complete Arduino source code.
- [Smart_Home_Security_System_report.pdf](./Smart_Home_Security_System_report.pdf) – Project documentation and report.
## Author

**Developed by:** Md. Imran Hossen

Academic project focused on Arduino programming, embedded systems, and smart home security.
