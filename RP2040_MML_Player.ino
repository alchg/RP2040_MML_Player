#include <hardware/pwm.h>
#include <LittleFS.h>

int16_t potTrack = 99;

class TimeChecker {
public:
  TimeChecker() {
    start_time = 0;
    elapsed_time = 0;
  }
  
  void start() {
    start_time = micros();
  }
  
  unsigned long stop() {
    elapsed_time = micros() - start_time;
    return elapsed_time;
  }
  
private:
  unsigned long start_time;
  unsigned long elapsed_time;
};
TimeChecker timeChecker;

#define CHANNELS  16


#define SAMPLE        64
#define WAVEFORMS     5
#define SINE_WAVE     0
#define SQUARE_WAVE   1
#define TRIANGLE_WAVE 2
#define SAWTOOTH_WAVE 3
#define NOISE_WAVE    4
#define MAX_AMPLITUDE 64
int16_t waveForm[WAVEFORMS][SAMPLE];
void initializeWaveform(){
  
  for(int i=0;i < WAVEFORMS;i++){
    for(int j=0;j < SAMPLE; j++){
      if(i == SINE_WAVE){
        waveForm[i][j] = MAX_AMPLITUDE * sin(2 * PI * j / SAMPLE);
      //0,6,12,18,24,30,35,40,45,49,53,56,59,61,62,63,64,63,62,61,59,56,53,49,45,40,35,30,24,18,12,6,0,-6,-12,-18,-24,-30,-35,-40,-45,-49,-53,-56,-59,-61,-62,-63,-64,-63,-62,-61,-59,-56,-53,-49,-45,-40,-35,-30,-24,-18,-12,-6
      } else if(i == SQUARE_WAVE){
        waveForm[i][j] = (j < SAMPLE / 2) ? MAX_AMPLITUDE : MAX_AMPLITUDE * -1;
        //64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64,-64
      } else if(i == TRIANGLE_WAVE){
        const int divider = 4;
        if(j < SAMPLE / 2){
          waveForm[i][j] = (j < SAMPLE / divider) ? j * (MAX_AMPLITUDE / (SAMPLE / divider)) : MAX_AMPLITUDE - (j - (SAMPLE / divider)) * (MAX_AMPLITUDE / (SAMPLE / divider));
        }else{
          waveForm[i][j] = (j < SAMPLE / divider * 3) ? (j - (SAMPLE / divider * 2)) * (MAX_AMPLITUDE / (SAMPLE / divider)) * -1 : (MAX_AMPLITUDE - (j - (SAMPLE / divider * 3)) * (MAX_AMPLITUDE / (SAMPLE / divider))) * -1;
        }
        //0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,60,56,52,48,44,40,36,32,28,24,20,16,12,8,4,0,-4,-8,-12,-16,-20,-24,-28,-32,-36,-40,-44,-48,-52,-56,-60,-64,-60,-56,-52,-48,-44,-40,-36,-32,-28,-24,-20,-16,-12,-8,-4
      }else if(i == SAWTOOTH_WAVE){
        waveForm[i][j] = (j < (SAMPLE / 2)) ? (MAX_AMPLITUDE - (j * (MAX_AMPLITUDE / (SAMPLE / 2)))) * -1 : (j - (SAMPLE / 2)) * (MAX_AMPLITUDE / (SAMPLE / 2));
        //-64,-62,-60,-58,-56,-54,-52,-50,-48,-46,-44,-42,-40,-38,-36,-34,-32,-30,-28,-26,-24,-22,-20,-18,-16,-14,-12,-10,-8,-6,-4,-2,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62
      }else if(i == NOISE_WAVE){
        waveForm[i][j] = MAX_AMPLITUDE * sin(2 * PI * j / SAMPLE);//random((MAX_AMPLITUDE * -1),(MAX_AMPLITUDE + 1));
        //16,59,25,-32,-49,41,21,14,-29,-18,-64,22,50,10,-31,-23,-10,-37,-14,-26,51,-29,58,16,-24,-15,26,38,-7,-60,-4,61,57,59,-19,58,10,-14,9,-23,-64,11,41,-16,-30,-1,-4,-57,33,17,40,-55,50,-3,47,-40,20,13,20,12,-40,-42,28,-2
      }
    }
  }
  /*
  for (int i = 0; i < SAMPLE; i++) {
    Serial.print(String(waveForm[NOISE_WAVE][i]) + ",");
  }
  Serial.println("finish");
  while(true){delay(100);}
  */
}

#define NOTES                   88
#define FREQUENCY_CORRECTION_C4 0.05
uint16_t frequencyData[NOTES];
void initializeFrequency(){
  // A0(27.500) to C8(4186.009)
  for (int i = 0; i < NOTES; i++) {
    frequencyData[i] = 27.5 * pow(2.0, (double)i / (12.0 - FREQUENCY_CORRECTION_C4)) + 0.5;
    //frequencyData[i] = 27.5 * pow(2.0, (double)i / 12.0) + 0.5;
    //28,29,31,33,35,37,39,41,44,46,49,52,55,58,62,65,69,73,78,82,87,92,98,104,110,117,123,131,139,147,156,165,175,185,196,208,220,233,247,262,277,294,311,330,349,370,392,415,440,466,494,523,554,587,622,659,698,740,784,831,880,932,988,1047,1109,1175,1245,1319,1397,1480,1568,1661,1760,1865,1976,2093,2217,2349,2489,2637,2794,2960,3136,3322,3520,3729,3951,4186
    //0 , 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
  }
  
  /*
  for (int i = 0; i < NOTES; i++) {
    Serial.print(String(frequencyData[i]) + ",");
  }
  Serial.println("");
  */
}

File mmlFile[CHANNELS];
void initializeMmlFile(){
  String temp;
  String number;
  String filename;
  
  for(int i=0;i < CHANNELS;i++){
    temp = "0" + String(i+1);
    number = temp.substring(temp.length() - 2);
    filename = "track" + number + ".mml";
    
    mmlFile[i] = LittleFS.open(filename, "r");
    
    /*
    if(mmlFile[i]){
      Serial.println(filename + ":" + String(mmlFile[i].available()));
    }else{
      Serial.println(filename + ":Failed to open file");
    }
    */
    
  }
}

bool isEven(uint16_t value){
  
  if((value >> 1) << 1 == value){
    return true;
  }
  
  return false;
}

