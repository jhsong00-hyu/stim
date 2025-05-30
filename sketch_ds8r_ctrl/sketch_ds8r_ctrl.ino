#define FREQ_0 0
#define FREQ_1 10
#define FREQ_2 50
#define FREQ_3 100
#define FREQ_4 200

#define PIN_OUT 12

#define PIN_BT_UP 2
#define PIN_BT_DN 3

#define PIN_BT_EN 4
#define PIN_BT_RD 5

int i;

int state_en = 0;
int state_up = 0;
int state_dn = 0;
int state_rd = 0;

int pressed_en = 0;
int pressed_up = 0;
int pressed_dn = 0;
int pressed_rd = 0;

int is_enabled = 0;
int is_sham = 0;
int is_pressed = 0;
int is_randomized = 0;

int current_freq = 0;

int list_freq[5] = {FREQ_0, FREQ_1, FREQ_2, FREQ_3, FREQ_4};
int list_rd[5];
int list_len = 5;
int rd_temp = 0;
int rd_cnt = 0;
int rd_skip = 0;

unsigned long prevMillis = 0;
unsigned long currentMillis = 0;
long time_stim = 3;
long time_rest = 0;
int is_stimming = 0;
int rd_pause = 0;

void setup() {
  pinMode(PIN_OUT, OUTPUT);

  pinMode(PIN_BT_UP, INPUT);
  pinMode(PIN_BT_DN, INPUT);
  pinMode(PIN_BT_EN, INPUT);
  pinMode(PIN_BT_RD, INPUT);

  digitalWrite(PIN_OUT, LOW);
  delay(1);

  Serial.begin(9600);
  randomSeed(analogRead(0));

  for(i=0; i<100; i++){
    Serial.println("");
  }
  Serial.println("DS8R Controller Initiated");
  Serial.println("===BUTTON CONFIG===");
  Serial.println("    EN     RD");
  Serial.println("    UP  DN");
  Serial.println("===================");
  Serial.print("Current frequency: ");
  Serial.print(list_freq[current_freq]);
  Serial.println(" Hz");
}

