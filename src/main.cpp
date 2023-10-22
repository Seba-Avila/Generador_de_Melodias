#include <Arduino.h>
#include "arduinoFFT.h"
#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT; //Creo objeto SerialBT para las funciones de la libreria BluetoothSerial
arduinoFFT FFT;           //Creo objeto FFT para las funciones de la libreria arduinoFFT

const uint16_t samples = 2048; //Cantidad de muestras para el calculo de la FFT. Es necesario que sea potencia de 2
const double samplingFrequency = 1500; //Hz, debe ser menor a 10000 debido al CAD

unsigned int sampling_period_us; //Periodo de muestreo de la señal entrante para la FFT
unsigned long microseconds;      //Esta variable funciona como marca de tiempo para muestrear en tiempo real

/*Los siguientes son vectores donde guardar las muestras de la señal 
en su parte real y su parte imaginaria. Para el calculo realizado solo 
asignaremos muestras a la parte real, la parte imaginaria se le asigna un valor de 0*/
double vReal[samples]; 
double vImag[samples];

char buffer[5]; //Declaro como vector porque la funcion readBytesUntil() solo toma de argumento un vector de tipo char o byte
byte cache;     //Declaro como byte la variable donde guardar los datos devueltos por la funcion readBytesUntil(), que solamente corresponde con la cantidad de bytes leidos

//Declaraciones Modos de Operacion
void Melodias(); //Funcion correspondiente a la pantalla del Generador de Melodias. Aqui se espera la eleccion de una nota mientras no se vuelva a la pantalla inicial
void Detector(); //Funcion correspondiente a la pantalla del Detector de Notas. Aqui se espera la confirmacion del inicio del muestreo y reproduccion de la señal del microfono



//Declaraciones Accion
/*Lo que antes eran 200 muestras para una frecuencia de PWM de 200 KHz, 
ahora son 50 muestras para una frecuencia de PWM de 20 KHz*/
byte muestras[50] PROGMEM={127,
142,
158,
173,
188,
201,
213,
224,
234,
241,
247,
251,
253,
253,
251,
247,
241,
234,
224,
213,
201,
188,
173,
158,
142,
127,
111,
95,
80,
65,
52,
40,
29,
19,
12,
6,
2,
0,
0,
2,
6,
12,
19,
29,
40,
52,
65,
80,
95,
111}; 

/*Las siguientes son las funciones donde se ejecutara cada PWM en base a las muestras antes definidas*/
void SenoidalC();
void SenoidalDb();
void SenoidalD();
void SenoidalEb();
void SenoidalE();
void SenoidalF();
void SenoidalGb();
void SenoidalG();
void SenoidalAb();
void SenoidalA();
void SenoidalBb();
void SenoidalB();

void Microfono(); //Funcion que ejecuta la accion del Detector de notas.



void setup() {
  Serial.begin(115200);
  pinMode(27, OUTPUT);  //Salida para LED indicador del modo Generador
  pinMode(12, OUTPUT);  //Salida para LED indicador del modo Detector

  SerialBT.begin("ESP32_Generador"); //Nombre del dispositivo Bluetooth
  Serial.println("The device started, now you can pair it with bluetooth!"); //Mensaje de aviso

  
  
  
  /*Más aclaraciones en archivo README*/
    
  ledcSetup(0, 20000, 8); //Aqui configuro la frecuencia y resolucion del canal PWM que voy a utilizar
  ledcAttachPin(32 , 0);  //Aqui asigno el canal configurado a un pin de la placa
  
  ledcWrite(0, 255);      //Pongo la salida PWM en estado inactivo (debido al optoacoplador, esto es nivel lógico ALTO) 

  Serial.println("\n\nInicio");  
  while(!SerialBT.available()){                   //Mientras no haya datos en el buffer serial (es decir, mientras que por Bluetooth no se reciba nada) espera
    Serial.println("Esperando...");
    delay(3000);
  }
  cache=SerialBT.readBytesUntil('\n', buffer, 3); //Aclaraciones sobre readBytesUntil() en el archivo README
}


/*En el void loop aparece el switch principal, donde se determinará, en base a lo decidido
por el usuario, el ingreso a uno u otro modo de operacion*/

void loop() {
  switch(buffer[0]){
    case 'G':
      Melodias();
      break;
    case 'T':
      Detector();
      break;
  }
    
  /*Cuando se rompa el bucle principal de algun modo de operacion, volverá al loop, 
  saldra del switch y esperará de vuelta la elección de modo*/
    
  Serial.println("\n\nVuelta a inicio");
  while(!SerialBT.available()){                   
    Serial.println("Esperando...");
    delay(3000);
    }
    cache=SerialBT.readBytesUntil('\n', buffer, 3);
}



//Modos de Operacion