#define STAT_NONE       0
#define BEAT_TICKS      480
#define MML_BUFFER_SIZE 200
struct TrackData{
  bool enable;
  char mml_buffer[MML_BUFFER_SIZE];
  uint16_t mml_index;
  uint16_t mml_count;
  bool enable_envelope;
  uint16_t waveform_type;
  bool enable_modulator;
  uint16_t waveform_type_modulator;
  uint16_t octave;
  uint16_t frequency;
  uint16_t frequency_modulator;
  float frequency_ratio;
  uint32_t ticks;
  uint32_t tick;
  uint16_t base_volume;
  uint16_t volume;
  uint16_t base_volume_modulator;
  uint16_t volume_modulator;
  uint16_t status_adsr;
  uint16_t ticks_attack;
  uint16_t ticks_decay;
  uint16_t volume_sustain;
  uint16_t ticks_release;
  bool enable_envelope_modulator;
  uint16_t status_adsr_modulator;
  uint16_t ticks_attack_modulator;
  uint16_t ticks_decay_modulator;
  uint16_t volume_sustain_modulator;
  uint16_t ticks_release_modulator;
};
TrackData trackData[CHANNELS];
void initializeMmlBuffer(){
  char c;
  
  for(int i=0;i < CHANNELS;i++){
    for(int j=0;j < MML_BUFFER_SIZE;j++){
      trackData[i].mml_buffer[j] = '\0';
      if(mmlFile[i].available()){
        c = mmlFile[i].read();
        trackData[i].mml_count--;
        
        if(c == ';'){
          while(mmlFile[i].available()){
            c = mmlFile[i].read();
            trackData[i].mml_count--;
            
            if(c == '\n'){
              if(mmlFile[i].available() == false){
                break;
              }
              c = mmlFile[i].read();
              trackData[i].mml_count--;
              
              if(c == '\r'){
                if(mmlFile[i].available() == false){
                  break;
                }
                c = mmlFile[i].read();
                trackData[i].mml_count--;
              }
              
              if(c != ';'){
                trackData[i].mml_buffer[j] = c;
                break;
              }
            }
          }
        }else{
          trackData[i].mml_buffer[j] = c;
        }        
      }
    }
  }
  /*
  for(int i=0;i < CHANNELS;i++){
    Serial.println("track_" + String(i));
    for(int j=0;j < MML_BUFFER_SIZE;j++){
      int c = trackData[i].mml_buffer[j];
      //Serial.print("(");
      //Serial.print(c);
      //Serial.print(")");
      Serial.print(trackData[i].mml_buffer[j]);
    }
    Serial.println(":END");
  }
  */
  
}

#define TIMBRES 10
struct TimbreData{
  uint16_t waveform_type;
  bool enable_envelope;
  uint16_t ticks_attack;
  uint16_t ticks_decay;
  uint16_t volume_sustain;
  uint16_t ticks_release;
  bool enable_modulator;
  uint16_t waveform_type_modulator;
  uint16_t base_volume_modulator;
  float frequency_ratio;
  bool enable_envelope_modulator;
  uint16_t ticks_attack_modulator;
  uint16_t ticks_decay_modulator;
  uint16_t volume_sustain_modulator;
  uint16_t ticks_release_modulator;
};
TimbreData timbreData[TIMBRES];

#define DEFAULT_TICKS_ATTACK    50
#define DEFAULT_TICKS_DECAY     50
#define DEFAULT_VOLUME_SUSTAIN  50
#define DEFAULT_TICKS_RELEASE   50
void initializeTimbreData(){
  String temp;
  String number;
  String filename;
  
  for(int i=0;i < TIMBRES;i++){
    timbreData[i].waveform_type = SINE_WAVE;
    timbreData[i].enable_envelope = true;
    timbreData[i].ticks_attack = DEFAULT_TICKS_ATTACK;
    timbreData[i].ticks_decay = DEFAULT_TICKS_DECAY;
    timbreData[i].volume_sustain = DEFAULT_VOLUME_SUSTAIN;
    timbreData[i].ticks_release = DEFAULT_TICKS_RELEASE;
    timbreData[i].enable_envelope_modulator = true;
    timbreData[i].ticks_attack_modulator = DEFAULT_TICKS_ATTACK;
    timbreData[i].ticks_decay_modulator = DEFAULT_TICKS_DECAY;
    timbreData[i].volume_sustain_modulator = DEFAULT_VOLUME_SUSTAIN;
    timbreData[i].ticks_release_modulator = DEFAULT_TICKS_RELEASE;
    
    temp = "0" + String(i);
    number = temp.substring(temp.length() - 2);
    filename = "timbre" + number + ".dat";
    
    File file = LittleFS.open(filename, "r");
    
    if(file){
      //Serial.println(filename + ":" + String(file.available()));
      
      while(file.available()){
        String line = file.readStringUntil('\n');
        line.trim();
        
        String key;
        String value;
        int separator_index = line.indexOf('=');
        if(separator_index != -1){
          key = line.substring(0 ,separator_index);
          value = line.substring(separator_index + 1);
        }
        
        if(key == "WaveformType"){
          int type = value.toInt();
          if(0 <= type && type < WAVEFORMS){
            timbreData[i].waveform_type = type;
          }
        }else if(key == "EnableEnvelope"){
          int envelope = value.toInt();
          if(envelope == 0){
            timbreData[i].enable_envelope = false;
          }
        }else if(key == "TicksAttack"){
          int attack = value.toInt();
          if(attack != 0){
            timbreData[i].ticks_attack = attack;
          }
        }else if(key == "TicksDecay"){
          timbreData[i].ticks_decay = value.toInt();
        }else if(key == "VolumeSustain"){
          timbreData[i].volume_sustain = value.toInt();
        }else if(key == "TicksRelease"){
          int release_ = value.toInt();
          if(release_ != 0){
            timbreData[i].ticks_release = release_;
          }
        }else if(key == "EnableModulator"){
          int enable = value.toInt();
          if(enable == 0){
            timbreData[i].enable_modulator = false;
          }else{
            timbreData[i].enable_modulator = true;
          }
        }else if(key == "VolumeModulator"){
          timbreData[i].base_volume_modulator = value.toInt();
        }else if(key == "WaveformTypeModulator"){
          int type = value.toInt();
          if(0 <= type && type < WAVEFORMS){
            timbreData[i].waveform_type_modulator = value.toInt();
          }
        }else if(key == "FrequencyRatio"){
          timbreData[i].frequency_ratio = value.toFloat();
        }else if(key == "EnableEnvelopeModulator"){
          int envelope = value.toInt();
          if(envelope == 0){
            timbreData[i].enable_envelope_modulator = false;
          }
        }else if(key == "TicksAttackModulator"){
          int attack = value.toInt();
          if(attack != 0){
            timbreData[i].ticks_attack_modulator = attack;
          }
        }else if(key == "TicksDecayModulator"){
          timbreData[i].ticks_decay_modulator = value.toInt();
        }else if(key == "VolumeSustainModulator"){
          timbreData[i].volume_sustain_modulator = value.toInt();
        }else if(key == "TicksReleaseModulator"){
          int release_ = value.toInt();
          if(release_ != 0){
            timbreData[i].ticks_release_modulator = release_;
          }
        }
      }
  
      file.close();
      /*
      Serial.println("TimbreDataIndex:" + String(i));
      Serial.println("WaveformType            :" + String(timbreData[i].waveform_type));
      Serial.println("EnableEnvelope          :" + String(timbreData[i].enable_envelope));
      Serial.println("TicksAttack             :" + String(timbreData[i].ticks_attack));
      Serial.println("TicksDecay              :" + String(timbreData[i].ticks_decay));
      Serial.println("VolumeSustain           :" + String(timbreData[i].volume_sustain));
      Serial.println("TicksRelease            :" + String(timbreData[i].ticks_release));
      Serial.println("EnableModulator         :" + String(timbreData[i].enable_modulator));
      Serial.println("VolumeModulator         :" + String(timbreData[i].base_volume_modulator));
      Serial.println("WaveformTypeModulator   :" + String(timbreData[i].waveform_type_modulator));
      Serial.println("FrequencyRatio          :" + String(timbreData[i].frequency_ratio));
      Serial.println("EnableEnvelopeModulator :" + String(timbreData[i].enable_envelope_modulator));
      Serial.println("TicksAttackModulator    :" + String(timbreData[i].ticks_attack_modulator));
      Serial.println("TicksDecayModulator     :" + String(timbreData[i].ticks_decay_modulator));
      Serial.println("VolumeSustainModulator  :" + String(timbreData[i].volume_sustain_modulator));
      Serial.println("TicksReleaseModulator   :" + String(timbreData[i].ticks_release_modulator));
      */
    }else{
      //Serial.println(filename + ":Failed to open file");
    }
  }
}

