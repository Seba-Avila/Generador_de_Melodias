# Generador_de_Melodias

-MUESTRAS:

Las muestras fueron obtenidas del siguiente código, aplicable para todas las frecuencias al mantener la misma relacion entre frecuencia de nota (F1), frecuencia de muestreo (Fs=Frecuencia nota*muestras), cantidad de muestras (n1) y frecuencia de PWM:

```
float F1 = 44;        //Frecuencia de la nota
double Fs = 2200;     //Fs=Frecuencia nota*muestras          
int n1 = 50;         //numero de muestras
double t=0;           //instante de muestra
byte samples1[50];   //vector donde guardar las muestras

for (int m = 0; m < n1; m++){
    t = (double) ((m/Fs)*1000);                                       //multiplico el valor de t ya que, al ser tan pequeño, sus decimales se pierden y figura como 0.
    samples1[m] = (byte) (127.0 * sin(2 * PI * 0.044 *  t) + 127.0 ); //calculo el valor de la senoidal en cada instante de t. Notar que la frecuencia fue reducida en proporcion a la cantidad de veces que se aumento t para que el calculo sea el mismo.
    if(m==0){Serial.print("muestra1:");}                              //en las siguientes lineas imprimo los valores obtenidos para poder copiarlos.
    Serial.print(samples1[m]);
    Serial.println(",");
  }
```

Las muestras fueron guardadas en la memoria flash, de programa, con tal de no ocupar espacio de RAM y asegurarlas en una memoria no volatil.

-CONFIGURACION PWM:

La frecuencia de muestreo está dada en base a la cantidad de muestras establecidas para la señal a muestrear (la senoidal generada anteriormente). Inicialmente elegimos un número considerable de muestras (primero 500, luego 400 y luego 200) para asegurarnos de que la representación de la señal sería fiel a la original. Sin embargo, una cantidad grande de muestras conlleva una gran frecuencia de muestreo, y para lograr representar las muestras de manera correcta a traves de la PWM es necesario que, en el tiempo que tarda en sucederse una muestra a otra, la señal PWM sea capaz de realizar varios ciclos con el ciclo de trabajo indicado por la muestra con tal de que el valor medio resultante sea perceptible por la carga. Esto implica que la PWM sea considerablemente grande respecto de la frecuencia de muestreo (mínimamente, el doble). Utilizando 50 muestras para notas de la segunda octava cuyas frecuencias van desde los 65 Hz a los 123 Hz (obteniendo frecuencias de muestreo de entre 3.25 KHz a 6.15 KHz), obtuvimos que una PWM de 20 KHz sería suficiente para representar todas las señales, evitando, además, problemas con la velocidad de conmutación de los componentes de potencia para el motor (problema que se nos habia presentado en anteriores versiones, cuando nuestras frecuencias de muestreo eran muy altas y, por ende, las PWM que comandaban al motor también).

-LECTURA DE DATOS:

A la hora de leer los datos recibidos por Bluetooth Utilizo readBytesUntil() porque la app y otros monitores utilizados para pruebas envian los datos con caracteres adicionales correspondientes al salto de renglon \n que no es informacion util, pero debo quitarlo del buffer serial para luego volver a entrar sin problemas al bucle while(!Serial.available()){}. Para asegurarnos de que el buffer fue limpiado, aun poniendo el limite de lectura en \n, dejamos que se lean hasta 3 bytes. Los datos se guardan en un vector tipo char que también llamamos buffer, teniendo en su elemento [0] el dato valido para los condicionales.
La variable cache guarda como dato el numero de bytes que llegan.

-SENOIDALES Y RECORRIDO DE MUESTRAS:

A las funciones encargadas de representar las senoidales se puede acceder tanto del modo Generador, como del modo Detector. Al entrar a la funcion propia de cada nota, en todos los casos voy a ingresar en un bucle que no se romperá a menos que se reciba un dato desde la app. Desde el modo Generador sucede lo siguiente: ese dato puede ser el caracter de otra nota o el caracter V para la vuelta a la pantalla principal, y será leído en la funcion principal del Generador, ya que: si es V, el bucle dara una vuelta más y se cerrará; si es una nota, entrará a algun otro caso del switch; si es otro caracter, generado por el boton de pausa, el dato sera leído, no se romperá el bucle ni entrará a ningún caso del switch y, como se limpia el buffer, volverá a quedarse en espera de un dato que ejecute una acción. Desde el modo Detector: ese dato puede ser V, P o p: si es V, se corta la salida al motor, se rompe el bucle en el que se aplica la FFT, y se rompe el bucle principal que pide por el inicio del micrófono; si es P, se corta la salida al motor, se rompe el bucle en el que se aplica la FFT, pero se queda en el bucle principal pidiendo por la reactivación del micrófono para medir; si es p, significa que se presiono el boton de parada del motor (que arranca luego de detectar una nota; mientras este activo el motor, la señal del micrófono no se procesa), por lo que lo unico que realiza es el corte de la salida al motor, ya que, una vez leido el dato, el buffer se limpia y, al ser un caracter que no corresponde con ninguna de las condiciones que rompen el bucle, simplemente vuelve a entrar para seguir procesando la señal de audio.


Para la representación de las senoidales, recorremos el vector de muestras con un for, reproduciendo la PWM con un ciclo de trabajo correspondiente a cada muestra, agregando un delay equivalente a un ciclo de la frecuencia de muestreo menos 7 uS (tiempo que toma en ejecutarse la función ledcWrite):
Delay = (1/Fmuestreo) - 7x10^(-6) = (1/(Fnota*50)) - 7x10^(-6)
