# Guion de exposición — Estación Central

Duración aproximada: 7–9 minutos. Las indicaciones entre corchetes son acciones; el texto entre comillas es lo que puedes decir.

## 1. Compilar en Visual Studio — 45 segundos

[Abre `EstacionTren.sln`. Selecciona **Release / Win32**. Muestra `#include <GL/glut.h>` al comienzo de `estacion_tren.cpp`.]

«Este proyecto es una simulación de una estación de tren desarrollada en C++ con OpenGL y GLUT. GLUT crea la ventana y recibe los controles; OpenGL dibuja los objetos, aplica las proyecciones y calcula la iluminación.»

[Pulsa **Ctrl+Mayús+B** y espera “Compilación correcta”. Después, **Ctrl+F5**.]

«Compilo en Win32 porque la biblioteca GLUT de este proyecto es de 32 bits. La escena y los sonidos se generan desde el código.»

Si la versión anterior está abierta, ciérrala antes de compilar para evitar el error LNK1168. Si Visual Studio detecta cambios en el proyecto, acepta recargarlos.

## 2. Presentar la simulación — 45 segundos

[Deja funcionar la vista inicial. Pulsa **V** una vez para ver todo el entorno; después **R**.]

«La escena incluye las estaciones Central y Parque. Hay dos trenes de cuatro vagones, tres andenes en Central, pasajeros, una pasarela, edificios, autos y un parque. Los trenes circulan automáticamente entre ambas estaciones.»

«Cada trayecto dura 27 segundos y cada parada, 9 segundos. El tren acelera al salir, frena al llegar, abre las puertas y las cierra antes de reanudar el viaje.»

[Pulsa **F** una vez para seguir el tren. Deja que llegue a Parque si quieres mostrar las puertas. Al iniciar el proyecto faltan aproximadamente 22 segundos para esa llegada.]

«La posición depende del tiempo transcurrido, no de avanzar una cantidad fija en cada imagen. Eso permite mantener la velocidad de simulación aunque cambie la frecuencia de dibujo.»

## 3. Perspectiva y ortográfica — 1 minuto

[Pulsa **R**, después **Espacio** para congelar el movimiento. Pulsa **V** tres veces para mirar a lo largo de las vías. Alterna **P**, **O**, **P**.]

«En perspectiva, los objetos lejanos se ven más pequeños. Las columnas tienen el mismo tamaño real, pero su tamaño aparente disminuye con la distancia. También vemos cómo los rieles convergen.»

«Al cambiar a ortográfica, mantengo la misma cámara y la misma escena. Los objetos iguales conservan su tamaño proyectado y los rieles permanecen paralelos. Esta proyección resulta útil para planos y vistas técnicas.»

«El cambio está en `aplicarProyeccion()`: utilizo `gluPerspective()` o `glOrtho()`. La cámara se configura por separado con `gluLookAt()`.»

[Pulsa **7** para mostrar que también alterna el modo. Pulsa **R** para continuar.]

## 4. Mañana, tarde y noche — 1 minuto

[Pulsa **J**, **E** y **N**, dejando unos segundos entre cada tecla.]

«Por la mañana predomina una luz clara; por la tarde, la dirección y el color del sol cambian hacia tonos cálidos. Por la noche, el ambiente se oscurece y destacan las lámparas y las ventanas.»

«OpenGL combina tres componentes: la ambiental aporta iluminación general; la difusa depende de la orientación de la superficie; y la especular genera los brillos que cambian con el punto de vista.»

[Pulsa **C**.]

«También existe un ciclo automático: un día completo dura tres minutos. Se interpolan los colores y las intensidades para que el cambio sea gradual.»

## 5. Rayos, normales y brillo — 1 minuto

[Pulsa **K** para acercarte al laboratorio; pulsa **N**.]

«La esfera tiene un material pulido. El rayo amarillo indica la luz que llega desde la lámpara. La línea cian es la normal, perpendicular a la superficie. El rayo rosa muestra la dirección de reflexión ideal.»

«La fórmula es R igual a I menos dos veces el producto escalar de I y N, multiplicado por N. Aquí I es la dirección incidente y N es la normal unitaria. El ángulo de incidencia y el de reflexión se miden respecto de esa normal.»

[Mueve la cámara con las flechas. Alterna **J** y **N**.]

«El brillo especular cambia al observar desde otra posición. La luz solar es direccional; una lámpara es puntual y pierde intensidad con la distancia. Los mensajes breves identifican la fuente que estoy mostrando.»