void setTimbreData(uint16_t channel_index ,uint16_t tone_index){
  
  trackData[channel_index].waveform_type = timbreData[tone_index].waveform_type;
  trackData[channel_index].enable_envelope = timbreData[tone_index].enable_envelope;
  trackData[channel_index].ticks_attack = timbreData[tone_index].ticks_attack;
  trackData[channel_index].ticks_decay = timbreData[tone_index].ticks_decay;
  trackData[channel_index].volume_sustain = timbreData[tone_index].volume_sustain;
  trackData[channel_index].ticks_release = timbreData[tone_index].ticks_release;
  trackData[channel_index].enable_modulator = timbreData[tone_index].enable_modulator;
  trackData[channel_index].base_volume_modulator = timbreData[tone_index].base_volume_modulator;
  trackData[channel_index].waveform_type_modulator = timbreData[tone_index].waveform_type_modulator;
  trackData[channel_index].frequency_ratio = timbreData[tone_index].frequency_ratio;
  trackData[channel_index].enable_envelope_modulator = timbreData[tone_index].enable_envelope_modulator;
  trackData[channel_index].ticks_attack_modulator = timbreData[tone_index].ticks_attack_modulator;
  trackData[channel_index].ticks_decay_modulator = timbreData[tone_index].ticks_decay_modulator;
  trackData[channel_index].volume_sustain_modulator = timbreData[tone_index].volume_sustain_modulator;
  trackData[channel_index].ticks_release_modulator = timbreData[tone_index].ticks_release_modulator;
  
}

char getChar(uint16_t channel_index){
  
  if(trackData[channel_index].mml_index == MML_BUFFER_SIZE){
    Serial.println("Out of buffer.");
    while(true){
      delay(100);
    }
  }
  
  char c = trackData[channel_index].mml_buffer[trackData[channel_index].mml_index];
  
  if(c != '\0'){
    trackData[channel_index].mml_index++;
  }
  /*
  if(channel_index == 0){
    Serial.print(c);
  }
  */
  return toupper(c);
}

#define DEFAULT_TEMPO     60
#define MAX_TEMPO         255
#define MIN_TEMPO         1
#define SECONDS_IN_MINUTE 60.0
uint16_t tickTimeMilli;
uint16_t tickTimeMicro;
void setTickTime(uint16_t tempo){
  double beat_duration;
  double tick_duration;
  
  //Serial.println(tempo);
  
  if(MIN_TEMPO <= tempo && tempo <= MAX_TEMPO){
    beat_duration = SECONDS_IN_MINUTE / tempo;
  }else{
    beat_duration = SECONDS_IN_MINUTE / DEFAULT_TEMPO;
  }
  
  tick_duration = beat_duration / BEAT_TICKS;
  
  /*
  Serial.println(beat_duration);
  Serial.println(tick_duration,6);
  Serial.println(tick_duration,12);
  */
  tickTimeMilli = tick_duration * 1000;
  tickTimeMicro = (tick_duration * 1000000) - (tickTimeMilli * 1000);
  /*
  Serial.println(tickTimeMilli);
  Serial.println(tickTimeMicro);
  */
}

bool isNumber(char c){
  
  if(c >= 48 && c <= 57){
    return true;
  }
  
  return false;
}

#define TEMPO_NUM_LENGTH_MAX  3
void setTempo(uint16_t channel_index){
  char      c;
  char      temp[TEMPO_NUM_LENGTH_MAX + 1] = {};
  uint16_t tempo;
  
  for(uint16_t i=0;i < TEMPO_NUM_LENGTH_MAX;i++){
    c = getChar(channel_index);
    if(isNumber(c)){
      temp[i] = c;
    }else{
      trackData[channel_index].mml_index--;
      break;
    }
  }
  
  tempo = atoi(temp);
  
  if(tempo != 0){
    setTickTime(tempo);
  }
}

void setOctave(uint16_t channel_index){
  char c;
  
  c = getChar(channel_index);
  if(isNumber(c)){
    trackData[channel_index].octave = c - '0';
  }else{
    trackData[channel_index].mml_index--;
  }
  //Serial.println("octave:" + String(trackData[channel_index].octave));
}