void Melodias(){
    Serial.println("\n\n\nMelodias");
    digitalWrite(27, HIGH);  //Enciendo LED indicador del modo

    /*El siguiente bucle se va a ejecutar mientras la lectura no sea V, caracter que se 
    envía al presionar el boton de retorno para volver a la pantalla principal en la app*/
    
    while(buffer[0]!='V'){
        while(!SerialBT.available()){                          //Aqui espero la llegada de datos desde la app
            Serial.println("Esperan2...");
            delay(2000);
        }
        cache=SerialBT.readBytesUntil('\n', buffer, 3);
    
        /*En base a lo recibido, ingreso al switch del Generador para determinar qué PWM reproduzco a la salida*/
        
        switch(buffer[0]){
            case 'C':
                SenoidalC();
                break;
            case 'c':
                SenoidalDb();
                break;
            case 'D':
                SenoidalD();
                break;
            case 'd':
                SenoidalEb();
                break;
            case 'E':
                SenoidalE();
                break;
            case 'F':
                SenoidalF();
                break;
            case 'f':
                SenoidalGb();
                break;
            case 'G':
                SenoidalG();
                break;
            case 'g':
                SenoidalAb();
                break;
            case 'A':
                SenoidalA();
                break;
            case 'a':
                SenoidalBb();
                break;
            case 'B':
                SenoidalB();
                break;
        }
    }
    digitalWrite(27, LOW);  //Si salgo del bucle, es decir, si se presionó el boton de retorno, apago el LED indicador ya que salgo del modo
}

void Detector(){
    digitalWrite(12, HIGH);  //Enciendo LED indicador del modo

    while(buffer[0]!='V'){                           //Mientras no se presione el botón de volver
            Serial.println("\n\n\nDetector");
            Serial.println("Enter para iniciar");
            while(!SerialBT.available()){                   //se espera un dato
                Serial.println("Esperan3...");
                delay(2000);
            }
            cache=SerialBT.readBytesUntil('\n', buffer, 3);
            if(buffer[0]=='E'){                             //Si es E, es decir, si se presiona el boton "Comienzo"
                Microfono();                                //ejecuto la funcion Microfono
            }
    }
    digitalWrite(12, LOW);  //Si salgo del bucle, es decir, si se presiona el botón de volver, apago el LED ya que salgo del modo
}


//ACCION

/*Aclaraciones en archivo README*/

void SenoidalC(){
    Serial.println("Senoidal C");
    while(!SerialBT.available()){
        for (int j = 0; j < 50; j++) {
            ledcWrite(0, muestras[j]);
            delayMicroseconds(300.69);
        }
    }
    ledcWrite(0, 255);                    //al romperse el bucle, la salida se pone a 1 (estado inactivo para el optoacoplador).
}

void SenoidalDb(){
  Serial.println("Senoidal c");
    while(!SerialBT.available()){
        for (int j = 0; j < 50; j++) {
            ledcWrite(0, muestras[j]);
            delayMicroseconds(282.85);
        }
    }
    ledcWrite(0, 255);
}

void SenoidalD(){
  Serial.println("Senoidal D");
    while(!SerialBT.available()){
        for (int j = 0; j < 50; j++) {
            ledcWrite(0, muestras[j]);
            delayMicroseconds(266.97);
        }
    }
    ledcWrite(0, 255);
}

void SenoidalEb(){
  Serial.println("Senoidal d");
    while(!SerialBT.available()){
        for (int j = 0; j < 50; j++) {
            ledcWrite(0, muestras[j]);
            delayMicroseconds(249.41);
        }
    }
    ledcWrite(0, 255);
}

void SenoidalE(){
  Serial.println("Senoidal E");
    while(!SerialBT.available()){
        for (int j = 0; j < 50; j++) {
            ledcWrite(0, muestras[j]);
            delayMicroseconds(236.90);
        }
    }
    ledcWrite(0, 255);
}

void SenoidalF(){
  Serial.println("Senoidal F");
    while(!SerialBT.available()){
        for (int j = 0; j < 50; j++) {
            ledcWrite(0, muestras[j]);
            delayMicroseconds(222.88);
        }
    }
    ledcWrite(0, 255);
}

void SenoidalGb(){
  Serial.println("Senoidal f");
    while(!SerialBT.available()){
        for (int j = 0; j < 50; j++) {
            ledcWrite(0, muestras[j]);
            delayMicroseconds(210.39);
        }
    }
    ledcWrite(0, 255);
}

void SenoidalG(){
  Serial.println("Senoidal G");
    while(!SerialBT.available()){
        for (int j = 0; j < 50; j++) {
            ledcWrite(0, muestras[j]);
            delayMicroseconds(197.08);
        }
    }
    ledcWrite(0, 255);
}

void SenoidalAb(){
  Serial.println("Senoidal g");
    while(!SerialBT.available()){
        for (int j = 0; j < 50; j++) {
            ledcWrite(0, muestras[j]);
            delayMicroseconds(185.31);
        }
    }
    ledcWrite(0, 255);
}

void SenoidalA(){
  Serial.println("Senoidal A");
    while(!SerialBT.available()){
        for (int j = 0; j < 50; j++) {
            ledcWrite(0, muestras[j]);
            delayMicroseconds(174.81);
        }
    }
    ledcWrite(0, 255);
}

