# Stage Genie Project
This Arduino project provides a way to control a relay with the added ability to record and replay its states. The states are stored in the Arduino's EEPROM.

## Components

- Arduino (with EEPROM)
- Relay (connected to pin 11)
- 4 Buttons (connected to analog pins A2, A1, A4, A3)
- 4 LEDs (connected to digital pins 4, 8, 6, 2)

| Component | Connection Pin |
| --------- | -------------- |
| Relay     | Digital Pin 11 |
| Button 1  | Analog Pin A2  |
| Button 2  | Analog Pin A1  |
| Button 3  | Analog Pin A4  |
| Button 4  | Analog Pin A3  |
| LED 1 (Record LED)  | Digital Pin 4  |
| LED 2 (Timer LED)   | Digital Pin 8  |
| LED 3 (Normal LED)  | Digital Pin 6  |
| LED 4 (Relay LED)   | Digital Pin 2  |

## Functionality

The program has three main states:

1. **Idle State (State 0)**: In this state, various operations can be triggered by pressing the buttons:

   - **Button KEY1 (A2)**: If pressed for 5 seconds, the EEPROM data is cleared, and the program moves to the Learning State (State 1).
   - **Button KEY2 (A1)**: When pressed, the program moves to the Trigger State (State 2), where it replays the recorded relay states.
   - **Button KEY3 (A4)**: Provides a way to manually control the relay.
   - **Button KEY4 (A3)**: Dumps the EEPROM data to the serial port.

2. **Learning State (State 1)**: In this state, the relay's states are recorded every 100 milliseconds. The program returns to the Idle State if KEY1 is pressed or if the maximum index (900) is reached.

3. **Trigger State (State 2)**: In this state, the program replays the recorded actions every 100 milliseconds. The program returns to the Idle State if KEY1 is pressed or if the recorded action list ends.

## LED Indicators

The LEDs provide status feedback:

- **RECORD_LED (Pin 4)**: ON when recording states, OFF otherwise.
- **TIMER_LED (Pin 8)**: Blinks when recording or replaying, ON in idle state.
- **NORMAL_LED (Pin 6)**: ON when idle, OFF when recording or replaying.
- **RELAY_LED (Pin 2)**: ON when the relay is ON, OFF otherwise.

## Running the Project

- Connect the components as described above.
- Upload the Arduino sketch to your board.
- Use the buttons to control the relay and record/replay its states.