#define MAX_OCTAVE  8
void incrementOctave(uint16_t channel_index){
  
  if(trackData[channel_index].octave < MAX_OCTAVE){
    trackData[channel_index].octave++;
  }
  //Serial.println("octave:" + String(trackData[channel_index].octave));
}

#define MIN_OCTAVE  0
void decrementOctave(uint16_t channel_index){
  
  if(trackData[channel_index].octave > MIN_OCTAVE){
    trackData[channel_index].octave--; 
  }
  //Serial.println("octave:" + String(trackData[channel_index].octave));
}

uint16_t getTicks(uint16_t duration){
  uint16_t value = BEAT_TICKS * 4;  // Whole Note
  return value / duration;
}


#define DEFAULT_TICKS_NUM_LENGTH_MAX  2
uint16_t defaultTicks;
void setDefaultTicks(uint16_t channel_index){
  char      c;
  char      temp[DEFAULT_TICKS_NUM_LENGTH_MAX + 1] = {};
  uint16_t  duration;
  
  for(uint16_t i=0;i < DEFAULT_TICKS_NUM_LENGTH_MAX;i++){
    c = getChar(channel_index);
    if(isNumber(c)){
      temp[i] = c;
    }else{
      trackData[channel_index].mml_index--;
      break;
    }
  }
  
  duration = atoi(temp);
  
  switch(duration){
    case 1:
    case 2:
    case 4:
    case 8:
    case 16:
    case 32:
      defaultTicks = getTicks(duration);
      break;
    default:
      break;
  }
}


bool isNoteNum(uint16_t duration){
    switch(duration){
      case 1:
      case 2:
      case 4:
      case 6:
      case 8:
      case 12:
      case 16:
      case 24:
      case 32:
      case 48:
        return true;
      default:
        return false;
    }
}

#define DOTTED_TICKS_MIN  30
uint16_t getDottedTicks(uint16_t channel_index ,uint16_t base_ticks){
  char c;
  uint16_t pre_ticks = base_ticks;
  uint16_t add_ticks;
  uint16_t ticks = base_ticks;
  
  if(base_ticks > DOTTED_TICKS_MIN){
    
    trackData[channel_index].mml_index--;
    while(pre_ticks > DOTTED_TICKS_MIN){
      c = getChar(channel_index);
      if(c == '.'){
        add_ticks = pre_ticks / 2;
        ticks = ticks + add_ticks;
        pre_ticks = add_ticks;
      }else{
        trackData[channel_index].mml_index--;
        break;
      }
    }
  }
  
  return ticks;
}

#define NOTE_DURATION_LENGTH_MAX  2
void addTieTicks(uint16_t channel_index,char note,char half_step){
  char c;
  uint16_t duration;
  uint16_t ticks;
  
  trackData[channel_index].mml_index--;
  while(trackData[channel_index].ticks < UINT32_MAX - (BEAT_TICKS * 4)){
    c = getChar(channel_index);
    
    if(c == '&'){
      
      c = getChar(channel_index);
      if(c == note){
        
        c = getChar(channel_index);
        
        if(c == half_step){
          c = getChar(channel_index);
        }
        
        if(isNumber(c)){
          trackData[channel_index].mml_index--;
          
          char temp[NOTE_DURATION_LENGTH_MAX + 1] = {};
          for(uint16_t i=0;i < NOTE_DURATION_LENGTH_MAX;i++){
            c = getChar(channel_index);
            if(isNumber(c)){
              temp[i] = c;
            }else{
              trackData[channel_index].mml_index--;
            }
          }
          
          duration = atoi(temp);
          if(isNoteNum(duration)){
            ticks = getTicks(duration);
          }else{
            return;
          }
          
          c = getChar(channel_index);
          if(c == '.'){
            ticks = getDottedTicks(channel_index,ticks);
          }else{
            trackData[channel_index].mml_index--;
          }
        }else if(c == '.'){
          ticks = getDottedTicks(channel_index,defaultTicks);
        }else{
          ticks = defaultTicks;
          trackData[channel_index].mml_index--;
        }
        
        trackData[channel_index].ticks = trackData[channel_index].ticks + ticks;
        
      }else{
        trackData[channel_index].mml_index--;
        break;
      }
      
    }else{
      trackData[channel_index].mml_index--;
      break;
    }
  }
  
  //Serial.println(String(note) + ":" + String(trackData[channel_index].ticks));
}
#define ATTACK                0
#define NOTE_INDEX_LENGTH_MAX 2
uint16_t phaseCarrier[CHANNELS];
uint16_t phaseModulator[CHANNELS];
bool setNote(uint16_t channel_index,char note){
  char c;
  char half_step = '\0';
  uint16_t index;
  uint16_t octave = trackData[channel_index].octave;
  
  if(note == 'R'){
    
    trackData[channel_index].frequency = 0;
    
  }else{
    
    if(octave == MIN_OCTAVE){
      switch(note){
      case 'A':
        index = 0;
        break;
      case 'B':
        index = 2;
        break;
      default:
        return false;
      }
    }else if(octave == MAX_OCTAVE){
      if(note == 'C'){
        index = NOTES - 1;
      }else{
        return false;
      }
    }else{
      
      switch(note){
        case 'C':
          index = 3;
          break;
        case 'D':
          index = 5;
          break;
        case 'E':
          index = 7;
          break;
        case 'F':
          index = 8;
          break;
        case 'G':
          index = 10;
          break;
        case 'A':
          index = 12;
          break;
        case 'B':
          index = 14;
          break;
      }
      
      index = ((octave - 1) * 12) + index;
    }
    
    c = getChar(channel_index);
    if(c == '+' || c == '#'){
      half_step = c;
      if(index < NOTES){
        index++;
      }else{
        return false;
      }
    }else if(c == '-'){
      half_step = c;
      if(index > 0){
        index--;
      }else{
        return false;
      }
    }else{
      trackData[channel_index].mml_index--;
    }
    
    trackData[channel_index].frequency = frequencyData[index];
    trackData[channel_index].frequency_modulator = trackData[channel_index].frequency * trackData[channel_index].frequency_ratio;
    phaseCarrier[channel_index] = 0;
    phaseModulator[channel_index] = 0;
  }
  //Serial.println("frequency:" + String(trackData[channel_index].frequency));
  //Serial.println("frequency_modulator:" + String(trackData[channel_index].frequency_modulator));
  
  c = getChar(channel_index);
  if(isNumber(c)){
    trackData[channel_index].mml_index--;
    
    char temp[NOTE_DURATION_LENGTH_MAX + 1] = {};
    for(uint16_t i=0;i < NOTE_DURATION_LENGTH_MAX;i++){
      c = getChar(channel_index);
      if(isNumber(c)){
        temp[i] = c;
      }else{
        trackData[channel_index].mml_index--;
      }
    }
    
    uint16_t duration = atoi(temp);
    /*
    if(channel_index == 4){
      Serial.println(String(note) + String(duration));
    }
    */
    if(isNoteNum(duration)){
      trackData[channel_index].ticks = getTicks(duration);
    }else{
      return false;
    }
    
    c = getChar(channel_index);
    if(c == '.'){
      trackData[channel_index].ticks = getDottedTicks(channel_index,trackData[channel_index].ticks);
      
      c = getChar(channel_index);
      if(c == '&'){
        addTieTicks(channel_index,note,half_step);
      }else{
        trackData[channel_index].mml_index--;
      }
      
    }else if(c == '&'){
      addTieTicks(channel_index,note,half_step);
    }else{
      trackData[channel_index].mml_index--;
    }
    
  }else if(c == '.'){
    trackData[channel_index].ticks = getDottedTicks(channel_index,defaultTicks);
    
    c = getChar(channel_index);
    if(c == '&'){
      addTieTicks(channel_index,note,half_step);
    }else{
      trackData[channel_index].mml_index--;
    }
    
  }else if(c == '&'){
    trackData[channel_index].ticks = defaultTicks;
    addTieTicks(channel_index,note,half_step);
  }else{
    trackData[channel_index].ticks = defaultTicks;
    trackData[channel_index].mml_index--;
  }
  
  /*
  if(channel_index == 0){
    Serial.println("ticks:" + String(trackData[channel_index].ticks));
  }
  */
  
  trackData[channel_index].tick = 1;
  trackData[channel_index].status_adsr = ATTACK;
  trackData[channel_index].status_adsr_modulator = ATTACK;
  trackData[channel_index].enable = true;
  
  return true;
}