void SenoidalBb(){
  Serial.println("Senoidal a");
    while(!SerialBT.available()){
        for (int j = 0; j < 50; j++) {
            ledcWrite(0, muestras[j]);
            delayMicroseconds(163.94);
        }
    }
    ledcWrite(0, 255);
}

void SenoidalB(){
  Serial.println("Senoidal B");
    while(!SerialBT.available()){
        for (int j = 0; j < 50; j++) {
            ledcWrite(0, muestras[j]);
            delayMicroseconds(155.60);
        }
    }
    ledcWrite(0, 255);
}




void Microfono(){
    sampling_period_us = round(1000000*(1.0/samplingFrequency)); //Calculo periodo de muestreo en microsegundos en base a la frecuencia


    while(buffer[0] != 'P' && buffer[0] != 'V'){  //Mientras los datos recibidos no correspondan al boton de Parada o Volver
        while(!SerialBT.available()){             //ejecuto la siguiente acción mientras no se reciban datos
            microseconds = micros();              //Marco el tiempo actual en microsegundos
            for(int i=0; i<samples; i++){         //El bucle se repite según cuantas muestras haya
                vReal[i] = analogRead(36);        //La lectura se guarda solo como parte real
                vImag[i] = 0;
                while(micros() - microseconds < sampling_period_us){} //Mientras no haya pasado el periodo de muestreo, espero
                microseconds += sampling_period_us;                   //Una vez se cumple el periodo de muestreo, sumo su valor para que la cuenta este actualizada en el siguiente ciclo del for
            }
            FFT = arduinoFFT(vReal, vImag, samples, samplingFrequency);  //Asigno al objeto FFT los parametros necesarios para el calculo
            FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);  //Establezco ventana de muestreo así como el tipo de FFT a aplicar a los datos asignados
            FFT.Compute(FFT_FORWARD);   //Computo la FFT
            FFT.ComplexToMagnitude();   //Obtengo magnitudes de los valores complejos arrojados por el calculo de la FFT
            double x = FFT.MajorPeak(); //En base a las magnitudes obtengo la amplitud de cada frecuencia. Asigno a la variable x la frecuencia de mayor amplitud (pico o fundamental)
            Serial.println(x, 6);       //Imprimo el valor de x con una precisión de 6 decimales
            int n = (int) x;            //Asigno a n unicamente la parte entera de x
            Serial.println(n);          //Imprimo n en monitor serial
            SerialBT.print(n);          //Envio valor de n por Bluetooth para que la app lo reproduzca

            

            /*Los siguientes condicionales verifican si la fundamental captada corresponde 
            a alguna de las 12 notas en 5 octavas distintas (tanto para cubrir varias frecuencias
            como para considerar el error del calculo de la FFT en el que a veces otorga como fundamental
            una frecuencia que es multiplo exacto de la buscada, es decir, un armónico)*/

            if(n==32 || n==65 || n==130 || n==261 || n==523){
                SerialBT.print("C");
                SenoidalC();
            }

            if(n==34 || n==69 || n==138 || n==277 || n==554){
                SerialBT.print("c");
                SenoidalDb();
            }

            if(n==36 || n==73 || n==146 || n==293 || n==587){
                SerialBT.print("D");
                SenoidalD();
            }

            if(n==38 || n==77 || n==155 || n==311 || n==622){
                SerialBT.print("d");
                SenoidalEb();
            }

            if(n==41 || n==82 || n==164 || n==329 || n==659){
                SerialBT.print("E");
                SenoidalE();
            }

            if(n==43 || n==87 || n==174 || n==349 || n==698){
                SerialBT.print("F");
                SenoidalF();
            }

            if(n==46 || n==92 || n==184 || n==369 || n==739){
                SerialBT.print("f");
                SenoidalGb();
            }

            if(n==48 || n==97 || n==195 || n==391 || n==783){
                SerialBT.print("G");
                SenoidalG();
            }

            if(n==51 || n==103 || n==207 || n==415 || n==830){
                SerialBT.print("g");
                SenoidalAb();
            }

            if(n==55 || n==110 || n==220 || n==440 || n==880){
                SerialBT.print("A");
                SenoidalA();
            }

            if(n==58 || n==116 || n==233 || n==466 || n==932){
                SerialBT.print("a");
                SenoidalBb();
            }

            if(n==61 || n==123 || n==246 || n==493 || n==987){
                SerialBT.print("B");
                SenoidalB();
            }

            delay(500); 
        }
        /*Leo el dato que rompió el bucle. Si es V, romperá tambien el bucle principal de Detector. 
        Si es P, romperá solo este bucle y volverá a esperar que se presione boton de Comienzo. 
        Si es p, es decir, se toca el boton de parada del motor, simplemente frena el mismo y volverá a muestrear*/
        cache=SerialBT.readBytesUntil('\n', buffer, 3); 
    }
}
