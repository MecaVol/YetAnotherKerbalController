#include <Arduino.h>

// Pin out data________________________________________________________________
// 74HC595 register#1, register#2 is daisy-chained to #1
int latchPin = 2;
int clockPin = 4;
int dataPin = 3;

// Not connected to registers, the 17th LED!
int stability_assist_LED = 33;

// Virtual pinout for 74HC595 registers
int FWD_LED = 0;
int node_LED = 1;
int TGT_pro_LED = 2;
int DOWN_LED = 3;
int TGT_retro_LED = 4;
int RIGHT_LED = 5;
int LEFT_LED = 6;
int BWD_LED = 7;
int RCS_LED = 8;
int mode_RCS_LED = 9;
int mode_SAS_LED = 10;
int SAS_LED = 11;
int UP_LED = 12;
int ref_surface_LED = 15;
int ref_target_LED = 14;
int ref_orbit_LED = 13;

// Buttons pin-out (direct)
int FWD_button = 7;
int RCS_button = 8;
int Mode_button = 9;
int UP_button = 10;
int Referential_button = 11;
int SAS_button = 12;
int RIGHT_button = 22;
int TGT_retro_button = 24;
int DOWN_button = 26;
int TGT_pro_button = 28;
int LEFT_button = 30;
int BWD_button = 32;
int Node_button = 34;
int MAN_SAS_button = 36;

// Global variables____________________________________________________________
// "Button was pressed" booleans
bool FWD_button_pressed = false;
bool RCS_button_pressed = false;
bool Mode_button_pressed = false;
bool UP_button_pressed = false;
bool Referential_button_pressed = false;
bool SAS_button_pressed = false;
bool RIGHT_button_pressed = false;
bool TGT_retro_button_pressed = false;
bool DOWN_button_pressed = false;
bool TGT_pro_button_pressed = false;
bool LEFT_button_pressed = false;
bool BWD_button_pressed = false;
bool Node_button_pressed = false;
bool MAN_SAS_button_pressed = false;

// System states
bool RCS = false;
bool SAS = false;
enum ControlMode { MODE_SAS, MODE_RCS };
ControlMode control_mode = MODE_SAS;
enum SpeedMode { ORBIT, TARGET, SURFACE };
SpeedMode speed_mode = SURFACE;
enum SASMode {
  STABILITY_ASSIST,
  PROGRADE,
  RETROGRADE,
  NORMAL,
  ANTI_NORMAL,
  RADIAL,
  ANTI_RADIAL,
  MANEUVER,
  TO_TARGET,
  ANTI_TARGET
};
SASMode sas_mode = STABILITY_ASSIST;
bool FWD = false;
bool BWD = false;
bool UP = false;
bool LEFT = false;
bool DOWN = false;
bool RIGHT = false;

// Last Serial Message
String last_message = "";

void process_incoming_serial() {

  FWD = false;
  BWD = false;
  UP = false;
  LEFT = false;
  DOWN = false;
  RIGHT = false;

  while (Serial.available() > 0) {
    char data = Serial.read();

    if (data == 'A')
      SAS = true;
    else if (data == 'B')
      SAS = false;
    else if (data == 'C')
      RCS = true;
    else if (data == 'D')
      RCS = false;

    else if (data == 'E')
      speed_mode = ORBIT;
    else if (data == 'F')
      speed_mode = TARGET;
    else if (data == 'G')
      speed_mode = SURFACE;

    else if (data == 'H')
      sas_mode = PROGRADE;
    else if (data == 'I')
      sas_mode = RETROGRADE;
    else if (data == 'J')
      sas_mode = NORMAL;
    else if (data == 'K')
      sas_mode = ANTI_NORMAL;
    else if (data == 'L')
      sas_mode = RADIAL;
    else if (data == 'M')
      sas_mode = ANTI_RADIAL;
    else if (data == 'N')
      sas_mode = STABILITY_ASSIST;
    else if (data == 'O')
      sas_mode = MANEUVER;
    else if (data == 'P')
      sas_mode = TO_TARGET;
    else if (data == 'Q')
      sas_mode = ANTI_TARGET;
    else if (data == 'R')
      FWD = true;
    else if (data == 'S')
      BWD = true;
    else if (data == 'T')
      UP = true;
    else if (data == 'U')
      DOWN = true;
    else if (data == 'V')
      LEFT = true;
    else if (data == 'W')
      RIGHT = true;
  }
}