void setTimbre(uint16_t channel_index){
  char c;
  
  c = getChar(channel_index);
  if(isNumber(c)){
    uint16_t index = c - '0';
    setTimbreData(channel_index,index);
  }else{
    trackData[channel_index].mml_index--;
  }
}


#define DEFAULT_VOLUME_NUM_LENGTH_MAX  3
void setBaseVolume(uint16_t channel_index){
  char      c;
  char      temp[DEFAULT_VOLUME_NUM_LENGTH_MAX + 1] = {};
  uint16_t  volume;
  
  for(uint16_t i=0;i < DEFAULT_VOLUME_NUM_LENGTH_MAX;i++){
    c = getChar(channel_index);
    if(isNumber(c)){
      temp[i] = c;
    }else{
      trackData[channel_index].mml_index--;
      break;
    }
  }
  
  volume = atoi(temp);
  
  if(volume >= 0 && volume <= 100){
    trackData[channel_index].base_volume = volume;
  }
  
}

bool setNoteOn;
void setTrackData(uint16_t channel_index){
  char c;
  bool finish;
  bool comment;
  
  while(finish == false){
    c = getChar(channel_index);
    if(comment){
      if(c == '\n'){
        comment = false;
      }
    }else{
      switch(c){
        case ';':
          comment = true;
          break;
        case 'T':
          setTempo(channel_index);
          break;
        case '>':
          incrementOctave(channel_index);
          break;
        case '<':
          decrementOctave(channel_index);
          break;
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'A':
        case 'B':
        case 'R':
          finish = setNote(channel_index,c);
          setNoteOn = true;
          break;
        case '@':
          setTimbre(channel_index);
          break;
        case 'O':
          setOctave(channel_index);
          break;
        case 'L':
          setDefaultTicks(channel_index);
          break;
        case 'V':
          setBaseVolume(channel_index);
          break;
        case '\0':
          finish = true;
          break;
      }
    }
    /*
    if(channel_index == 1){
      if(finish){
        Serial.println("finish");
      }else{
        Serial.println("next");
      }
    }
    */
  }
}

#define DEFAULT_TIMBRE      0
#define DEFAULT_OCTAVE      4
#define DEFAULT_BASE_VOLUME 75
void initializeTrackData(){

  setTickTime(0);
  defaultTicks = BEAT_TICKS;
  
  for(int i=0;i < CHANNELS;i++){
    trackData[i].mml_count = mmlFile[i].available();
    trackData[i].octave = DEFAULT_OCTAVE;
    trackData[i].base_volume = DEFAULT_BASE_VOLUME;
    trackData[i].base_volume_modulator = DEFAULT_BASE_VOLUME;
    setTimbreData(i,DEFAULT_TIMBRE);
  }
  
  initializeMmlBuffer();
  
  for(int i=0;i < CHANNELS;i++){
    setTrackData(i);
  }
  
}

uint16_t calculatePhaseModulation(uint16_t carrier_phase ,int16_t modulator_volume){
  int16_t phase;
  int16_t carrier_value;
  
  carrier_value = map(carrier_phase ,0 ,SAMPLE -1,-SAMPLE , SAMPLE -1);
  phase = map(carrier_value + modulator_volume,-128,127,0,63);
  
  return phase;
}

#define VOLUME_MAX 100
uint16_t pwmPin;
uint16_t sliceNum;
void setPwmLevel(){
  
  for(int i=0;i<CHANNELS;i++){
    if(trackData[i].waveform_type == NOISE_WAVE){
      phaseCarrier[i] += random(UINT16_MAX);
    }else{
      phaseCarrier[i] += trackData[i].frequency;
    }
    if(trackData[i].waveform_type_modulator == NOISE_WAVE){
      phaseModulator[i] += random(trackData[i].frequency_modulator);
    }else{
      phaseModulator[i] += trackData[i].frequency_modulator;
    }
  }
  
  uint16_t level = MAX_AMPLITUDE * CHANNELS;
  for(int i=0;i<CHANNELS;i++){
    if(trackData[i].enable){
      if(trackData[i].enable_modulator){
        level += waveForm[trackData[i].waveform_type][calculatePhaseModulation(phaseCarrier[i] >> 10, waveForm[trackData[i].waveform_type_modulator][phaseModulator[i] >> 10] * trackData[i].volume_modulator / VOLUME_MAX)] * trackData[i].volume / VOLUME_MAX;
      }else{
        level += waveForm[trackData[i].waveform_type][phaseCarrier[i] >> 10] * trackData[i].volume / VOLUME_MAX;
      }
    }
  }
  
  if(isEven(pwmPin)){
    pwm_set_chan_level(sliceNum, PWM_CHAN_A, level);
  }else{
    pwm_set_chan_level(sliceNum, PWM_CHAN_B, level);
  }
  
  pwm_clear_irq(sliceNum);
}


