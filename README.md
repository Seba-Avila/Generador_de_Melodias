# Prototipo_Generador_de_Melodias

-MUESTRAS:

Las muestras fueron obtenidas del siguiente código, aplicable para todas las frecuencias al mantener la misma relacion entre frecuencia de nota (F1), frecuencia de muestreo (Fs=Frecuencia nota*muestras), cantidad de muestras (n1) y frecuencia de PWM:

```
float F1 = 44;        //Frecuencia de la nota
double Fs = 8800;     //Fs=Frecuencia nota*muestras          
int n1 = 200;         //numero de muestras
double t=0;           //instante de muestra
byte samples1[200];   //vector donde guardar las muestras

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

Habiendo verificado las frecuencias a la que mejor trabajan las PWM de las notas (principalmente a la hora de filtrarlas), determinamos que, al configurarlas, hacerlo con una frecuencia unas 15 veces mayor a la de muestreo otorgaba resultados óptimos para el filtro que construimos. Para iniciar, elegimos notas unicamente de la primer octava, con frecuencias de 32,70 Hz a 61,74 Hz. Redondeando los valores, las frecuencias de muestreo correspondientes van de 6600 Hz a 12400 Hz, mientras que las de PWM van de 99000 Hz a 186000 Hz. Por eso, utilizando un canal a 200000 Hz tenemos más que suficiente velocidad para recorrer las muestras de todas las notas. La resolución es la misma que la de muestreo, 8 bits.

-LECTURA DE DATOS:

A la hora de leer los datos recibidos por Bluetooth Utilizo readBytesUntil() porque la app y otros monitores utilizados para pruebas envian los datos con caracteres adicionales correspondientes al salto de renglon \n que no es informacion util, pero debo quitarlo del buffer serial para luego volver a entrar sin problemas al bucle while(!Serial.available()){}. Para asegurarnos de que el buffer fue limpiado, aun poniendo el limite de lectura en \n, dejamos que se lean hasta 3 bytes. Los datos se guardan en un vector tipo char que también llamamos buffer, teniendo en su elemento [0] el dato valido para los condicionales.
La variable cache guarda como dato el numero de bytes que llegan.

-SENOIDALES Y RECORRIDO DE MUESTRAS:

Al entrar a la funcion propia de cada nota, en todos los casos voy a ingresar en un bucle que no se romperá a menos que se reciba un dato desde la app. Ese dato puede ser el caracter de otra nota o el caracter V para la vuelta a la pantalla principal, y será leído en la funcion principal del Generador, ya que: si es V, el bucle dara una vuelta más y se cerrará; si es una nota, entrará a algun otro caso del switch; si es otro caracter, generado por el boton de pausa, el dato sera leído, no se romperá el bucle ni entrará a ningún caso del switch y, como se limpia el buffer, volverá a quedarse en espera de un dato que ejecute una acción.

Recorremos el vector de muestras con un for, reproduciendo la PWM con un ciclo de trabajo correspondiente a cada muestra, agregando un delay equivalente a un ciclo de la frecuencia de muestreo menos 7 uS (que toma en ejecutarse la función ledcWrite):
Delay = (1/Fmuestreo) - 7x10^(-6) = (1/(Fnota*200)) - 7x10^(-6)
