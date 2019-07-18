#include "EmonLib.h"
#include <KNoTThing.h>

EnergyMonitor emon1; //CRIA UMA INSTÂNCIA
EnergyMonitor SCT013;

KNoTThing thing;

#define THING_NAME "SmartEnergy"

#define CURRENT_SENSOR_ID     1
#define CURRENT_SENSOR_NAME   "Corrente(mA)"
#define TENSION_SENSOR_ID     2
#define TENSION_SENSOR_NAME   "Tensão(V)"
#define STATE_ID              3
#define STATE_NAME            "Estado da rede"

#define circuito 2
#define pinSCT A0
#define pinST A1

#define VOLT_CAL 440.5 //VALOR DE CALIBRAÇÃO (DEVE SER AJUSTADO EM PARALELO COM UM MULTÍMETRO)

//variáveis para a monitoração de tensão e corrente
static double leitura = 0;
static int ampere = 0;
static int32_t Vrms = 220;
static uint8_t state = true;


static int current_read(int32_t *val, int32_t *multiplier){
    leitura = SCT013.calcIrms(1480); // Calcula o valor da Corrente
    ampere = (int)(leitura*1000);
   
    if(ampere <= 180){
      ampere = 0;
    }
     Serial.println("lendo corrente");
    *multiplier = 1;
    *val =  ampere;   // Calcula o valor da Corrente
    
    return 0;
}

static int tension_read(int32_t *val, int32_t *multiplier){
    emon1.calcVI(20,2000); //FUNÇÃO DE CÁLCULO (20 SEMICICLOS, TEMPO LIMITE PARA FAZER A MEDIÇÃO)   
    
    Vrms =(int)emon1.Vrms;
    *multiplier = 1;
    *val = Vrms;   // Calcula o valor da tensão


    return 0;
}

static int state_read (uint8_t *val){
    *val = state;
    return 0;
}

static int state_write (uint8_t *val){
    state = *val;

    if(state == true){
        digitalWrite(circuito, HIGH);
        Serial.println("LIGADO");
    }if(state == false){
        digitalWrite(circuito, LOW);
        Serial.println("DESLIGADO");
    }

    return 0;
}

void setup(){

    pinMode(pinSCT,INPUT);
    pinMode(pinST,INPUT);
    pinMode(circuito, OUTPUT);
    digitalWrite(circuito, HIGH);
    
    SCT013.current(pinSCT, 38);
    emon1.voltage(pinST, VOLT_CAL, 1.7); //PASSA PARA A FUNÇÃO OS PARÂMETROS (PINO ANALÓGIO / VALOR

    Serial.begin(9600);

    thing.init(THING_NAME);

    thing.registerIntData(CURRENT_SENSOR_NAME, CURRENT_SENSOR_ID, KNOT_TYPE_ID_CURRENT, KNOT_UNIT_CURRENT_MA, current_read, NULL);
    thing.registerIntData(TENSION_SENSOR_NAME , TENSION_SENSOR_ID, KNOT_TYPE_ID_VOLTAGE, KNOT_UNIT_VOLTAGE_V, tension_read, NULL);
    
    thing.registerDefaultConfig(CURRENT_SENSOR_ID, KNOT_EVT_FLAG_TIME, 5, 0, 0, 0, 0);
    thing.registerDefaultConfig(TENSION_SENSOR_ID, KNOT_EVT_FLAG_TIME, 5, 0, 0, 0, 0);

    thing.registerBoolData(STATE_NAME, STATE_ID, KNOT_TYPE_ID_SWITCH, KNOT_UNIT_NOT_APPLICABLE, state_read, state_write);
    thing.registerDefaultConfig(STATE_ID, KNOT_EVT_FLAG_CHANGE, 0, 0, 0, 0, 0);

    Serial.println(F(THING_NAME));
}
 
void loop(){
    thing.run(); 
}