void setup() {
  Serial.begin(115200);
  delay(3000);
  
  initializeFrequency();
  initializeWaveform();
  
  if (!LittleFS.begin()) {
    Serial.println("LittleFS initialization failed.");
    while(true){
      delay(100);
    }
  }
  
  initializeTimbreData();
  initializeMmlFile();
  initializeTrackData();
  
  pinMode(LED_BUILTIN, OUTPUT);
  
  pwmPin = 0;
  
  gpio_set_function(pwmPin, GPIO_FUNC_PWM);
  
  sliceNum = pwm_gpio_to_slice_num(pwmPin);
  
  pwm_set_irq_enabled(sliceNum, true);
  
  irq_set_exclusive_handler(PWM_IRQ_WRAP, setPwmLevel);
  irq_set_enabled(PWM_IRQ_WRAP, true);
  
  //pwm_set_clkdiv(sliceNum, 7.8125);  // 16 Mhz(128 / 7.8125)
  //pwm_set_wrap(sliceNum, 256); // 62500 Hz
  //pwm_set_clkdiv(sliceNum, 4.0);  // 32 Mhz(128 / 4.0)
  //pwm_set_wrap(sliceNum, 512); // 62500 Hz
  //pwm_set_clkdiv(sliceNum, 2.0);  // 64 Mhz(128 / 2.0)
  //pwm_set_wrap(sliceNum, 1024); // 62500 Hz
  pwm_set_clkdiv(sliceNum, 1.0); // 128 Mhz(128 / 1.0)
  pwm_set_wrap(sliceNum, 2048);  // 62500 Hz
  
  
  if(isEven(pwmPin)){
    pwm_set_chan_level(sliceNum, PWM_CHAN_A, 0);
  }else{
    pwm_set_chan_level(sliceNum, PWM_CHAN_B, 0);
  }
  
  pwm_set_enabled(sliceNum, true);
  
  digitalWrite(LED_BUILTIN, HIGH);
  
  /*
  //#define SINE_WAVE     0
  //#define SQUARE_WAVE   1
  //#define TRIANGLE_WAVE 2
  //#define SAWTOOTH_WAVE 3
  potTrack = 0;
  //trackData[potTrack].waveform_type = SINE_WAVE;
  trackData[potTrack].ticks_attack = 50;
  trackData[potTrack].ticks_decay = 50;
  trackData[potTrack].volume_sustain = 50;
  trackData[potTrack].ticks_release = 3840;
  //trackData[potTrack].waveform_type_modulator = SINE_WAVE;
  trackData[potTrack].base_volume_modulator = 100;
  //trackData[potTrack].frequency_ratio = 1;
  trackData[potTrack].enable_envelope_modulator = true;
  trackData[potTrack].ticks_attack_modulator = 10;
  trackData[potTrack].ticks_decay_modulator = 300;
  trackData[potTrack].volume_sustain_modulator = 15;
  trackData[potTrack].ticks_release_modulator = 3840;
  */
}

void setMmlBuffer(uint16_t channel_index){
  
  if(trackData[channel_index].mml_index == 0){
    return;
  }
  uint16_t  readbyte;
  if(trackData[channel_index].mml_count <= trackData[channel_index].mml_index){
    readbyte = trackData[channel_index].mml_count;
  }else{
    readbyte = trackData[channel_index].mml_index;
  }
  
  uint16_t start_index;
  for(uint16_t i=0;i < MML_BUFFER_SIZE;i++){
    if(trackData[channel_index].mml_index + i >= MML_BUFFER_SIZE){
      break;
    }
    trackData[channel_index].mml_buffer[i] = trackData[channel_index].mml_buffer[trackData[channel_index].mml_index + i];
    start_index = i + 1;
  }
  
  uint8_t   buffer[MML_BUFFER_SIZE];
  mmlFile[channel_index].read(buffer,readbyte);
  
  uint16_t j = 0;
  for(uint16_t i=start_index;i < MML_BUFFER_SIZE;i++){
    if(j > readbyte - 1){
      trackData[channel_index].mml_buffer[i] = '\0';
      break;
    }else{
      trackData[channel_index].mml_buffer[i] = buffer[j];
      j++;
    }
  }
  
  /*
  for(int i=0;i < CHANNELS;i++){
    Serial.println("track_" + String(i));
    for(int j=0;j < MML_BUFFER_SIZE;j++){
      //int c = trackData[i].mml_buffer[j];
      //Serial.print("(");
      //Serial.print(c);
      //Serial.print(")");
      Serial.print(trackData[i].mml_buffer[j]);
    }
    Serial.println(":END");
  }
  while(true){delay(100);}
  */
  
  
  trackData[channel_index].mml_count = trackData[channel_index].mml_count - readbyte;
  trackData[channel_index].mml_index = 0;
}

