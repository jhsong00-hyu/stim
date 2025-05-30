#define FREQ_0 0
#define FREQ_1 10
#define FREQ_2 50
#define FREQ_3 100
#define FREQ_4 200

#define LIST_LEN 5

#define PIN_OUT 12

#define PIN_BT_UP 2
#define PIN_BT_DN 3

#define PIN_BT_EN 4
#define PIN_BT_RD 5


int i;

int prev_en = 0;
int prev_up = 0;
int prev_dn = 0;
int prev_rd = 0;

int stat_en = 0;
int stat_up = 0;
int stat_dn = 0;
int stat_rd = 0;

int up = 0;
int down = 0;
int enable = 0;
int rd = 0;

int is_enabled = 0;
int is_sham = 0;
int is_pressed = 0;
int is_randomized = 0;

int current_freq = 0;


//int list_freq[5] = {FREQ_0, FREQ_1};

int list_freq[LIST_LEN] = {FREQ_0, FREQ_1, FREQ_2, FREQ_3, FREQ_4};
//int list_rd[5];
int list_rd[LIST_LEN];
int rd_temp = 0;
int rd_cnt = 0;
int rd_skip = 0;

unsigned long prevMillis = 0;
unsigned long currentMillis = 0;
int time_stim = 3;
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
  stat_en = digitalRead(PIN_BT_EN);
  stat_rd = digitalRead(PIN_BT_RD);
  stat_up = digitalRead(PIN_BT_UP);
  stat_dn = digitalRead(PIN_BT_DN);

  up = (stat_up == 1) && (prev_up == 0);
  down = (stat_dn == 1) && (prev_dn == 0);
  enable = (stat_en == 1) && (prev_en == 0);
  rd = (stat_rd == 1) && (prev_rd == 0);

  prev_up = stat_up;
  prev_dn = stat_dn;
  prev_en = stat_en;
  prev_rd = stat_rd;

  is_pressed = stat_en + stat_rd + stat_up + stat_dn;

  currentMillis = millis();

  if(is_enabled == 0){ // Control state
    digitalWrite(PIN_OUT, 0);
    prevMillis = 0;
    rd_cnt = 0;
    //UP button cluster =========
    if(up){
      if(current_freq == LIST_LEN-1){
        current_freq = 0;
      }
      else{
        current_freq++;
      }
      Serial.print("Current frequency: ");
      Serial.print(list_freq[current_freq]);
      Serial.println(" Hz");
      if(is_randomized) Serial.println("** Random mode activated **");
    }
    //=============================

    //DOWN button cluster =========
    if(down){
      if(current_freq == 0){
        current_freq = LIST_LEN-1;
      }
      else{
        current_freq--;
      }
      Serial.print("Current frequency: ");
      Serial.print(list_freq[current_freq]);
      Serial.println(" Hz");
      if(is_randomized) Serial.println("** Random mode activated **");
    }
    //=============================

    //RANDOM button cluster =========
    if(rd){
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
    }
    //=============================

    //EN button cluster ===========
    if(enable){
      if(is_randomized == 0){
        Serial.println("Starting stim. Press any button to stop.");
        is_enabled = 1;
        is_sham = 0;
        is_stimming = 1;
        
        if(list_freq[current_freq] == 0){
          is_sham = 1;
          time_rest = 1000;
        }
        else{
          time_rest = (1000 / list_freq[current_freq]) - time_stim;
        }

        delay(500);
      }
      else if(is_randomized){ // Enabled with randomization on, creating random frequency sequence
        is_enabled = 1;
        is_stimming = 0;
        rd_cnt = 0;
        for(i=0; i < LIST_LEN; i++){
          list_rd[i] = -1;
        }
        while(rd_cnt < LIST_LEN){
          rd_temp = list_freq[random(0,LIST_LEN)];
          rd_skip = 0;
          for(i = 0; i < LIST_LEN; i++){
            if(list_rd[i] == rd_temp) rd_skip = 1;
          }
          if(rd_skip == 0){
            list_rd[rd_cnt] = rd_temp;
            rd_cnt++;
          }
        }
        rd_pause = 1;
        rd_cnt = 0;
        Serial.println("Random frequency sequence started.");
        Serial.println("Press EN to start Stim 1/5. Press any other button to exit.");
        delay(100);
      }
    }
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
    else digitalWrite(PIN_OUT, 0);

    if(is_pressed){
      Serial.println("Stim stopped.");
      Serial.print("Current frequency: ");
      Serial.print(list_freq[current_freq]);
      Serial.println(" Hz");
      if(is_randomized) Serial.println("** Random mode activated **");
      is_enabled = 0;
    }
  }
    
  else{ //Stim phase, random on
    
    if(rd_pause == 1){ //Waiting for user input
      if(enable){
        rd_pause = 0;

        is_sham = 0;

        if(list_rd[rd_cnt] == 0){
          is_sham = 1;
          time_rest = 1000;
        }
        else{
          time_rest = (1000 / list_rd[rd_cnt]) - time_stim;
        }

        Serial.print("Stimming. Sequence ");
        Serial.print(rd_cnt+1);
        Serial.println("/5");
        Serial.println("Press EN to finish sequence. Press any other button to terminate.");
        enable = 0;
        delay(100);
      }
      else if(up + down + rd){
        is_enabled = 0;

        Serial.println("Sequence aborted.");
        Serial.print("Current frequency: ");
        Serial.print(list_freq[current_freq]);
        Serial.println(" Hz");
        if(is_randomized) Serial.println("** Random mode activated **");
        delay(100);
      }
    }

    if(rd_pause == 0){

      if(is_sham == 0){
        if(is_stimming == 1){
          if(currentMillis >= prevMillis + time_stim){
            digitalWrite(PIN_OUT, 0);
            is_stimming = 0;
            prevMillis = currentMillis;
          }
        }
        else{
          if(currentMillis >= prevMillis + time_rest){
            digitalWrite(PIN_OUT, 1);
            is_stimming = 1;
            prevMillis = currentMillis;
          }
        }
      }
      else digitalWrite(PIN_OUT, 0);

      if(enable){
        enable = 0;
        rd_pause = 1;
        Serial.print("Sequence ");
        Serial.print(rd_cnt+1);
        Serial.print("/5 finished. Press EN to continue. ");
        Serial.println("Press any other button to terminate.");
        rd_cnt++;
        delay(10);
        enable = 0;
      }
      else if(up + down + rd){
        is_enabled = 0;
        Serial.println("Aborted");
      }
    }

    if(rd_cnt == LIST_LEN){ //Termination at the end of the sequence
        is_enabled = 0;

        Serial.println("==========");
        Serial.println("Procedure finished.");
        Serial.print("Sequence of frequency: ");
        for(i=0;i<LIST_LEN;i++){
          Serial.print(list_rd[i]);
          Serial.print("Hz ");
        }
        Serial.println("");
        Serial.println("==========");
        Serial.print("Current frequency: ");
        Serial.print(list_freq[current_freq]);
        Serial.println(" Hz");
        if(is_randomized) Serial.println("** Random mode activated **");
    }
  }
}
