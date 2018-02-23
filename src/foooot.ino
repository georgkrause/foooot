
// some hardware settings
const int num_buttons = 4;
int leds[num_buttons] = {12, 11, 10, 9}; // Pin numbers of the LEDs
int buttons[num_buttons] = {8, 7, 6, 5}; // Pin numbers of the Buttons

// some technical settings
unsigned long debounce_delay = 50;
int long_button_time = 700;

// initialize startup values
int page = 0;
bool page_set = false;

bool channel[num_buttons * num_buttons];
bool active_button[num_buttons];

unsigned long button_start[num_buttons];
unsigned long button_debounce[num_buttons];


unsigned long last_led_switch;

void setup() {
  for (int i = 0; i < num_buttons; i++) {
    // set everything up
    // pin modes...
    pinMode(leds[i], OUTPUT);
    pinMode(buttons[i], INPUT_PULLUP);

    // button states...
    button_start[i] = 0;
    active_button[i] = false;

    // boot signal
    digitalWrite(leds[i], HIGH);
  }
  // Start Midi transmitting
  Serial.begin(31250);

  // Boot ready
  delay(1000);
  for (int i = 0; i < num_buttons; i++) {
    digitalWrite(leds[i], LOW);
  }
}

void loop() {
  // loop over all buttons
  for (int i = 0; i < num_buttons; i++) {
    // read button state
    int reading = digitalRead(buttons[i]);

    //check if button state changed to active
    if (reading == LOW && active_button[i] == false && button_start[i] == 0) {
      // change button state
      active_button[i] = true;
      button_start[i] = millis();

    // check if button state is still active
    } else if (reading == LOW && active_button[i] == true) {
      // check if its a long press
      if ((button_start[i] != 0) && (millis() - button_start[i] > long_button_time)) {
        if (i != page)
          set_page(i); // change page on long press
        page_set = true; 

        // LED blink if page is changed
        if (millis() - last_led_switch > 100) {
          digitalWrite(leds[i], digitalRead(leds[i]) ? LOW : HIGH);
          last_led_switch = millis();
        }
      }
    }

    // check if button is released
    if (reading == HIGH && active_button[i] == true) {
      active_button[i] = false;

      if (!page_set && (millis() - button_start[i] > debounce_delay)) {
        toggle_channel(i); // toggle channel only if its not a long press
      }

      // display current channel state when button is released
      // this is needed because the LED blinks when the page is changed 
      if (page_set) {
        if(channel[page*num_buttons + i]) {
          digitalWrite(leds[i], HIGH);
        } else {
          digitalWrite(leds[i], LOW);
        }
      }
      // reset status stuff
      button_start[i] = 0;
      page_set = false;
    }
  }
}

// function to 
void set_page(int new_page) {
  page = new_page;

  // update LEDs
  for (int i = 0; i < num_buttons; i++) {
    if (channel[page * num_buttons + i]) {
      digitalWrite(leds[i], HIGH);
    } else {
      digitalWrite(leds[i], LOW);
    }
  }
}

void toggle_channel(int i) {
  int ch = page * num_buttons + i; // calculates channel number
  digitalWrite(leds[i], channel[ch] ? LOW : HIGH); // update LED

  // Send Midi
  if(channel[ch]) {
    channel[ch] = false;
    Serial.write(byte(176));
    Serial.write(byte(20+ch));
    Serial.write(byte(0));
  } else {
    channel[ch] = true;
    Serial.write(byte(176));
    Serial.write(byte(20+ch));
    Serial.write(byte(127));
  }
}