void readPot(uint16_t channel_index){
  
  uint16_t value0 = analogRead(26);
  if(value0 <= 1024 / 20 * 1){
    trackData[channel_index].waveform_type = SINE_WAVE;
    trackData[channel_index].waveform_type_modulator = SINE_WAVE;
  }else if(value0 <= 1024 / 20 * 2){
    trackData[channel_index].waveform_type = SINE_WAVE;
    trackData[channel_index].waveform_type_modulator = SQUARE_WAVE;
  }else if(value0 <= 1024 / 20 * 3){
    trackData[channel_index].waveform_type = SINE_WAVE;
    trackData[channel_index].waveform_type_modulator = TRIANGLE_WAVE;
  }else if(value0 <= 1024 / 20 * 4){
    trackData[channel_index].waveform_type = SINE_WAVE;
    trackData[channel_index].waveform_type_modulator = SAWTOOTH_WAVE;
  }else if(value0 <= 1024 / 20 * 5){
    trackData[channel_index].waveform_type = SINE_WAVE;
    trackData[channel_index].waveform_type_modulator = NOISE_WAVE;
  }else if(value0 <= 1024 / 20 * 6){
    trackData[channel_index].waveform_type = SQUARE_WAVE;
    trackData[channel_index].waveform_type_modulator = SINE_WAVE;
  }else if(value0 <= 1024 / 20 * 7){
    trackData[channel_index].waveform_type = SQUARE_WAVE;
    trackData[channel_index].waveform_type_modulator = SQUARE_WAVE;
  }else if(value0 <= 1024 / 20 * 8){
    trackData[channel_index].waveform_type = SQUARE_WAVE;
    trackData[channel_index].waveform_type_modulator = TRIANGLE_WAVE;
  }else if(value0 <= 1024 / 20 * 9){
    trackData[channel_index].waveform_type = SQUARE_WAVE;
    trackData[channel_index].waveform_type_modulator = SAWTOOTH_WAVE;
  }else if(value0 <= 1024 / 20 * 10){
    trackData[channel_index].waveform_type = SQUARE_WAVE;
    trackData[channel_index].waveform_type_modulator = NOISE_WAVE;
  }else if(value0 <= 1024 / 20 * 11){
    trackData[channel_index].waveform_type = TRIANGLE_WAVE;
    trackData[channel_index].waveform_type_modulator = SINE_WAVE;
  }else if(value0 <= 1024 / 20 * 12){
    trackData[channel_index].waveform_type = TRIANGLE_WAVE;
    trackData[channel_index].waveform_type_modulator = SQUARE_WAVE;
  }else if(value0 <= 1024 / 20 * 13){
    trackData[channel_index].waveform_type = TRIANGLE_WAVE;
    trackData[channel_index].waveform_type_modulator = TRIANGLE_WAVE;
  }else if(value0 <= 1024 / 20 * 14){
    trackData[channel_index].waveform_type = TRIANGLE_WAVE;
    trackData[channel_index].waveform_type_modulator = SAWTOOTH_WAVE;
  }else if(value0 <= 1024 / 20 * 15){
    trackData[channel_index].waveform_type = TRIANGLE_WAVE;
    trackData[channel_index].waveform_type_modulator = NOISE_WAVE;
  }else if(value0 <= 1024 / 20 * 16){
    trackData[channel_index].waveform_type = SAWTOOTH_WAVE;
    trackData[channel_index].waveform_type_modulator = SINE_WAVE;
  }else if(value0 <= 1024 / 20 * 17){
    trackData[channel_index].waveform_type = SAWTOOTH_WAVE;
    trackData[channel_index].waveform_type_modulator = SQUARE_WAVE;
  }else if(value0 <= 1024 / 20 * 18){
    trackData[channel_index].waveform_type = SAWTOOTH_WAVE;
    trackData[channel_index].waveform_type_modulator = TRIANGLE_WAVE;
  }else if(value0 <= 1024 / 20 * 19){
    trackData[channel_index].waveform_type = SAWTOOTH_WAVE;
    trackData[channel_index].waveform_type_modulator = SAWTOOTH_WAVE;
  }else if(value0 <= 1024 / 20 * 20){
    trackData[channel_index].waveform_type = SAWTOOTH_WAVE;
    trackData[channel_index].waveform_type_modulator = NOISE_WAVE;
  }
  uint16_t value1 = analogRead(27);
  //trackData[channel_index].base_volume_modulator = map(value1,0,1023,0,UINT16_MAX);
  //trackData[channel_index].base_volume_modulator = map(value1,0,1023,0,100);
  trackData[channel_index].volume_sustain_modulator = map(value1,0,1023,0,100);
  uint16_t value2 = analogRead(28);
  //trackData[0].frequency_ratio = float(map(value2,0,1023,0,1000)) / 1000;
  trackData[channel_index].frequency_ratio = float(map(value2,0,1023,0,10000)) / 1000;
  //trackData[channel_index].frequency_ratio = float(map(value2,0,1023,0,100)) / 100;
  //Serial.println(String(trackData[0].waveform_type) + "," + String(trackData[0].waveform_type_modulator) + "," + String(trackData[0].base_volume_modulator) + "," + String(trackData[0].frequency_ratio,3));
  Serial.println(String(trackData[0].waveform_type) + "," + String(trackData[0].waveform_type_modulator) + "," + String(trackData[0].volume_sustain_modulator) + "," + String(trackData[0].frequency_ratio,3));
  
  trackData[channel_index].enable_modulator = true;
}

void tickDelay(unsigned long elapsed_time_micro){
  unsigned long correct_time_mill;
  unsigned long correct_time_micro;
  bool borrow = false;
  /*
  Serial.println("tickTimeMilli:" + String(tickTimeMilli));
  Serial.println("tickTimeMicro:" + String(tickTimeMicro));
  */
  correct_time_mill = elapsed_time_micro / 1000;
  correct_time_micro = elapsed_time_micro % 1000;
  /*
  Serial.println("correct_time_mill:" + String(correct_time_mill));
  Serial.println("correct_time_micro:" + String(correct_time_micro));
  */
  
  if(correct_time_mill > tickTimeMilli){
    Serial.println("Processing time exceeded.");
    correct_time_mill = tickTimeMilli;
    correct_time_micro = tickTimeMicro;
    
  }else if(correct_time_mill == tickTimeMilli){
    if(correct_time_micro > tickTimeMicro){
      Serial.println("Processing time exceeded.");
      correct_time_mill = tickTimeMilli;
      correct_time_micro = tickTimeMicro;
    }
  }else if(correct_time_mill < tickTimeMilli){
    if(correct_time_micro > tickTimeMicro){
      borrow = true;
    }
  }
  
  uint16_t wait_mill;
  uint16_t wait_micro;
  if(borrow){
    wait_mill = tickTimeMilli - correct_time_mill - 1;
    wait_micro = 1000 + tickTimeMicro - correct_time_micro;
  }else{
    wait_mill = tickTimeMilli - correct_time_mill;
    wait_micro = tickTimeMicro - correct_time_micro;
  }
  
  /*
  Serial.println("wait_mill:" + String(wait_mill));
  Serial.println("wait_micro:" + String(wait_micro));
  //while(true){delay(100);}
  */
  
  delay(wait_mill);
  delayMicroseconds(wait_micro);
}