void update_system_state() {
  // All LEDs off
  int value = 0;
  digitalWrite(stability_assist_LED, LOW);

  if (RCS) {
    value += pow(2, RCS_LED);
  }
  if (SAS) {
    value += pow(2, SAS_LED);
  }
  switch (control_mode) {
    case MODE_SAS:
      value += pow(2, mode_SAS_LED);
      break;
    case MODE_RCS:
      value += pow(2, mode_RCS_LED);
      break;
  }
  switch (speed_mode) {
    case ORBIT:
      value += pow(2, ref_orbit_LED);
      break;
    case TARGET:
      value += pow(2, ref_target_LED);
      break;
    case SURFACE:
      value += pow(2, ref_surface_LED);
      break;
  }

  if (SAS && control_mode == MODE_SAS) {
    switch (sas_mode) {
      case PROGRADE:
        value += pow(2, FWD_LED);
        break;
      case RETROGRADE:
        value += pow(2, BWD_LED);
        break;
      case NORMAL:
        value += pow(2, UP_LED);
        break;
      case ANTI_NORMAL:
        value += pow(2, DOWN_LED);
        break;
      case RADIAL:
        value += pow(2, LEFT_LED);
        break;
      case ANTI_RADIAL:
        value += pow(2, RIGHT_LED);
        break;
      case STABILITY_ASSIST:
        digitalWrite(stability_assist_LED, HIGH);
        break;
      case MANEUVER:
        value += pow(2, node_LED);
        break;
      case TO_TARGET:
        value += pow(2, TGT_pro_LED);
        break;
      case ANTI_TARGET:
        value += pow(2, TGT_retro_LED);
        break;
    }
  } else {  // light buttons if pressed in RCS mode
    if (UP_button_pressed) {
      value += pow(2, UP_LED);
    } else if (DOWN_button_pressed) {
      value += pow(2, DOWN_LED);
    } else if (LEFT_button_pressed) {
      value += pow(2, LEFT_LED);
    } else if (RIGHT_button_pressed) {
      value += pow(2, RIGHT_LED);
    } else if (FWD_button_pressed) {
      value += pow(2, FWD_LED);
    } else if (BWD_button_pressed) {
      value += pow(2, BWD_LED);
    }
  }

  // Write the state to registers
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, value % (int)pow(2, 8));
  shiftOut(dataPin, clockPin, MSBFIRST, value / (int)pow(2, 8));
  digitalWrite(latchPin, HIGH);
}

void send_state_to_serial() {
  String new_message = "";

  if (SAS) {
    new_message += 'A';
  } else {
    new_message += 'B';
  }

  if (RCS) {
    new_message += 'C';
  } else {
    new_message += 'D';
  }

  if (speed_mode == ORBIT)
    new_message += 'E';
  else if (speed_mode == TARGET)
    new_message += 'F';
  else if (speed_mode == SURFACE)
    new_message += 'G';

  if (sas_mode == PROGRADE)
    new_message += 'H';
  else if (sas_mode == RETROGRADE)
    new_message += 'I';
  else if (sas_mode == NORMAL)
    new_message += 'J';
  else if (sas_mode == ANTI_NORMAL)
    new_message += 'K';
  else if (sas_mode == RADIAL)
    new_message += 'L';
  else if (sas_mode == ANTI_RADIAL)
    new_message += 'M';
  else if (sas_mode == STABILITY_ASSIST)
    new_message += 'N';
  else if (sas_mode == MANEUVER)
    new_message += 'O';
  else if (sas_mode == TO_TARGET)
    new_message += 'P';
  else if (sas_mode == ANTI_TARGET)
    new_message += 'Q';

  if (FWD) new_message += 'R';
  if (BWD) new_message += 'S';
  if (UP) new_message += 'T';
  if (DOWN) new_message += 'U';
  if (LEFT) new_message += 'V';
  if (RIGHT) new_message += 'W';

  if (!(FWD || BWD || UP || DOWN || LEFT || RIGHT)) { // no RCS translation
    new_message += 'X';
  }

  if (new_message != last_message) {
    Serial.println(new_message);
    last_message = new_message;
  }
}