«Estas flechas explican la geometría: no son un trazador de rayos. OpenGL calcula la iluminación de las superficies; esta versión no dibuja sombras proyectadas ni reflejos del entorno.»

## 6. Los nueve controles pedidos — 2 minutos

[En el laboratorio, activa y desactiva cada opción antes de pasar a la siguiente.]

| Acción | Qué decir |
|---|---|
| Pulsa **1** | «Rotación: `glRotatef` cambia la orientación de la pieza. El anillo permite observar el giro.» |
| Pulsa **2** | «Escala con seno: `glScalef` aumenta y reduce el tamaño suavemente. La escala permanece positiva.» |
| Pulsa **3** | «Traslación: `glTranslatef` desplaza la pieza usando seno y coseno. Esto cambia su posición, no la cámara.» |
| Pulsa **4** dos veces | «Desactivo y activo la iluminación y sus normales. Sin iluminación se ven los colores base; con ella se aprecia mejor el volumen.» |
| Pulsa **5** | «Las luces puntuales pasan a rojo, verde y azul. El color de la luz se combina con el material del objeto.» |
| Pulsa **6**, espera y vuelve a pulsar **6** | «La cámara orbita automáticamente alrededor del objeto observado.» |
| Pulsa **7** | «Cambio de perspectiva a ortográfica conservando la cámara.» |
| Pulsa **R**, luego **8** y nuevamente **8** | «Sin la prueba de profundidad pueden aparecer objetos detrás dibujados por encima. Con `GL_DEPTH_TEST`, OpenGL determina qué superficie está más cerca.» |
| Pulsa **9**, luego **?** para despejar los paneles | «Divido la ventana en cuatro viewports: vista principal, planta, frontal y lateral. Cada uno tiene su propia cámara y proyección.» |

«Las primeras tres transformaciones se aplican a la pieza del laboratorio. Así puedo estudiar los cambios sin desarmar la estación ni modificar la cámara.»

## 7. Personas y sonido — 40 segundos

[Pulsa **R** y **F** dos veces. Si las cubiertas tapan la vista, usa **H**.]

«Los pasajeros tienen movimiento de brazos y piernas. Algunos esperan y otros caminan. Durante la parada, pequeños grupos simulan subir al tren.»

[Pulsa **B** para la bocina y **M** para silenciar y volver a activar.]

«El sonido se sintetiza en el programa. Hay motor, ruedas, bocina y freno; el ambiente también cambia entre día y noche. No necesito cargar canciones ni archivos externos.»

## 8. Cierre — 20 segundos

[Pulsa **R**.]

«El proyecto reúne modelado con primitivas, transformaciones, cámara, dos proyecciones, materiales, luces, profundidad, múltiples vistas y animación. Los controles y mensajes permiten experimentar y ver inmediatamente qué produce cada concepto.»

## Respuestas breves para preguntas

- **¿Dónde cambia la proyección?** En `aplicarProyeccion()`, con `gluPerspective` y `glOrtho`.
- **¿Dónde se mueve la cámara?** En `colocarCamara()`, mediante `gluLookAt`.
- **¿Qué habilita la profundidad?** `GLUT_DEPTH` solicita el buffer; `GL_DEPTH_TEST` lo usa y `glClear` lo limpia en cada cuadro.
- **¿Por qué hay normales?** Indican la orientación de la superficie; GLUT las genera para sus primitivas y los planos propios usan `glNormal3f`.
- **¿Por qué `GL_NORMALIZE`?** Mantiene las normales unitarias después de escalar los objetos.
- **¿Cómo se diferencia el sol de una lámpara?** En la posición de luz, `w=0` indica dirección y `w=1` una posición puntual.
- **¿Por qué la esfera brilla?** Por el componente especular y el parámetro `GL_SHININESS` del material.
- **¿Las bombillas iluminan por su emisión?** No; la emisión hace visible la bombilla. La luz sobre otras superficies procede de `GL_LIGHTn`.
- **¿El zoom ortográfico funciona igual que el de perspectiva?** No: aquí se modifica el volumen de `glOrtho` para ampliar la imagen.
- **¿Las puertas se abren en movimiento?** No. La lógica del recorrido permite abrirlas únicamente durante las paradas.
- **¿Por qué SAFESEH está en No?** Para enlazar con la biblioteca GLUT clásica de 32 bits instalada; no es una función de gráficos.