#define DECAY   1
#define SUSTAIN 2
#define RELEASE 3
uint32_t channelIndex;
unsigned long startTime;
unsigned long completionTime;
unsigned long elapsedTime;
void loop() {
  
  //timeChecker.start();
  startTime = micros();
  
  setNoteOn = false;
  for(uint16_t i=0;i < CHANNELS;i++){
    if(trackData[i].enable == false){
      setTrackData(i);
      if(i == potTrack){
        readPot(i);
      }
    }
  }
  
  bool stopPlaying = true;
  for(uint32_t i=0;i<CHANNELS;i++){
    if(trackData[i].enable == true){
      stopPlaying = false;
      break;
    }
  }
  if(stopPlaying){
    Serial.println("Performance completed.");
    for(uint32_t i=0;i<CHANNELS;i++){
      mmlFile[i].close();
    }
    while(true){delay(100);}
  }
  
  if(setNoteOn == false){
    setMmlBuffer(channelIndex);
  }
  
  for(uint16_t i=0;i < CHANNELS;i++){
    
    if(trackData[i].frequency == 0){
      trackData[i].volume = 0;
    }else if(trackData[i].enable_envelope){
      
      if(trackData[i].status_adsr == ATTACK){
        trackData[i].volume = trackData[i].base_volume * (trackData[i].tick * 100 / trackData[i].ticks_attack) / 100;
        if(trackData[i].tick == trackData[i].ticks_attack){
          trackData[i].status_adsr = DECAY;
        }
      }else if(trackData[i].status_adsr == DECAY){
        if(trackData[i].base_volume > trackData[i].volume_sustain){
          trackData[i].volume = trackData[i].base_volume - ((trackData[i].base_volume - trackData[i].volume_sustain) * ((trackData[i].tick - trackData[i].ticks_attack) * 100 / trackData[i].ticks_decay) / 100);
        }
        if(trackData[i].tick == trackData[i].ticks_attack + trackData[i].ticks_decay){
          trackData[i].status_adsr = SUSTAIN;
          if(trackData[i].ticks <= trackData[i].ticks_attack + trackData[i].ticks_decay + trackData[i].ticks_release){
            trackData[i].status_adsr = RELEASE;
          }
        }
      }else if(trackData[i].status_adsr == SUSTAIN){
        if(trackData[i].tick == trackData[i].ticks - trackData[i].ticks_release - 1){
            trackData[i].status_adsr = RELEASE;
        }
      }else if(trackData[i].status_adsr == RELEASE){
        uint16_t volume_sustain;
        if(trackData[i].base_volume > trackData[i].volume_sustain){
          volume_sustain = trackData[i].volume_sustain;
        }else{
          volume_sustain = trackData[i].base_volume;
        }
        
        uint16_t ticks_attack_decay = trackData[i].ticks_attack + trackData[i].ticks_decay;
        uint16_t tick;
        if(trackData[i].ticks <= (ticks_attack_decay + trackData[i].ticks_release)){
          tick = trackData[i].tick - ticks_attack_decay;
        }else{
          tick = trackData[i].tick - (trackData[i].ticks - trackData[i].ticks_release);
        }
        trackData[i].volume = volume_sustain - (volume_sustain * (tick * 100 / trackData[i].ticks_release) / 100);
        
      }
      
    }else if(trackData[i].enable_envelope == false){
      trackData[i].volume = trackData[i].base_volume;
    }
    
    
    if(trackData[i].frequency_modulator == 0){
      trackData[i].volume_modulator = 0;
    }else if(trackData[i].enable_envelope_modulator){
      
      if(trackData[i].status_adsr_modulator == ATTACK){
        trackData[i].volume_modulator = trackData[i].base_volume_modulator * (trackData[i].tick * 100 / trackData[i].ticks_attack_modulator) / 100;
        if(trackData[i].tick == trackData[i].ticks_attack_modulator){
          trackData[i].status_adsr_modulator = DECAY;
        }
      }else if(trackData[i].status_adsr_modulator == DECAY){
        if(trackData[i].base_volume_modulator > trackData[i].volume_sustain_modulator){
          trackData[i].volume_modulator = trackData[i].base_volume_modulator - ((trackData[i].base_volume_modulator - trackData[i].volume_sustain_modulator) * ((trackData[i].tick - trackData[i].ticks_attack_modulator) * 100 / trackData[i].ticks_decay_modulator) / 100);
        }
        if(trackData[i].tick == trackData[i].ticks_attack_modulator + trackData[i].ticks_decay_modulator){
          trackData[i].status_adsr_modulator = SUSTAIN;
          if(trackData[i].ticks <= trackData[i].ticks_attack_modulator + trackData[i].ticks_decay_modulator + trackData[i].ticks_release_modulator){
            trackData[i].status_adsr_modulator = RELEASE;
          }
        }
      }else if(trackData[i].status_adsr_modulator == SUSTAIN){
        if(trackData[i].tick == trackData[i].ticks - trackData[i].ticks_release_modulator - 1){
            trackData[i].status_adsr_modulator = RELEASE;
        }
      }else if(trackData[i].status_adsr_modulator == RELEASE){
        uint16_t volume_sustain;
        if(trackData[i].base_volume_modulator > trackData[i].volume_sustain_modulator){
          volume_sustain = trackData[i].volume_sustain_modulator;
        }else{
          volume_sustain = trackData[i].base_volume_modulator;
        }
        
        uint16_t ticks_attack_decay = trackData[i].ticks_attack_modulator + trackData[i].ticks_decay_modulator;
        uint16_t tick;
        if(trackData[i].ticks <= (ticks_attack_decay + trackData[i].ticks_release_modulator)){
          tick = trackData[i].tick - ticks_attack_decay;
        }else{
          tick = trackData[i].tick - (trackData[i].ticks - trackData[i].ticks_release_modulator);
        }
        trackData[i].volume_modulator = volume_sustain - (volume_sustain * (tick * 100 / trackData[i].ticks_release_modulator) / 100);
        
      }
      
    }else if(trackData[i].enable_envelope_modulator == false){
      trackData[i].volume_modulator = trackData[i].base_volume_modulator;
    }
  }
  
  
  for(uint32_t i=0;i<CHANNELS;i++){
    if(trackData[i].tick >= trackData[i].ticks){
      trackData[i].enable = false;
    }
  }
  
  
  for(uint32_t i=0;i<CHANNELS;i++){
    if(trackData[i].enable){
      trackData[i].tick++;
    }
  }
  
  channelIndex++;
  if(channelIndex >= CHANNELS){
    channelIndex = 0;
  }
  
  completionTime = micros();
  if(completionTime >= startTime){
    elapsedTime = completionTime - startTime;
  }else{
    elapsedTime = (ULONG_MAX - startTime) + completionTime + 1;
  }
  
  /*
  if(elapsedTime > 300){
    Serial.println("elapsed:" + String(elapsedTime));
    Serial.println("setNoteOn:" + String(setNoteOn));
  }
  */
  
  tickDelay(elapsedTime);
  
  //Serial.println(timeChecker.stop());
}