void check_and_send_buttons_state() {
  bool FWD_press = digitalRead(FWD_button);
  bool RCS_press = digitalRead(RCS_button);
  bool Mode_press = digitalRead(Mode_button);
  bool UP_press = digitalRead(UP_button);
  bool Referential_press = digitalRead(Referential_button);
  bool SAS_press = digitalRead(SAS_button);
  bool RIGHT_press = digitalRead(RIGHT_button);
  bool TGT_retro_press = digitalRead(TGT_retro_button);
  bool DOWN_press = digitalRead(DOWN_button);
  bool TGT_pro_press = digitalRead(TGT_pro_button);
  bool LEFT_press = digitalRead(LEFT_button);
  bool BWD_press = digitalRead(BWD_button);
  bool Node_press = digitalRead(Node_button);
  bool MAN_SAS_press = digitalRead(MAN_SAS_button);

  if (RCS_press) {
    if (!RCS_button_pressed) {
      RCS = !RCS;
      RCS_button_pressed = true;
    }
  } else {
    RCS_button_pressed = false;
  }

  if (SAS_press) {
    if (!SAS_button_pressed) {
      SAS = !SAS;
      SAS_button_pressed = true;
    }
  } else {
    SAS_button_pressed = false;
  }

  if (Mode_press) {
    if (!Mode_button_pressed) {
      if (control_mode == MODE_SAS)
        control_mode = MODE_RCS;
      else
        control_mode = MODE_SAS;
      Mode_button_pressed = true;
    }
  } else {
    Mode_button_pressed = false;
  }

  if (Referential_press) {
    if (!Referential_button_pressed) {
      if (speed_mode == ORBIT)
        speed_mode = TARGET;
      else if (speed_mode == TARGET)
        speed_mode = SURFACE;
      else if (speed_mode == SURFACE)
        speed_mode = ORBIT;
      Referential_button_pressed = true;
    }
  } else {
    Referential_button_pressed = false;
  }

  // In SAS mode
  if (control_mode == MODE_SAS) {
    if (FWD_press) {
      if (!FWD_button_pressed) {
        sas_mode = PROGRADE;
        FWD_button_pressed = true;
      }
    } else {
      FWD_button_pressed = false;
    }
    if (BWD_press) {
      if (!BWD_button_pressed) {
        sas_mode = RETROGRADE;
        BWD_button_pressed = true;
      }
    } else {
      BWD_button_pressed = false;
    }
    if (UP_press) {
      if (!UP_button_pressed) {
        sas_mode = NORMAL;
        UP_button_pressed = true;
      }
    } else {
      UP_button_pressed = false;
    }
    if (DOWN_press) {
      if (!DOWN_button_pressed) {
        sas_mode = ANTI_NORMAL;
        DOWN_button_pressed = true;
      }
    } else {
      DOWN_button_pressed = false;
    }
    if (LEFT_press) {
      if (!LEFT_button_pressed) {
        sas_mode = RADIAL;
        LEFT_button_pressed = true;
      }
    } else {
      LEFT_button_pressed = false;
    }
    if (RIGHT_press) {
      if (!RIGHT_button_pressed) {
        sas_mode = ANTI_RADIAL;
        RIGHT_button_pressed = true;
      }
    } else {
      RIGHT_button_pressed = false;
    }
    if (MAN_SAS_press) {
      if (!MAN_SAS_button_pressed) {
        sas_mode = STABILITY_ASSIST;
        MAN_SAS_button_pressed = true;
      }
    } else {
      MAN_SAS_button_pressed = false;
    }
    if (Node_press) {
      if (!Node_button_pressed) {
        sas_mode = MANEUVER;
        Node_button_pressed = true;
      }
    } else {
      Node_button_pressed = false;
    }
    if (TGT_pro_press) {
      if (!TGT_pro_button_pressed) {
        sas_mode = TO_TARGET;
        TGT_pro_button_pressed = true;
      }
    } else {
      TGT_pro_button_pressed = false;
    }
    if (TGT_retro_press) {
      if (!TGT_retro_button_pressed) {
        sas_mode = ANTI_TARGET;
        TGT_retro_button_pressed = true;
      }
    } else {
      TGT_retro_button_pressed = false;
    }

  } else {  // In RCS mode
    if (FWD_press) {
      FWD = true;
    } else {
      FWD = false;
    }
    if (BWD_press) {
      BWD = true;
    } else {
      BWD = false;
    }
    if (UP_press) {
      UP = true;
    } else {
      UP = false;
    }
    if (DOWN_press) {
      DOWN = true;
    } else {
      DOWN = false;
    }
    if (LEFT_press) {
      LEFT = true;
    } else {
      LEFT = false;
    }
    if (RIGHT_press) {
      RIGHT = true;
    } else {
      RIGHT = false;
    }
  }
}

void setup() {
  // Serial connection setup
  Serial.begin(9600);
  Serial.setTimeout(10);

  // Pin mode setup
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  pinMode(stability_assist_LED, OUTPUT);

  pinMode(FWD_button, INPUT);
  pinMode(RCS_button, INPUT);
  pinMode(Mode_button, INPUT);
  pinMode(UP_button, INPUT);
  pinMode(Referential_button, INPUT);
  pinMode(SAS_button, INPUT);
  pinMode(RIGHT_button, INPUT);
  pinMode(TGT_retro_button, INPUT);
  pinMode(DOWN_button, INPUT);
  pinMode(TGT_pro_button, INPUT);
  pinMode(LEFT_button, INPUT);
  pinMode(BWD_button, INPUT);
  pinMode(Node_button, INPUT);
  pinMode(MAN_SAS_button, INPUT);

  update_system_state();  // Initializing LEDs
}

void loop() {
  process_incoming_serial();

  update_system_state();

  check_and_send_buttons_state();

  send_state_to_serial();
}