void loop() {
  pressed_en = digitalRead(PIN_BT_EN);
  pressed_rd = digitalRead(PIN_BT_RD);
  pressed_up = digitalRead(PIN_BT_UP);
  pressed_dn = digitalRead(PIN_BT_DN);

  is_pressed = pressed_en + pressed_rd + pressed_up + pressed_dn;

  currentMillis = millis();

  if(is_enabled == 0){ // Control state
    digitalWrite(PIN_OUT, 0);
    //UP button cluster =========
    if(pressed_up == 1 && state_up == 0){
      state_up = 1;
      if(current_freq == list_len-1){
        current_freq = 0;
      }
      else{
        current_freq++;
      }
      Serial.print("Current frequency: ");
      Serial.print(list_freq[current_freq]);
      Serial.println(" Hz");
      if(is_randomized) Serial.println("** Random mode activated **");
      delay(50);
    }
    if(pressed_up == 0 && state_up == 1) state_up = 0;
    //=============================

    //DOWN button cluster =========
    if(pressed_dn == 1 && state_dn == 0){
      state_dn = 1;
      if(current_freq == 0){
        current_freq = list_len-1;
      }
      else{
        current_freq--;
      }
      Serial.print("Current frequency: ");
      Serial.print(list_freq[current_freq]);
      Serial.println(" Hz");
      if(is_randomized) Serial.println("** Random mode activated **");
      delay(50);
    }
    if(pressed_dn == 0 && state_dn == 1) state_dn = 0;
    //=============================

    //RANDOM button cluster =========
    if(pressed_rd == 1 && state_rd == 0){
      state_rd = 1;
      if(is_randomized == 0){
        is_randomized = 1;
        Serial.println("Random mode activated");
        Serial.println("Current frequency: RANDOM");
      }
      else{
        is_randomized = 0;
        Serial.println("Random mode deactivated");
        Serial.print("Current frequency: ");
        Serial.print(list_freq[current_freq]);
        Serial.println(" Hz");
      }
      delay(50);
    }
    if(pressed_rd == 0 && state_rd == 1) state_rd = 0;
    //=============================

    //EN button cluster ===========
    if(pressed_en == 1 && state_en == 0){
      if(is_randomized == 0){
        state_en = 1;
        Serial.println("Starting stim. Press any button to stop.");
        is_enabled = 1;
        is_sham = 0;
        is_stimming = 1;
        switch(current_freq){
          case 0: // 0Hz
            is_sham = 1;
            break;
          //===================
          case 1: // 10Hz
            time_rest = 100 - time_stim;
            break;
          //===================
          case 2: // 50Hz
            time_rest = 20 - time_stim;
            break;
          //===================
          case 3: // 100Hz
            time_rest = 10 - time_stim;
            break;
          //===================
          case 4: // 200Hz
            time_rest = 5 - time_stim;
            break;
          //===================
          default: // error
            time_rest = 0;
            is_sham = 1;
            break;
          //===================
        }
      }
      else if(is_randomized){ // Enabled with randomization on, creating random frequency sequence
        state_en = 1;
        is_enabled = 1;
        is_stimming = 0;
        rd_cnt = 0;
        for(i=0; i < list_len; i++){
          list_rd[i] = -1;
        }
        while(rd_cnt < list_len){
          rd_temp = list_freq[random(0,5)];
          rd_skip = 0;
          for(i = 0; i < list_len; i++){
            if(list_rd[i] == rd_temp) rd_skip = 1;
          }
          if(rd_skip == 0){
            list_rd[rd_cnt] = rd_temp;
            rd_cnt++;
          }
        }
        rd_pause = 1;
        Serial.println("Random frequency sequence started.");
        Serial.println("Press EN to start Stim 1/5. Press any other button to exit.");
      }
      delay(500);
      
      rd_cnt = 0;
    }
    if(pressed_en == 0 && state_en == 1) state_en = 0;
    //==============================
  }

  else if(is_enabled && is_randomized == 0){ //Stim phase, random off
    if(is_sham == 0){
      if(is_stimming == 0){
        if(currentMillis >= prevMillis + time_rest){
          digitalWrite(PIN_OUT, 1);
          is_stimming = 1;
          prevMillis = currentMillis;
        };
      }
      else{
        if(currentMillis >= prevMillis + time_stim){
          digitalWrite(PIN_OUT, 0);
          is_stimming = 0;
          prevMillis = currentMillis;
        }
      }
    }

    if(is_pressed > 0){
      Serial.println("Stim stopped.");
      Serial.print("Current frequency: ");
      Serial.print(list_freq[current_freq]);
      Serial.println(" Hz");
      if(is_randomized) Serial.println("** Random mode activated **");
      is_enabled = 0;

      state_en = 1;
      state_up = 1;
      state_dn = 1;
      state_rd = 1;

      delay(500);
    }
  }
    
  else{ //Stim phase, random on
    
    if(rd_pause == 1){ //Waiting for user input
      if(pressed_en == 1){
        rd_pause = 0;
        state_en = 1;
      }
      else if(pressed_up + pressed_dn + pressed_rd){
        is_enabled = 0;

        state_en = 1;
        state_up = 1;
        state_dn = 1;
        state_rd = 1;
        Serial.println("Sequence aborted.");
        Serial.print("Current frequency: ");
        Serial.print(list_freq[current_freq]);
        Serial.println(" Hz");
        if(is_randomized) Serial.println("** Random mode activated **");

        delay(500);
      }
    }

    if(rd_pause == 0){

      if(pressed_en == 0 && state_en == 1) state_en = 0;
      
      is_sham = 0;
      switch(list_rd[rd_cnt]){
        case FREQ_0: // 0Hz
          is_sham = 1;
          break;
        case FREQ_1: // 10Hz
          time_rest = 100 - time_stim;
          break;
        case FREQ_2: // 50Hz
          time_rest = 20 - time_stim;
          break;
        case FREQ_3: // 100Hz
          time_rest = 10 - time_stim;
          break;
        case FREQ_4: // 200Hz
          time_rest = 5 - time_stim;
          break;
        default:
          time_rest = 0;
          is_sham = 0;
          Serial.println("Error");
          is_enabled = 0;
      }

      if(is_sham == 0){
        if(is_stimming == 1){
          if(currentMillis >= prevMillis + time_stim){
            digitalWrite(PIN_OUT, 0);
            is_stimming = 0;
          }
        }
        else{
          if(currentMillis >= prevMillis + time_rest){
            digitalWrite(PIN_OUT, 1);
            is_stimming = 1;
          }
        }
      }
      else digitalWrite(PIN_OUT, 0);

      if(pressed_en == 1 && state_en == 0){
        rd_pause = 1;
        rd_cnt++;
        delay(500);
        state_en = 1;
      }
    }

    if(rd_cnt == list_len){ //Termination at the end of the sequence
        is_enabled = 0;

        state_en = 1;
        state_up = 1;
        state_dn = 1;
        state_rd = 1;

        Serial.println("Procedure finished.");
        Serial.print("Sequence of frequency: ");
        for(i=0;i<list_len;i++){
          Serial.print(list_rd[i]);
          Serial.print("Hz ");
        }
        Serial.println("");
        Serial.print("Current frequency: ");
        Serial.print(list_freq[current_freq]);
        Serial.println(" Hz");
        if(is_randomized) Serial.println("** Random mode activated **");

        delay(500);
    }
  }
}
