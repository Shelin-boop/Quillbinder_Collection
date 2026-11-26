# Manual de usuario de {{APP}}

Este manual se ha traducido automáticamente y todavía no lo ha revisado una persona hispanohablante. Donde algo no quede claro, el manual en inglés es la referencia.

{{APP}} es un estudio de escritura para textos largos: novelas, guiones de cine, obras de teatro, guiones de cómic, tesis y libros de no ficción. Escribes en piezas pequeñas (escenas, secciones, notas), las ordenas con libertad y, al final, las *compilas* en un único manuscrito.

Este manual corresponde a la versión {{VERSION}}. Pulsa **F1** en cualquier momento para abrirlo; se abre en la parte que corresponde a lo que tienes delante: el documento, el corcho, el esquema o el texto combinado, el archivador, la pestaña del inspector que tiene el teclado o el cuadro de diálogo que está abierto (un cuadro de diálogo que no tiene una parte propia abre el principio). La ventana tiene una **lista de contenidos** a la izquierda (un clic en un encabezado salta a él) y un **campo de búsqueda** sobre el texto: pulsa **Ctrl+F** para ir a él, escribe una palabra y **Return** muestra la siguiente coincidencia y **Shift+Return** la anterior (los botones **Siguiente** y **Anterior** hacen lo mismo); la línea junto al campo indica qué coincidencia de cuántas se muestra, y todas las coincidencias aparecen marcadas en el texto. Lo que está pensado para quienes desarrollan {{APP}} (plugins, paquetes, la carpeta del proyecto en detalle, variables de entorno) se encuentra en la guía del desarrollador, **Ayuda > Solución de problemas y desarrollo > Guía del desarrollador: actualizaciones y complementos**.

---

## 1. Primeros pasos

### Obtener el programa

{{APP}} se descarga de la página **Releases** de su página de proyecto, https://github.com/Shelin-boop/Quillbinder_Collection/releases. Cada versión incluye el programa en un único archivo (*{{APP}}-&lt;version&gt;-x86_64.AppImage*), una actualización del programa para quienes ya tienen un AppImage de {{APP}} (sección 17) y los complementos (archivos *.qbpkg*, sección 17). Haz ejecutable el AppImage (clic derecho > **Propiedades** > **Permisos** > *Es ejecutable*, o `chmod +x` en un terminal) y haz doble clic en él; **Ayuda > Añadir al menú de aplicaciones** lo coloca en el menú de tu escritorio. El archivo *SHA256SUMS.txt* de cada versión enumera las sumas de comprobación de sus archivos.

**{{APP}} es un software {{LABEL}}.** Haz copias de seguridad de tus proyectos (sección 15) y conserva una copia de tus manuscritos fuera del programa.

### Empezar un proyecto

Cuando {{APP}} se inicia sin ningún proyecto, se abre la **ventana de inicio**:

- **Nuevo proyecto**: elige una categoría y una plantilla (por ejemplo En blanco, Novela, Novela con partes o Relato corto), escribe un nombre, elige dónde guardarlo y haz clic en **Crear**. **Formato** decide cómo se almacena el proyecto: como proyecto de {{APP}} (lo habitual) o como proyecto de Scrivener 3 (*.scriv*), de modo que también puedas abrirlo en Scrivener (sección 14, *Trabajar en el formato de Scrivener*).
- **Proyectos recientes**: haz doble clic en un proyecto en el que hayas trabajado antes.
- **Abrir otro…**: busca un proyecto en tu disco.
- **Importar proyecto de Scrivener…**: crea un proyecto de {{APP}} a partir de un proyecto de Scrivener (consulta la sección 14). Para seguir trabajando en el propio proyecto de Scrivener, abre en su lugar su archivo *.scrivx* con **Abrir otro…**.

{{APP}} vuelve a abrir el último proyecto al iniciarse (puedes desactivarlo en **Archivo > Opciones > General**).

**El primer inicio** muestra una sola página con todo ya rellenado (**Aceptar** lo conserva): las copias de seguridad (si se hace una copia del proyecto al cerrarlo, cuántas copias se conservan, 25 al principio, y dónde), desde un AppImage de Linux la pregunta de si se añade {{APP}} al menú de aplicaciones (y si se mueve el AppImage a *~/Applications*) y el aviso sobre la búsqueda de actualizaciones con su interruptor. El proyecto de bienvenida se abre al pulsar **Aceptar**; antes no se muestra ninguna ventana vacía del programa. Todo ello se puede cambiar más tarde en **Archivo > Opciones**; allí, en **General > Inicio**, también se elige qué ajustes tienen preferencia: los propios de un proyecto o siempre los del programa (sección 16, *Ajustes para un proyecto*).

### El proyecto de bienvenida y la Guía de inicio rápido

La primera vez que se inicia {{APP}}, abre el **proyecto de bienvenida**: un proyecto breve que muestra, en un documento cada uno, lo que {{APP}} ofrece más allá de lo habitual (la barra de iconos, un zoom para cada parte de la ventana, mapas mentales, fichas de objetos, los comentarios de una carpeta entera, Comparar y transferir, proyectos de Scrivener, temas, actualizaciones y complementos, y lo que protege tu texto). Empieza por el documento **Bienvenida** que está arriba del todo en el archivador; sus enlaces llevan a cada parada del recorrido. Nada de lo que hagas en el proyecto de bienvenida puede dañar tus propios proyectos. (Si se indica un proyecto en la línea de comandos, o se hace doble clic en un archivo de proyecto, se abre ese en su lugar; el proyecto de bienvenida llega entonces en el siguiente inicio.)

El proyecto de bienvenida se guarda como «Welcome to {{APP}}» en tu carpeta *Documentos*. **Ayuda > Proyecto de bienvenida…** lo abre de nuevo tal como lo dejaste, o crea una copia nueva para empezar de cero (**Crear una copia nueva**; la copia antigua va a la papelera del escritorio). Bórralo cuando quieras; el comando crea uno nuevo.

**Ayuda > Guía de inicio rápido** muestra en una página dónde están los ajustes importantes (temas y modo oscuro, colores y fuentes, formato, zoom, barras de herramientas, atajos, copias de seguridad...) y resume lo que {{APP}} puede hacer.

### Guardar

Nunca necesitas guardar a mano. {{APP}} guarda automáticamente unos segundos después de que dejes de escribir, cuando cambias a otro programa y cuando cierras el proyecto. Si sigues escribiendo, guarda en la primera pausa breve (un respiro entre palabras) una vez transcurridos 5 segundos, y como máximo a los 10 segundos aunque no haya ninguna pausa. **Ctrl+S** guarda al instante si quieres estar seguro.

Cada archivo se escribe primero con un nombre temporal y sustituye al antiguo solo cuando se ha escrito entero y el ordenador lo ha dejado en el disco. Si el disco está lleno o la unidad falla al guardar, el archivo antiguo se queda como estaba y tus cambios permanecen en la ventana hasta que guardar vuelva a funcionar.

**El estado de guardado siempre está a la vista** (0.70). La barra de pie del primer editor (el tramo vacío entre el total de la carpeta y el recuento de palabras, o a la izquierda en las demás barras de pie) indica **Guardado**, **Aún sin guardar** (unos segundos después de escribir) o, sobre fondo rojo, **Sin guardar: el último guardado falló a las 14:03**; mientras los guardados fallan, el título de la ventana también lleva una marca. Un guardado fallido muestra además un mensaje que nombra los documentos sin guardar y ofrece **Guardar los textos en otro lugar…**; nunca se apodera del teclado, de modo que la siguiente tecla que pulses sigue yendo a tu texto. El programa lo intenta de nuevo por sí solo (cada 15 segundos al principio y luego cada minuto), el mensaje vuelve a aparecer tras cinco minutos de fallos y la señal roja permanece hasta que un guardado funciona. Los mapas mentales se guardan y se comprueban del mismo modo: un tablero que no se puede guardar se conserva en la ventana, se cuenta como no guardado al cerrar el proyecto y pasa a la copia de emergencia.

**Mensajes breves** (0.93.0). En la parte inferior de la ventana ya no hay una fila de barra de estado. Los mensajes que antes se mostraban allí («Guardado de nuevo: todos los cambios están en el disco.», el resultado de una copia de seguridad y similares) aparecen durante unos segundos en el mismo lugar que el texto de guardado, en la barra de pie del primer editor (el de la izquierda o el de arriba cuando hay dos), y después vuelve el texto de guardado. Un mensaje demasiado largo para el espacio se acorta; deja el puntero sobre él para leerlo entero. En un editor muy estrecho, el texto se sustituye por una pequeña marca de color (verde guardado, amarillo aún sin guardar, rojo fallido) con el texto como indicación emergente. En el modo de composición, y mientras no hay ningún proyecto abierto, no se muestra nada.

**Archivos sobre los que {{APP}} no escribirá** (0.70). Un archivo de texto dañado (un archivo RTF con el principio roto o cortado) o que otro programa guardó con otra codificación de caracteres (por ejemplo, un archivo Markdown guardado como Windows-1252 o UTF-16 tras editarlo fuera del programa) se muestra hasta donde se puede leer y no se puede editar; un mensaje lo indica y el archivo no se modifica nunca. Copia su texto en un documento nuevo para seguir, o guarda el archivo en UTF-8 con el programa que lo escribió y luego cierra y vuelve a abrir el proyecto; o restáuralo a una de sus instantáneas (desde 0.81.1): primero el archivo dañado se conserva como copia en *settings/conflicts* de la carpeta del proyecto, y el texto de la instantánea pasa a ser el texto del documento, que se puede editar y se vuelve a guardar. Unir documentos y Dividir rechazan un documento así indicando el motivo, y Reemplazar en el proyecto, Convertir comillas y los cambios de estilo lo dejan como está y lo nombran. Si un comando lo modifica de todos modos, el texto modificado no se pierde: se conserva como archivo RTF en *settings/conflicts* y un mensaje indica dónde (0.81.1). Un archivo de mapa mental que no se puede leer se trata igual. Los caracteres que no se pueden almacenar (caracteres de control de datos pegados) se omiten como antes; ahora un breve mensaje en la barra de pie lo indica una vez por documento.

**Un guardado se completa por entero** (0.65). Cuando un guardado escribe varios archivos (el texto de un documento nuevo y el archivo del proyecto, un texto y sus imágenes, la hoja de Objetos), primero se escriben todos junto a sus ubicaciones, luego se anotan en un pequeño diario (*settings/journal.json*) y solo después se colocan juntos en su sitio. Si el programa o el ordenador se detienen a mitad, la siguiente apertura termina ese guardado a partir del diario o, si el diario aún no estaba completo, deja el proyecto como estaba en el guardado anterior; un aviso indica cuál de los dos casos ha sido. Un diario que no se puede leer (un fallo del disco) nunca se toma por uno vacío: él y los archivos de su guardado se conservan en una carpeta dentro de *settings/conflicts*, no se coloca ni se borra nada, y el aviso lo dice (0.81.1). En una carpeta sincronizada, el diario de un guardado del otro ordenador se deja en paz durante 15 minutos (ese guardado puede seguir en marcha allí, o sus archivos estar aún llegando): este ordenador no guarda el proyecto mientras tanto y lo intenta de nuevo por sí solo, y si el otro ordenador se detuvo durante ese guardado, este lo termina pasados esos 15 minutos (0.81.1). Si un archivo de un guardado no se puede escribir, ninguno sustituye nada y tus cambios permanecen en la ventana para el siguiente intento. Desde 0.93.0, una unidad que notifica un error al sincronizar una carpeta después de colocar los archivos (un error real, no un sistema de archivos que no puede sincronizar) hace que el guardado cuente como fallido: el diario se conserva, el mensaje dice que la unidad notificó un error, y el siguiente guardado o apertura termina el guardado a partir del diario. Mientras un diario que no se puede leer no pudo apartarse (no se puede escribir en la carpeta *settings/conflicts*), ningún guardado escribe un diario nuevo encima: el guardado espera, su mensaje nombra el archivo y el programa guarda por sí solo en cuanto el archivo se pueda apartar. Un archivo temporal de un guardado anterior que estorba se aparta, nunca se sobrescribe. Una sincronización de carpeta que falla antes de sustituir nada es un fallo limpio: no se cambió nada. Cuando el error llega después de que los archivos estén en su sitio, el programa anota lo que hay en el disco (para que los reintentos no notifiquen falsos conflictos de «modificado en otro lugar» ni escriban dos veces las imágenes) y sincroniza de nuevo la carpeta con cada guardado: «Guardado» siempre va precedido de una sincronización de carpeta comprobada. Un sistema de archivos que responde con un error a una sincronización de carpeta ya al abrir el proyecto se recuerda como uno que no puede sincronizar carpetas, y sus guardados no se notifican como fallidos.

Si cierras la sesión o apagas el ordenador mientras {{APP}} está abierto, guarda todo antes de terminar (entonces no se hace copia de seguridad). Si ese guardado falla (un disco lleno, una unidad que ya no está), los textos y notas que no se guardaron (y, desde 0.93.0, el archivo del proyecto tal como está en memoria) se escriben como archivos RTF en una carpeta de emergencia, *~/.local/share/{{APP}}/Emergency*, y el siguiente inicio indica dónde están y cuántos de los archivos se escribieron (el aviso dice «incompleto» y nombra lo que falta cuando un archivo no se pudo escribir). Si termina de forma inesperada, el siguiente inicio te lo comunica (sección 20, *Informes de fallos*).

**Cerrar un proyecto que no se puede guardar** pregunta qué hacer; **Mantener abierto** es la opción predeterminada. **Guardar los textos en otro lugar…** escribe los textos y notas aún sin guardar como archivos RTF en una carpeta que elijas (otra unidad, una memoria USB), y el proyecto sigue abierto. Desde 0.93.0 esta copia de emergencia también contiene el archivo del proyecto tal como está en la ventana (el archivador, los títulos y las etiquetas), los tableros de mapas mentales y los objetos modificados, y se hace también cuando solo cambiaron el archivador o los Objetos. El mensaje indica cuántos archivos se escribieron, como «n de m», y nombra los archivos que faltan y por qué; una copia a la que le faltan archivos lleva el encabezado **incompleta** (con una señal de advertencia) y nunca parece una copia terminada. Una carpeta que ya existe no se reutiliza nunca: una segunda copia en el mismo segundo recibe un número. El archivo del proyecto en la copia se llama como el archivo del proyecto con «.copy» añadido (para que la carpeta no se tome por un proyecto por error); el README de la copia explica cómo devolverla a su sitio: cierra el proyecto, cambia el nombre del archivo del proyecto en la carpeta del proyecto (se conserva como estaba), copia allí el archivo de la copia de emergencia, ponle el nombre del archivo del proyecto y vuelve a abrir el proyecto. Para un proyecto en formato Scrivener, el README dice que el archivo pertenece a la copia de trabajo y no indica ningún camino de vuelta. Cuando, después de todo, no había nada sin guardar (un reintento funcionó entretanto), el mensaje dice que todo está guardado y que no hizo falta ninguna copia. **Cerrar sin guardar** lo cierra y pierde esos cambios.

### Dónde vive tu trabajo

Un proyecto es una carpeta llamada *TuTítulo*{{SUFFIX}}. Dentro, cada documento es un archivo de texto corriente, de modo que tu escritura sigue siendo legible incluso sin {{APP}}. Haz doble clic en el archivo *TuTítulo*{{FILE}} dentro de la carpeta para abrir el proyecto. No cambies el nombre ni muevas a mano los archivos dentro de la carpeta del proyecto.

Para encontrar el archivo de un documento fuera de {{APP}}, abre *contents-by-title.txt* en la carpeta del proyecto (0.65): enumera todos los documentos del archivador con su título, sangrado según su posición, y los archivos que contienen su texto, notas o medios (por ejemplo *content/<id>/text.md*). {{APP}} lo escribe con cada guardado que cambia el archivador y nunca lo lee él mismo. (Un proyecto en formato Scrivener no tiene tal lista; el *.scrivx* de Scrivener nombra sus documentos.)

Un proyecto debería estar abierto en una sola ventana a la vez. Si ya está abierto en otra ventana o en otro ordenador (por ejemplo, a través de una carpeta sincronizada), {{APP}} te avisa antes de abrirlo. Si inicias {{APP}} de nuevo con un proyecto que está abierto en una ventana del programa (desde el gestor de archivos o un acceso directo del escritorio), esa ventana pasa al primer plano en lugar de abrir el proyecto dos veces; otro proyecto se abre en una ventana propia.

**Un proyecto en una carpeta sincronizada** (OneDrive, Dropbox, Nextcloud, Syncthing o cualquier otro programa de sincronización) se puede usar en varios ordenadores, de uno en uno. Ciérralo en un ordenador y deja que el programa de sincronización termine antes de abrirlo en el otro. {{APP}} ayuda con ello a nivel de archivos, sea cual sea el programa de sincronización:

- Mientras un proyecto está abierto, un pequeño archivo de bloqueo en él nombra el ordenador y se renueva cada pocos minutos. Contiene el nombre del ordenador, tu nombre de usuario en él, la versión y el número de proceso del programa, cuándo se abrió el proyecto y cuándo se renovó el bloqueo por última vez, y un código que distingue unos ordenadores de otros (un hash, no legible como nombre). Cualquiera con acceso a la carpeta compartida puede verlo. En **Archivo > Opciones > General > Guardado**, **Nombre de este equipo en las carpetas compartidas** establece un nombre propio («Portátil»); se usa en lugar del nombre del equipo en los archivos de bloqueo, en los nombres de las versiones conservadas y en *settings/ui-<computer>.json*, y entonces se omite tu nombre de usuario. En otro ordenador, al abrir el proyecto se muestra entonces el nombre de ese equipo, desde cuándo tiene abierto el proyecto y su última señal de vida; **Abrir de todos modos** o **Cancelar**. Un bloqueo que no se ha renovado en más de 15 minutos (el proyecto no se cerró allí, o ese ordenador está apagado) se muestra como sobrante, y **Abrir** es la opción predeterminada.
- Si el proyecto se abre en otro ordenador mientras está abierto aquí, un aviso lo indica y ofrece cerrarlo aquí.
- Si un archivo del proyecto se modificó en otro lugar (en otro ordenador mediante el programa de sincronización, o por otro programa) después de que {{APP}} lo leyera, y aquí también se modificó, la otra versión se conserva antes de guardar la tuya: como **instantánea** del documento (compárala en la pestaña Instantáneas del inspector) y como copia en la carpeta *settings/conflicts* del proyecto. Un aviso indica de qué documentos se trata. Si la otra versión no se puede conservar (por ejemplo, porque el disco está lleno o no se puede escribir en *settings/conflicts*), la tuya no se guarda encima: el guardado falla con un mensaje que nombra el archivo, y tus cambios permanecen sin guardar hasta que se pueda conservar la otra versión. Un documento que aquí solo has mirado nunca se guarda encima de una versión más reciente de otro lugar.
- Abrir y cerrar un proyecto sin cambiar nada no modifica ninguno de sus archivos; la disposición de la ventana y los lugares en los que estuviste en cada documento se conservan por ordenador (*settings/ui-<computer>.json*). En el formato Scrivener tampoco se toca el *.scriv*, y este recibe un archivo de bloqueo de {{APP}} (*quillbinder.lock*) mientras está abierto; Scrivener no lo lee. Antes de cada guardado en el *.scriv*, {{APP}} busca el bloqueo propio de Scrivener: si Scrivener abrió el proyecto después que {{APP}}, ya no se escribe nada más en el *.scriv* (un aviso lo indica una vez y después un breve mensaje en la barra de pie); tus cambios se quedan en la copia de trabajo de {{APP}}. Cierra el proyecto en Scrivener y vuelve a guardar (**Archivo > Guardar**). Si cierras primero el proyecto aquí, los cambios pasan al *.scriv* la próxima vez que lo abras aquí; donde Scrivener haya cambiado algo entretanto, se lee su versión.
- Si el proyecto enumera el texto de un documento cuyo archivo aún no ha llegado a este ordenador (el programa de sincronización no ha terminado), un aviso lo nombra tras abrir; ese documento se muestra vacío, no se puede editar y no se guarda nada en su lugar. Cierra el proyecto, espera a que el programa de sincronización termine y ábrelo de nuevo. Lo mismo vale para un texto que solo se pudo leer en parte (0.65).
- También se detecta un archivo modificado en otro lugar cuando conservó su tamaño y su hora (algunos programas de sincronización fijan la hora ellos mismos): {{APP}} compara su contenido con lo que leyó o escribió (0.65).
- El contenido decide también en el otro sentido (0.92.0): un archivo cuya **hora por sí sola** cambió no es otra versión. Algunos programas de sincronización vuelven a fijar la hora de un archivo unos segundos después de que {{APP}} lo escribiera, cuando termina su subida; no se conserva nada por ello, no llega ningún aviso y el archivo no se vuelve a escribir. Un archivo de proyecto que ya contiene lo que escribiría un guardado también se deja en paz, de modo que un guardado que no cambia nada no da al programa de sincronización nada que transportar.
- El aviso **Otras versiones conservadas** tiene el botón **No volver a mostrar hasta que se cierre {{APP}}**. Las otras versiones se siguen conservando como instantáneas y como copias en *settings/conflicts*; solo el aviso se queda sin aparecer hasta que salgas del programa. El siguiente inicio lo vuelve a mostrar.
- **Un candado en lo que otro dispositivo está trabajando (0.92.0).** Mientras un proyecto está abierto, cada dispositivo anota también, junto al archivo de bloqueo, qué documentos tiene abiertos en un editor o ha modificado desde que abrió el proyecto. Cuando un segundo dispositivo abre el proyecto, su archivador muestra un pequeño candado en el extremo derecho de las filas de esos documentos y de las carpetas que los contienen. Apunta al candado con el puntero: la indicación emergente nombra el dispositivo y cuándo abrió el proyecto. Es una pista, no una protección: no se bloquea nada y sigues pudiendo editar el documento. La marca viaja a través de tu programa de sincronización, así que puede tener algunos minutos de antigüedad; requiere 0.92.0 o posterior en ambos dispositivos (Scrivener no anuncia nada). Una lista cuyo dispositivo no ha dado señales de vida durante más de 15 minutos no se muestra. La nota es un pequeño archivo por dispositivo (*settings/lock-work-…json*; para un proyecto en formato Scrivener, *quillbinder.lock-work-…json* dentro del *.scriv*); contiene el nombre del dispositivo como en el archivo de bloqueo, la hora y los números internos de los documentos, sin títulos ni texto. Se escribe solo cuando cambia la lista, nunca por pulsación de tecla, se elimina al cerrar el proyecto y se deja fuera de las copias de seguridad.
- **Formato Scrivener, dos ordenadores** (0.65): los documentos añadidos al *.scriv* en otro lugar (Scrivener en otro ordenador) mientras el proyecto está abierto aquí se conservan, y el archivador que los nombra no se sustituye; un aviso te pide cerrar el proyecto y abrirlo de nuevo, lo que muestra tanto sus documentos como los tuyos. Las imágenes, los PDF, los archivos de audio y vídeo y los mapas mentales modificados en el *.scriv* en otro lugar se conservan tal cual (antes, la versión de aquí se copiaba encima); cuando uno se modificó en ambos lados, la otra versión se conserva primero en *settings/conflicts*.

Un proyecto en **formato Scrivener** es, en cambio, una carpeta *TuTítulo.scriv*; ábrelo con su archivo *.scrivx*. {{APP}} guarda tus cambios en ella, de modo que el proyecto sigue siendo un proyecto de Scrivener. **Archivo > Guardar como…** guarda un proyecto con otro nombre o en otro lugar, en cualquiera de los dos formatos (sección 14).

### Privacidad: qué sale de tu ordenador

{{APP}} se conecta a internet para una sola cosa: buscar actualizaciones. Descarga un pequeño archivo, no envía nada sobre ti y puedes desactivarlo en el menú Ayuda. (La sección 17, *Buscar actualizaciones*, explica exactamente qué se descarga y cuándo. Antes de 0.92.0 el programa no buscaba actualizaciones en absoluto.) No hay estadísticas de uso ni informes de fallos. Otras tres cosas van a internet, y solo cuando tú las inicias:

- **Archivo > Importar > Página web…** descarga la página que indiques y sus imágenes desde ese sitio. Como un navegador, envía el nombre de un navegador común (Firefox en Linux), los idiomas de tu sistema (para el idioma de la página) y, como toda conexión, la dirección de internet de tu ordenador.
- **Descargar un modelo de idioma** para el complemento Ayudante de traducción (sección 10) lo descarga de las versiones del repositorio de {{APP}} en GitHub. Envía el nombre del complemento («Quillbinder-Translation-Addon») y, como toda conexión, tu dirección de internet.
- **Descargar un modelo de voz** (el complemento Transcripción de voz, sección 10) descarga el modelo de Hugging Face (huggingface.co, una empresa de EE. UU.) o de la dirección que establezcas en sus opciones. Envía «{{APP}}/<version>» como nombre del programa y, como toda conexión, tu dirección de internet. Un modelo solo se instala cuando su suma de comprobación es la conocida; un archivo de otra dirección o uno que hayas descargado tú mismo solo se usa después de que lo aceptes.

Lo que otros pueden ver de tu trabajo:

- **Los proyectos en carpetas compartidas** llevan un archivo de bloqueo mientras están abiertos (véase arriba): nombre del equipo, nombre de usuario y horas. Establece un nombre propio en **Archivo > Opciones > General > Guardado** para omitirlos.
- **La información del autor** (nombre, dirección, teléfono, correo electrónico en **Archivo > Opciones**) se queda en los ajustes del programa de este ordenador; nunca pasa a los ajustes propios de un proyecto. Llega a un manuscrito solo allí donde el formato de compilación usa los marcadores de posición (<$author>, <$address> ...).
- **El texto eliminado no desaparece en todas partes**: las instantáneas conservan versiones anteriores de un documento (**Proyecto > Eliminar instantáneas antiguas** las quita) y las copias de seguridad conservan el proyecto entero tal como era (sección 15). Las copias de seguridad son archivos ZIP corrientes sin contraseña; una carpeta de copias de seguridad dentro de una carpeta sincronizada la sube el programa de sincronización.
- **{{APP}} no cifra tus archivos** (0.65): los textos, las notas, los ajustes del proyecto y las copias de seguridad son archivos corrientes, legibles por cualquiera que pueda leer la carpeta en la que están. Para mantener privado un manuscrito, guarda el proyecto y sus copias de seguridad en un disco cifrado (el instalador de Kubuntu ofrece cifrar todo el disco; para una memoria USB o una unidad externa, los Plasma Vaults de KDE o una unidad cifrada con LUKS) y mantén la carpeta de copias de seguridad fuera de las carpetas compartidas que no controles.
- **Los informes de fallos** permanecen en este ordenador hasta que tú mismo copias o envías uno (sección 20). Describen el estado del programa y nombran el proyecto abierto y su carpeta. En Windows se conserva un volcado de memoria junto a cada informe; puede contener texto que estabas escribiendo, y el informe lo indica.

### Añadir {{APP}} al menú de aplicaciones

La primera vez que inicias el archivo del programa ({{APP}}.AppImage), la página del primer inicio ofrece añadir {{APP}} al menú de aplicaciones. Después puedes iniciarlo como cualquier otro programa, y al hacer doble clic en un proyecto este se abre. Ningún cuadro confirma la entrada; un fallo sí se comunica. La misma página (y **Ayuda > Añadir al menú de aplicaciones**) ofrece **mover el AppImage a ~/Applications** (marcado al principio cuando el archivo aún no está allí), para que la entrada del menú lo inicie desde un lugar fijo donde el archivo se quede (la casilla lo dice con estas palabras, sin paréntesis); si el movimiento falla (un disco lleno, un archivo con ese nombre que ya existe allí), no se pierde nada: el archivo se queda donde estaba, la entrada apunta allí y un mensaje dice por qué. Si más tarde mueves tú mismo el AppImage, la entrada sigue nombrando el lugar antiguo: usa de nuevo **Ayuda > Añadir al menú de aplicaciones**, que escribe el lugar actual del archivo (una entrada antigua se reemplaza, no se duplica). Tras un movimiento, las actualizaciones del programa, la copia *.before-update* y **Ayuda > Solución de problemas y desarrollo > Información del sistema** usan de inmediato el nuevo lugar.

El único comando del menú Ayuda, **Ayuda > Añadir al menú de aplicaciones**, sigue el estado de la entrada: añade la entrada mientras no hay ninguna y, una vez que existe, se llama *Quitar del menú de aplicaciones* y elimina lo que creó la adición (el archivo de la entrada, el icono y los tipos de archivo), dejando el AppImage donde está. Si mueves tú mismo el archivo del programa, quita la entrada y añádela de nuevo.

### En Windows: todo en una carpeta

La versión de {{APP}} para Windows (experimental, publicada desde 0.91.1) es una carpeta que descomprimes donde quieras, por ejemplo *C:\Users\<tu nombre>\Quillbinder*; no necesita instalación ni derechos de administrador. Guarda sus ajustes, complementos, actualizaciones del programa, fuentes instaladas e informes de fallos en la carpeta *data* dentro de esa carpeta, y nada en las carpetas propias de Windows ni en el registro. Toda la carpeta se puede mover o copiar (a una memoria USB o a otro ordenador); borrarla elimina el programa. Una carpeta en la que el programa no puede escribir (como *Program Files*) se rechaza con un mensaje que indica dónde ponerlo en su lugar. Las actualizaciones del programa se instalan con **Ayuda > Complementos y actualizaciones > Instalar actualización o complemento**; para pasar a una versión que has descargado de nuevo, descomprímela y copia la carpeta *data* en la carpeta nueva. Tus proyectos se quedan donde los tengas.

Hay dos maneras de obtener esa carpeta (0.90.4). El **programa de instalación** (*...-setup.exe*) pide una carpeta y descomprime {{APP}} en ella; no necesita derechos de administrador y no cambia nada más en el ordenador: ninguna entrada en Aplicaciones, nada en el registro, ninguna entrada en el menú Inicio (la guía que se describe más abajo las ofrece). Descomprimir una versión más reciente en la misma carpeta conserva *data*, es decir, tus ajustes y complementos. El **archivo zip** contiene la misma carpeta: descomprímelo por completo. Windows puede avisar sobre cualquiera de los dos archivos porque no están firmados (SmartScreen: *Más información* y luego *Ejecutar de todas formas*).

Dos cosas difieren de Linux por culpa del propio Windows (0.90.4). La tecla **AltGr** escribe su carácter, como cabe esperar: en un teclado alemán, AltGr+7, 8, 9, 0 y < dan { [ ] } y |, aunque {{APP}} tiene comandos en Ctrl+Alt con esas teclas; para los comandos pulsa las teclas Ctrl y Alt de la izquierda. Y los archivos que solo se necesitan un momento (una página web guardada descomprimida para verla, o su copia para el navegador) se guardan en la carpeta temporal de Windows y se eliminan al cerrar {{APP}}. Transcripción de voz usa el procesador en Windows, no la tarjeta gráfica.

En el primer inicio, una guía indica dónde guarda el programa sus archivos y avisa cuando su carpeta está dentro de la carpeta de un servicio de sincronización (OneDrive, Dropbox, Google Drive...) o tiene una ruta larga. También pregunta por lo siguiente, que **Ayuda > Solución de problemas y desarrollo > Este equipo** y **Archivo > Opciones > General > Este equipo** vuelven a mostrar:

- **Copias de seguridad**: no hay carpeta de copias predeterminada. Hasta que elijas una, no se hace ninguna copia automática; la barra de pie del primer editor muestra **Sin copias de seguridad** junto al texto de guardado (haz clic en él para elegir la carpeta) y la primera copia que se ejecutaría pide una carpeta. Lo mejor es otra unidad.
- **Notas del Bloc de notas rápidas y transcripciones**: en Documentos (lo habitual), en la carpeta del programa o en una carpeta propia. Un cambio da a ambas herramientas el nuevo lugar; lo que escribieron antes se queda donde está.
- **Windows**: una entrada en el menú Inicio, un acceso directo en el escritorio y los tipos de archivo de proyecto (los archivos *.qbx* se abren con {{APP}}, que se ofrece en **Abrir con** para los archivos *.scrivx* de Scrivener). Son las únicas cosas que el programa coloca fuera de su carpeta, cada una solo cuando la añades y cada una con **Quitar**. Quítalas antes de borrar o mover la carpeta.

Las fuentes instaladas con **Formato > Fuente > Instalar archivos de fuente** se quedan en la carpeta del programa y son solo de {{APP}}; otros programas no las ven.

Los complementos con código de programa tienen sus propias versiones para Windows (los nombres de sus archivos terminan en *-windows*): **Reproducción multimedia** reproduce lo que el propio Windows reproduce, **Formatos adicionales de vídeo y audio** añade FFmpeg, y **Transcripción de voz** escribe lo que se dice usando el procesador (las tarjetas gráficas todavía no se usan en Windows). **Ayudante de traducción** forma parte de la versión completa también en Windows (0.92.0). Los complementos sin código de programa (temas, plantillas, formatos de compilación) son los mismos archivos en todos los sistemas.

### En tabletas Android

La versión de {{APP}} para Android (experimental, publicada desde 0.91.3) es una aplicación para tabletas con Android 9 o posterior (ARM de 64 bits). Tiene las mismas ventanas que en un ordenador, pensadas para los dedos: un dedo desplaza y toca, dos dedos hacen zoom, un dedo que descansa medio segundo sobre algo abre su menú de clic derecho (en un texto, lo hace un toque sobre el texto seleccionado), y las barras de herramientas se pliegan en tantas filas como necesite la pantalla. Descarga el archivo APK en la tableta y ábrelo; Android pregunta si el navegador o la aplicación Archivos puede instalar aplicaciones, lo que permites para esta instalación. Una versión nueva llega como un APK nuevo y se instala sobre el anterior, conservando tus ajustes y complementos; **Ayuda > Complementos y actualizaciones > Buscar actualizaciones ahora** te avisa cuando hay una y abre su descarga en el navegador (los complementos más nuevos se instalan dentro de {{APP}}).

Con teclado y ratón externos (desde 0.92.0): Ctrl + rueda del ratón hace zoom, y Ctrl + clic o Shift + clic eligen varios documentos, como en un ordenador. La línea entre dos paneles muestra un agarre; un ratón pulsado a unos píxeles de ella se lo lleva. El botón **Atrás** de Android cierra el menú, la indicación o la ventana que está delante; cada ventana tiene un botón que la cierra, y una ventana mayor que la pantalla se desplaza. En una tableta, el alfiler del mapa mental fijado se muestra en la fila seleccionada del archivador (haz clic en él, o usa el menú de clic derecho de la fila: *Fijar el mapa mental de este elemento*, *Soltar el mapa mental fijado*), y un toque en el candado de una fila muestra quién trabaja en ella. Un dedo sobre la vista general del mapa mental funciona como el ratón: un toque mueve la vista allí, un dedo arrastra el rectángulo de la parte mostrada y el agarre de su esquina cambia su tamaño. **Archivo > Guardar como…** pregunta primero el formato en una tableta y luego abre la ventana de archivos.

En el primer inicio, {{APP}} pide acceso a los archivos de la tableta. Con él, los proyectos van a *Documents/Quillbinder*, las copias de seguridad a *Documents/Quillbinder/Backups*, las exportaciones a *Documents/Quillbinder/Exports* y las notas del Bloc de notas rápidas a *Documents/Quillbinder/Scratch Pad*, donde otras aplicaciones, una aplicación de sincronización y un ordenador conectado por cable pueden acceder a ellos; el botón abre la página *Acceso a todos los archivos* de Android, donde lo activas para {{APP}}. Sin él, todo se queda en la carpeta propia de la aplicación, que ninguna otra aplicación ve y que Android borra junto con la aplicación; *Archivo > Archivos de esta tableta* vuelve a preguntar e indica dónde está cada cosa. Cuando Android aparta la aplicación (el botón Inicio, otra aplicación), {{APP}} guarda de inmediato.

Los diccionarios ortográficos (inglés y alemán; los demás idiomas son complementos, véase la sección 4), las herramientas de escritura, los importadores de páginas web y de archivos antiguos de Word y (desde 0.92.0) Ayudante de traducción forman parte de la aplicación; Ayudante de traducción se instala en el primer inicio de una versión y descarga un modelo de idioma solo cuando eliges uno. Los complementos para Android (los nombres de sus archivos terminan en *-android-arm64*) se instalan con **Ayuda > Complementos y actualizaciones > Instalar actualización o complemento**; la ventana de archivos se abre en la carpeta Descargas, y {{APP}} se reinicia cuando el complemento lo necesita. Los paquetes hechos para Linux o Windows y las actualizaciones del programa se rechazan indicando el motivo: la propia aplicación solo se actualiza con un APK nuevo. Todavía no están en la versión de Android: Transcripción de voz, audio y vídeo en el editor, e informes de fallos. El texto copiado dentro de {{APP}} conserva su formato al pegarlo; el portapapeles de Android solo lleva texto sin formato, así que el texto copiado a otras aplicaciones llega sin formato.

---

## 2. La ventana principal

La ventana tiene tres columnas:

| Área | Qué hace |
|------|----------|
| **Archivador** (izquierda) | La tabla de contenidos de todo tu proyecto: cada documento y carpeta. |
| **Editor** (centro) | Muestra lo que seleccionaste en el archivador: texto, un corcho de fichas o un esquema. |
| **Inspector** (derecha) | Información sobre el documento actual: sinopsis, notas, marcadores, metadatos, instantáneas y comentarios. |

La primera vez que {{APP}} se inicia en una pantalla de menos de 1600 px de ancho, la ventana se abre maximizada. El tamaño, la posición y el estado de la ventana (maximizada o no) se guardan un instante después de cada cambio, de modo que un fallo no los pierde. Oculta el archivador con **Ctrl+Alt+B** y el inspector con **Ctrl+Alt+I** para ganar espacio. Así es la ventana mientras es más ancha que alta; una ventana más alta que ancha organiza las mismas tres áreas de otra manera (sección siguiente). Los colores y las fuentes de las tres áreas, y los del corcho, el esquema y el modo de composición, proceden del tema (**Ver > Temas**) y se pueden cambiar área por área (sección 16), también con un clic derecho en el área (**Apariencia**). Cada proyecto conserva su propio aspecto.

La **barra de herramientas** de arriba contiene, de izquierda a derecha: el botón **Ver** (archivador, colecciones, inspector, disposiciones), documento nuevo (con un menú para carpetas, plantillas e importación), carpeta nueva, papelera, **deshacer** y **rehacer**, el botón de **búsqueda** (búsqueda en el proyecto; la flecha junto a él enumera las demás herramientas de búsqueda: Buscar en el documento, Reemplazar en el proyecto, Buscar por formato), el campo de **búsqueda rápida**, los tres botones de vista (texto, corcho, esquema), los marcadores del proyecto, la referencia rápida, los comentarios y notas al pie, compilar, el modo de composición, el **micrófono** de Transcripción de voz (cuando ese complemento está instalado) y el botón del inspector. Tú decides qué contiene (consulta «Barras de herramientas propias» más abajo).

La **barra de formato** situada debajo contiene el estilo, la fuente, el tamaño, el interlineado, el formato de texto predeterminado, la ventana de fuentes, negrita, cursiva, subrayado, tachado, resaltado, color del texto, alineación, listas, el espacio antes y después del párrafo, la sangría de la primera línea y de todo el párrafo, comentarios, notas al pie, anotaciones en línea e imágenes. Muestra u oculta las dos barras con **Ver > Barras de herramientas**.

**Cuadros de diálogo.** Una ventana que abre el programa (Opciones, Compilar, Objetivos del proyecto...) nunca es más grande que la pantalla sin los paneles del escritorio: en la pantalla de un portátil pequeño, su contenido se desplaza o se reduce, y sus botones permanecen a la vista. Al cerrarse, el teclado vuelve adonde estaba, normalmente al texto, de modo que puedes seguir escribiendo de inmediato.

**¿Dónde estoy?** Haz doble clic en el espacio junto al texto (el papel junto a la columna o el área que lo rodea): el archivador selecciona el documento que está junto al clic, abre las carpetas que lo contienen y hace parpadear su fila dos veces en naranja. El editor conserva lo que muestra. **Navegar > Mostrar en el archivador** (**Ctrl+Shift+R**) hace lo mismo con el documento donde está el cursor.

El **pie** de cada editor muestra el recuento de palabras y caracteres (selecciona texto para contar solo la selección) y el objetivo del documento. Haz clic en el recuento para ver detalles. Cuando el editor muestra una carpeta (o un documento con subdocumentos), el pie también muestra el **total** de la carpeta: las palabras de su propio texto y de todo lo que contiene, por ejemplo «Carpeta: 12.345 palabras». El corcho y el esquema muestran el mismo total en el centro de su pie; con varios elementos mostrados, es el total de todos ellos. Un clic en un total muestra sus detalles (palabras, caracteres, caracteres sin espacios, párrafos y páginas), igual que el recuento de un texto. El inspector lo muestra bajo **Total** en los metadatos de una carpeta, y dejar el ratón sobre una carpeta del archivador también lo muestra (con las palabras propias de la carpeta cuando tiene texto propio).

### Los menús

| Menú | Qué contiene |
|------|--------------|
| **Archivo** | Proyectos, guardado, el formato del proyecto ({{APP}} o Scrivener), copias de seguridad, importación y exportación, comparar y transferir con un archivo de Word o LibreOffice, compilar, impresión, conversión de archivos, opciones |
| **Edición** | Deshacer, portapapeles, selección, buscar y reemplazar (en el texto, en el proyecto, por formato), mover documentos en el archivador, herramientas de escritura |
| **Insertar** | Imágenes, tablas, enlaces a documentos y objetos, enlaces web, comentarios, notas al pie, marcas de tiempo de medios, numeración automática, marcadores de posición, caracteres especiales, fecha y hora |
| **Ver** | Los tres modos de vista, el editor dividido (**Editor dividido**), las posiciones del archivador, el editor y el inspector y las disposiciones guardadas (**Disposición**), los temas de color (**Temas**), archivador, colecciones e inspector (y **Teclas de adición rápida en el archivador**), barras de herramientas, opciones de edición de texto (vista de página, regla, desplazamiento tipo máquina de escribir), opciones del corcho y del esquema, colores de etiqueta, zoom, modo de composición, pantalla completa |
| **Navegar** | Atrás y adelante, documento anterior y siguiente, abrir documentos en el otro editor, en el documento de apoyo o como referencia rápida, el bloqueo del editor, en qué editor carga el archivador, las pestañas del inspector (todas, con los nombres de sus indicaciones emergentes: **Sinopsis y notas**, **Marcadores**, **Metadatos**, **Instantáneas**, **Comentarios y notas al pie**, el mapa mental, los objetos y las pestañas de las herramientas de escritura) |
| **Proyecto** | Documentos nuevos, ajustes del proyecto (que también se guardan en un archivo y se cargan desde él), objetivos, palabras clave, estadísticas, historial de escritura, papelera, eliminar instantáneas antiguas |
| **Documentos** | Instantáneas, cambiar nombre, duplicar, dividir, unir, convertir, icono, etiqueta, estado, marcadores |
| **Formato** | Fuente (con instalación de fuentes), párrafo, estilo, listas, resaltado, copiar formato, modo de revisión |
| **Ventana** | Bloc de notas rápidas, ventanas de referencia rápida |
| **Ayuda** | **Buscar comando**, este manual, la Guía de inicio rápido, el proyecto de bienvenida, la lista de todos los marcadores de posición, la lista de atajos de teclado, la ayuda de los mapas mentales, el submenú **Complementos y actualizaciones** (Gestor de complementos, Instalar actualización o complemento, la búsqueda de actualizaciones), añadir {{APP}} al menú de aplicaciones (y quitarlo), el submenú **Solución de problemas y desarrollo** (información del sistema, Este equipo, informes de fallos, Abrir carpeta de plugins, la guía del desarrollador), Notificar un problema, las licencias de terceros, Acerca de |

**Buscar comando** (**Ayuda > Buscar comando…**) abre una ventana con un campo de búsqueda y la lista de todos los comandos de los menús, cada uno con su menú y su atajo. Escribe parte de un nombre (varias palabras acotan la búsqueda, en cualquier orden); **Return** o un doble clic ejecuta el comando elegido. Un comando que en ese momento está desactivado aparece en gris y no se ejecuta. No tiene atajo propio.

Un submenú muestra nombres cortos: **Archivo > Importar > Archivos**, **Documentos > Dividir > en la selección**, **Navegar > Abrir > como referencia rápida**. El atajo de un comando se muestra junto a él y se puede cambiar (sección 19). Cada entrada de un menú tiene una letra propia (subrayada cuando mantienes pulsada **Alt**), de modo que **Alt** y las letras llegan a todos los comandos.

Sin un proyecto abierto, los menús **Insertar**, **Navegar**, **Proyecto**, **Documentos** y **Formato** aparecen en gris, igual que los comandos de los demás menús que necesitan un texto o un archivador (Buscar, Zoom, la disposición del editor, Enfocar el archivador...). Lo que funciona sin proyecto: crear, abrir e importar proyectos, convertir archivos, instalar actualizaciones y complementos, las opciones, los temas y las disposiciones, las herramientas de escritura que no necesitan proyecto, y la Ayuda.

### Los paneles según la forma de la ventana

La ventana organiza el archivador, el editor y el inspector según su forma, y cambia por sí sola cuando la forma cambia (arrastras la ventana para que sea más alta que ancha, o giras una tableta). Una ventana cuadrada cuenta como ancha.

- **Ventana ancha** (más ancha que alta, o cuadrada): tres columnas. Elige su orden en **Ver > Disposición > Ventana ancha**: cualquiera de los seis órdenes de archivador, editor e inspector. Los anchos los fijan las líneas entre las columnas.
- **Ventana vertical** (más alta que ancha): dos filas bajo las barras de herramientas. Una fila es el editor, a todo el ancho de la ventana; la otra contiene el archivador y el inspector uno junto a otro. En **Ver > Disposición > Ventana vertical**, **Editor en el centro** pone la fila del editor sobre la otra, **Editor abajo** debajo de ella; **Archivador a la izquierda** y **Archivador a la derecha** eligen el lado del archivador (el inspector toma el otro). Las alturas de las dos filas y los anchos del archivador y del inspector los fijan las líneas entre ellos. Con el teclado en pantalla visible (Android), la fila del archivador y el inspector mantiene su altura; mueve la línea si la quieres distinta.
- Un archivador o un inspector oculto deja su espacio al otro en la fila vertical, a todo el ancho; con ambos ocultos, el editor ocupa toda la ventana. Dos editores (**Ver > Editor dividido**) permanecen dentro del lugar del editor.
- **Ver > Disposición > Restablecer la disposición** devuelve la forma que tiene ahora la ventana a sus valores iniciales: archivador, editor, inspector en una ventana ancha; el editor en el centro y el archivador a la izquierda en una vertical.

Ambos grupos están siempre en el menú; cada uno surte efecto cuando la ventana tiene esa forma, y cada forma vuelve tal como la dejaste (lugares y tamaños). Las elecciones pertenecen a este ordenador o tableta y se guardan en los ajustes del programa, nunca en un proyecto: un proyecto que compartes entre ordenadores conserva la disposición de cada dispositivo, y no se escribe nada de ello en los archivos del proyecto.

### Disposiciones

Una disposición recuerda cómo está organizada la ventana: si se muestran el archivador, las colecciones, el inspector, la barra de herramientas y la barra de formato, qué anchura tienen las columnas, cómo está dividido el editor, qué vista usa cada editor y en qué editor carga el archivador. No dice nada sobre los documentos, así que toda disposición funciona en cualquier proyecto.

Elige una disposición en **Ver > Disposición** o en el botón **Ver** de la barra de herramientas. Hay siete disposiciones incorporadas:

| Disposición | Organización |
|-------------|--------------|
| **Predeterminada** | Archivador, un editor, inspector |
| **Archivador y editor** | Archivador y texto, sin inspector |
| **Solo editor** | Solo el texto con las dos barras |
| **Esquema y editor** | Esquema a la izquierda, texto a la derecha. El archivador carga grupos en el esquema; un clic en una fila abre ese documento a la derecha |
| **Corcho y editor** | Corcho arriba, texto abajo, conectados del mismo modo |
| **Dos editores** | Dos textos uno junto a otro |
| **Planificación** | Archivador, corcho e inspector con sinopsis y notas |

**Ver > Disposición > Guardar la disposición actual** guarda la ventana tal como está ahora con un nombre que elijas; marca «Recordar también el tamaño de la ventana» si la disposición debe cambiar el tamaño de la ventana. **Gestionar disposiciones** muestra todas las disposiciones con una pequeña imagen y permite usar, actualizar (sustituir por la ventana actual), cambiar el nombre, eliminar, exportar e importar tus propias disposiciones. Las disposiciones incorporadas no se pueden modificar; guarda en su lugar una ventana modificada con un nombre nuevo. Cada disposición puede tener un atajo de teclado (sección 19, grupo «Disposiciones»).

### Las barras de herramientas y sus posiciones

Desde 0.91 la ventana tiene ocho barras de herramientas en dos filas (diez desde 0.95, con dos que están desactivadas al principio). La primera fila contiene **Principal** (Ver, texto nuevo, carpeta nueva, papelera, Deshacer, Rehacer), **Temas** (el botón de temas, desactivado al principio), **Perfiles** (el perfil en uso y un menú para cambiarlo, desactivada al principio; desde 0.95), **Búsqueda** (el botón de búsqueda, el campo de búsqueda rápida, los modos de vista) y **Herramientas** (marcadores, Referencia rápida, comentarios, Compilar, Mostrar invisibles, Modo de composición, los botones de las herramientas de escritura, el inspector). La segunda fila contiene **Estilo y fuente**, **Carácter y alineación**, **Párrafo e inserción**, la **barra de iconos** y **Etiquetas** (un botón por cada color de etiqueta del proyecto abierto y **Sin etiqueta**; desactivada al principio; desde 0.95).

- **Mover una barra de herramientas.** Cada barra de herramientas tiene un agarre al principio (dos columnas de puntos; el puntero se convierte en una cruz con flechas sobre él). Arrastra el agarre y la barra va adonde la lleves:
  - **A lo largo de su fila.** Mientras el puntero permanece entre los centros de las dos barras vecinas, la barra lo sigue de inmediato. El espacio libre delante de una barra es tan ancho como tú lo hagas. Al moverse hacia la derecha, la barra ocupa primero el espacio libre que hay detrás; cuando llega a la siguiente barra, la empuja, hasta donde las barras posteriores pueden estar juntas al final de la fila. Al moverse hacia la izquierda, se detiene donde termina la barra anterior, y las barras posteriores se quedan donde están (arrástralas hasta ella si quieres juntarlas de nuevo). La primera barra de una fila se queda en el borde de la ventana.
  - **A otro lugar del orden.** Lleva el puntero más allá del centro de una barra vecina, o sobre cualquier otra barra de la fila. La barra se queda por el momento donde está, y una línea del color de resaltado muestra dónde se colocará; suelta el botón del ratón para ponerla allí. Así es posible cualquier orden, también para la primera y la última barra de una fila.
  - **A otra fila.** Lleva el puntero a la otra fila: la línea muestra allí el lugar. Cualquier barra puede estar en cualquier fila, una barra de formato junto a la barra principal o la barra de búsqueda entre las barras de formato.
  - **A una fila propia, o al borde inferior.** Lleva el puntero justo debajo de las filas de barras (aparece una línea a lo ancho de la ventana) para abrir una fila nueva, o al borde inferior de la ventana para poner allí la barra. El borde inferior admite barras una junto a otra y en filas, como el superior.
  - **Esc**, o soltar donde no se muestra ninguna línea (por ejemplo, en medio de la ventana), lo deja todo como estaba.

  Las barras posteriores al antiguo lugar se juntan, y el espacio libre que había delante de la barra se queda en su fila; en su nuevo lugar la barra queda justo detrás de la barra anterior, y desde allí puedes tirar de ella más a la derecha. En una ventana demasiado estrecha para el espacio libre que dejaste, el espacio se reduce antes de que una barra pierda botones; vuelve cuando la ventana vuelve a ser ancha. Una barra de herramientas desactivada conserva su lugar. Una barra de herramientas no puede estar a los lados ni flotar como ventana propia. {{APP}} recuerda las filas, el orden y los espacios.
- **Desactivar una barra de herramientas.** **Ver > Barras de herramientas** tiene un interruptor para cada barra; la misma lista aparece con un clic derecho en una barra o en el espacio libre junto a las barras. **Mostrar la barra de herramientas** y **Mostrar la barra de formato** activan o desactivan una fila entera como antes (las cuatro barras de la primera fila, las tres barras de formato), y las disposiciones usan estos dos; el interruptor de una sola barra no se puede usar mientras su fila está desactivada.
- **Ver > Barras de herramientas > Restablecer la posición de las barras de herramientas** devuelve todas las barras a sus lugares habituales (las dos filas en su orden habitual, nada en el borde inferior) y cierra los espacios entre ellas.

### Barras de herramientas propias

**Ver > Barras de herramientas > Personalizar las barras de herramientas** (o un clic derecho en una barra) abre una ventana con dos listas: los comandos disponibles a la izquierda, el contenido de la barra a la derecha. Elige la barra de herramientas en la parte superior.

- Añade un comando con el botón de flecha o con un doble clic. Todos los comandos de los menús están disponibles, también los de las herramientas de escritura (Edición > Herramientas de escritura); el campo de búsqueda sobre la lista lo encuentra.
- Quita un botón con la otra flecha o con un doble clic, y muévelo con los botones de arriba y abajo o arrastrándolo.
- **Separador** y **Espacio flexible** se pueden usar tantas veces como quieras. Un espacio flexible empuja lo que sigue hacia la derecha; dos de ellos centran lo que queda entre ellos.
- Algunas entradas son más de un comando: **Ver**, **Añadir**, **Búsqueda**, el **campo de búsqueda rápida**, **Modo de vista**, **Marcadores del proyecto**, **Comentarios y notas al pie**, **Disposiciones** y **Temas** (el botón de temas con su menú) para las cuatro barras de la primera fila; las listas de estilo, fuente, tamaño e interlineado y los cuatro botones de párrafo para las tres barras de formato. Un control así existe una sola vez: añadido a una barra, se traslada allí desde la barra que lo tenía. Un control de la primera fila no puede pasar a una barra de formato, ni al revés.
- Debajo de las listas eliges iconos pequeños, medianos o grandes y si las barras de la primera fila muestran el texto junto a los iconos o debajo de ellos.
- **Conjunto predeterminado** devuelve a la barra los botones con los que venía el programa.

Si una barra contiene más de lo que cabe en la ventana, una flecha en su extremo abre el resto. Una barra de herramientas que se queda sin ningún botón se desactiva; si la vuelves a activar en **Ver > Barras de herramientas**, tiene su conjunto predeterminado. Una barra de herramientas que organizaste en una versión anterior a 0.91 se repartió entre las nuevas barras: cada botón pasó a la barra que lo contiene en los conjuntos predeterminados, y lo que habías quitado sigue quitado.

### La barra de iconos

La **barra de iconos** contiene hasta diez iconos de documento que usas a menudo. Un clic en uno de ellos da ese icono a los documentos y carpetas seleccionados (en el archivador, o en el corcho o el esquema en el que trabajas); el botón **↺** que va después los devuelve a su icono predeterminado, y el último botón elige los iconos de la barra. Muestra u oculta la barra con **Ver > Barras de herramientas > Barra de iconos**.

**Ver > Barras de herramientas > Elegir iconos para la barra de iconos…** (también en el menú de clic derecho de la barra) abre una ventana con todos los iconos a la izquierda, incluidos tus propios iconos de proyecto, y los iconos de la barra a la derecha. Añade con **Añadir →** o con un doble clic, quita con **← Quitar** o con un doble clic, y cambia el orden con **Subir** y **Bajar**. **Conjunto predeterminado** devuelve los diez iconos con los que venía el programa (Libro verde, Corrección, Marcado, Sin marcar, Signo de interrogación, Bandera roja, Bloc amarillo, Ficha de personaje, Ficha de lugar, Advertencia).

---

## 3. El archivador

Cada proyecto tiene tres carpetas especiales:

- **Borrador** (en algunas plantillas *Manuscrito*): todo lo que se convertirá en tu libro. Solo se compila lo que está dentro del Borrador.
- **Investigación**: notas, fichas de personajes, imágenes, PDF y cualquier otra cosa que reúnas.
- **Papelera**: los elementos eliminados esperan aquí hasta que eliges **Proyecto > Vaciar papelera**. Su pregunta, como toda pregunta antes de algo que no se puede deshacer, nombra el paso en su botón y tiene **Cancelar** como opción predeterminada, de modo que la tecla Enter nunca borra nada (0.70).

### Trabajar con elementos

- **Documento nuevo**: **Ctrl+N** (o el botón ➕). Se añade debajo del elemento seleccionado. Un documento llamado «Untitled» (o sin título) toma la primera línea de su texto como título en cuanto escribes: como máximo 60 caracteres, cortado en una palabra, sin anotaciones en línea, notas al pie en línea ni imágenes. El título sigue a la primera línea hasta que cambias tú mismo el nombre del documento; un documento cuyo texto borras vuelve a ser «Untitled». Los documentos «Untitled» que ya tienen texto reciben su nombre al abrir el proyecto. El título es un título corriente: se guarda, se compila (cuando se compilan los títulos, una primera línea que es un encabezado aparece dos veces) y se escribe en los proyectos de Scrivener.
- **Carpeta nueva**: **Alt+Shift+N**.
- **Cambiar el nombre**: **F2**, o doble clic lento en el título.
- **Mover**: arrastra los elementos con el ratón, o usa **Ctrl+Alt+teclas de flecha**: Arriba y Abajo mueven dentro de la carpeta; Izquierda saca de la carpeta; Derecha mete en el elemento superior. Si se suelta *entre* dos elementos, el documento va allí; si se suelta *sobre* un elemento, pasa a ser su subdocumento. Si anidas documentos por accidente, desactiva soltar sobre documentos de texto en **Archivo > Opciones > Comportamientos** (las carpetas siempre aceptan lo que se suelta).
- **Abrir o cerrar una rama entera**: mantén pulsada **Alt** y haz clic en la flecha de una carpeta o grupo.
- **Eliminar**: pulsa **Delete**. El elemento va a la Papelera; un corcho o un esquema que lo mostraba como uno de varios documentos elegidos lo suelta de inmediato (0.94.0).
- **Duplicar**: **Ctrl+D**.
- **Dividir y unir**: **Documentos > Dividir > en la selección** (**Ctrl+K**) corta el documento en dos en el cursor; no funciona dentro de una tabla ni del pie de una imagen (pon el cursor antes o después). **Documentos > Unir** (**Ctrl+Shift+M**) une los documentos seleccionados en el primero, con sus sinopsis, palabras clave y notas; los demás van a la Papelera.
- **Cambiar el icono**: clic derecho > **Cambiar icono** (o **Documentos > Cambiar icono**), o la barra de iconos (sección 2). Consulta «Iconos» más abajo.

**Clic derecho.** En un documento, el menú de clic derecho tiene los comandos para los documentos seleccionados (cambiar nombre, duplicar, mover a la Papelera, instantáneas, etiqueta, estado y más). En el espacio vacío debajo de los documentos solo ofrece lo que crea algo nuevo: **Nuevo texto**, **Nueva carpeta**, **Nuevo a partir de plantilla** y la **Apariencia** del archivador.

Un documento puede tener subdocumentos, igual que una carpeta. Las carpetas y los documentos se comportan del mismo modo; las carpetas simplemente tienen otro aspecto y se abren como corcho de forma predeterminada.

**Las teclas del archivador (0.94.0).** Con el teclado en el archivador, **Return** abre el documento seleccionado en el editor y lleva el teclado allí (como hace un gestor de archivos), y las teclas de flecha se mueven por la lista. **Espacio** forma parte de escribir un nombre: escribe las primeras letras de un título, espacios incluidos («Capítulo t»), y la selección salta al primer título que empieza así. La entrada **Ver > Teclas de adición rápida en el archivador** está desactivada al principio (también después de una actualización). Una vez activada, el archivador tiene las teclas de adición rápida: **Return** crea un documento nuevo vacío debajo del seleccionado y **Espacio** abre los documentos seleccionados como paneles de Referencia rápida (y en la lista de una colección o de una búsqueda).

Un clic en el archivador carga el documento en el editor. Después de **Atrás**, un enlace a un documento o con un editor bloqueado, el editor puede mostrar otro documento distinto del seleccionado en el archivador: una **barra gris** marca entonces en el archivador el documento del editor. **Navegar > Mostrar en el archivador** (**Ctrl+Shift+R**) lo selecciona.

**Archivador estrecho, carpetas profundas.** Cada nivel de carpetas se sangra 12 px (la mitad de lo que era antes; cámbialo en **Archivo > Opciones > Apariencia > Archivador > Sangría por nivel**). Cuando los títulos son más largos que el ancho del archivador, aparece una barra de desplazamiento en la parte inferior del archivador, de modo que puedes leer el final de los títulos largos sin ensanchar el archivador.

### Iconos

**Documentos > Cambiar icono** (también en el menú de clic derecho del archivador, el corcho y el esquema) está construido como el menú de iconos de Scrivener 3 para Windows:

- **Restablecer el icono predeterminado**: vuelve el icono estándar de un documento, carpeta o archivo.
- **Icono a partir de texto…**: crea un icono de hasta tres caracteres (por ejemplo *PR*, *Cap1* o *A*). Elige el color del recuadro, de las letras y del borde; la pequeña imagen muestra el resultado. El icono se guarda en el proyecto y se da de inmediato a los documentos seleccionados.
- **Gestionar iconos…**: los iconos de este proyecto. **Añadir imagen…** convierte imágenes (PNG, SVG, JPEG y otras) en iconos, **A partir de texto…** es lo mismo que arriba, **Cambiar nombre…** y **Eliminar** modifican la lista (los documentos con un icono eliminado recuperan su icono estándar), **Usar para los documentos seleccionados** da el icono elegido a los documentos seleccionados. Los iconos se guardan en el proyecto (carpeta *icons*), de modo que viajan con él.
- **Recientes**: los últimos cinco iconos que elegiste.
- **Libro**, **Personajes**, **Bandera**, **Lugares**, **Notas**, **Tareas pendientes**, **TV**, **Etapa**: submenús con los iconos de cada grupo en varios colores (por ejemplo libro azul, verde, rojo y amarillo; siete banderas; cinco blocs de notas; Marcado y Sin marcar, Aprobado, Suspenso y No claro en Tareas pendientes).
- Después los iconos sueltos: Contraportada, Pizarra, Calendario, Claqueta, Reloj, Engranaje, Ojo, Cifras y gráficos, Archivador de cajones, Páginas preliminares, Información, Atril, Bombilla, Lupa, Música, Notas musicales, Esquema, Lápiz, Corrección, Signo de interrogación, Comentarios del revisor, Tijeras, Bocadillo de diálogo, Comillas, Estadísticas, Carpeta de sincronización, Tablas, Tubo de ensayo, Bocadillo de pensamiento y Advertencia.
- **Iconos del proyecto**: tus propios iconos (cuando el proyecto tiene alguno), y **Más iconos (escritorio)**: los iconos del tema del escritorio que ofrecían las versiones anteriores.

**Color de etiqueta en los iconos.** Las partes blancas de muchos iconos (las hojas de un bloc de notas, la tela de una bandera blanca, el papel de una hoja, el recuadro de un icono hecho a partir de texto) adoptan el color de etiqueta del documento. Actívalo o desactívalo con **Ver > Usar el color de etiqueta en > Iconos** (o **Archivo > Opciones > Apariencia > Archivador**).

**Iconos y Scrivener.** Los iconos de {{APP}} están dibujados de nuevo, pero cada uno lleva el nombre del icono de Scrivener al que corresponde. Un proyecto en formato Scrivener (sección 14) conserva esos nombres, de modo que Scrivener muestra su propio icono para ellos; los iconos creados en Scrivener vuelven aquí como el icono correspondiente. Tus propios iconos van a la carpeta *Icons* del *.scriv*. Un icono para el que este programa no tiene imagen conserva su nombre y se muestra aquí como el icono más cercano.

### Etiquetas, estado e «Incluir en la compilación»

Haz clic derecho en un elemento para establecer su **etiqueta** (un color, por ejemplo para el personaje del punto de vista) y su **estado** (por ejemplo *Primer borrador* o *Terminado*). Cambia las listas en **Proyecto > Ajustes del proyecto**.

Un documento o carpeta con etiqueta muestra su color también en el inspector: la ficha de la sinopsis y las notas del documento toman un tono claro del color de la etiqueta (el 15 % de su saturación), de modo que ves de un vistazo qué etiqueta tiene el elemento. Los elementos sin etiqueta, o con una gris, conservan el color normal de la ficha.

La barra de herramientas **Etiquetas** (**Ver > Barras de herramientas > Etiquetas**, desactivada al principio) muestra uno junto a otro todos los colores de etiqueta del proyecto abierto, con un botón para ninguna etiqueta. Un clic establece de inmediato la etiqueta de los documentos seleccionados, igual que **Documentos > Etiqueta**; la información emergente de cada color nombra la etiqueta. Las etiquetas con las que empieza un proyecto nuevo se establecen en **Archivo > Opciones > Apariencia > Colores estándar**.

Los elementos con **Incluir en la compilación** desactivado se quedan en el Borrador pero se omiten al compilar.

### Colecciones

Las colecciones son listas adicionales de documentos, por ejemplo «Requiere investigación» o «Escenas con Ana». Muéstralas con el botón **Colecciones**. Usa el botón ➕ sobre el archivador para crear una colección a partir de los documentos seleccionados, o haz clic derecho en documentos y elige **Añadir a la colección**. Quitar un documento de una colección no lo elimina. Las búsquedas guardadas (sección 9) también son colecciones; se actualizan solas.

- **Añade documentos** arrastrándolos desde el archivador (o desde el icono del encabezado de un editor, un corcho, un esquema) a la pestaña de la colección.
- **Dentro de la lista** de una colección, arrastra los documentos al orden que prefieras (el orden del archivador no cambia), suelta documentos en un lugar y pulsa **Delete** para sacar de la colección los documentos seleccionados. Las colecciones de búsqueda no se pueden cambiar de este modo.
- **Arrastra las pestañas** para poner las colecciones en otro orden.
- Los documentos de la lista se pueden arrastrar como los del archivador, y **Espacio** abre los seleccionados como paneles de Referencia rápida.

---

## 4. Escribir

### Formato de texto

| Acción | Atajo |
|--------|-------|
| Negrita / cursiva / subrayado | Ctrl+B / Ctrl+I / Ctrl+U |
| Resaltar | Ctrl+Shift+H |
| Superíndice / subíndice | Ctrl+Shift++ / Ctrl+= (disposición alemana: Ctrl+Shift+D para el subíndice, consulta «Atajos y la disposición del teclado» más abajo) |
| Alinear a la izquierda / centrar / alinear a la derecha | Ctrl+{ / Ctrl+\| / Ctrl+} (disposición alemana: Alt+Shift+L / Alt+Shift+E / Alt+Shift+R) |
| Ventana de fuentes | Ctrl+T |
| Tamaño de fuente mayor / menor | Ctrl+> / Ctrl+< |
| Ventana de párrafo | Ctrl+Alt+P |
| Copiar formato / pegar formato | Ctrl+Alt+C / Ctrl+Alt+V |
| Ampliar / reducir / restablecer zoom | Ctrl++ / Ctrl+- / Ctrl+0 |
| Zoom del área bajo el ratón | Ctrl+rueda del ratón |
| Mostrar u ocultar la regla | Ctrl+Alt+R |
| Pegar sin formato | Ctrl+Shift+V |
| Pegar con formato (desde otros programas) | Ctrl+Alt+Shift+V |
| Deshacer / rehacer | Ctrl+Z / Ctrl+Shift+Z (consulta «Deshacer y rehacer» más abajo) |

La negrita, la cursiva, el superíndice y similares actúan sobre la selección. Sin selección, dentro de una palabra cambian la palabra entera; al final de una palabra (o sin nada alrededor del cursor) cambian lo que escribas a continuación: escribe «H», pulsa Ctrl+=, escribe «2», pulsa Ctrl+= otra vez y escribe «O» para obtener H₂O.

Los **estilos** (Encabezado 1–6, Título, Subtítulo, Cita en bloque y los tuyos) se eligen en la lista a la izquierda de la barra de formato. Los estilos le dicen a Compilar qué es un párrafo, así que úsalos en lugar de poner el texto grande y en negrita a mano. Consulta «Estilos» más abajo.

**Lo que muestra la barra de formato.** La barra muestra el texto tal como lo ves, para toda la selección. **Negrita**, **Cursiva**, **Subrayado**, **Tachado**, el superíndice y el subíndice están activados cuando todo el texto seleccionado tiene ese aspecto, también cuando su estilo lo hace así (la negrita de un encabezado). Un clic cambia lo que ves: **Negrita** en un encabezado en negrita lo deja sin negrita, y sigue sin negrita cuando el proyecto se guarda y se vuelve a abrir. La fuente, el tamaño, el interlineado, los botones de alineación y la lista de estilos muestran también la selección; cuando la selección contiene dos fuentes, dos tamaños, dos alineaciones o párrafos de dos estilos, ese campo queda vacío (ningún botón de alineación está activado). Un tamaño escrito en el campo vacío da ese tamaño a toda la selección.

**Listas**: usa los botones de lista, o escribe «- » o «1. » al principio de un párrafo.

**Tipografía inteligente**: las comillas rectas se convierten en comillas tipográficas, y dos guiones se convierten en una raya mientras escribes. Desactívalo en **Archivo > Opciones > Edición**.

**Modo de revisión** (**Formato > Modo de revisión**): todo lo que escribes se colorea con uno de cinco colores de revisión, de modo que puedes ver qué cambió en una ronda de edición. Elige **Desactivado** para detenerlo.

**Espaciado y sangrías en la barra de formato.** Cuatro botones después de los botones de lista establecen los párrafos que contienen el cursor o la selección: el **espacio antes** y el **espacio después** del párrafo (0, 3, 6, 9, 12 o 18 pt), la **sangría de la primera línea** y la **sangría de todo el párrafo** (en centímetros o pulgadas, como los muestra la regla). El valor del párrafo en el cursor aparece marcado; **Otro...** pide cualquier valor. **Edición > Deshacer** revierte un cambio. A una barra de formato que organizaste tú mismo se le añaden los cuatro botones una vez, después de los botones de lista; **Ver > Barras de herramientas > Personalizar las barras de herramientas** los mueve o los quita.

**Deshacer y rehacer.** **Edición > Deshacer** y **Edición > Rehacer** actúan en el campo de texto que tiene el teclado (el editor, la sinopsis, las notas, un título) e indican qué revertirán: «Deshacer escritura», «Deshacer pegado», «Deshacer negrita», «Rehacer eliminar». Con el teclado en otro lugar, por ejemplo en el archivador, aparecen en gris: mover documentos a la Papelera, vaciar la Papelera y eliminar instantáneas no se pueden deshacer de este modo (un documento de la Papelera vuelve arrastrándolo fuera; para las instantáneas, consulta la sección 11). La flecha junto a los botones **Deshacer** y **Rehacer** de la barra de herramientas enumera los pasos, el siguiente primero; al elegir uno se revierte (o se restablece) ese paso y todos los anteriores de la lista. Cada comando del menú Formato es un paso.

**Color del texto y resaltado en la barra de formato.** Los dos botones con una barra de color (un bolígrafo para resaltar, una letra *T* para el color del texto) funcionan como los de Scrivener y Word: un clic aplica el color que muestra la barra al texto seleccionado o, sin selección, a lo que escribas a continuación. Deja el ratón un momento sobre un botón y debajo se abre una pequeña ventana con **rojo, azul, amarillo, verde, blanco y negro**, además de **Sin resaltado** (o **Automático** para el color del texto) y **…** para cualquier otro color. Un clic en un color lo aplica y lo convierte en el color del botón, que lo conserva para el siguiente clic (también después de reiniciar). Un resaltado se dibuja exactamente en el color que elegiste, tanto sobre una página clara como sobre una oscura. Donde el texto sobre él sería difícil de leer (por debajo de un contraste de 4,5 : 1), el texto se muestra oscuro o claro solo en pantalla; su color guardado, y el resaltado en los archivos guardados y compilados, se quedan como los elegiste.

### Fuentes

Todo el texto de un proyecto tiene un aspecto: el **formato predeterminado** (véase más abajo). Encima de él puedes dar a cualquier fragmento de texto una fuente propia.

- **Barra de formato**: la lista de fuentes y el campo de tamaño cambian de inmediato el texto seleccionado. Sin selección cambian el **párrafo entero** que contiene el cursor (y lo que escribas a continuación allí). Las flechas del campo de tamaño cambian el texto con cada clic; un tamaño que escribes surte efecto con **Enter**. La barra de formato, el menú Formato y sus atajos actúan sobre el texto en el que estuvo el cursor por última vez: un editor, las notas del inspector, el texto de un marcador en el inspector o un campo de texto largo de un objeto (allí: negrita, cursiva, subrayado, tachado, color del texto y resaltado). Un clic en el campo de tamaño no cambia eso.
- Tras un cambio en la barra de formato, una barra sobre el editor ofrece lo mismo para todo el proyecto: en un párrafo con estilo (un encabezado, por ejemplo), **Cambiar el estilo «Encabezado 1»** da al estilo la nueva fuente o tamaño, y todo el texto de ese estilo en todos los documentos lo sigue; en un párrafo sin estilo, **Usar para todo el texto** lo convierte en la fuente o el tamaño del proyecto (consulta «Formato predeterminado» más abajo). Haz clic en **×**, o simplemente sigue escribiendo en otro sitio, para dejar el cambio donde está. Cada documento que cambia recibe antes una instantánea.
- **Formato > Fuente > Mostrar fuentes…** (**Ctrl+T**) abre la ventana de fuentes: fuente, tipo (fina, normal, negrita, cursiva y lo que más ofrezca la fuente), tamaño, subrayado, tachado, mayúsculas (versalitas o todo en mayúsculas), espaciado entre letras en porcentaje y color del texto. Solo se aplica lo que cambias en la ventana, de modo que una selección con palabras en negrita y normales las conserva. **Fuente del proyecto** devuelve el texto a la fuente, el tamaño y el color del proyecto.
- **Negrita, Cursiva, Subrayado, Tachado**: un botón está activado cuando *todo* el texto seleccionado tiene ese formato, como sea que lo hayas seleccionado (también de derecha a izquierda). Una selección que solo es negrita en parte muestra Negrita desactivada, y **Negrita** pone entonces todo en negrita; un segundo clic lo deja todo normal. En la ventana de fuentes, un tipo que eliges también se aplica a toda la selección.
- **Formato > Fuente > Más grande / Más pequeño** (**Ctrl+>** / **Ctrl+<**) cambian el tamaño en un punto; cada parte de la selección conserva su propio tamaño y crece o se reduce.
- **Formato > Fuente > Color del texto…** y **Color del texto automático**.

Elegir la fuente o el tamaño propios del proyecto se guarda como «sin fuente propia»: ese texto sigue al proyecto cuando más tarde cambias el formato predeterminado.

**Fuentes con varios diseños.** Algunas fuentes guardan varios diseños de un mismo tipo bajo un solo nombre: Sitka de Windows 11, por ejemplo, contiene los tamaños ópticos Small, Text, Subheading, Heading, Display y Banner (cada uno dibujado para su propio intervalo de tamaños, los mayores más estrechos y con minúsculas más pequeñas), y algunas fuentes contienen versiones condensadas o anchas de la misma manera. {{APP}} enumera cada diseño como una fuente aparte (por ejemplo **Sitka Small**, **Sitka Text**, **Sitka Banner**), solo con los tipos de ese diseño, de modo que la negrita y la cursiva se quedan dentro del diseño que elegiste; antes de 0.40.1, el texto en negrita podía salir en otro diseño y verse más pequeño y estrecho. Los archivos de fuente no se modifican. La fuente con el nombre original conserva su propio diseño, de modo que el texto compuesto antes con ella mantiene su aspecto.

**Instalar fuentes.** **Formato > Fuente** tiene los comandos para fuentes que aún no están en tu ordenador:

- **Instalar archivos de fuente…**: elige archivos de fuente (TTF, OTF, TTC, PFB, WOFF o un archivo ZIP que los contenga). Se copian en tu propia carpeta de fuentes (*~/.local/share/fonts*), y las listas de fuentes de {{APP}} las conocen de inmediato; otros programas las ven tras su siguiente inicio. No hace falta contraseña, y las fuentes son solo para tu usuario.
- **Abrir carpeta de fuentes**: abre esa carpeta en el gestor de archivos; las fuentes que pongas allí tú mismo se encuentran después de **Recargar la lista de fuentes**.
- **Abrir el gestor de fuentes del escritorio**: abre el gestor de fuentes de KDE (Preferencias del sistema > Texto y fuentes > Gestión de fuentes) o el programa de fuentes de otro escritorio, donde también puedes instalar fuentes para todos los usuarios y eliminarlas.
- **Recargar la lista de fuentes**: busca fuentes nuevas y las nombra.

### Formato de párrafo

**Formato > Párrafo > Párrafo…** (**Ctrl+Alt+P**) establece, para los párrafos que contienen el cursor o la selección: alineación, sangría izquierda y derecha, sangría de la primera línea (un valor negativo da una sangría francesa), espacio antes y después, interlineado (un múltiplo de la altura de línea, una altura exacta o una altura mínima) y tabuladores (izquierda, centrado, derecha, punto decimal). Solo se aplica lo que cambias. **Restablecer valores predeterminados** quita el formato que estableciste a mano; el párrafo vuelve a seguir a su estilo y al proyecto.

Formas rápidas: los botones de alineación, la lista de interlineado de la barra de formato, **Formato > Párrafo > Interlineado** y **Aumentar sangría / Disminuir sangría**.

Aplicar un estilo de párrafo sustituye el formato establecido a mano en los párrafos a los que llega: sangrías, espaciado, alineación y tabuladores, y las fuentes, tamaños, colores, espaciado entre letras y mayúsculas del texto que contienen. La negrita, la cursiva, el subrayado, el tachado, los resaltados y los estilos de carácter se conservan. Selecciona texto a través de varios párrafos para dar a todos el estilo, a cada párrafo entero; una selección que termina justo al principio de un párrafo (por ejemplo después de **Shift+Abajo**) deja ese párrafo en paz.

**Formato > Copiar formato** (**Ctrl+Alt+C**) recuerda el formato de fuente y de párrafo en el cursor; **Pegar formato** (**Ctrl+Alt+V**) lo aplica a la selección, o a todo el párrafo cuando no hay nada seleccionado.

### Caracteres invisibles

**Ver > Edición de texto > Mostrar invisibles** (también el botón ¶ de la barra de herramientas) muestra los caracteres que no se ven: un punto por cada espacio, una flecha por cada tabulador, un círculo pequeño por un espacio de no separación, una flecha doblada por un salto de línea (**Shift+Enter**) y ¶ al final de cada párrafo. Se dibujan solo en la pantalla, en el texto principal de cada editor; nunca forman parte del texto, de una impresión ni de la compilación. La elección se conserva. Su color es el del texto, difuminado en el papel; un color propio se establece en **Archivo > Opciones > Apariencia > Marcas de texto** (o clic derecho en el texto > **Apariencia**), y pertenece al aspecto del proyecto como el papel y el texto (el modo de composición conserva el color de texto difuminado).

### La regla

**Ver > Edición de texto > Mostrar la regla** (**Ctrl+Alt+R**) muestra una regla sobre el texto. Pertenece al párrafo que contiene el cursor.

- El marcador de arriba es la **sangría de la primera línea**; los marcadores de abajo son la **sangría izquierda** y la **sangría derecha**. Arrástralos. La sangría izquierda arrastra consigo la primera línea.
- **Haz clic** en la mitad inferior de la regla para añadir un tabulador. Arrastra un tabulador para moverlo; arrástralo hacia arriba o hacia abajo fuera de la regla para quitarlo. **Clic derecho** en un tabulador para hacerlo izquierdo, centrado, derecho o decimal.
- Los marcadores se ajustan a cuartos de centímetro (dieciseisavos de pulgada); mantén pulsada **Shift** para colocarlos libremente.
- **Doble clic** en la regla para la ventana de párrafo. El menú de clic derecho cambia entre centímetros y pulgadas (también en **Archivo > Opciones > Edición**).

Los tabuladores se miden desde el comienzo de la primera línea del párrafo. Sin tabuladores propios, un párrafo tiene una posición de tabulación cada 2,1 cm.

La regla muestra longitudes reales. El zoom agranda las letras en la pantalla pero no las sangrías ni los tabuladores, así que compara las distancias al 100 %.

### Formato predeterminado

- **Este proyecto**: **Proyecto > Ajustes del proyecto > Formato** (también el botón de deslizadores de la barra de formato) establece la fuente, el tamaño, el interlineado, la sangría de la primera línea, el espacio después de los párrafos y la alineación del texto de todo el proyecto (cuatro botones: izquierda, centrado, derecha, justificado). En el Panel de estilos, **Sin estilo** > **Redefinir a partir de la selección** toma estos valores del párrafo en el cursor (consulta «Estilos» más abajo). **Aplicar** cambia todos los documentos a la vez y deja la ventana abierta, de modo que puedes ver el resultado detrás de ella; **Aceptar** hace lo mismo y cierra la ventana. **Usar el actual** toma los valores del párrafo que contiene el cursor.
  - Con **Cambiar también el texto con fuente, tamaño o espaciado propios** (activado de forma predeterminada), el texto que se formateó a mano o que llegó con su propia fuente desde una importación también sigue, siempre que tenga la fuente, el tamaño y el espaciado principales de su documento. Lo que difiere del texto principal se queda como está: una línea de encabezado mayor, un pasaje en otra fuente, un párrafo sin sangría. Desactiva la casilla para dejar en paz todo el texto formateado a mano.
  - Cada documento que cambia recibe antes una instantánea (inspector, pestaña de instantáneas, «Antes de que cambiara el formato del proyecto»), de modo que puedes volver atrás. Los documentos de guion conservan su propio formato.
- **Proyectos nuevos**: **Archivo > Opciones > Edición > Formato de los proyectos nuevos**, con los mismos cuatro botones de alineación y **Usar el formato del editor actual**. Los proyectos existentes conservan su formato. Lo habitual es 11,0 pt, interlineado 1,09, sin sangría de primera línea y 6 pt después de los párrafos. Los proyectos importados de Scrivener empiezan también con estos valores (con la fuente más usada en el proyecto de Scrivener).
- **Volver al formato predeterminado**: selecciona documentos en el archivador y elige **Documentos > Convertir > Texto al formato predeterminado…**. Tú decides si se quitan las fuentes y los colores, las sangrías y el espaciado, la alineación y los tabuladores. La negrita, la cursiva, los estilos, las notas y los enlaces siempre se conservan. Antes se toma una instantánea de cada documento (inspector, pestaña de instantáneas), de modo que puedes volver atrás.

### Estilos

Un estilo guarda un formato bajo un nombre. El texto recuerda el nombre, así que cuando cambias el estilo, todo el texto que lo usa cambia con él, en todo el proyecto.

Hay dos clases:

- Los **estilos de párrafo** se aplican a párrafos enteros. Contienen formato de párrafo (alineación, sangrías, espaciado, interlineado, tabuladores) y, si quieres, formato de carácter para el párrafo (fuente, tamaño, negrita, cursiva, color). En las listas se marcan con «¶», o «¶a» cuando incluyen formato de carácter.
- Los **estilos de carácter** se aplican al texto seleccionado o a la palabra en el cursor. Contienen solo formato de carácter. Se marcan con «a».

Deja el texto del cuerpo en **Sin estilo**. Sigue el formato predeterminado del proyecto, y Compilar puede darle cualquier aspecto. Usa estilos solo para el texto que deba verse distinto del texto del cuerpo.

**Aplicar un estilo**: elígelo en la lista a la izquierda de la barra de formato, en **Formato > Estilo**, en el menú de un clic derecho en el texto o en el **Panel de estilos** (**Formato > Estilo > Panel de estilos**), una pequeña ventana que permanece abierta junto al texto; un doble clic en una fila aplica el estilo. **Sin estilo** quita un estilo de carácter de la selección; donde no hay ninguno, quita el estilo de párrafo. Las ventanas y preguntas que abre el Panel de estilos aparecen encima del panel.

**La lista de estilos** de la barra de formato y del menú de clic derecho (**Estilo**) muestra los estilos que eliges, en el orden que eliges: **Personalizar la lista de estilos…** (al final de la lista y en **Formato > Estilo**) enumera todas las entradas; desmarca las que no necesites, arrástralas a otro orden o usa las flechas, y **Todos, en el orden de los estilos** recupera la lista completa. Con más de diez entradas, la lista muestra diez y se desplaza. Debajo, siempre a la vista: **Nuevo estilo a partir de la selección…**, **Redefinir el estilo a partir de la selección…**, **Panel de estilos** y **Personalizar la lista de estilos…**. Un estilo que no está en la lista sigue nombrándose en la barra de formato cuando el texto lo usa; **Formato > Estilo** siempre enumera todos los estilos. La elección pertenece al proyecto (un estilo nuevo aparece al final de la lista) y forma parte de los estilos de los archivos de ajustes del proyecto.

**Sin estilo es el estilo del texto principal.** En el Panel de estilos, haz clic derecho en **Sin estilo** (o selecciónalo y usa el botón Redefinir) y elige **Redefinir a partir de la selección…**: la fuente, el tamaño, el interlineado, la sangría de la primera línea, el espacio después de los párrafos y la alineación del párrafo en el cursor pasan a ser el estilo del texto principal del proyecto (los valores de **Proyecto > Ajustes del proyecto > Formato**). Una pregunta muestra antes los valores. Todo el texto sin estilo de todo el proyecto lo sigue, también el texto con fuente, tamaño o espaciado propios; el texto con estilo conserva lo que establece su estilo. Cada documento que cambia recibe antes una instantánea.

**Crear un estilo**:

1. Da a un texto el formato que debe tener el estilo y haz clic en él o selecciona una parte.
2. Elige **Formato > Estilo > Nuevo estilo a partir de la selección…** (o el botón de más del Panel de estilos).
3. Escribe un nombre y elige qué guardar: todo el formato (un estilo de párrafo con formato de carácter), solo el formato de párrafo o los atributos de carácter (un estilo de carácter).
4. Decide si la fuente y su tamaño forman parte del estilo. Sin ellos, el estilo conserva la fuente del texto que lo rodea.
5. Para los estilos de párrafo, elige qué estilo recibe el párrafo siguiente cuando pulsas Enter al final.

Un estilo de párrafo guarda solo lo que difiere del texto del cuerpo. Todo lo demás sigue el formato del proyecto, de modo que una nueva fuente predeterminada llega también al texto con estilo.

**Cambiar un estilo**:

- **Formato > Estilo > Redefinir el estilo a partir de la selección…** toma el formato del texto en el cursor, como arriba. Un estilo de párrafo sigue siendo un estilo de párrafo, y uno de carácter, uno de carácter.
- **Formato > Estilo > Editar estilo** (o el botón del lápiz del Panel de estilos) abre una ventana con el nombre, **Fuente…**, **Párrafo…** y el estilo del párrafo siguiente. Una vista previa muestra el resultado.

Después {{APP}} recorre todo el proyecto y actualiza el texto, en los documentos abiertos y en los archivos de los demás. Lo que toca el cambio (fuente, tamaño, negrita, cursiva, color, espaciado entre letras, mayúsculas, alineación, sangrías, espaciado, interlineado, tabuladores) llega a **todo** el texto del estilo, también donde ese texto tenía un valor propio: establecido a mano o traído por una importación (los encabezados importados suelen nombrar su fuente, tamaño y espaciado en cada párrafo). El formato que el cambio no toca se queda como está, por ejemplo una palabra coloreada a mano en un encabezado cuando solo cambió el tamaño del encabezado. Cada documento que cambia recibe antes una instantánea («Antes de que cambiara el estilo ...»). Esto no se puede deshacer con **Edición > Deshacer**; restaura una instantánea, redefine el estilo de nuevo o usa **Restablecer la definición incorporada** en el Panel de estilos para un estilo incorporado.

**Cambiar el nombre y eliminar**: cambia el nombre de un estilo en **Editar estilo**. Los encabezados, Título, Subtítulo, Cita en bloque, Centrado y Pie de imagen conservan sus nombres, porque Compilar y los archivos Markdown los conocen por su nombre. Cuando eliminas un estilo (**Formato > Estilo > Eliminar estilo**), el texto que lo usaba conserva su aspecto como formato establecido a mano.

**Otros proyectos**: **Formato > Estilo > Importar estilos…** lee los estilos de otro proyecto. **Usar estos estilos en los proyectos nuevos** convierte los estilos del proyecto abierto en el conjunto inicial de cada proyecto nuevo; **Archivo > Opciones > Edición** vuelve a los estilos incorporados.

En el Panel de estilos, los botones de flecha cambian el orden de los estilos en todas las listas.

### Markdown o RTF: cómo se almacenan los documentos

Cada proyecto almacena sus documentos en uno de dos formatos. Lo eliges al crear o importar un proyecto, y puedes cambiarlo más tarde en **Proyecto > Ajustes del proyecto > Formato**.

- **Markdown** (recomendado): archivos de texto sin formato que cualquier programa puede abrir, ahora y dentro de cincuenta años. El aspecto del texto procede del formato predeterminado. Las fuentes y el formato de párrafo que estableces a mano se escriben en el texto como pequeños marcadores, solo donde los usaste (sección 18); todo lo demás se queda sin formato.
- **RTF**: archivos de texto enriquecido como los de un procesador de textos. Cada párrafo lleva su formato completo, de modo que un cambio del formato predeterminado no llega al texto existente.

Cambiar convierte todos los documentos. De RTF a Markdown se conservan las fuentes, los colores, la alineación, las sangrías izquierda y derecha y los tabuladores. El interlineado, la sangría de la primera línea y el espaciado de párrafo del texto que venía de otros programas siguen después al proyecto. Haz antes una copia de seguridad (**Archivo > Copia de seguridad > Hacer copia de seguridad ahora**).

El texto pegado desde otros programas en un proyecto Markdown toma el aspecto del texto que lo rodea (la negrita, la cursiva y los enlaces se conservan). El texto copiado dentro de {{APP}} conserva sus fuentes. **Edición > Pegar con formato** (**Ctrl+Alt+Shift+V**) conserva también el aspecto del texto de un procesador de textos o de una página web: fuentes, tamaños, colores, y la alineación, las sangrías y el espaciado de sus párrafos (el primer párrafo pegado trae su formato de párrafo solo a un párrafo vacío). **Edición > Pegar y adaptar el estilo** (**Ctrl+Shift+V**) hace lo contrario y pega texto sin formato. El menú de clic derecho del texto ofrece los tres, uno debajo de otro: **Pegar**, **Pegar y adaptar el estilo** y **Pegar con formato**.

Para convertir un solo archivo fuera de un proyecto, usa **Archivo > Convertir archivo Markdown ⇄ RTF…**.

### Comentarios y notas al pie

- **Comentario** (**Shift+F2**): selecciona texto y añade un comentario. El texto se resalta y el comentario aparece en el inspector.
- **Nota al pie** (**Ctrl+Shift+F2**): selecciona la palabra a la que pertenece la nota al pie; escribe la nota al pie en el inspector.
- **Anotación en línea** (**Ctrl+Shift+A**): una nota escrita directamente en el texto, mostrada en color dentro de un bocadillo. Las anotaciones se quitan al compilar (a menos que elijas exportarlas como comentarios).
- **Nota al pie en línea** (**Ctrl+Alt+F**): una nota al pie escrita directamente en el texto.

Para cambiar el color de una anotación en línea, selecciónala (**Edición > Seleccionar > Seleccionar anotación o nota** selecciona la nota entera en el cursor) y elige un color de texto; la anotación conserva ese color. **Formato > Fuente > Color del texto automático** le devuelve el rojo habitual.

Haz clic en un comentario del inspector para saltar a él. Haz clic derecho en él para cambiar su color, convertirlo en otra clase de nota o eliminarlo.

Un clic en texto comentado abre el inspector con ese comentario. **Alt+clic** abre en cambio el comentario o la nota al pie en una pequeña ventana junto al ratón, donde puedes leerlo y modificarlo sin el inspector; haz clic en otro sitio o pulsa **Esc** para cerrarla.

### Imágenes y enlaces

- **Insertar > Imagen desde archivo…** coloca una imagen en el texto; también puedes arrastrar una imagen al texto o pegarla. Haz clic derecho en la imagen y elige **Tamaño de la imagen…** para cambiar el tamaño con que aparece.
- **Insertar > Enlazar con un documento…** (**Ctrl+L**) enlaza el texto seleccionado con otro documento del proyecto. Al hacer clic en el enlace se abre ese documento (en el otro editor, en el mismo editor, en su documento de apoyo o en un panel de Referencia rápida; consulta **Archivo > Opciones > Comportamientos**).
- **Insertar > Insertar enlace web…** enlaza con una página web.
- Los enlaces web y de correo (*http*, *https*, *mailto*) se abren de inmediato. Cualquier otro destino de un enlace, en un texto, un mapa mental, un marcador o una página web importada (un archivo, una carpeta, una dirección de red, el tipo de enlace de otro programa), se muestra primero completo y solo se abre cuando haces clic en **Abrir**; tales enlaces pueden venir con un proyecto, una página web o un archivo de Word de otra persona. Para un archivo o carpeta de tu ordenador, la pregunta nombra su clase (por ejemplo «documento de texto sin formato (.txt)»), su nombre completo y su carpeta; un nombre largo se divide en líneas para que se pueda leer su final, y un archivo que no existe se indica como ausente. Los enlaces a programas, scripts, accesos directos e instaladores nunca se abren; ábrelos tú mismo desde tu gestor de archivos si confías en ellos.

### Tablas

**Insertar > Tabla…** pregunta el número de filas y columnas y el diseño, y coloca la tabla en el cursor. Dentro de una tabla, **Tab** va a la celda siguiente (en la última celda añade una fila) y **Shift+Tab** vuelve atrás. **Formato > Tabla**, también en el menú de clic derecho dentro de una tabla, tiene el resto:

- **Insertar fila encima / debajo**, **Insertar columna a la izquierda / derecha**.
- **Eliminar filas** y **Eliminar columnas** (las de las celdas seleccionadas o las de la celda con el cursor), **Eliminar tabla**.
- **Combinar celdas** une las celdas seleccionadas en una; **Dividir celda** vuelve a dividir una celda combinada.
- **Color de celda…** da a las celdas seleccionadas un fondo propio; **Color de celda del diseño** lo quita de nuevo.
- **Propiedades de la tabla…** cambia el diseño.

**El diseño.** Hay cinco diseños preparados: **Cuadrícula**, **Cuadrícula con fila de encabezado**, **Filas con bandas**, **Líneas entre las filas** y **Sin líneas**. Elige uno y cambia lo que quieras; el diseño pasa entonces a ser **Propio**:

- **Líneas**: grosor (0: sin líneas), color y **Solo entre las filas** (sin líneas en los lados).
- **Celdas**: el espacio dentro de una celda y el espacio entre celdas (0: las celdas vecinas comparten una línea).
- **Filas de encabezado**: cuántas filas de la parte superior son encabezados, con su color. La impresión y la compilación las repiten en la parte superior de cada página a la que pasa una tabla larga. **Una fila de cada dos** recibe un color propio si eliges uno.
- **Lugar en el texto**: el ancho (una proporción del ancho del texto, o tan ancho como su contenido), los anchos de las columnas (todos iguales, o proporciones como «30, 70»), la alineación y dónde se coloca: **En líneas propias**, o **A la izquierda** o **A la derecha** con el texto fluyendo a su lado.

Las filas y columnas que añadas más tarde conservan el diseño (las bandas continúan). El texto de las celdas de una tabla no se justifica, tampoco cuando el texto del cuerpo lo está (una celda estrecha dispersaría sus palabras); una alineación que das a un párrafo de una celda se conserva. Las tablas se conservan en ambos formatos de almacenamiento y en formato Scrivener; las tablas de archivos RTF de otros programas (por ejemplo, un archivo de Word guardado como RTF) también se convierten en tablas.

### Imágenes junto al texto, pies de imagen y descripciones

Haz clic derecho en una imagen y elige **Imagen…** (o pon el cursor junto a ella y elige **Formato > Imagen…**):

- **Ancho en pantalla**: la altura se ajusta; **Tamaño original** vuelve al tamaño propio de la imagen.
- **Posición**: **En la línea, como una letra** (como antes), **A la izquierda** o **A la derecha** con el texto fluyendo alrededor, o **Centrada en líneas propias**.
- **Pie de imagen**: se muestra debajo de la imagen y se compila con ella. Una imagen en la línea no tiene pie; dale primero una posición.
- **Descripción**: lo que muestra la imagen, en una o dos frases. No se muestra en el texto. Los libros electrónicos, los archivos de Word y Markdown la conservan con la imagen (como su «texto alternativo»), y los lectores de pantalla se la leen a quienes no pueden ver la imagen.

La imagen con su pie se mueve como una sola pieza; escribe el pie debajo de la imagen como cualquier otro texto (tiene el estilo **Pie de imagen**).

### Buscar y reemplazar

- **Ctrl+F** (**Edición > Buscar > Buscar**) abre la barra de búsqueda sobre el texto. Muestra cuántas coincidencias hay y las resalta; **F3** y **Shift+F3** van a la coincidencia siguiente y a la anterior. **Coincidir mayúsculas y minúsculas** y **Palabras completas** acotan la búsqueda.
- **Ctrl+H** añade la fila de reemplazo: **Reemplazar** cambia la coincidencia seleccionada y pasa a la siguiente, **Reemplazar todo** cambia todas las coincidencias del texto que ves (un solo paso para **Deshacer**). El texto nuevo adopta el formato del texto al que sustituye, y los comentarios siguen unidos.
- **Ctrl+E** (**Usar la selección para buscar**) pone el texto seleccionado en la barra de búsqueda.
- En el texto combinado, buscar y reemplazar actúan sobre todos los documentos mostrados.
- Mientras el teclado está en un **documento de apoyo**, Ctrl+F busca en el documento de apoyo.
- **Ctrl+Shift+F** busca en todo el proyecto, **Edición > Buscar > Reemplazar en el proyecto** reemplaza en todo el proyecto y **Edición > Buscar > Buscar por formato** busca resaltados, comentarios, colores y estilos (todo en la sección 9).
- En el corcho y en el esquema, Ctrl+F abre en cambio la **barra de filtro** (sección 5).

### Ortografía

Las palabras mal escritas llevan un subrayado ondulado rojo; haz clic derecho en cualquier parte de la palabra subrayada para ver sugerencias o añadirla a tu diccionario (también en palabras con apóstrofo, como «Kayel's» o «D'Nersun»: el menú siempre ofrece la palabra entera que está subrayada). En una palabra mal escrita el menú es corto, de modo que se abre justo en el ratón: las sugerencias, **Añadir «...» al diccionario**, **Ignorar «...»** y **Más comandos** con todo lo que el menú contiene en otros casos (Cortar, Copiar, Pegar, Estilo, comentarios, Apariencia...). El propio nombre del programa y sus términos ({{APP}}, esquema, subdocumento, documento de apoyo) siempre cuentan como correctos. Elige el idioma en **Archivo > Opciones > Correcciones** (página Ortografía); los diccionarios que instalaste como complementos (uno por idioma del programa, cada uno con su licencia) aparecen en esa lista; allí también eliges el subrayado (ondas, puntos o una línea recta), si se comprueban las palabras con números («Route66») y **Ver la lista personal de palabras...** muestra las palabras que añadiste y quita las que añadiste por error. Los diccionarios proceden de tu sistema y de complementos. En Ubuntu y Kubuntu está instalado el inglés; para otros idiomas, instala el paquete *hunspell* correspondiente en Discover (por ejemplo «hunspell-de-de» para alemán).

### Inserciones especiales

El menú **Insertar** ofrece también la fecha y la hora actuales, caracteres especiales (raya, semirraya, puntos suspensivos, espacio de no separación, signo de sección, viñeta), **numeraciones automáticas** y **marcadores de posición**, que Compilar sustituye (sección 13).

### Correcciones mientras escribes

**Archivo > Opciones > Correcciones** establece qué se corrige mientras escribes. Una palabra se corrige cuando la terminas, es decir, cuando escribes después de ella un espacio, un Return o un signo de puntuación; **Ctrl+Z** justo después revierte solo la corrección.

- **Corregir las mayúsculas de las frases**: la primera palabra de una frase recibe mayúscula (no después de abreviaturas como «Dr.» o «e.g.»). **Poner «i» en mayúscula**: «i» e «i'm» pasan a «I» e «I'm». Ambas opciones están desactivadas al principio.
- **Comillas inteligentes** en el estilo que elijas (“inglés”, „alemán“, »alemán«, «francés», ”sueco”); un apóstrofo dentro de una palabra es siempre ’. Donde la comilla simple de cierre no es el apóstrofo (alemán ‚…‘, francés ‹…›), un ' escrito justo después de una palabra cierra una comilla simple que está abierta en el párrafo; si luego sigues escribiendo letras («geht's»), vuelve a ser el apóstrofo. Dos guiones se convierten en una raya (—); tres puntos pueden convertirse en un solo carácter de puntos suspensivos (desactivado al principio). En el modo guion se pueden desactivar.
- **Comillas que ya hay en el proyecto** (desde 0.61.1): en **Comillas inteligentes**, **Convertir en comillas tipográficas…** convierte las " y ' rectas del proyecto en las marcas del estilo elegido junto a **Usar comillas inteligentes**, y **Convertir en comillas rectas…** devuelve a " y ' todas las comillas tipográficas y los apóstrofos. Se abre una ventana como la de Reemplazar en el proyecto (sección 9): la dirección y el estilo una vez más, dónde convertir (títulos, texto, notas, sinopsis, metadatos personalizados, comentarios y notas al pie), el ámbito (todo el proyecto o los documentos seleccionados en el archivador, la Papelera solo cuando la marcas) y una instantánea de cada texto antes de que cambie (marcada al principio). **Contar** muestra cuántas marcas cambiarían; **Convertir** pregunta una vez más y lo hace. El formato se conserva (una comilla en negrita sigue en negrita). Una marca abre después de nada, de un espacio, de un paréntesis, de una raya o de una marca de apertura, y cierra en los demás casos; un apóstrofo dentro de una palabra o antes de un número ('90s) pasa a ser ’. Una marca recta con un espacio a ambos lados se queda como está, y un apóstrofo al comienzo de una palabra ('tis) pasa a ser una marca de apertura: revísalos después. Buscar y reemplazar no puede hacer esto, porque una marca tiene que convertirse en una de apertura o de cierre según sus vecinas.
- **Detección de datos**: una dirección web que escribes se convierte en un enlace; **[[Título]]** se convierte en un enlace al documento con ese título y, si no hay ninguno, se crea junto al texto un documento vacío nuevo con ese nombre (los «enlaces de documento» de Scrivener).
- **Sustituciones adicionales**: una lista de lo que escribes y en qué se convierte, por ejemplo «(c)» y ©, «->» y →. **Editar sustituciones...** cambia la lista; **Sustituciones estándar** recupera la lista con la que viene el programa.
- **Autocompletado**: en el modo guion, la lista de nombres de personajes y encabezados de escena que se ofrece mientras escribes.

### Zoom, desplazamiento tipo máquina de escribir y ancho fijo

- **Ctrl+rueda del ratón** hace zoom en el área que está bajo el ratón, dondequiera que esté: los editores, el archivador, el inspector, el esquema, el corcho (las fichas y su texto juntos), un mapa mental, el modo de composición y las ventanas propias (Referencia rápida, las herramientas de escritura); el teclado puede estar en otro lugar. Cada área conserva su zoom hasta que lo vuelves a cambiar.
- **Ctrl++ / Ctrl+-** o el cuadro de zoom del pie amplían el texto en pantalla sin cambiar el tamaño de la fuente. **Ver > Restablecer zoom** (**Ctrl+0**) vuelve al zoom de texto predeterminado.
- **Archivo > Opciones > Edición** establece el zoom con el que empieza cada área («Zoom de texto predeterminado», para cada área de la ventana y cada parte con zoom propio: la sinopsis, las notas, las listas de marcadores, el texto de marcador, la lista y el texto de instantáneas, la lista y la ficha de objetos en el inspector y en la ventana Objetos, los mapas mentales nuevos) y si los editores se vuelven a abrir con el zoom que tuvieron la última vez («Recargar el último zoom seleccionado de los editores principales»).
- **Dónde estabas.** {{APP}} recuerda, con el proyecto, la posición del cursor y la parte del texto en pantalla de cada documento: al mostrarse de nuevo (desde el archivador, un enlace, Atrás y Adelante, en el documento de apoyo, un panel de Referencia rápida o tras reabrir el proyecto), un documento se abre donde lo dejaste. Una carpeta en el texto combinado recuerda cuál de sus documentos tenía el cursor. Un resultado de búsqueda o un enlace que apunta a un lugar concreto va allí en su lugar. Las imágenes conservan su zoom y los archivos PDF su zoom y su página.
- **El desplazamiento tipo máquina de escribir** (**Ver > Edición de texto**) mantiene la línea en la que escribes en el centro de la pantalla.
- **El editor de ancho fijo** muestra el texto como una columna centrada de ancho cómodo (500 px al 100 % salvo que lo cambies), por ancha que sea la ventana. Desactívalo, o cambia el ancho, en **Archivo > Opciones > Apariencia > Editor principal** («Usar editor de ancho fijo», «Ancho de la columna de texto»), o con un clic derecho en el papel (**Apariencia > Ancho de la columna de texto**). El papel de la columna es #d1d495 y el área junto a ella #999999 de forma predeterminada («Papel» y «Junto a la columna de texto» entre los colores del Editor principal); permanecen igual cuando eliges otro tema, hasta que los cambias o los restableces.
- **Borde junto al texto.** A izquierda y derecha del texto, el papel tiene un borde de su propio color, para que el texto no toque el borde del papel: 10 px al 100 % en los proyectos nuevos, y crece con el zoom. Está fuera de la columna de texto, de modo que las líneas se cortan donde se cortaban. Cámbialo en **Archivo > Opciones > Apariencia > Editor principal** («Borde junto al texto», de 0 a 100 px) o con un clic derecho en el papel (**Apariencia > Borde junto al texto**). Los proyectos creados antes de 0.40.2 conservan el aspecto que tenían, sin borde, hasta que les des uno. Un clic en el borde pone el cursor al principio o al final de la línea contigua.
- **Hacer clic y clic derecho** nunca mueven la vista: el cursor va donde haces clic y el texto se queda donde está en la pantalla (antes de 0.40.2, un clic o clic derecho cerca del borde superior o inferior, o un clic derecho junto al texto, podía hacer que el texto saltara).

### Vista de página

**Ver > Edición de texto > Vista de página** (también en el menú del botón **Ver** de la barra de herramientas) muestra tu texto en hojas de papel en lugar de una columna interminable. El pie muestra entonces la página del cursor y el número de páginas, por ejemplo «Página 3 de 12». Elige el comando otra vez para volver a la columna. El color del escritorio alrededor de las páginas está en **Archivo > Opciones > Apariencia > Editor principal** (o clic derecho > **Apariencia** mientras se muestran las páginas).

Las páginas son una manera de mirar tu texto, nada más. No muestran el aspecto que tendrá un manuscrito compilado o impreso: escribes en piezas, y los títulos, las fuentes y el tamaño de página se deciden al compilar (sección 13). Nada de las páginas se almacena en tus documentos.

- **Ver > Edición de texto > Tamaño y márgenes de página…** establece el papel (A4, A5, US Letter y otros, o un tamaño propio), vertical u horizontal, y los cuatro márgenes. El ajuste pertenece al proyecto; marca **Usar esta página también para los proyectos nuevos** para convertirlo en tu valor habitual.
- El zoom amplía las hojas, los márgenes y las letras a la vez. Una hoja más ancha que el editor se puede desplazar hacia los lados. Como en la vista de columna, las sangrías y los tabuladores conservan su longitud real, de modo que los saltos de línea pueden diferir un poco entre niveles de zoom.
- En el texto combinado, cada documento empieza en una hoja propia, y su última hoja termina justo debajo del texto, de modo que los documentos cortos no llenan la pantalla de papel vacío.
- La vista de página se aplica a todo lo que muestra tus textos en la ventana principal y en las ventanas de referencia rápida: ambos editores y los documentos de apoyo. Las notas conservan su aspecto sencillo, y el modo de composición tiene su propio papel.
- El color alrededor de las hojas es **Alrededor de las páginas (vista de página)** en **Archivo > Opciones > Apariencia > Editor principal**.

### Escritura de guiones

Los guiones de cine, las obras de teatro y los guiones de cómic se escriben en **modo guion**. Un guion se compone de *elementos*: encabezado de escena, acción, personaje, acotación, diálogo, transición, etcétera. Cada elemento tiene su lugar en la página, y en el modo guion las teclas **Tab** y **Enter** te llevan de un elemento al siguiente, de modo que nunca estableces una sangría a mano.

**Activarlo.** Haz clic en un documento y elige **Formato > Escritura de guiones > Modo guion** (**Ctrl+8**). La entrada del menú nombra el formato de guion del proyecto, por ejemplo *Modo guion - Guion de cine*. El modo guion pertenece al documento: los guiones y los textos corrientes conviven en un proyecto, y los guiones tienen un **icono amarillo** en el archivador. Elige el comando otra vez para desactivarlo; el texto conserva su aspecto. Un documento nuevo creado entre guiones (en una carpeta cuyos textos son todos guiones) también es un guion.

**El pie.** En un guion, el pie del editor muestra, en lugar del recuento de palabras, lo que harán **Tab** y **Enter** en el cursor, y a la derecha un menú desplegable con el elemento del párrafo actual. Elige otro elemento en ese menú y el párrafo se reformatea. El texto que no es un elemento del guion se muestra como **Texto general**.

**Tab y Enter.**

- **Enter** al final de un párrafo inicia el elemento que suele seguir: acción tras un encabezado de escena, diálogo tras el nombre de un personaje, etcétera.
- **Enter** en una línea vacía abre la lista de elementos en el cursor; pulsa la letra subrayada de la lista (E para encabezado de escena, P para personaje...). **Formato > Escritura de guiones > Mostrar la lista de elementos** (**Ctrl+\\**) abre la misma lista en cualquier momento.
- **Tab** en una línea vacía cambia la línea a otro elemento; púlsalo otra vez para el siguiente, **Shift+Tab** para volver atrás.
- **Tab** al final de un párrafo hace lo que conviene al elemento: tras el nombre de un personaje o una línea de diálogo inicia una acotación, y en un encabezado de escena escribe « - », listo para la hora del día.
- **Enter** en medio de un párrafo lo divide; ambas mitades conservan el elemento. **Shift+Enter** es un salto de línea dentro del párrafo.
- Los elementos escritos en mayúsculas (encabezados de escena, nombres de personajes, transiciones) convierten en mayúsculas lo que escribes. Los paréntesis y prefijos como «SFX: » se escriben por ti.

**Autocompletado.** Mientras escribes, una pequeña lista ofrece lo que encaja con el elemento: INT. y EXT. al principio de un encabezado de escena, las horas del día después de su « - », los lugares y nombres de personajes ya usados en el documento, y transiciones habituales como CUT TO:. **Enter** o **Tab** toma la entrada marcada, las teclas de flecha eligen otra, **Esc** cierra la lista.

**Cambiar elementos.** **Formato > Escritura de guiones > Cambiar elemento a** cambia el párrafo con el cursor, o todos los párrafos seleccionados. Elegir allí un elemento en un documento corriente convierte el documento en un guion. Cada elemento es un comando, así que puedes dar a tus favoritos un atajo en **Archivo > Opciones > Teclado**.

**Formatos de guion.** Un proyecto usa un formato de guion para todos sus guiones. **Formato > Escritura de guiones > Formato de guion** enumera *Guion de cine*, *Obra de teatro* y *Guion de cómic*, los formatos de los complementos y los tuyos. Cuando eliges otro formato, los guiones del proyecto reciben la nueva disposición: cada elemento se convierte en el elemento del mismo tipo en el nuevo formato (la acción pasa a ser una acotación escénica, por ejemplo). Un elemento sin equivalente se convierte en texto general que conserva su aspecto; la pregunta previa al cambio nombra esos elementos.

**Ajustes del guion.** **Formato > Escritura de guiones > Ajustes del guion…** cambia el formato del proyecto o construye uno nuevo:

- el nombre del formato, su fuente (de forma predeterminada la fuente de máquina de escribir de tu sistema: Courier Prime, Courier New u otra similar) y tamaño, y si las longitudes están en pulgadas o centímetros;
- la lista de elementos: añádelos, quítalos y reordénalos (el orden es el de la lista de elementos y el de **Tab** en una línea vacía);
- para cada elemento: nombre, clase (lo que es para otros programas: encabezado de escena, acción, personaje...) y su letra en la lista; negrita, cursiva, subrayado, mayúsculas y color; alineación, sangrías desde ambos márgenes, líneas vacías antes y interlineado; lo que hacen **Enter** y **Tab**; un texto que se escribe antes y después del párrafo; y las listas para el autocompletado;
- una vista previa de todos los elementos.

Un elemento al que se cambia el nombre conserva sus párrafos en todos los guiones del proyecto. El botón **Gestionar** parte de otro formato, carga o guarda un archivo de formato (*.qbscript*) y guarda el formato para usarlo en otros proyectos.

**Conviene saber.**

- Las teclas y las listas funcionan igual en el texto combinado, en el segundo editor, en los documentos de apoyo, en los paneles de Referencia rápida y en el modo de composición. Solo los editores de la ventana principal tienen el pie con las indicaciones; en otros sitios, **Ctrl+\\** muestra la lista de elementos.
- La vista de página (arriba) muestra un guion en hojas; con la fuente de máquina de escribir habitual a 12 puntos, una página de un guion de cine equivale aproximadamente a un minuto de película, pero las páginas en pantalla no son las páginas del archivo compilado.
- La fuente y las sangrías de los elementos pertenecen al formato de guion, no al documento. Los archivos de tus guiones contienen solo los nombres de los elementos (sección 18).
- Importar y exportar guiones, en los formatos de otros programas de guion: sección 14. Compilar un guion entero: sección 13.
- Los guiones requieren la versión 0.10 o posterior. Un proyecto que contiene guiones lo indica en su archivo de proyecto, y desde esta versión {{APP}} pregunta antes de abrir un proyecto que necesita una versión más reciente que la que se está ejecutando. Las versiones 0.9 y anteriores no preguntan: muestran los guiones como texto corriente y pierden los elementos de cada guion que miras allí.

---

## 5. Tres maneras de ver tu proyecto

Elige la vista con los tres botones de la barra de herramientas o con **Ctrl+1**, **Ctrl+2** y **Ctrl+3**.

### Texto combinado (Ctrl+1): varios documentos como un solo texto

Selecciona varios documentos en el archivador (Ctrl+clic o Shift+clic) o una carpeta y pulsa **Ctrl+1**. Todos los textos aparecen uno tras otro, separados por una fina línea discontinua, y puedes escribir en todos ellos como si fueran un solo documento. Los cambios van automáticamente a los documentos correctos. Muestra los títulos de los documentos entre ellos con **Ver > Edición de texto > Mostrar títulos en el texto combinado**.

### Corcho (Ctrl+2): fichas

Cada documento es una ficha que muestra su título y su sinopsis; un documento sin sinopsis muestra en su lugar las primeras líneas de su texto (atenuadas, en cursiva). La ficha de una imagen muestra la imagen, y también la de un documento cuya sinopsis es una imagen (sección 7). Arrastra las fichas para reordenar los documentos. Haz doble clic en una ficha para editar su título o su sinopsis. Pulsa **Enter** para abrir las fichas seleccionadas como texto.

En el pie del corcho:

- **Cuadrícula** o **libre**: en el modo libre puedes colocar las fichas en cualquier sitio, como en un corcho de verdad. **Confirmar el orden** reordena entonces los documentos del archivador para que coincidan con las fichas de izquierda a derecha y de arriba abajo. La disposición que eliges para una carpeta se recuerda con esa carpeta (desde 0.90.0): la próxima vez que la muestres, su corcho tendrá la misma disposición. Una carpeta para la que nunca elegiste una disposición conserva la que muestra el editor.
- **Hilos de etiqueta**: dispone las fichas en filas por etiqueta.
- **Tamaño de ficha** de las fichas, con el deslizador, las teclas de flecha o la rueda del ratón sobre él; se conserva de inmediato (**Ctrl+rueda del ratón** sobre el corcho amplía las fichas y su texto juntos, como las demás áreas; consulta la sección 4).

Un doble clic en un lugar vacío del corcho crea una ficha nueva (**Archivo > Opciones > Comportamientos > Doble clic**). Cuando editas una sinopsis en una ficha, **Return** termina la edición y **Shift+Return** crea una línea nueva (**Archivo > Opciones > Comportamientos > Tecla Return**, donde Return también se puede configurar para crear una ficha nueva o una fila nueva del esquema después de la seleccionada).

Los sellos de estado, el modo libre y la disposición por etiqueta también están en **Ver > Opciones del corcho**; los tintes de etiqueta para las fichas y el archivador, en **Ver > Usar el color de etiqueta en**. Las columnas del esquema también se enumeran en **Ver > Opciones del esquema**.

### Esquema (Ctrl+3): una tabla

Muestra los documentos como una tabla con título, sinopsis, etiqueta, estado, recuentos de palabras, objetivos, fechas y tus metadatos personalizados. Haz clic en el botón de columnas del pie para elegir columnas. Haz clic en el encabezado de una columna para ordenar (solo para ver; el orden del archivador no cambia). Arrastra las filas para reordenar los documentos.

### Filtrar el corcho y el esquema

Mientras el corcho o el esquema tiene el teclado, **Ctrl+F** (**Edición > Buscar > Buscar**) abre una **barra de filtro** en la parte superior del editor. Mientras escribes, solo permanecen las fichas o filas que coinciden; la barra indica cuántas («2 de 9»). En el esquema, los grupos de los documentos que coinciden permanecen para que veas a dónde pertenecen.

- Haz clic en el **triángulo** a la izquierda del campo para ver las opciones: dónde buscar (**Títulos**, **Sinopsis**, **Palabras clave**, **Texto**), una **etiqueta**, un **estado**, una **palabra clave**, si el documento está **incluido en la compilación** y **Mostrar lo que no coincide**. Varias opciones juntas acotan más la lista.
- El filtro permanece mientras abres otras carpetas, de modo que puedes recorrer el proyecto mirando solo, por ejemplo, las escenas de un personaje.
- **Hecho** quita el filtro y vuelve a mostrar todo.

Arrastrar fichas sigue reordenando los documentos: una ficha soltada entre dos fichas que ves queda junto a ellas en el archivador. **Confirmar el orden** del corcho libre está desactivado mientras un filtro oculta fichas.

---

## 6. Dos editores, marcadores y referencia rápida

### Editor dividido

Haz clic en el botón de dividir a la derecha del encabezado del editor para mostrar un segundo editor. Alt+clic divide en la otra dirección. **Ver > Editor dividido** ofrece las mismas opciones. **F6** salta entre los editores. Un clic en el archivador muestra el documento en el editor que usaste por última vez; **Navegar > La selección del archivador afecta a** lo cambia al editor izquierdo o superior, al editor derecho o inferior o al otro editor, el que sea en el que estés escribiendo.

En un corcho o un esquema, el botón del pie **La selección afecta al otro editor** (también en **Ver > Opciones del corcho** y **Opciones del esquema**) hace que el otro editor muestre la ficha en la que haces clic. Es una manera rápida de hojear escenas manteniendo la vista general del capítulo.

**Navegar > Editor > Fijar en su lugar** mantiene un editor en su documento, aunque hagas clic en otro sitio del archivador.

### Documentos de apoyo

Cada editor tiene un **documento de apoyo**: una segunda vista sencilla dentro del editor que contiene otro documento como referencia, como unas notas o una ficha de personaje. Con dos editores y sus documentos de apoyo puedes ver cuatro documentos a la vez.

- **Para abrirlo**, mantén pulsada **Alt** y arrastra un documento desde el archivador, un corcho o un esquema a la barra de encabezado del editor. Sin Alt, al soltar se carga el documento en el propio editor. **Navegar > Abrir > en el documento de apoyo** y el menú de clic derecho de un documento hacen lo mismo con el documento seleccionado.
- El documento de apoyo tiene una barra de encabezado estrecha con el título y no tiene pie. Muestra el texto de un documento (o una imagen o un PDF); no tiene corcho, esquema ni texto combinado. Puedes escribir y dar formato en él como en cualquier editor.
- **Para moverlo**, haz clic derecho en su barra de encabezado y elige izquierda, derecha, arriba o abajo. La primera vez se abre junto a un editor ancho y debajo de uno estrecho; después, donde lo pusiste por última vez. Arrastra la línea entre el editor y el documento de apoyo para cambiar sus tamaños.
- **Para mostrar otro documento**, suéltalo en la barra de encabezado del documento de apoyo, o arrástralo con Alt otra vez a la barra de encabezado del editor.
- **Para cerrarlo**, haz clic en la **X** a la derecha de su barra de encabezado, o elige **Navegar > Editor > Cerrar documento de apoyo**. El botón a la izquierda de la X lo cierra y abre en cambio el documento como ventana de referencia rápida.

**Navegar con un documento de apoyo.** Mientras un editor muestra un corcho o un esquema y tiene abierto su documento de apoyo, el botón **La selección afecta a** del pie tiene tres posiciones. El primer clic hace que tu selección se abra en el otro editor, como de costumbre. El segundo clic gira la flecha para que apunte al interior del recuadro: ahora la ficha o fila en la que haces clic se abre en el documento de apoyo. Un tercer clic lo desactiva. Una disposición útil: el manuscrito en el editor izquierdo, un esquema de tu investigación en el editor derecho con su documento de apoyo debajo, y **Navegar > La selección del archivador afecta a > Editor izquierdo o superior**. El archivador gobierna entonces el manuscrito, y el esquema gobierna el documento de apoyo.

El documento de apoyo, su posición y su documento se recuerdan con el proyecto.

### El encabezado del editor

El encabezado sobre cada editor muestra el título del documento y contiene los botones **atrás** y **adelante** (**Ctrl+[** y **Ctrl+]**), un menú de las secciones del documento y botones para el documento anterior y el siguiente (**Alt+Shift+Arriba / Abajo**).

- **Historial**: cada editor recuerda los documentos que ha mostrado. Mantén pulsado el botón de atrás o adelante (o haz clic derecho en él) para ver una lista y elige un documento para ir directamente a él; **Navegar > Atrás a** y **Adelante a** muestran las mismas listas. El historial se conserva con el proyecto.
- **El icono** junto al título representa el documento. Haz clic en él para ver un menú (Fijar en su lugar, Mostrar en el archivador...). Arrástralo como un documento del archivador: a la barra de encabezado del otro editor, al archivador, a la pestaña de una colección, al botón Referencia rápida o Marcadores de la barra de herramientas, o al texto para crear un enlace.
- **Documentos > Agrupar** (**Ctrl+Shift+J**, también en el menú contextual del archivador) mueve los elementos seleccionados a una carpeta nueva en el lugar del primero; su título queda listo para escribir. Con el teclado en el archivador, **Edición > Deshacer** revierte el paso entero.
- **Documentos > Convertir > Ficha a objeto** (también en el menú contextual del archivador) convierte cada documento de ficha seleccionado en un Objeto (sección sobre los Objetos), por ejemplo una ficha de personaje o de lugar: el título es el nombre, el texto va a **Otro**, una imagen en el texto pasa a ser la imagen. El nombre de la carpeta (Personajes, Lugares) elige la clase; para otra carpeta se te pregunta. El elemento del archivador se queda: al seleccionarlo se muestra el Objeto en el inspector, y Return lo abre en la ventana Objetos. El documento conserva su texto (un proyecto de Scrivener ve un documento normal); el vínculo se conserva en los datos de Objetos propios del proyecto. **Edición > Deshacer** con el teclado en el archivador lo revierte.
- **Navegar > Ir a** enumera todo el archivador como un menú y carga el documento elegido en el editor activo. **Navegar > Grupo contenedor** muestra la carpeta o el grupo al que pertenece el documento, con el documento seleccionado allí.
- Un **doble clic en el divisor** entre dos editores los deja del mismo tamaño.

### Arrastrar y soltar de un vistazo

| Arrastra... | sobre... | y... |
|---|---|---|
| un documento (archivador, colección, corcho, esquema, icono del encabezado, marcador) | la barra de encabezado de un editor | se muestra en ese editor; con **Alt**: en su documento de apoyo |
| | texto en un editor | un enlace al documento; en un texto seleccionado, ese texto se convierte en el enlace |
| | texto, con **Alt** pulsada | se inserta el texto del documento |
| una imagen del archivador | texto en un editor | se inserta la imagen |
| un documento | el botón **Referencia rápida** de la barra de herramientas | cada documento se abre en un panel propio |
| | el botón **Marcadores** de la barra de herramientas | se añade a los marcadores del proyecto |
| | la pestaña de una colección, una lista de marcadores | se añade |
| | la barra de encabezado de un panel de Referencia rápida | se muestra en ese panel |
| | el gestor de archivos o el escritorio | se guarda allí como archivo (consulta «Exportar documentos») |
| texto seleccionado | el archivador | un documento nuevo con ese texto; el texto se queda donde estaba |
| archivos del gestor de archivos | el archivador o un corcho | se importan |
| un enlace web de un navegador | una carpeta del archivador, el corcho, el esquema | se importa la página (complemento Importación de páginas web, sección 14) |
| un archivo de imagen, o una imagen del archivador | la ficha del inspector (modo imagen) | pasa a ser la imagen de la ficha |
| palabras clave de **Proyecto > Palabras clave del proyecto** | un documento del archivador, una colección, el corcho, el esquema; la lista de palabras clave del inspector | se asignan; con **Alt**: junto con el grupo al que pertenecen |

Algunos escritorios usan Alt+arrastrar para mover ventanas. Si Alt+arrastrar no llega al programa, cambia la tecla modificadora para mover ventanas en los ajustes de tu escritorio (KDE: Preferencias del sistema, comportamiento de las ventanas; el lugar exacto difiere entre versiones), o usa en su lugar los comandos del menú (**Navegar > Abrir > en el documento de apoyo**, **Proyecto > Palabras clave del proyecto > Asignar a los documentos seleccionados**).

### Paneles de Referencia rápida

Un panel de Referencia rápida es un documento en una ventana propia, por ejemplo para tener a la vista una ficha de personaje mientras escribes. El texto de un panel se puede editar y formatear como en el editor (los atajos de los menús Formato e Insertar funcionan allí).

**Para abrir paneles:**

- **Navegar > Abrir > como referencia rápida**, o el botón Referencia rápida de la barra de herramientas, para los documentos seleccionados.
- Pulsa **Espacio** en el corcho o en el esquema, o en el archivador y en una lista de colección o de resultados de búsqueda cuando **Ver > Teclas de adición rápida en el archivador** está activado (consulta la sección 2, las teclas del archivador). Sin esa opción, Espacio en el archivador sirve para escribir un nombre y saltar a él.
- Arrastra documentos al **botón Referencia rápida** de la barra de herramientas: cada uno se abre en un panel propio, y el editor conserva su documento.
- **Navegar > Abrir referencia rápida** muestra todo el archivador como un menú, de modo que puedes abrir un documento sin seleccionarlo antes.

**En un panel:**

- El **título** de la barra de encabezado se puede editar (cambia el nombre del documento). El **icono** junto a él abre un menú (Mostrar en el archivador, Abrir en el editor, añadir a los marcadores del proyecto o quitar de ellos) y se puede arrastrar como el propio documento. Suelta un documento en la barra de encabezado para mostrarlo en este panel.
- El botón de la **parte superior derecha** muestra la **sinopsis**, las **notas**, las **palabras clave** o los **metadatos** del documento debajo de su texto. Lo que cambias allí cambia el documento.
- La **barra de pie** muestra el recuento de palabras y permite establecer la **etiqueta** y el **estado**.
- El **botón de marcadores** a la izquierda de la barra de pie abre los **marcadores del proyecto** junto al documento. Un clic en un documento marcado lo muestra en el panel, de modo que un solo panel basta para moverte entre tus notas. Arrastra los marcadores a otro orden, suelta documentos y quita un marcador con el botón ➖ (el documento permanece en el archivador). Los archivos y enlaces web se abren con un doble clic.
- El botón ➕ debajo de los marcadores crea un **documento nuevo** y lo marca. La primera vez se te pide la carpeta (por ejemplo una carpeta «Notas» en Investigación); marca **No volver a preguntar** para usar esa carpeta a partir de entonces. Cámbiala en **Proyecto > Ajustes del proyecto > Carpetas especiales**.
- El botón **Flotante** mantiene el panel por encima de la ventana principal; **Ventana > Hacer flotantes los paneles de referencia rápida** lo hace con todos.

El menú Ventana enumera los paneles abiertos y los cierra todos a la vez. Los paneles que están abiertos cuando cierras un proyecto vuelven cuando lo abres de nuevo, con su lugar, tamaño, panel inferior y barra lateral (desactívalo en **Archivo > Opciones > Comportamientos**).

### Audio y vídeo

Los archivos de audio y vídeo del archivador (por ejemplo entrevistas en la carpeta Investigación) se reproducen dentro de {{APP}} cuando está instalado el complemento **Reproducción multimedia** (sección 17; viene con la actualización 0.12). Sin él, tal archivo muestra **Abrir con la aplicación predeterminada**, como antes.

**Reproducir.** Haz clic en el archivo del archivador. Un vídeo muestra su imagen; un archivo de audio muestra su título, artista y duración. La **barra de pie** del editor tiene los controles de reproducción:

- el botón de **pausa y rebobinado** a la izquierda (véase más abajo),
- **ir al inicio**, **retroceder un poco**, **reproducir / pausa**, **avanzar un poco**,
- la **posición**: arrastra el deslizador, o haz clic junto a su control,
- el tiempo, la **velocidad** (de 0,5× a 2×; el habla más lenta es más fácil de transcribir; la voz se vuelve más grave o más aguda con la velocidad, como una cinta) y el **volumen** (clic: silenciar; la flecha: un deslizador).

Un clic en la imagen de un vídeo lo reproduce o lo pausa; un clic derecho ofrece también **Abrir con la aplicación predeterminada**. Con el teclado en el reproductor: **Espacio** reproduce o pausa, las **teclas de flecha izquierda** y **derecha** saltan, **Inicio** va al comienzo.

**Transcribir.** Divide el editor, muestra el archivo de audio en un editor y tu transcripción en el otro, y sigue escribiendo: los comandos de **Navegar > Multimedia** funcionan dondequiera que esté el teclado, también en los paneles de Referencia rápida y en el modo de composición.

| Comando | Atajo |
|---------|-------|
| Reproducir / Pausa | Ctrl+Return |
| Retroceder un poco / Avanzar un poco (5 segundos) | Ctrl+Alt+, / Ctrl+Alt+. |
| Insertar marca de tiempo del medio (también **Insertar > Insertar marca de tiempo del medio**: el mismo comando, mostrado en ambos menús) | Ctrl+Shift+Return |
| Detener, Ir al inicio, Pausar y rebobinar, Más rápido, Más lento, Velocidad normal, Subir el volumen, Bajar el volumen, Silenciar, Ir a la marca de tiempo en el cursor | sin atajo (asígnales uno en **Archivo > Opciones > Teclado**) |

Los comandos actúan sobre el archivo que abriste o usaste por última vez (en cualquiera de los dos editores, un documento de apoyo o un panel de Referencia rápida).

**Pausar y rebobinar.** Activado (el botón se ve pulsado), cada pausa retrocede un poco, 2 segundos salvo que establezcas otro valor en **Archivo > Opciones > Comportamientos > Reproducción**. Cuando pulsas Ctrl+Return para escribir lo que oíste, oyes de nuevo las últimas palabras al continuar.

**Marcas de tiempo.** **Insertar marca de tiempo del medio** escribe en el cursor el tiempo del reproductor, como `[03:07]` (con horas a partir de una hora: `[1:02:03]`). **Ir a la marca de tiempo en el cursor** hace lo contrario: toma la marca de tiempo de la línea del cursor (la más cercana anterior o alrededor) y mueve el reproductor allí. Las marcas de tiempo son texto corriente; también se pueden escribir a mano (`3:07`, `[03:07]`, `1:02:03.5`).

**En un documento de apoyo o en un panel de Referencia rápida**, el reproductor tiene sus controles de reproducción en la parte inferior. Un archivo de audio en el documento de apoyo del editor de la transcripción es una buena disposición para un solo editor.

**Dónde se quedó.** Cuando vuelves a abrir un archivo de audio o vídeo, continúa donde lo pausaste (desactívalo en **Archivo > Opciones > Comportamientos > Reproducción**). Mostrar otro documento en el editor detiene el archivo.

**Qué se reproduce.** El complemento *Reproducción multimedia* (2.0) reproduce formatos de uso libre: audio MP3, Ogg Vorbis, Opus, FLAC, ALAC, WAV, AIFF y WavPack; vídeo WebM, MKV y Ogg con imágenes VP8, VP9 o Theora (también MJPEG y otros formatos libres). El complemento trae todo lo que necesita.

**Más formatos.** H.264, H.265 (HEVC), AAC (M4A), AC-3, E-AC-3, DTS, vídeo MPEG-1/2/4, VC-1 y WMV, WMA y AMR están sujetos a licencias de patentes en algunos países; vienen con el complemento aparte **Formatos adicionales de vídeo y audio**. La forma más sencilla de obtener ambos es el paquete único **media-all-formats.qbpkg** de la página Releases: instalado como cualquier complemento (sección 17), instala *Reproducción multimedia* y *Formatos adicionales de vídeo y audio* juntos. Ambos aparecen entonces en **Ayuda > Complementos y actualizaciones > Gestor de complementos**, donde los formatos adicionales se pueden desactivar o quitar por separado. Que haga falta una licencia de patente para usar estos formatos depende del país. Mientras está instalado y activado, *Reproducción multimedia* lo usa, y también se reproducen los archivos MP4/M4V/MOV, AVI, MPEG, WMV, FLV y 3GP (no WMA Pro ni WMV 9). Sin él, tal archivo muestra bajo el reproductor un mensaje que nombra sus formatos y este complemento; todo lo demás funciona como antes. El vídeo AV1 no se reproduce con la decodificación incorporada.

Desde 0.80.0 el reproductor siempre decodifica con su propio FFmpeg: el Qt que {{APP}} usa desde entonces (la compilación propia de Qt 6.12 de The Qt Company) no tiene forma de reproducir a través del **GStreamer** del sistema, por lo que **Archivo > Opciones > Comportamientos > Reproducción > Decodificación** muestra esa opción como no disponible.

### Marcadores

Los **marcadores de documento** (inspector, pestaña de marcadores) enlazan un documento con otros documentos, archivos de tu disco o páginas web. Los **marcadores del proyecto** están disponibles desde todos los documentos, desde el botón de marcadores de la barra de herramientas y desde la barra lateral de los paneles de Referencia rápida. La pestaña de marcadores del inspector muestra ambas listas, una sobre otra: **Marcadores del documento** y **Marcadores del proyecto**, cada una con sus propios botones ➕ y ➖. Un clic en un marcador muestra el documento debajo de las listas; un documento que el editor ya muestra aparece allí como copia de solo lectura (escribe en el editor), de modo que el editor conserva su ancho de línea. Un doble clic abre el marcador. Arrastra documentos, archivos o enlaces web a la lista de marcadores, o usa el botón ➕; un enlace arrastrado desde un navegador web conserva el título de la página. Arrastra un marcador hacia arriba o hacia abajo para cambiar el orden. Los divisores entre las dos listas y el texto de debajo se pueden mover; los tamaños que les des se guardan con el proyecto y vuelven tras un reinicio o una actualización del programa, también cuando en ese momento no hay ningún marcador elegido. Los documentos soltados en el **botón Marcadores** de la barra de herramientas pasan a ser marcadores del proyecto; también los documentos seleccionados con **Documentos > Añadir a los marcadores del proyecto**; el inspector muestra entonces su pestaña de marcadores con la nueva entrada seleccionada.

---

## 7. El inspector

Los botones de la parte superior del inspector cambian entre:

- **Sinopsis y notas**: la sinopsis es un breve resumen que aparece en la ficha. Puedes usar una imagen en lugar de texto: haz clic en el botón de imagen de la ficha y suelta un archivo de imagen o una imagen del archivador en el área gris (o haz doble clic en el área para elegir un archivo; clic derecho para quitar la imagen). Debajo están las **notas del documento** (o, cambiando con el pequeño menú, las **notas del proyecto**, compartidas por todo el proyecto). **Navegar > Inspeccionar > Ir a la sinopsis** (**Ctrl+Alt+Y**) e **Ir a las notas del documento** (**Ctrl+Alt+N**) llevan el teclado allí desde el texto; **Alt+2** vuelve al editor. Desde 0.93.0 las notas (las del documento y las del proyecto) empiezan como texto sin formato: sin fuente, tamaño, sangría de primera línea, espaciado de párrafo ni estilo del formato de texto del proyecto, en la fuente de la interfaz del programa con el zoom de las notas. El formato aparece solo cuando lo aplicas tú mismo (la barra de formato, los menús, los atajos) y entonces se conserva. Las notas que ya tienen texto conservan el aspecto que tenían. El texto pegado en las notas es texto sin formato; **Edición > Pegar con formato** (**Ctrl+Alt+Shift+V**) conserva el formato que trae.
- **Marcadores**: los marcadores del documento y los del proyecto. Un clic en un marcador de un documento del proyecto muestra su texto debajo de las listas. Puedes escribir y dar formato allí como en un editor (la barra de formato, el menú Formato y sus atajos actúan sobre él mientras el cursor está en él): todo va de inmediato al documento y a su archivo, y un editor que muestra el mismo documento muestra el cambio al instante. El texto de allí tiene un zoom propio.
- **Mapa mental**: un tablero de notas para el documento o la carpeta (consulta «Mapas mentales» más abajo).
- **Objetos**: los personajes, ubicaciones, entornos, puntos de interés, clima y otras cosas de tu historia, cada uno con una ficha propia y vínculos entre ellos (consulta «Objetos» más abajo).
- **Metadatos**: etiqueta, estado, tipo de sección, incluir en la compilación, fechas de creación y modificación, **metadatos personalizados** y **palabras clave**.
- **Instantáneas** (sección 11).
- **Comentarios y notas al pie**.

Las herramientas de escritura de los complementos pueden añadir pestañas propias; sus botones van primero en la fila, por ejemplo **Transcripciones** de Transcripción de voz (sección 10).

**El zoom propio del inspector.** **Ctrl+rueda del ratón** sobre una parte del inspector hace esa parte más grande o más pequeña, sin cambiar los editores. Las pestañas con dos partes tienen un zoom para cada una: **Sinopsis y notas** (la ficha, las notas), **Marcadores** (la lista de los marcadores del documento, la lista de los marcadores del proyecto, el texto marcado debajo de ellas: tres zooms), **Instantáneas** (la lista, el texto de la instantánea), **Objetos** (los botones de categoría con la lista, la ficha). La lista **Zoom** de la parte inferior muestra y establece el zoom de la parte que tiene el teclado, o de la parte en la que hiciste zoom por última vez; su título nombra la parte («Zoom: Notas»), y en la pestaña Mapa mental es el zoom del tablero. Las demás pestañas (Metadatos, Comentarios) y las filas de botones de la parte superior e inferior comparten un zoom; con **Ver > Zoom por separado en las pestañas del inspector** (también en el menú de clic derecho de la lista Zoom), Metadatos y Comentarios tienen uno cada una.

**El inspector recuerda** con el proyecto qué pestaña estaba abierta, dónde pusiste los divisores dentro de las pestañas, el marcador mostrado y el objeto mostrado en la pestaña Objetos.

**Comentarios de una carpeta.** Con una carpeta en el editor (como corcho o esquema), la pestaña de comentarios y notas al pie enumera los comentarios y las notas al pie de cada documento dentro de la carpeta y sus subcarpetas, agrupados por documento. Un clic en el nombre de un documento lo abre; un clic en **Comentario ▸** (o **Nota al pie ▸**) abre el documento y muestra ese lugar; el texto de un comentario o nota al pie se puede cambiar allí mismo. En el texto combinado (varios documentos como un solo texto), la pestaña enumera las notas de todos los documentos mostrados.

### Mapas mentales

Cada documento y carpeta puede tener un **mapa mental**: un tablero en el que colocas notas en cualquier sitio, las conectas con líneas y flechas, las coloreas y las agrupas sobre formas de fondo. Los tableros se pueden guardar como archivos de Scapple, y los archivos de Scapple se pueden leer (consulta *Archivos y Scapple* más abajo). Haz clic en el botón **Mapa mental** del inspector (entre Marcadores y Metadatos). El botón aparece subrayado cuando el documento tiene un mapa mental.

**Un mapa mental fijado (0.92.0).** Normalmente la pestaña muestra el mapa mental del documento o carpeta que tienes seleccionado. En su lugar puedes fijarlo en un elemento: la pestaña sigue entonces mostrando el mapa mental de ese elemento, sea lo que sea lo que selecciones en el archivador o abras en el editor. Es para un tablero que quieres tener a tu lado mientras escribes, como un mapa de la trama guardado en una carpeta. Tres lugares hacen lo mismo:

- **En el archivador**, pasa el ratón sobre una fila: aparece un pequeño alfiler cerca de su extremo derecho. Haz clic en él y ese elemento queda fijado; su alfiler permanece y aparece relleno. Un clic en el alfiler relleno lo suelta. Hacer clic en un alfiler no selecciona nada.
- **En la pestaña Mapa mental**, el botón de alfiler a la derecha de la pequeña barra fija el tablero que muestra la pestaña y lo vuelve a soltar. Mientras un tablero está fijado, la barra nombra su elemento (*Fijado: Trama*, justo después del botón de vista general; un título largo se acorta, y el título entero se muestra al dejar el ratón sobre él), de modo que queda claro que el tablero no pertenece al documento seleccionado.
- **En los menús**, **Navegar > Inspeccionar > Fijar el mapa mental de este elemento** y **Navegar > Inspeccionar > Soltar el mapa mental fijado** (al principio no tienen atajos; dales alguno en las opciones si quieres).

Mientras un elemento está fijado, una barra delgada en la parte inferior del archivador dice *Mapa mental fijado:* con su título y tiene el botón **Soltar**. Como máximo un elemento de un proyecto está fijado; fijar otro traslada la fijación. También se puede fijar un elemento sin mapa mental: la pestaña muestra un tablero vacío para empezar. Solo la pestaña Mapa mental queda fijada; las demás pestañas del inspector siguen al documento seleccionado como siempre, y un tablero abierto en una ventana grande sigue siendo lo que es. La fijación se conserva con el proyecto en este ordenador cuando lo cierras, y se suelta cuando el elemento va a la Papelera.

**En el inspector** el tablero llena la pestaña. La pequeña barra sobre él tiene **Nueva nota**, **Nueva forma de fondo**, **Ajustar el zoom**, **Menú** (todos los comandos: Archivo, Edición, Ver, Notas, Formato), el botón de **vista general** (consulta *Mirar alrededor*) y **Abrir en una ventana**. La barra de debajo cuenta las notas y tiene el deslizador de zoom. **Abrir en una ventana** muestra el mismo tablero en una ventana grande con los mismos menús en su barra de menús y un inspector a la derecha (**Formato de nota**, **Estilos de nota**, **Ajustes del documento**); los cambios en un lugar se ven de inmediato en el otro. Un clic derecho en el tablero abre los comandos de lo que hay bajo el ratón. Una nota nueva (botón, menú o doble clic) toma el teclado de inmediato, también cuando antes estaba en el archivador: lo que escribes sustituye su texto seleccionado (0.94.0).

Los comandos de un mapa mental están en estos menús propios, no en los menús de la ventana principal. Más abajo, **Menú > Notas > Conectar** significa: el botón **Menú** del tablero del inspector, luego **Notas**, luego **Conectar** (en la ventana grande: **Notas > Conectar** de su barra de menús). Mientras el tablero tiene el teclado, sus teclas van primero: **Ctrl+Z** deshace un cambio del tablero, **Ctrl+F** busca en sus notas, **Ctrl+P** lo imprime.

**Ayuda del mapa mental.** **Ayuda > Ayuda del mapa mental** (también el botón **?** en el extremo derecho de la barra del tablero, y **Menú > Ayuda** del tablero) muestra una guía breve con imágenes: cómo crear notas, cómo trazar una línea (arrastra una nota sobre otra), cómo trazar una flecha (mantén pulsada **Alt** al soltar; **Ctrl+Alt** para una flecha hacia atrás, **Ctrl+Alt+Shift** para ambos extremos), palabras sobre una línea y formas de fondo. Su botón **Abrir tablero de ejemplo** (también **Menú > Ayuda > Abrir tablero de ejemplo**) abre un tablero de práctica en una ventana propia, con notas para arrastrar unas sobre otras y ejemplos terminados de líneas y flechas. Nada del tablero de ejemplo pertenece a tu proyecto, y no se guarda.

**Notas.**

- **Doble clic** en un lugar vacío: una nota nueva, lista para escribir. **Ctrl+Enter** crea una nota nueva debajo de la seleccionada (apilada). Con **Ctrl** en el doble clic, la nota nueva se conecta a las notas seleccionadas; con **Alt**, mediante una flecha hacia ella; con **Ctrl+Alt**, mediante una flecha hacia atrás.
- **Doble clic** en una nota para cambiar su texto; **Esc** o un clic fuera termina la edición. Una nota corta se hace tan estrecha como su texto; una larga se ajusta al ancho de las notas nuevas (ambos en **Menú > Archivo > Opciones del mapa mental…**).
- **Arrastra** las notas para moverlas; con **Ctrl** al soltar se copian. Las **teclas de flecha** mueven las notas seleccionadas cuando **Menú > Edición > Las teclas de flecha mueven las notas** (**M**) está activado (con **Shift**, 10 px); si no, seleccionan la nota siguiente en esa dirección. **Enter** o **F2** edita la nota seleccionada, **Delete** quita las notas seleccionadas.
- **Arrastra una nota sobre otra** para conectar las dos con una línea; otra vez para quitar la línea. Con **Alt** la línea recibe una flecha hacia la nota sobre la que se soltó; con **Ctrl+Alt** una flecha hacia atrás; con **Ctrl+Alt+Shift** flechas en ambos extremos.
- **Doble clic en una línea** para poner una nota nueva en ella, o en la etiqueta de una línea para cambiarla (**Menú > Notas > Etiqueta de la conexión…**).
- **Menú > Notas > Conectar** (**Ctrl+.**), **Conectar con flecha** (**Ctrl+Alt+.**), **Quitar flechas** (**Ctrl+,**) y **Desconectar** actúan sobre las notas seleccionadas: la primera que seleccionaste es el centro.
- **Menú > Notas > Nueva nota conectada** crea una nota a la izquierda, a la derecha, encima o debajo de la seleccionada, conectada a ella (con o sin flecha).
- **Menú > Notas > Dividir** (**Ctrl+Shift+K**) divide una nota en el cursor; **Unir** une las notas seleccionadas.
- **Menú > Notas > Apilar** (**Ctrl+'**), **Alinear**, **Distribuir**, **Igualar ancho / alto**, **Ajuste automático**, **Traer al frente**, **Enviar al fondo**.
- **Menú > Edición > Añadir enlace…** (**Ctrl+K**) enlaza texto de una nota con una página web o un archivo; los documentos arrastrados desde el archivador al tablero pasan a ser notas con enlaces a ellos. Un clic en un enlace lo abre: un documento se abre como lo hacen los enlaces a documentos en el texto (**Archivo > Opciones > Comportamientos**).
- **Menú > Notas > Añadir las notas al archivador** convierte las notas seleccionadas en documentos dentro del documento o carpeta al que pertenece el tablero, en el orden de lectura del tablero.

Las **formas de fondo** (**Menú > Notas > Nueva forma de fondo**, **Ctrl+Shift+G**) son áreas de color detrás de las notas. Creada con notas seleccionadas, la forma las rodea. Una forma **magnética** (**Menú > Notas > Magnética**, **Ctrl+Shift+M**) arrastra consigo las notas que hay sobre ella cuando se mueve. Una forma nueva empieza en gris medio (#808080) y no magnética; **Archivo > Opciones > Extras > Mapa mental** (y **Menú > Archivo > Opciones del mapa mental…**) establece el relleno, el borde y **Magnética** con los que empiezan las formas nuevas. (Antes de 0.93.0 una forma nueva era gris claro y magnética; un color que habías elegido allí se conserva, y un **Magnética** almacenado se desactivó una vez.)

**El menú de clic derecho de una nota** (o de una forma de fondo) cambia las notas seleccionadas: **Color de relleno**, **Color del texto** y **Color del borde** ofrecen los colores de las etiquetas de tu proyecto, los seis colores de la barra de formato (rojo, azul, amarillo, verde, blanco, negro) y **Otro...** para cualquier color, la misma elección en todas partes de {{APP}}; **Forma** es **Rectángulo**, **Rectángulo redondeado**, **Nube** o **Dentado** (una nota sin borde recibe uno fino, para que se vea la forma); **Borde** establece el grosor; **Iconos** pone hasta ocho iconos en una nota, uno en cada esquina y uno en el centro de cada lado, elegidos entre los iconos del archivador (**Documentos > Cambiar icono**), tus propios iconos incluidos. Haz clic derecho cerca del lugar de un icono para quitar solo ese; **Quitar todos los iconos** los borra todos.

**El menú de clic derecho de una línea** pertenece solo a esa línea, de modo que las demás líneas de las mismas notas no se modifican: sus flechas (**Sin flechas**, una flecha hacia cualquiera de las dos notas o **Flechas en ambos extremos**), **Color de la línea**, **Icono en el centro** (un icono dibujado en el centro de la línea), la etiqueta, **Insertar una nota en la línea** y **Desconectar**. Cada cambio se puede deshacer por separado.

Los iconos, los colores de línea y los iconos de línea son aportaciones propias de {{APP}}; se conservan en el archivo del tablero en elementos que Scapple no lee (un tablero exportado como archivo de Scapple tiene las notas, las líneas y las flechas sin ellos). **Archivo > Opciones > Extras > Mapa mental** establece el tamaño de los iconos y la forma de las notas nuevas.

**Aspecto.** **Menú > Formato** tiene negrita, cursiva, subrayado, tachado, más grande y más pequeño (**Ctrl+Shift+>** y **Ctrl+Shift+<**), colores de texto, de relleno y de borde, grosor del borde (ninguno, de 1 a 5 px) y estilo (redondeado, cuadrado, dentado, nube), sombra y **Atenuar** (**Ctrl+Shift+F**, la nota se muestra pálida). Los **Estilos de nota** guardan un aspecto bajo un nombre; **Menú > Formato > Estilo de nota** aplica uno (**Ctrl+Alt+1** a **9**, **Ctrl+Alt+0** para Predeterminado), crea uno a partir de la nota seleccionada o copia y pega el aspecto de una nota (**Alt+Shift+C** / **Alt+Shift+V**). Los **Ajustes del documento** del inspector de la ventana establecen el fondo, el color del texto, la fuente y la dirección de escritura del tablero (y **Establecer como valores predeterminados** para los tableros nuevos).

**Fondo.** Un clic derecho en un lugar vacío del tablero (en el inspector o en la ventana), **Menú > Formato > Color de fondo** o el botón de fondo de los **Ajustes del documento** ofrece los colores de las etiquetas de tu proyecto, los seis colores de la barra de formato, el valor predeterminado (**Predeterminado para temas claros** o **Predeterminado para temas oscuros** en los menús, **Usar el valor predeterminado** en los Ajustes del documento) y **Otro...**. Un tablero tiene un fondo propio para los temas claros y otro para los temas oscuros: el color que eliges se conserva para la clase de tema en uso, y la otra clase conserva el suyo. Un tablero sin color propio muestra el valor predeterminado de **Archivo > Opciones > Extras > Mapa mental** («Fondo de los tableros», uno para los temas claros y otro para los oscuros) y lo sigue cuando lo cambias. **Establecer como valores predeterminados** en los Ajustes del documento convierte el fondo del tablero en el valor predeterminado de la clase de tema en uso. Un tablero conserva su color de fondo también cuando aún no hay nada en él.

**Zoom y lugar.** Cada tablero recuerda, con el proyecto, su zoom y la parte que se ve en pantalla, uno para el inspector y otro para la ventana grande. Un tablero en el que nunca has hecho zoom se abre con el zoom predeterminado de los mapas mentales (**Archivo > Opciones > Edición**, «Zoom de texto predeterminado»).

**Seleccionar.** Clic, **Shift+clic** o **Ctrl+clic** para añadir, arrastra un marco alrededor de las notas, **Ctrl+A**. **Menú > Edición > Seleccionar** tiene las notas apiladas debajo, las notas sobre las formas seleccionadas (**Alt+A**), las notas conectadas (**Ctrl+Shift+A**) y los grupos conectados enteros; debajo están **Invertir la selección** (**Alt+Shift+I**) y **Deseleccionar** (**Ctrl+D**).

**Mirar alrededor.** Haz zoom con **Ctrl+rueda del ratón**, **Ctrl++** y **Ctrl+-**, **Tamaño real** (**Ctrl+0**), **Ajustar el zoom** (**Ctrl+9**), **Zoom a la selección** (**Ctrl+Shift+8**) y **Zoom aleatorio** (**Ctrl+Shift+R**). Mantén pulsada **Espacio** y arrastra para mover el tablero; mantén pulsada **Z** para ver un momento todo el tablero. **Menú > Edición > Buscar** (**Ctrl+F**) busca y reemplaza texto en las notas. **Menú > Ver > Guías de página** muestra dónde terminan las páginas al imprimir.

**La vista general (0.93.0).** Un pequeño mapa en la esquina superior izquierda del tablero muestra todo el tablero en pequeño, y un marco para la parte que ves ahora; el marco se mueve y cambia de tamaño mientras te desplazas y haces zoom. Las notas, las formas de fondo y las líneas se dibujan con sus colores; el texto de una nota solo se muestra donde todavía se puede leer a ese tamaño; si no, la nota es su marco de color. Haz clic en la vista general para llevar la vista a ese punto; arrastra el marco para mover la vista con él. El zoom se mantiene, y un tablero bloqueado (**Impedir la edición**) también se puede recorrer. La rueda del ratón sobre la vista general funciona como sobre el tablero (**Ctrl+rueda** hace zoom). Arrastra el agarre de su esquina inferior derecha para cambiar su ancho (empieza en una quinta parte del ancho de la vista, como mínimo 80 px, y nunca es más ancha que la mitad de la vista); la altura sigue la forma de lo que muestra, nunca más de la mitad de la vista, y el ancho se reduce con ella cuando la altura la superaría. La vista general pertenece solo a la vista: no está en las exportaciones, las impresiones ni la imagen del tablero. Al principio es **automática**: se muestra cuando una parte del tablero queda fuera de la vista, y se oculta cuando todo cabe (con un pequeño margen, para que no parpadee en el límite). El botón de **vista general** en la barra del tablero del inspector (justo después de **Menú**) alterna entre **automática** y **siempre visible** (el botón aparece pulsado); el mismo interruptor es la entrada **Vista general siempre visible** del menú Ver del tablero (en el inspector: el botón **Menú** y luego **Ver**; en la ventana grande: su barra de menús). La elección y los tamaños se conservan en este ordenador, en los ajustes del programa (una elección para todos los tableros; el tamaño de la vista general del inspector y el de la ventana grande, por separado); nada va al proyecto.

**Texto, imágenes y archivos.** Pega o suelta texto para crear una nota (un texto con líneas vacías puede convertirse en varias notas), suelta una imagen para una nota de imagen y suelta un archivo de texto para crear notas con sus párrafos.

**Archivos y Scapple.** **Menú > Archivo > Exportar** escribe el tablero como PDF (**Ctrl+Shift+E**), imagen PNG, texto sin formato, lista de texto, RTF, esquema OPML, sus imágenes o un archivo de Scapple (*.scap*); **Menú > Archivo > Importar mapa mental o archivo de Scapple…** lee un archivo *.scap* en el tablero. **Imprimir** y **Vista previa de impresión** lo imprimen. **Menú > Archivo > Impedir la edición** bloquea el tablero.

**Opciones del mapa mental** (**Menú > Archivo > Opciones del mapa mental…**, y lo mismo en **Archivo > Opciones > Extras > Mapa mental** de la ventana principal): si **Enter** termina la edición de una nota (entonces **Shift+Enter** crea una línea nueva), si **Enter** en una nota seleccionada crea una nueva, si las notas nuevas adoptan el aspecto de la nota seleccionada, líneas discontinuas sin flechas, notas estrechas para textos cortos, lo pálidas que son las notas atenuadas, y el tamaño, el ancho, el color y el borde de las notas y formas nuevas.

**Dónde se guarda el tablero.** Un tablero se guarda solo, como un archivo con la estructura de los archivos de Scapple (*content/<ID>/mindmap.scap* en la carpeta del proyecto; *Files/Data/<UUID>/quillbinder-mindmap.scap* en un proyecto de Scrivener, junto a los archivos propios del documento). Un documento recibe el archivo solo cuando se pone algo en su tablero. Deshacer (**Ctrl+Z**) funciona para cada tablero por separado.

{{APP}} lee los archivos de Scapple tal como los escribe Scapple 1.x, hasta donde se conoce su estructura; no fue posible probarlos con el propio Scapple (consulta las notas de la versión 0.17.0 en el archivo de estado).

### Objetos: personajes, ubicaciones y más

Los **Objetos** son las personas, los lugares y las cosas de tu historia, cada uno con una ficha de campos: un personaje con raza, estatura, color de ojos, rasgos e historia de fondo; un lugar con su clase, su entorno y su clima; y así sucesivamente. Pertenecen a todo el proyecto, no a un documento, y se guardan aparte del archivador. Los vínculos los conectan, de modo que puedes recorrer un trasfondo con clics: la ficha del personaje, la aldea donde vive, el paisaje que la rodea, su clima.

Haz clic en el botón **Objetos** del inspector (a la derecha del botón **Mapa mental**), o elige **Navegar > Inspeccionar > Objetos del proyecto: personajes, ubicaciones, entornos, ...**. **Ventana > Objetos…** (o el botón de ventana de la barra de la pestaña) muestra los mismos objetos en una ventana grande propia, con la lista a la izquierda y la ficha a la derecha. Los cambios en un lugar se ven de inmediato en el otro.

**Categorías.** Los botones de colores de la parte superior son las categorías: **Personajes**, **Ubicaciones**, **Entornos**, **Puntos de interés** y **Clima** para empezar. Haz clic en una para ver sus objetos. Puedes cambiarles el nombre, darles otros colores, cambiar su orden, quitarlas y añadir categorías propias (facciones, hechizos, barcos...): consulta *Plantillas y colores* más abajo.

**Crear objetos.** **+** en la barra de la pestaña crea un objeto nuevo en la categoría elegida (en la carpeta del objeto seleccionado); escribe su nombre en la parte superior de la ficha. El botón con la carpeta crea una **carpeta** para ordenar, por ejemplo «Aldeas» dentro de Ubicaciones; las carpetas pueden contener carpetas. Arrastra objetos y carpetas en la lista para moverlos. **F2** o un clic derecho cambia el nombre, y el menú de clic derecho también tiene **Duplicar** y **Eliminar…** (también en el menú **≡** de la barra). El campo **Filtro** muestra solo los objetos cuyo nombre o texto contiene lo que escribes.

**La ficha.** Debajo de la franja de color con la categoría y el nombre están los campos de la plantilla de la categoría. Todo lo que escribes se guarda solo, como tus documentos. Las clases de campos son:

- **Texto corto** (una línea) y **Fecha** (de la forma que quieras, como «primavera de 1203» o «el tercer invierno»).
- **Número** con una unidad (cm, años, °C...).
- **Texto largo**, que puede contener enlaces (véase más abajo) y palabras en **negrita**, en *cursiva*, subrayadas, tachadas, coloreadas y resaltadas: con el cursor en el campo, la barra de formato, el menú Formato y sus atajos (**Ctrl+B**, **Ctrl+I**, **Ctrl+U**; también en la ventana Objetos) actúan sobre él. Las versiones 0.20 y 0.21 de {{APP}} también los muestran.
- **Elección**: una lista para elegir; también puedes escribir otra cosa.
- **Sí / no**.
- **Imagen**: haz clic en el área (o suelta una imagen en ella) para elegir una; se copia en el proyecto y se reduce si es mayor de 1600 píxeles. El clic derecho la quita.
- **Vínculo a objetos**: los objetos vinculados se muestran como botones de colores. **+** añade uno: escribe para buscar y luego elige un objeto, o elige **Nuevo en Ubicaciones: «Ashvale»** para crear un objeto nuevo con ese nombre y vincularlo de inmediato. Un clic en un objeto vinculado muestra su ficha; un clic derecho quita el vínculo.
- **Vínculo a documentos**: documentos del archivador que pertenecen al objeto, como «Aparece en» de un personaje o «Escenas ambientadas aquí» de un lugar. Un clic abre el documento en el editor.

**Botón de color.** El botón **Color** de la franja da a un objeto un color propio (o le devuelve el color de la categoría), por ejemplo para marcar a los personajes principales.

**Vínculos dentro de textos largos.** En un campo de texto largo, selecciona una palabra y elige **Enlazar con un objeto…** (o **Enlazar con un documento…**) en su menú de clic derecho; sin selección, el nombre del objeto se inserta como enlace. Un clic en el enlace lo sigue; **Quitar enlace** en el menú de clic derecho lo quita de nuevo. En el texto de un documento, **Insertar > Enlazar con un objeto…** pone en el texto un enlace a un objeto; un clic en él muestra el objeto en el inspector. Cuando el manuscrito se compila, estos enlaces se convierten en palabras corrientes.

**Ir adelante y atrás.** **‹** y **›** en la barra de la pestaña van atrás y adelante por los objetos que has mirado, como un navegador web.

**Vinculado desde.** En la parte inferior de cada ficha, **Vinculado desde** enumera los objetos que enlazan con este (en un campo de vínculo o en un texto), de modo que la aldea muestra quién vive en ella. Un clic va allí.

**En este documento.** Cuando el documento del editor está vinculado a objetos (mediante un campo de vínculo a documentos, por ejemplo «Aparece en»), la pestaña muestra **En este documento:** con esos objetos, y el botón **Objetos** aparece subrayado como otras pestañas con contenido.

**Eliminar.** Eliminar un objeto pregunta primero. Los vínculos a él se quitan de los demás objetos (en los textos la palabra vinculada permanece, como texto sin formato). Un vínculo a él en un documento dice que el objeto ya no existe cuando haces clic. Eliminar una carpeta conserva sus objetos; suben un nivel.

**Plantillas y colores.** **Proyecto > Plantillas de objetos y colores…** (también en el menú **≡** de la pestaña y en el menú de clic derecho de un botón de categoría) edita las categorías y sus campos:

- A la izquierda, las categorías: **Añadir**, **Quitar** y las flechas para el orden. Quitar una categoría que tiene objetos pregunta si también deben eliminarse; **No** no cambia nada.
- A la derecha, el nombre y el **Color** de la categoría, y sus campos: **Añadir campo**, **Quitar**, **Subir**, **Bajar**. Para cada campo, su nombre y su clase; un número puede tener una unidad, una elección su lista de opciones (una por línea) y un campo de vínculo las categorías con las que puede vincular (ninguna marcada: todas).
- **Usar para los proyectos nuevos** conserva estas categorías y plantillas para los proyectos que crees a partir de ahora. **Plantillas estándar** restablece una categoría a la plantilla con la que venía, o añade las categorías estándar que quitaste.
- Quitar un campo de una plantilla oculta lo que los objetos tienen en él, pero no lo elimina de sus archivos. Para un campo estándar, **Plantillas estándar > Restablecer esta categoría al estándar** lo recupera con su contenido; un campo propio no vuelve, así que cambia el nombre de un campo en lugar de quitarlo si es posible que quieras recuperar su contenido.

**Dónde se guardan los objetos.** En la carpeta del proyecto, en una carpeta *objects*: un pequeño archivo por objeto (*objects/characters/<ID>.json* y así sucesivamente), las plantillas, los colores y las carpetas en *objects/schema.json*, y las imágenes en *objects/images*. El archivo del proyecto en sí no se modifica, de modo que las versiones anteriores de {{APP}} abren el proyecto como antes y dejan los objetos en paz. Un proyecto en formato Scrivener conserva sus objetos en su copia de trabajo (sección 14); Scrivener no los conoce. Las copias de seguridad los incluyen.

---

## 8. Organizar tu proyecto

### Palabras clave

Las palabras clave son etiquetas, por ejemplo los personajes o los lugares de una escena. Añádelas en el inspector o gestiónalas todas con **Proyecto > Palabras clave del proyecto** (**Ctrl+Alt+K**), donde puedes darles colores, asignarlas a los documentos seleccionados y encontrar todos los documentos con una palabra clave. También puedes **arrastrar palabras clave** desde esa ventana a documentos del archivador, una colección, el corcho o el esquema (si el documento es uno de varios seleccionados, todos reciben las palabras clave), o a la lista de palabras clave del inspector o de un panel de Referencia rápida. Mantén pulsada **Alt** al soltar para asignar también el grupo de una palabra clave. Dentro de la ventana, arrastra una palabra clave sobre otra para ponerla en ese grupo. Los colores de las palabras clave aparecen en las fichas.

### Metadatos personalizados

En **Proyecto > Ajustes del proyecto > Metadatos personalizados** puedes añadir tus propios campos: texto, casilla, lista (un conjunto fijo de opciones) o fecha, por ejemplo *Punto de vista*, *Ubicación* o *Día de la historia*. Aparecen en el inspector y como columnas del esquema.

### Guardar los ajustes del proyecto en un archivo

**Proyecto > Guardar los ajustes del proyecto en un archivo…** escribe los ajustes del proyecto abierto en un archivo propio (*.qbsettings*): etiquetas, estados, tipos de sección, metadatos personalizados, palabras clave, el formato predeterminado, los estilos, el formato de guion, los objetivos, los ajustes de compilación, la vista de página y el tamaño de página, los ajustes de impresión, el aspecto (tema, colores, fuentes, ancho de columna, borde), las opciones propias del proyecto (Archivo > Opciones con «Este proyecto») y las plantillas y colores de la hoja de Objetos. El texto de tus documentos no forma parte de él. Las etiquetas, los tipos de sección y las carpetas que nombran los ajustes de compilación se guardan por sus nombres, de modo que el archivo se puede leer en otro proyecto.

**Proyecto > Cargar los ajustes del proyecto desde un archivo…** lee un archivo así en el proyecto abierto. Cuando los ajustes del proyecto son los mismos que los del archivo, lo dice y no cambia nada. Cuando difieren, una ventana enumera cada sección con lo que difiere (por ejemplo «solo en el archivo: Idea; solo en el proyecto: Revisar») y te permite elegir para cada sección:

- **Tomar del archivo**: los ajustes del archivo para esta sección.
- **Conservar los del proyecto**: la sección se queda como está.
- **Predeterminado de Quillbinder**: lo que recibe un proyecto nuevo (los ajustes para proyectos nuevos de **Archivo > Opciones**, las etiquetas y estados estándar...).

**Tomar todo del archivo** y **Conservar todo lo del proyecto** establecen cada sección a la vez. Elijas lo que elijas, no se pierde nada que usen tus documentos: una etiqueta, un estado, un tipo de sección, una palabra clave, un campo de metadatos personalizados, una categoría de objetos o un estilo que los documentos todavía usan permanece en el proyecto aunque el archivo no lo tenga, y el mensaje final nombra lo que se conservó. Cuando cambian el formato o un estilo, el texto de todos los documentos lo sigue, como cuando lo cambias tú mismo. Antes de cargar, {{APP}} conserva los ajustes tal como estaban en un archivo en la carpeta *settings* del proyecto («settings-before-loading-*fecha*.qbsettings»); carga ese archivo para volver atrás.

### Tipos de sección

Los tipos de sección le dicen a Compilar qué es cada documento: *Parte*, *Capítulo*, *Escena*, *Sección*, etcétera. Establécelos por documento en el inspector, o deja que {{APP}} decida por la posición (**Ajustes del proyecto > Tipos de sección**, «Tipos predeterminados según la estructura»), por ejemplo: las carpetas del primer nivel son capítulos, los documentos de texto dentro de ellas son escenas.

### Plantillas de documento

Coloca documentos de plantilla (por ejemplo una ficha de personaje) en una carpeta y luego elige esa carpeta en **Ajustes del proyecto > Carpetas especiales**. Las plantillas aparecen en **Proyecto > Nuevo a partir de plantilla**. En **Documentos > Plantilla predeterminada para subdocumentos** puedes hacer que una carpeta use una plantilla para cada documento nuevo que crees dentro de ella.

### Plantillas de proyecto

**Archivo > Guardar como plantilla…** guarda el proyecto actual (estructura, ajustes y cualquier documento de ejemplo) como plantilla para proyectos nuevos.

---

## 9. Buscar

### Búsqueda rápida

Escribe unas letras en el campo de la barra de herramientas. Una lista muestra los documentos cuyo **título**, **sinopsis** o **texto** las contiene, hasta diez de cada clase bajo su propio encabezado. Elige uno para abrirlo: los lugares del texto que contienen lo que escribiste quedan **marcados**, y el primero se selecciona y se desplaza a la vista. Las marcas desaparecen cuando eliges otro documento en el archivador. Es el camino más rápido a un documento cuyo nombre conoces.

### Búsqueda en el proyecto

**Ctrl+Shift+F**, o la lupa de la barra de herramientas, muestra un campo de búsqueda sobre el archivador. Escribe, y el archivador se sustituye por la lista de resultados de color púrpura.

- Haz clic en un resultado para abrirlo. **Las coincidencias se resaltan en el texto**, la primera se selecciona y se desplaza a la vista, y **F3** / **Shift+F3** saltan de una a otra. **Ctrl+F** abre la barra de búsqueda ya rellenada con tu término de búsqueda.
- La **X** en el encabezado de los resultados vuelve al archivador; el editor conserva el documento que muestra. Haz clic de nuevo en la lupa de la barra de herramientas para ocultar el campo de búsqueda.
- Con las colecciones mostradas, la pestaña **Resultados de la búsqueda** vuelve a ejecutar tu última búsqueda. La última búsqueda se conserva con el proyecto.

La lupa a la izquierda del campo de búsqueda abre las opciones:

| Grupo | Entradas |
|-------|----------|
| **Buscar en** | **Todo**, o uno de: Título, Texto, Sinopsis, Notas, Palabras clave, Etiqueta, Estado, Metadatos personalizados. Un clic elige ese único lugar. Mantén pulsada **Alt** mientras haces clic (o mientras abres el menú) para elegir varios lugares. |
| **Operador** | **Todas las palabras** (cada palabra debe aparecer, en cualquier orden), **Cualquier palabra**, **Frase exacta**, **Expresión regular** |
| **Opciones** | Palabras completas, Distinguir mayúsculas y minúsculas, Invertir los resultados (los documentos que *no* coinciden), Buscar solo en el Borrador, Buscar solo en la selección del archivador (los documentos seleccionados en el archivador y todo lo que contienen), Buscar en los documentos «incluidos» / Buscar en los documentos «excluidos» (según **Incluir en la compilación**), Incluir la Papelera |

Ejemplo: para ver todo lo que todavía tiene el estado «Pendiente», busca `Pendiente` y elige **Estado** en Buscar en.

**Guardar la búsqueda como colección…** al final del menú conserva la búsqueda, con sus opciones, como una colección que se ejecuta de nuevo cada vez que la abres.

### Reemplazar en el proyecto

**Edición > Buscar > Reemplazar en el proyecto** reemplaza un texto en todo el proyecto a la vez.

1. Escribe el texto que se va a **Reemplazar** y el texto que irá en su lugar (**Por**). Deja «Por» vacío para quitar el texto.
2. Elige **Coincidir mayúsculas y minúsculas**, **Solo palabras completas** o **Expresión regular** (entonces «Por» puede usar `\1` a `\9` para los grupos de la expresión).
3. Marca dónde reemplazar: **Títulos**, **Texto**, **Notas**, **Sinopsis**, **Metadatos personalizados** (campos de texto), **Comentarios y notas al pie** (los textos que se muestran en el inspector).
4. Elige **Todo el proyecto** o **Solo los documentos seleccionados** (los documentos seleccionados en el archivador, con lo que contienen). La Papelera se deja en paz a menos que la marques.
5. **Contar** te dice cuántas apariciones cambiarían y dónde. **Reemplazar todo** pregunta una vez más y luego lo hace.

Reemplazar en el proyecto no se puede deshacer en conjunto. Deja marcada la opción **Tomar una instantánea de cada texto antes de que cambie**: cada texto modificado tiene entonces una instantánea llamada «Antes de reemplazar …» a la que volver (sección 11). Los títulos y las sinopsis no tienen instantáneas; para un cambio grande, haz antes una copia de seguridad con **Archivo > Copia de seguridad > Hacer copia de seguridad ahora**. Un texto abierto en un editor también se puede revertir allí con **Edición > Deshacer**.

El formato se conserva: el texto nuevo adopta el formato del texto al que sustituye. Un texto que no está abierto en un editor se cambia en su archivo, de modo que **Edición > Deshacer** no revierte ese cambio (la instantánea tomada antes sí); lo mismo vale para Convertir comillas, las fichas de comentarios y Comparar y transferir.

### Buscar por formato

**Edición > Buscar > Buscar por formato** (**Ctrl+F3**) abre una pequeña ventana que permanece abierta mientras trabajas. Elige qué buscar:

| Buscar | Encuentra | Opción adicional |
|--------|-----------|------------------|
| Texto resaltado | texto marcado con el resaltador | limitar a un color |
| Comentarios y notas al pie | texto con un comentario o una nota al pie adjuntos (inspector) | limitar a un color de comentario |
| Anotaciones en línea, Notas al pie en línea | notas escritas en el texto | color de anotación |
| Color de revisión | texto escrito en el modo de revisión | qué revisión |
| Texto en color | texto cuyo color estableciste a mano | limitar a un color |
| Enlaces | enlaces web y enlaces a documentos | |
| Formato de carácter | texto en negrita, cursiva, subrayado o tachado (establecido a mano, no por un estilo) | cuál de ellos |
| Estilo | texto con un estilo de párrafo o de carácter | qué estilo |
| Imágenes | imágenes en el texto | |

**Que contenga el texto** acota la búsqueda a los lugares que contienen una palabra (para los comentarios y las notas al pie, también cuenta el texto propio de la nota). **Siguiente** y **Anterior** seleccionan el lugar siguiente. Con **Buscar en: Todos los documentos**, la búsqueda continúa en los documentos siguientes del archivador y los abre; con **Solo el editor actual** se queda donde estás. **Ctrl+G** y **Ctrl+Shift+G** repiten la búsqueda sin la ventana. **Cerrar** (o **Esc**) cierra la ventana.

---

## 10. Objetivos, estadísticas y herramientas de escritura

- **Proyecto > Objetivos del proyecto** (**Ctrl+Shift+T**): fija un objetivo de palabras o de caracteres para el Borrador y para cada sesión de escritura, y, si quieres, una fecha límite y tus días de escritura. Así, {{APP}} puede calcular cuánto necesitas escribir en cada sesión. El pie de página muestra tu progreso junto al recuento de palabras (desde la versión 0.94.0): una barra pequeña y, por ejemplo, «83 / 1000 hoy» para la sesión y «12 400 / 80 000 borrador» para el Borrador, cada una solo mientras su objetivo está fijado; al hacer clic en ella se abre esta ventana. Las dos barras finas bajo el cuadro de búsqueda rápida (azul: Borrador; naranja: sesión) se mantienen, y se pueden desactivar en **Archivo > Opciones > Apariencia > Interfaz general**. Las palabras que entran en el proyecto de una sola vez no cuentan como escritas en la sesión (0.94.0): el texto importado (**Archivo > Importar**, **Importar y dividir**, archivos arrastrados, páginas web), un pegado de más de 200 palabras y las palabras que un proyecto ya contiene cuando lo abres por primera vez. **Restablecer sesión ahora** pone el recuento a 0 y la información emergente del cuadro de búsqueda rápida lo muestra de inmediato. La ventana de objetivos reserva el espacio de sus barras, de modo que los campos no se mueven al fijar un objetivo.
- **Objetivos de documento**: haz clic en el objetivo del pie del editor.
- **Objetivos de recorte** (0.95.0): un objetivo puede ser un número negativo, en todos los campos de objetivo (el Borrador, la sesión y un documento). Es un objetivo para recortar: «-2000» para una sesión significa recortar 2000 palabras hoy, «-4000» para un documento significa reducirlo a 4000 palabras. Su barra se llena a medida que baja el recuento (una barra de sesión según las palabras eliminadas, una barra de documento o del Borrador hasta que el recuento baja al número), y un número negativo se muestra en rojo. Con un objetivo de sesión negativo, el recuento de hoy es neto: cuenta las palabras eliminadas como un número por debajo de 0, en rojo; con un objetivo positivo sigue como antes y nunca baja de 0. Scrivener no admite objetivos negativos: en un proyecto en formato Scrivener el *.scriv* guarda «sin objetivo» para ellos y {{APP}} conserva el número en su copia de trabajo.
- **Proyecto > Estadísticas del proyecto**: palabras y caracteres del Borrador, del texto compilado y de todo el proyecto, además de una estimación del número de páginas. En un proyecto grande el recuento tarda un momento; el puntero de ocupado lo indica.
- **Proyecto > Historial de escritura**: cuántas palabras escribiste cada día.

Las herramientas de escritura que siguen son complementos (sección 17): el **Generador de nombres**, el **Bloc de notas rápidas** y las **Estadísticas del texto** (llegaron con la actualización 0.11 como un solo paquete) y la **Transcripción de voz** (0.61); cada una se puede desactivar o quitar en **Ayuda > Complementos y actualizaciones > Gestor de complementos**. Sin ellas, **Edición > Herramientas de escritura** lo indica; el Gestor de complementos está en el menú Ayuda.

### Estadísticas del texto

**Proyecto > Estadísticas del texto…** cuenta una parte de tu proyecto. Arriba eliges qué texto:

- **Texto seleccionado** (se elige al abrir la ventana con texto seleccionado),
- **Texto en el editor** (en el texto combinado: todos los documentos mostrados),
- **Documentos seleccionados en el archivador**, con los documentos que contienen,
- **Borrador (incluido en la compilación)**, o **Todo el proyecto** (sin la Papelera).

La ventana tiene cuatro pestañas:

- **Resumen**: palabras (contadas exactamente igual que en la barra del pie y en los objetivos), caracteres con y sin espacios, párrafos, frases, palabras distintas, páginas de 250 palabras, tiempo de lectura a 230 palabras por minuto y promedios (letras por palabra, palabras por frase y por párrafo, la frase más larga; apunta a ella para leerla).
- **Legibilidad (estimaciones)**, también en la primera pestaña. Para el inglés: el *índice de facilidad de lectura de Flesch* (de 0 a 100; cuanto más alto, más fácil; de 60 a 70 es inglés llano) y el *nivel de grado de Flesch-Kincaid* (el curso escolar estadounidense necesario para seguir el texto). Para el alemán: el *índice de Flesch de Amstad* (la misma escala) y la *Wiener Sachtextformel* (aproximadamente el curso escolar, de 4 fácil a 15 muy difícil). Para todos los idiomas: el *LIX* (palabras por frase más el porcentaje de palabras de más de seis letras; por debajo de 30, muy fácil; por encima de 60, muy difícil). El idioma se detecta solo o se elige arriba. Estas puntuaciones son orientativas, no veredictos: las sílabas y las frases se detectan con reglas aproximadas, y las fórmulas se crearon para prosa informativa, no para diálogos ni poesía.
- **Frecuencia de palabras**: cada palabra con su recuento y su porcentaje, de la más frecuente a la menos frecuente. **Ocultar palabras comunes** omite artículos, pronombres, preposiciones y similares (inglés y alemán). Puedes fijar una longitud mínima, buscar una palabra o contar las mayúsculas por separado. Haz doble clic en una palabra para buscarla en el proyecto.
- **Frases repetidas**: grupos de dos a seis palabras que usaste más de una vez dentro de frases, como una descripción que diste dos veces. Se omiten las frases formadas solo por palabras comunes. Haz doble clic para buscar en el proyecto.
- **Documentos**: una tabla de los documentos con palabras, caracteres, frases, palabras por frase y su porcentaje del total. Haz clic en el título de una columna para ordenar; haz doble clic en un documento para abrirlo.

**Exportar tabla…** guarda la tabla de la pestaña actual como un archivo CSV para una hoja de cálculo.

### Generador de nombres

**Edición > Herramientas de escritura > Generador de nombres…** crea nombres para personajes.

1. Marca las listas de **nombres de pila** (62 orígenes de Europa, América, Asia y África, por ejemplo inglés, estadounidense, irlandés y gaélico irlandés, alemán, austríaco, suizo, neerlandés, francés, canadiense francés, italiano, español, mexicano, portugués, brasileño, danés, sueco, noruego, finés, islandés, polaco, checo, húngaro, ruso, griego, turco, hebreo, árabe, persa, indio, japonés, chino, coreano, yoruba, suajili y zulú; todavía no hay listas escocesas ni galesas). Los **apellidos** salen de la misma lista que el nombre de pila, o bien desmarca **La lista del nombre de pila** y elige otras listas (un nombre de pila italiano con un apellido irlandés).
2. Elige **Género**, **Número de nombres** y qué **Mostrar** (nombre completo, solo el nombre de pila, solo el apellido, iniciales y apellido). **Con segundo nombre** añade un segundo nombre de pila.
3. **Ajuste fino**: **comunes ↔ raros** elige entre los nombres más comunes (a la izquierda) o los raros (a la derecha); funciona con las listas marcadas *por frecuencia* (por ejemplo estadounidense, sueco, húngaro, hebreo), que incluyen la frecuencia de cada nombre. **Aliteración** da a ese porcentaje de los nombres un apellido que empieza por la misma letra que el nombre de pila (Peter Parker); **Apellidos dobles** une dos apellidos con un guion. **El nombre de pila empieza por** / **El apellido empieza por** limitan los nombres a los que empiezan por esas letras.
4. Haz clic en **Generar nombres**. Selecciona nombres y haz clic en **Copiar** (todos los nombres si no hay ninguno seleccionado) o en **Insertar en el texto** (también con doble clic) para ponerlos en el cursor del editor.

Las mujeres reciben la forma femenina del apellido cuando el idioma la tiene (polaco *Kowalska*, checo *Nováková*, ruso *Иванова*). Los nombres japoneses, chinos, coreanos, húngaros y vietnamitas muestran primero el apellido. Algunas listas están en su propia escritura (ruso, griego, hebreo, árabe y otras); los nombres de la lista indican cuáles.

**Añadir lista propia…** convierte un archivo de texto con un nombre por línea en una lista propia (nombres de pila, nombres de pila femeninos o masculinos, o apellidos). Elige la lista y haz clic en **Quitar lista propia** para borrarla. Las listas de nombres proceden del proyecto de código abierto *Faker* (licencia MIT); son listas de nombres en uso, no listas de nombres históricos verificadas.

### Bloc de notas rápidas

El bloc de notas rápidas guarda notas fuera de todos los proyectos: ideas mientras hay otro proyecto abierto, citas copiadas de una página web, una imagen de una parte de la pantalla. Ábrelo con **Ventana > Bloc de notas rápidas** (**Alt+Shift+Return**).

- **+** crea una nota (la flecha a su lado: texto con formato o texto sin formato). El título sobre el texto es el nombre de la nota. **−** elimina la nota seleccionada.
- Las notas con formato admiten **negrita**, *cursiva* y subrayado (con los botones o con **Ctrl+B / I / U**) e imágenes: pega una imagen o haz clic en la **cámara** para capturar una parte de la pantalla. La cámara solo está disponible en las notas con formato.
- Todo se guarda solo mientras escribes. Si una nota no se puede guardar (disco lleno, carpeta de notas que ya no existe), la línea de estado lo indica y la nota sigue abierta con su texto: otra nota, una nota nueva u otra carpeta de notas solo se abre una vez guardada, y una ventana ofrece **Copiar texto** y **Reintentar**. Si el programa termina mientras una nota no se puede guardar, su texto va a la carpeta de emergencia *~/.local/share/{{APP}}/Emergency* (0.65).
- **Enviar al proyecto** muestra todos los proyectos abiertos, también los abiertos en otras ventanas de {{APP}}. Para cada uno: **Nuevo documento en la carpeta Investigación**, **Nuevo documento en** una carpeta de tu elección, o **Añadir al final de** un documento de texto. El formato y las imágenes se incluyen. La nota permanece en el bloc de notas rápidas hasta que la elimines.

Las notas son archivos normales en una carpeta, normalmente *Documents/{{APP}} Scratch Pad* (el texto sin formato como .txt, las notas con formato como .html con sus imágenes en la subcarpeta oculta *.images*). Elige otra carpeta, el tipo de las notas nuevas y si el bloc de notas rápidas permanece sobre las demás ventanas en **Archivo > Opciones > General > Bloc de notas rápidas**. Las notas modificadas en otro lugar (otra ventana de {{APP}}, una carpeta sincronizada) aparecen de inmediato.

**Desde cualquier programa.** El atajo del programa funciona mientras {{APP}} tiene el teclado. Para abrir el bloc de notas rápidas desde cualquier programa, haz que el escritorio inicie {{APP}} con `--scratch-pad`: en KDE, **Preferencias del sistema > Teclado > Atajos > Añadir nuevo > Comando o script**, introduce el comando que se muestra en **Archivo > Opciones > Teclado** (botón **Copiar comando**) y pulsa las teclas que quieras. El bloc de notas rápidas se abre entonces por sí solo, sin ventana de proyecto, y envía las notas a los proyectos que estén abiertos. Si {{APP}} se añadió al menú de aplicaciones (**Ayuda > Añadir al menú de aplicaciones**), la entrada del menú también tiene la acción **Bloc de notas rápidas**.

**La cámara** hace la captura con el programa de capturas de KDE, *Spectacle*, cuando está instalado (necesario en Wayland, donde los programas no pueden leer la pantalla por sí mismos): arrastra sobre la parte que quieras. Sin Spectacle, en X11, {{APP}} muestra la pantalla y tú arrastras sobre la parte deseada; **Esc** cancela, **Enter** captura toda la pantalla.

### Transcripción de voz

Habla en lugar de escribir: el complemento **Transcripción de voz** (versión 0.61 o posterior) escribe lo que dices. Todo ocurre en este equipo; no se envía nada a ningún sitio mientras hablas.

**Antes de la primera grabación: elige un modelo.** El reconocimiento de voz necesita un modelo, que se descarga una sola vez. Abre **Archivo > Opciones > Voz** (o la flecha junto al botón del micrófono, **Opciones de transcripción de voz…**), elige un modelo y pulsa **Descargar y usar**:

- **Small**: una descarga de 190 MB, rápido; adecuado para voz clara, traducción solo aproximada.
- **Medium** (recomendado): 539 MB; sigue el ritmo del habla en la mayoría de los procesadores de los últimos años; buen reconocimiento y buena traducción.
- **Large v3**: unos 1,1 GB; el mejor, pero en un procesador suele ser más lento que el habla, de modo que en tramos largos el texto se queda atrás. Una tarjeta gráfica ayuda.
- **Large v3 Turbo**: 574 MB; cercano a Large v3 y más rápido, pero no puede traducir.

Los modelos proceden de los archivos de modelo de whisper.cpp en Hugging Face. En **Origen** puedes indicar otra dirección, o usar **Usar un archivo que descargué…** para un archivo de modelo que hayas obtenido tú mismo. Antes de usar un modelo se comprueba: primero su suma de verificación (SHA-256), después la descripción del modelo dentro del archivo y una breve transcripción de prueba. Solo cuando todo está bien sustituye al modelo en uso, que entonces se elimina. Un modelo de la fuente oficial debe tener la suma de verificación conocida; si no, no se instala (la descarga puede estar dañada). Un archivo de otra dirección, o uno que descargaste tú, cuya suma de verificación no sea la conocida, solo se lee después de que lo aceptes, porque el motor de voz tiene que leerlo. Las demás direcciones deben empezar por *https://*. Una descarga interrumpida continúa donde se quedó la próxima vez. **Prueba de velocidad** muestra cuánto tarda este equipo en procesar una grabación corta con el modelo y el dispositivo elegidos.

**Grabación.** Pulsa el botón del micrófono de la barra de herramientas Herramientas (o **Edición > Herramientas de escritura > Transcripción de voz**). El inspector se abre en su pestaña **Transcripciones** (el micrófono, el primero de la fila de botones del inspector) y empieza una nueva transcripción, con un nombre formado por la fecha y la hora. Habla; una pausa breve termina un fragmento y su texto aparece unos segundos después. Una pausa más larga (3 s, salvo que la cambies) inicia un párrafo nuevo. Pulsa de nuevo el botón para detener: se escriben los fragmentos que aún esperan y después se puede editar el texto. La flecha junto al botón permite elegir el micrófono.

**Las transcripciones** son archivos de texto sin formato en una carpeta propia, fuera de todos los proyectos (normalmente *Documents/{{APP}} Transcriptions*), de modo que tus proyectos no se tocan. En la pestaña puedes editar una transcripción (se guarda sola), cambiarle el nombre, copiar todo el texto para pegarlo donde lo necesites, eliminarla (va a la papelera del escritorio) o abrir la carpeta. Mientras se escribe una grabación, su texto es de solo lectura. Si una transcripción editada no se puede guardar, sigue abierta (la línea de estado indica el motivo); si mientras tanto empieza una grabación o el programa termina, su texto va a la carpeta de emergencia *~/.local/share/{{APP}}/Emergency* (0.65).

**Grabar ahora, escribir el texto después** (0.64). Escribir el texto requiere mucha potencia de cálculo; en un portátil con batería (o en un teléfono) puedes solo grabar y dejar que el texto se escriba más tarde. Elige **Solo grabar** en **Archivo > Opciones > Voz** (*Al iniciar el micrófono*; en Android es lo habitual). El botón del micrófono entonces solo graba: no se ejecuta nada más y no se carga ningún modelo. La grabación se guarda como un archivo de sonido (WAV, unos 1,9 MB por minuto) junto a las transcripciones, con el mismo tipo de nombre, y aparece en la lista de la pestaña con un botón **Transcribir** en su línea; **Transcribir todo** escribe el texto de todas las grabaciones que aún no lo tienen, una tras otra. El texto aparece entonces como en una grabación en directo y se puede editar. Una grabación se conserva después de escribir su texto, salvo que marques **Eliminar una grabación cuando se haya escrito su texto**; al cambiar el nombre de una transcripción o eliminarla, su grabación se lleva con ella. Para escribir el texto en otro equipo, elige una carpeta de transcripciones que tu programa de sincronización mantenga igual en ambos. Si una grabación se detiene mientras se escribe su texto (o el programa termina), conserva su botón **Transcribir** y puede ejecutarse de nuevo.

**Idiomas.** El **idioma principal** es el idioma del texto; el **idioma alternativo** es otro idioma que también hablas (ambos se eligen entre los once idiomas del programa: inglés, alemán, español, francés, portugués, italiano, ruso, indonesio, turco, polaco y neerlandés). Con el inglés como idioma principal, lo que dices en el idioma alternativo se escribe en inglés (whisper solo traduce al inglés). Las palabras del idioma alternativo dentro de frases en inglés pueden entenderse mal (en una prueba, «Bahnhof» se convirtió en «Bon Huff»); ponlas en **Palabras de ayuda** y se escribirán como las dices («Bahnhof», «Brötchen»). Con otro idioma principal, o con Large v3 Turbo, el habla se queda en el idioma en que se dijo. **Conservar lo dicho junto a la traducción** añade el original entre paréntesis después de un fragmento traducido (tarda más). **Palabras de ayuda**: nombres y palabras que usas, para que se escriban a tu manera.

**Otras opciones.** **Calcular con**: el procesador, o una tarjeta gráfica que whisper.cpp pueda usar a través de Vulkan (se muestra cuando se encuentra una; las tarjetas con muy poca memoria para el modelo aparecen en la lista pero no se pueden elegir). En Windows el complemento usa por ahora solo el procesador: Small o Medium funcionan mejor allí. **Hilos del procesador**: uno menos de los que tiene el procesador mantiene fluida la escritura. **Precisión**: *Cuidadosa* sopesa varios textos posibles y detecta primero el idioma de cada fragmento: mejor, pero más lenta. **Micrófono**, **Sensibilidad** (baja para una sala ruidosa, alta para una voz tenue), **Pausa que termina un fragmento**, **Párrafo nuevo tras una pausa de** (*nunca* lo desactiva), la **carpeta** y, en **Avanzado**: omitir texto en el silencio y el ruido (whisper a veces inventa allí una línea como «Thank you for watching») y usar el texto anterior como contexto para el siguiente fragmento.

---

### Ayudante de traducción

Desde la versión 0.92.0, el complemento **Ayudante de traducción** (versión 1.5; hasta la 1.4 se llamaba Traducción y corrección) es uno de los complementos propios de {{APP}} y viene incluido en la versión completa; para un {{APP}} que ya tengas, es un paquete aparte en la página de Releases (sección 17). Traduce entre dos cualquiera de once idiomas y revisa la ortografía y la gramática de textos en inglés, en tu propio equipo: el texto nunca sale de él. {{APP}} no tiene funciones de IA propias; este complemento trabaja con modelos de lenguaje y se puede desactivar o quitar en el Gestor de complementos como cualquier otro.

- **Esta herramienta solo da sugerencias. Sus traducciones no son precisas: revisa cada una, lo cual requiere un buen conocimiento práctico de ambos idiomas.** La misma frase aparece en la ventana del complemento y en su descripción.
- Tiene una pestaña propia en el inspector y una ventana propia (su comando está en el submenú Herramientas de escritura del menú Edición, y tiene un botón en la barra de herramientas). El texto entra, el resultado sale con cada cambio marcado, en directo mientras escribes; un clic en una palabra conserva el original u ofrece otras palabras.
- Necesita un modelo, que eliges y descargas una sola vez en las opciones del complemento (de unos cientos de megabytes a varios gigabytes; no se ofrece un modelo demasiado grande para el equipo). Los modelos se obtienen de las releases *models-1*, *models-2*, *models-3* y *models-4* del repositorio de {{APP}}; cada uno se instala solo si su suma de verificación es la conocida, y después el complemento funciona sin ninguna conexión. Un modelo, CoEdIT Large, solo puede usarse con fines no comerciales; el complemento lo advierte antes de descargarlo.
- **Una palabra sola** (1.3). Una palabra en inglés: el resultado es la palabra tal como debe ser y, debajo, lo que significa (los significados proceden del diccionario WordNet 3.0, que está integrado en el complemento; si no tiene una palabra, el modelo Qwen3 que hayas descargado le escribe un significado, con una advertencia de que puede ser inexacto, y sin ese modelo el complemento indica lo que puedes descargar), sus sugerencias de ortografía y palabras de significado parecido. Una palabra en alemán: el complemento la busca primero en el **diccionario alemán**, un extracto de Wiktionary que descargas por separado en las opciones del complemento (su licencia, CC BY-SA 4.0, se muestra antes de la descarga): la forma base, las clases de palabra, los significados y las palabras inglesas de cada significado. Se muestran los cinco primeros significados de una clase; el resto queda tras «más»; un clic en una palabra inglesa la convierte en el resultado, y «Similares» sigue entonces esa palabra y su clase de palabra. Para lo que el diccionario no dice, y para una palabra que no tiene, el modelo pequeño traduce la palabra y la salida advierte de que puede ser inexacta. Del inglés al alemán funciona igual para una palabra inglesa, según las clases de palabra que conoce WordNet.
- **Intercambiar y Traducir de vuelta** (1.3). **Intercambiar** convierte alemán a inglés en inglés a alemán y viceversa: el resultado pasa al texto y se traduce en sentido contrario. **Traducir de vuelta** traduce el resultado al idioma de tu texto y lo muestra debajo del resultado, hasta que cambies el texto; necesita el modelo para el camino de vuelta (OPUS-MT inglés a alemán (pequeño) para un texto alemán) y ofrece descargarlo si falta. Mientras traduces, la segunda fila de controles muestra el modelo de traducción (al elegir otro cambia el ajuste y la traducción) y la lista de estilos se oculta.
- **Textos cortos** (1.3). Una sola palabra y un texto de hasta tres palabras nunca se dan al modelo de traducción grande (es mejor para frases completas y poco fiable para palabras sueltas): primero el diccionario y, si no, el modelo pequeño, también cuando se ha elegido el grande. Para un texto corto de varias palabras, «Otras traducciones:» enumera las mejores traducciones completas del modelo, sin nombres inventados por él.
- **Versión 1.4.** Una sola palabra, en cualquiera de los dos sentidos, muestra ahora, bajo el resultado, las traducciones de cada clase de palabra que tiene (una palabra alemana que es verbo en minúscula y sustantivo con mayúscula recibe ambas; una palabra que el diccionario no tiene se traduce en las dos formas), después «Traducciones similares» (palabras de significado parecido al resultado en inglés) o, del inglés al alemán, «Palabras alemanas similares»: las palabras alemanas cuyos significados nombran la palabra inglesa o uno de sus sinónimos (esto requiere el diccionario alemán; sin él, una línea lo indica), y luego la descripción breve. Con «Automático», una sola palabra alemana que no es una palabra inglesa se reconoce como alemana cuando el diccionario alemán está instalado. Antes de traducir alemán, las mayúsculas de los sustantivos alemanes se corrigen en la copia que va al modelo (tu texto no se toca): una palabra que el diccionario conoce solo como sustantivo recibe su mayúscula, una palabra conocida de ambas formas («essen») conserva la forma que escribiste, salvo justo después de un artículo o de una preposición con artículo («das Essen», «im Essen»); sin el diccionario, una nota bajo la traducción indica que los sustantivos alemanes necesitan mayúsculas. Las alternativas que se diferencian entre sí o de la traducción solo en las mayúsculas se muestran una sola vez. El área de palabra ya no muestra una línea «Ortografía» para las palabras del resultado (las palabras en las que haces clic en tu propio texto la conservan), y las reglas de sufijos de las palabras similares ya no leen «number» como «numb». Cuando falta algo (un modelo de traducción, el diccionario alemán), un aviso en la parte superior de la pestaña y la ventana del complemento lo nombra y tiene un botón que abre las opciones del complemento. Un modelo que no se ha usado durante 15 minutos se descarga para liberar memoria (**Archivo > Opciones > Corrección > Velocidad > Descargar modelos tras**; «nunca» lo mantiene cargado). Las sugerencias de los campos vacíos se ajustan a un ancho estrecho.
- **Versión 1.5: once idiomas.** El complemento traduce entre dos cualesquiera de inglés, alemán, español, francés, portugués (Brasil), italiano, ruso, indonesio, turco, polaco y neerlandés: con **Traducir** elegido, dos listas desplegables nombran el idioma de tu texto y el idioma de la traducción. Cada idioma tiene dos modelos OPUS-MT pequeños, hacia el inglés y desde él (sus licencias, Apache-2.0 o CC BY 4.0, se muestran en las opciones antes de una descarga); un par sin inglés (de español a ruso, por ejemplo) pasa por el inglés con ambos modelos, y una nota bajo el resultado lo dice, porque un error del primer paso se arrastra al segundo. Cada idioma salvo el inglés tiene además un **diccionario** opcional de Wikcionario (CC BY-SA 4.0, una descarga aparte cuya licencia se muestra antes), que se usa para una palabra sola de ese idioma como se usaba el diccionario alemán. Un idioma sin diccionario utilizable recibe solo las traducciones del modelo. Una palabra sola se sigue corrigiendo como sugerencia: la línea de ortografía y las palabras de debajo dicen que son sugerencias que hay que revisar. Los textos propios del complemento vienen en los once idiomas del programa (el idioma que elegiste para {{APP}}; inglés donde un texto no está traducido). Los ajustes de la 1.4 siguen funcionando: el diccionario alemán y los modelos que descargaste se quedan, y de alemán a inglés es la primera opción. Sus opciones están ahora en **Archivo > Opciones > Ayudante de traducción**.
- Los modelos que descargaste con la versión 1.1 se quedan como están. La versión 1.1 obtenía los modelos del antiguo repositorio del complemento; la 1.2 conoce el nuevo lugar.
- La descripción propia del complemento, con sus reglas de corrección (ortografía estadounidense, comillas curvas, sin rayas) y en qué es bueno cada modelo, está en *addons/translation-proofreading/source/README.md* en el código fuente de {{APP}}.

## 11. Instantáneas

Una instantánea es una copia guardada de un documento en un momento determinado. Haz una antes de una gran reescritura:

- Un documento con al menos una instantánea muestra una esquina doblada (clara y gris) en la parte superior derecha de su icono en el archivador.
- **Ctrl+5** toma una instantánea; **Ctrl+Shift+5** pide un título.
- Con **Archivo > Opciones > General > Guardado**, **Archivo > Guardar** (**Ctrl+S**) toma una instantánea de cada texto que cambiaste desde el último Archivo > Guardar (con el título «Guardado»); **Archivo > Opciones > Comportamientos > Instantáneas** muestra un mensaje breve en la barra de estado cuando se toman instantáneas.
- En la pestaña de instantáneas del inspector puedes **comparar** una instantánea con el texto actual (las adiciones se muestran en color, las eliminaciones tachadas; elige si comparar palabras, frases, párrafos o caracteres) y **revertir** a ella. Antes de revertir, {{APP}} puede tomar una instantánea del texto actual, para que no se pierda nada. La instantánea se lee primero (0.65): una que no se puede leer entera deja el texto actual como está, una que no contiene texto lo sustituye solo tras una segunda pregunta y, cuando se pidió la instantánea del texto actual pero no se puede tomar, no se revierte nada. Revertir es también la forma de reparar un documento cuyo archivo está dañado o en otra codificación de caracteres (se muestra de solo lectura): el archivo dañado se conserva antes como copia en *settings/conflicts* y el inspector indica dónde (0.81.1).
- **Proyecto > Eliminar instantáneas antiguas…** (desde 0.61.1, debajo de Vaciar papelera…) elimina todas las instantáneas del proyecto, también las de documentos de la Papelera, para que el proyecto vuelva a ser más pequeño. Los textos se quedan como están ahora; solo se van sus versiones anteriores. Una pregunta previa indica cuántas instantáneas hay, de cuántos documentos y cuánto espacio ocupan; esto no se puede deshacer, así que haz antes una copia de seguridad (**Archivo > Copia de seguridad**). El comando está desactivado mientras el proyecto no tiene instantáneas. En un proyecto en formato de Scrivener, las instantáneas también salen del *.scriv* (con el guardado que sigue de inmediato); desde 0.61.1 ocurre lo mismo con una sola instantánea que eliminas en el inspector.

---

## 12. Modo de composición

**F11** llena la pantalla con nada más que tu texto sobre una hoja de papel. Mueve el ratón al borde inferior para mostrar la barra de control: zoom, ancho del papel, desvanecimiento del fondo, desplazamiento tipo máquina de escribir, recuento de palabras y un pequeño inspector con sinopsis y notas. **Alt+Shift+Up / Down** pasa al documento anterior o siguiente; **Ctrl+[** y **Ctrl+]** retroceden y avanzan por los documentos que has visto. Los atajos de los menús Formato e Insertar (negrita, estilos, comentarios ...) también funcionan aquí. **Esc** o **F11** vuelve a la ventana principal.

**Archivo > Opciones > Comportamientos > Modo de composición** elige la pantalla en la que se abre (la actual, la principal o una por nombre), si la ventana principal se aparta (se minimiza mientras el modo de composición está abierto) y si **Esc** lo cierra (**F11** siempre lo hace).

La barra de control es oscura en todos los temas, de modo que sus botones, la lista de zoom y sus textos siguen siendo legibles también en los temas claros (0.94.0). Los colores del texto, el papel y el fondo proceden del tema. Para cambiarlos, por ejemplo a negro sobre blanco o verde sobre negro, ve a **Archivo > Opciones > Apariencia > Modo de composición**; el cambio se ve de inmediato, incluso con el modo de composición abierto. Se puede fijar una imagen de fondo por proyecto en **Ajustes del proyecto > Carpetas especiales**.

---

## 13. Compilar: crear el manuscrito

**Archivo > Compilar…** (**Ctrl+Shift+E**) reúne todo el Borrador en un solo archivo.

### La ventana de compilación

La ventana de compilación tiene **Compilar para** arriba, los formatos a la izquierda, las disposiciones de sección en el centro y los contenidos, los metadatos, las opciones y los reemplazos a la derecha (cuatro pestañas con iconos; mantén el ratón sobre una pestaña para ver su nombre). La columna Formatos es al menos tan ancha como su nombre más largo; arrastra las líneas entre las tres columnas para ensanchar una, y la ventana recuerda los anchos para la próxima vez.

1. **Compilar para**: el formato de salida. Integrados, sin necesidad de otro programa: **Documento de Word (.docx)**, **PDF (.pdf)**, **Libro electrónico (.epub)** y, para guiones, **Fountain** y **Final Draft**. PDF y OpenDocument a través de LibreOffice, EPUB a través de Pandoc y Markdown aparecen también si están instalados el plugin y el programa correspondientes (véase más abajo). Junto a él: cuántos documentos y aproximadamente cuántas palabras se compilarán.
2. **Formatos** (izquierda), en **Mis formatos** y **Formatos integrados**: el formato de compilación decide el aspecto del manuscrito: fuentes, interlineado, tamaño de página, márgenes, encabezados y pies de página, el aspecto de capítulos y escenas, cómo salen los estilos, las notas y las citas. Los formatos integrados (*Manuscrito (Times)*, *Manuscrito (Courier)*, *Moderno*, *Libro de bolsillo (6 x 9 in)*, *Libro electrónico*, *Predeterminado (formato del editor)* y *Esquema (sinopsis)*) no se pueden modificar. Los botones bajo la lista: **+** (duplicar el formato seleccionado para crear tu propia versión y editarla), el lápiz (editar tu propio formato), **-** (eliminarlo), **Importar...** y **Exportar...** (un archivo *.qbformat* para usar en otro equipo o para regalar; un formato importado cuyo nombre ya existe recibe un número).
3. **Disposiciones de sección** (centro): **Fuente** fija una fuente para todo el manuscrito o lo deja a las disposiciones («Determinada por la disposición de sección»). La tabla enumera cada tipo de sección de tu proyecto con la disposición que recibe en este formato; elige otra en la lista. Bajo la tabla, una pequeña imagen muestra cómo empieza una sección con la disposición seleccionada. **Asignar disposiciones de sección...** abre una ventana más grande con los tipos de sección a la izquierda e imágenes de todas las disposiciones del formato a la derecha. Los formatos integrados conocen los nombres habituales de los tipos de sección: en *Manuscrito (Courier)*, por ejemplo, *Parte* recibe **Título de parte**, *Capítulo* recibe **Encabezado de capítulo** y *Escena* o *Sección* recibe **Texto de sección**.
4. **Contenidos** (primera pestaña): lo que se compila, con el tipo de sección de cada documento y su disposición. Elige una carpeta para compilar solo una parte del Borrador (por ejemplo un capítulo), añade material preliminar (portada) y material final (agradecimientos). **El nombre del autor** (0.94.0): la portada de las plantillas de proyecto dice «por» y el nombre del autor. La primera compilación que escribe una portada pide ese nombre (nombre y apellido; se guarda con la información del autor en **Archivo > Opciones > General**, de modo que se pide una sola vez); si lo dejas vacío, la línea «por» se omite en la portada, en todos los formatos. El campo Autor de la pestaña Metadatos de la ventana de compilación, si contiene un nombre, se usa en su lugar para el archivo compilado. Desmarca un documento para excluirlo; esto desactiva su ajuste **Incluir en la compilación**. **Añadir fichas de objetos** pone una ficha por cada objeto de la pestaña Objetos (sección 7) después del material final: un título por categoría (Personajes, Ubicaciones ...) y luego cada objeto con sus campos; el botón contiguo elige las categorías. Al principio está desactivado.
5. **Metadatos** (segunda pestaña): título, subtítulo, autor, idioma (por ejemplo *es-ES*), tema, palabras clave, editorial, fecha, descripción, la portada de un libro electrónico (una imagen del archivador) y el identificador del libro. Los campos vacíos toman el título del proyecto, tu nombre y la fecha de hoy. Los datos pasan a las propiedades del documento del PDF, al libro electrónico y al archivo de Word; aquí también funcionan marcadores de posición como `<$projecttitle>`.
6. **Opciones** (tercera pestaña): si se compila el texto de las carpetas.
7. **Reemplazos** (cuarta pestaña): reemplazos de este proyecto, hechos después de los del formato, por ejemplo un nombre escrito de una manera en tus borradores y de otra en el libro. Cada fila se puede activar y desactivar, y puede ser una expresión regular, distinguir mayúsculas y minúsculas o buscar solo palabras completas.
8. **Vista previa** muestra las páginas del manuscrito como PDF en una ventana propia, con el formato y los contenidos que elegiste, antes de guardar nada.

La fuente, los reemplazos y las fichas de objetos se guardan con el proyecto como las demás opciones (las versiones antiguas de {{APP}} las ignoran).

Haz clic en **Compilar…**, elige dónde guardar y, si quieres, el archivo terminado se abre en tu procesador de textos o lector. **Guardar ajustes** conserva las opciones sin compilar.

**Fuentes y formato de párrafo aplicados a mano.** Con el formato *Predeterminado (formato del editor)*, el archivo compilado los conserva todos: fuentes, tamaños, colores, mayúsculas, espaciado entre letras, sangrías, espaciado, interlineado y tabulaciones. Cualquier otro formato tiene un aspecto propio: sustituye fuente, tamaño, sangrías y espaciado, y conserva el color del texto, las mayúsculas, el espaciado entre letras, la alineación y las tabulaciones.

### Editar un formato de compilación

**Duplicar y editar…** y **Editar…** abren el editor de formatos. La lista de la izquierda contiene sus páginas:

- **Texto**: fuente, tamaño, interlineado, sangría de primera línea, espacio después de los párrafos, texto justificado, la fuente y el tamaño de los títulos, títulos en negrita. **Usar el formato del editor del proyecto** conserva en su lugar el aspecto del editor.
- **Disposiciones de sección**: las disposiciones del formato (añadir, duplicar, quitar) con título, prefijo y sufijo (`Chapter <$W:chapter>`), estilo y alineación del título, mayúsculas, líneas en blanco antes del título, del texto, de la sinopsis y de las notas, página nueva, el separador antes de la sección, sin sangría en el primer párrafo. La vista previa muestra cómo empieza una sección con esta disposición.
- **Asignaciones**: qué disposición recibe cada tipo de sección en este formato.
- **Estilos**: cómo sale en este formato un estilo de tu proyecto. **Añadir un estilo…** y luego elige: **Conservar su aspecto**, **Cambiar su aspecto** (fuente, tamaño, negrita, cursiva, subrayado, versalitas o mayúsculas, color, alineación, sangrías, espacio antes y después, interlineado, texto antes y después, mayúsculas, empezar en página nueva; una casilla a medio marcar, o «sin cambios», deja el estilo como está), **Convertirlo en texto normal** o **Omitir el texto** (por ejemplo, notas para ti mismo en un estilo propio).
- **Página**: tamaño de papel y márgenes, márgenes simétricos para libros (interior y exterior), encabezados y pies de página (izquierda, centro, derecha; marcadores de posición como `<$p>` para el número de página y `<$title>`), páginas enfrentadas (textos izquierdos en páginas izquierdas, textos derechos en páginas derechas), encabezado y pie en la primera página y en las páginas donde empieza un capítulo, capítulos en página derecha, el primer número de página y su estilo (1, 2, 3 / i, ii, iii / I, II, III), el tamaño de encabezados y pies de página.
- **Notas al pie y comentarios**: notas al pie al final de la página (en los libros electrónicos: notas emergentes), al final de cada capítulo o al final del texto (PDF y libros electrónicos; un archivo de Word siempre tiene sus notas al pie al final de la página), con el título de las notas y su tamaño; anotaciones en línea como comentarios o eliminadas; comentarios enlazados como comentarios del archivo de Word; resaltados eliminados.
- **Reemplazos**: texto reemplazado solo en el archivo compilado (tu proyecto no cambia): activado o desactivado, mayúsculas y minúsculas, palabras completas, expresiones regulares (`\1` a `\9` en «Con»).
- **Transformaciones**: comillas rectas o tipográficas, rayas como guiones, cursiva como subrayado (la convención del manuscrito), varios espacios como uno, sin espacios al final de los párrafos, texto sin formato, sin negrita, cursiva, fuentes ni colores.
- **Contenidos, PDF y libro electrónico**: un índice (su título y cuántos niveles de título), marcadores de PDF para los títulos, enlaces web en los que se puede hacer clic en el PDF, un archivo por cada sección que empieza en página nueva en un libro electrónico y el índice también como página del libro electrónico.

Tus formatos se guardan en *~/.local/share/{{APP}}/CompileFormats* y están disponibles en todos los proyectos.

### PDF

La salida PDF integrada maqueta las páginas por sí misma: notas al pie al final de la página donde se citan (una larga continúa en la página siguiente), encabezados y pies de página, capítulos en páginas derechas si el formato lo indica, títulos unidos al texto que sigue, sin líneas sueltas al principio o al final de una página, un índice con números de página en los que se puede hacer clic, marcadores para los títulos en el panel lateral del lector y enlaces activos. Las fuentes se incrustan. Las imágenes conservan el tamaño que tienen en el texto; una imagen mayor que el área de texto se reduce para que quepa. Las imágenes y las tablas junto al texto tienen el texto fluyendo a su alrededor, como en el editor. Una tabla continúa por tantas páginas como necesite, fila a fila, con sus filas de encabezado en la parte superior de cada página.

### Libros electrónicos

La salida de libro electrónico integrada escribe EPUB 3 (con un índice también para lectores antiguos): un archivo por capítulo, los títulos en el índice del lector, notas al pie como notas emergentes (o al final de cada capítulo o del libro), imágenes y la portada de **Metadatos…**. Las tablas conservan su diseño; una imagen con leyenda es una figura (junto al texto si lo está en el editor) y su descripción es el texto alternativo de la imagen, que leen los lectores de pantalla. El libro recibe un identificador la primera vez que se compila; se queda con el proyecto, de modo que un lector toma una nueva versión como el mismo libro. Los lectores eligen sus propias fuentes y tamaño de página, por lo que los ajustes de página, los encabezados y los pies de página no se aplican.

### Word

El archivo de Word usa estilos con nombre (Título, Subtítulo, Título 1 a 3, Cita, Centrado, Leyenda y tus propios estilos de párrafo y de carácter), de modo que sus títulos aparecen en el panel de navegación de Word. Con **Incluir un índice**, se pone un campo de índice al principio y el archivo pide al procesador de textos que actualice sus campos al abrirlo, lo que rellena los números de página; si un procesador de textos no lo hace, actualiza tú mismo el campo allí. Los encabezados de páginas enfrentadas, la primera página y el primer número de página proceden de los ajustes de **Página** del formato. Las tablas son tablas de Word con sus líneas, colores, celdas combinadas y filas de encabezado (repetidas en cada página en Word). Una imagen junto al texto se coloca en una tabla de una celda sin líneas, junto con su leyenda, para que Word y LibreOffice dejen fluir el texto alrededor de ambas; la descripción de una imagen es su texto alternativo.

### Títulos y marcadores de posición

En una disposición de sección, el título puede contener marcadores de posición que Compilar sustituye:

| Marcador de posición | Resultado |
|-------------|--------|
| `<$n>` | 1, 2, 3 … |
| `<$w>` / `<$W>` / `<$WW>` | uno, dos … / Uno, Dos … / UNO, DOS … |
| `<$R>` / `<$r>` | I, II, III … / i, ii, iii … |
| `<$L>` / `<$l>` | A, B, C … / a, b, c … |
| `<$n:name>` | un contador aparte llamado *name* (funciona con todos los estilos de número anteriores) |
| `<$title>` | el título del documento |
| `<$projecttitle>` / `<$shorttitle>` | el título del proyecto / sus tres primeras palabras en mayúsculas |
| `<$author>`, `<$forename>`, `<$surname>` | tu nombre de **Archivo > Opciones** o de los ajustes del proyecto |
| `<$wc>` / `<$wc100>` / `<$wc1000>` | el recuento de palabras, exacto o redondeado |
| `<$date>` / `<$year>` | la fecha de hoy (aaaa-mm-dd) / el año |
| `<$p>` | el número de página (solo en encabezados y pies de página) |
| `<$pages>` | el número de páginas (encabezados y pies de página de archivos PDF e impresiones) |

Ejemplo: el título `Chapter <$W:chapter>` se convierte en *Capítulo Uno*, *Capítulo Dos*, y así sucesivamente.

Los marcadores de posición también se pueden escribir en el propio texto (**Insertar > Numeración automática** e **Insertar > Marcadores de posición**). **Ayuda > Lista de todos los marcadores de posición…** los enumera todos; selecciona filas y pulsa **Ctrl+C** para copiarlas.

### Estilos al compilar

Los títulos, Título, Subtítulo, Cita en bloque, Centrado y Leyenda reciben su aspecto del formato de compilación. Cualquier otro estilo trae consigo su propio formato (sangrías, espaciado, alineación, fuente, color), en todos los formatos de compilación. Con **Predeterminado (formato del editor)**, también se trasladan los cambios que hiciste en los estilos integrados. La página **Estilos** de un formato cambia esto para cada estilo (véase *Editar un formato de compilación*).

### Notas al compilar

En la página **Notas al pie y comentarios** de un formato de compilación eliges dónde van las notas al pie en un PDF o un libro electrónico (al final de la página, al final de cada capítulo, al final del texto), si las anotaciones en línea se convierten en comentarios en el archivo de Word o se eliminan, si se conservan los comentarios enlazados y si se eliminan los resaltados. En un archivo de Word, las notas al pie son siempre notas al pie reales al final de la página.

### Más formatos de salida (plugins)

{{APP}} también incluye estos formatos. Cada uno necesita un programa que se instala por separado; PDF y EPUB a través de LibreOffice y Pandoc son una alternativa a los integrados (parten del archivo de Word y se parecen a él):

| Formato | Necesita |
|--------|-------|
| PDF (mediante LibreOffice) | LibreOffice (instalado en la mayoría de los sistemas Ubuntu y Kubuntu) |
| Texto OpenDocument (.odt, mediante LibreOffice) | LibreOffice |
| Libro electrónico (.epub, mediante Pandoc) | Pandoc (instala «pandoc» en Discover) |
| Markdown (.md) | nada |

**Archivo > Opciones > Plugins** muestra qué formatos están listos y a cuáles les falta un programa. Se pueden instalar más formatos como complementos (sección 17).

### Compilar guiones

Para guiones se crean dos salidas que no necesitan ningún otro programa:

- **Guion Fountain (.fountain)**: un archivo de texto sin formato que leen Highland, Fade In, Beat, Trelby y otros programas de guion.
- **Final Draft (.fdx)**: el formato de archivo de Final Draft, que la mayoría de los programas de guion también pueden abrir.

En ambos, cada párrafo conserva lo que es (encabezado de escena, acción, personaje, acotación, diálogo, transición, plano); el otro programa maqueta el guion a su manera. Los títulos de carpetas y documentos que añade el formato de compilación se convierten en secciones (Fountain) o en encabezados de acto (Final Draft); el texto que no es un elemento de guion se convierte en acción. El título del proyecto y tu nombre (Archivo > Opciones) forman la portada. Las anotaciones en línea se convierten en notas en un archivo Fountain y se omiten en un archivo Final Draft.

Para Word o PDF, elige el formato de compilación **Predeterminado (formato del editor)**: conserva las sangrías, las mayúsculas y la fuente de los elementos tal como los ves en el editor. Los demás formatos de compilación sustituyen la fuente y el tamaño, pero conservan las sangrías.

---

## 14. Importar y exportar

### Importar archivos

**Archivo > Importar > Archivos…** (**Ctrl+Shift+I**) añade al archivador archivos de texto (.txt, .md, .rtf, .docx, .odt; los antiguos archivos de Word .doc con el complemento descrito más abajo), guiones (.fountain, .fdx), imágenes, PDF, audio y vídeo, y otros archivos. También puedes arrastrar archivos desde el gestor de archivos hasta el archivador. Los archivos de texto se convierten en documentos editables; las imágenes y los PDF se muestran en el editor; el audio y el vídeo se reproducen allí con el complemento Reproducción multimedia (sección 6). Un archivo que no se puede copiar al proyecto (disco lleno, archivo que no se puede leer) se nombra en un mensaje y no se añade, de modo que no queda ningún elemento vacío (0.92.0).

Cómo se leen los archivos se ajusta en **Archivo > Opciones > Compartir > Importar**: si los comentarios de los archivos de Word, LibreOffice y RTF se convierten en anotaciones en línea (si no, quedan como comentarios junto al texto), si las notas al pie se convierten en notas al pie en línea (lo habitual) o en notas al pie enlazadas, si se omiten los estilos de párrafo y de carácter de los archivos (el texto recibe el aspecto de tu proyecto; se conservan la negrita, la cursiva, las notas y los enlaces), la codificación de los archivos de texto sin formato (automática: UTF-8 y, si no, Windows occidental) y si los archivos *.html* se convierten en documentos de texto en lugar de documentos de página web.

Adónde van los archivos: selecciona una carpeta en el archivador para importar dentro de ella (los archivos van al final) o un documento para ponerlos justo debajo de él. El Borrador admite solo texto; los demás archivos que se suelten allí van a Investigación.

**Importar y dividir…** lee un archivo largo y lo corta en muchos documentos, por ejemplo en cada línea que contiene solo `#`, o en cada doble línea vacía. Desde 0.94.0 también puede cortar un archivo .docx, .odt o .md **por sus títulos**: el Título 1 se convierte en una carpeta, el Título 2 en una carpeta o un documento (lo eliges en la ventana), el texto entre títulos se convierte en documentos y los títulos inferiores se quedan en el texto. La ventana muestra como árbol el archivador que resultaría antes de importar. El título de un documento que ningún título nombra sigue una regla, escrita en la ventana: la primera línea de su texto que no está vacía (acortada a 120 caracteres si es más larga) o «<nombre del archivo> <número>» cuando **Usar la primera línea de cada sección como título** está desactivado.

### Archivos antiguos de Word (.doc)

Word 97 a 2003 guardaba los documentos como *.doc* (las plantillas como *.dot*). Se convierten en documentos de texto con el complemento **Importación de Word 97–2003** (sección 17); sin él, un archivo .doc se añade al archivador tal cual y se te dice qué complemento lo lee. El complemento no necesita ningún otro programa. Conserva:

- el texto, incluidos el griego, el cirílico, las escrituras asiáticas y similares;
- títulos (Título 1 a 6), título, subtítulo, citas y leyendas, que reciben los estilos de tu proyecto; un estilo de párrafo del documento que tu proyecto también tiene (con el mismo nombre) recibe ese estilo;
- negrita, cursiva, subrayado, tachado, versalitas y mayúsculas, superíndice y subíndice, resaltado; párrafos centrados, alineados a la derecha y justificados;
- listas con viñetas y numeradas con sus niveles (los números mismos los crea {{APP}}, de modo que la numeración «Capítulo 1» de los títulos no se traslada);
- tablas, como una línea por fila con las celdas separadas por tabulaciones;
- notas al pie y notas finales (como notas al pie), comentarios con las palabras comentadas (el nombre del autor va primero en el comentario);
- enlaces, el resultado de los campos (fechas, números de página tal como se mostraron por última vez), saltos de página y de sección, saltos de línea;
- imágenes en el texto (JPEG, PNG, BMP, TIFF);
- en los proyectos RTF también fuentes, tamaños, colores y sangrías aplicados a mano (los proyectos Markdown conservan el aspecto del proyecto, como con toda importación).

Se omiten: encabezados y pies de página, imágenes flotantes y dibujos, imágenes en los antiguos formatos de metarchivo (WMF, EMF), objetos incrustados, texto oculto y texto eliminado con control de cambios (el texto insertado se queda). El texto de los cuadros de texto se añade al final del documento. Tras la importación, un mensaje nombra lo que se omitió.

**Archivo > Opciones > Compartir > Conversión** puede hacer de LibreOffice la primera opción para los archivos *.doc* (también lee encabezados, dibujos y archivos de Word 6/95); cuando falla la vía elegida, se prueba la otra. Los archivos de Word 6.0 y Word 95, y los archivos que el complemento no puede leer, se convierten con LibreOffice cuando está instalado (en Ubuntu: `sudo apt install libreoffice-writer`); si no, el mensaje lo indica. Los archivos protegidos con contraseña deben abrirse y guardarse antes sin la contraseña. Un archivo *.doc* que en realidad es un archivo RTF, un archivo de Word 2007 (.docx) o una página web, como los guardaron algunos programas, se lee como lo que es, también sin el complemento.

### Importar páginas web

**Archivo > Importar > Página web…** trae una página de la web a tu proyecto. El complemento **Importación de páginas web** (sección 17) hace el trabajo; sin él, el comando te lo dice.

1. Selecciona la carpeta que debe recibir la página (si no, va a **Investigación**).
2. Elige **Archivo > Importar > Página web…**. Si el portapapeles contiene una dirección web, ya aparece escrita; si no, escribe o pega una.
3. Haz clic en **Cargar** (o pulsa Enter). Se obtienen la página y sus imágenes, y la ventana muestra lo que se importará, con el número de palabras e imágenes, el autor, el sitio y la fecha.
4. Corrige el **Título** si quieres, revisa las opciones de abajo y haz clic en **Importar**.

**Solo el artículo** conserva el texto que buscabas y omite menús, anuncios, avisos de cookies, botones para compartir, secciones de comentarios y listas de otros artículos. {{APP}} encuentra el artículo como lo hacen las herramientas de lectura (gana el texto con más frases, comas y menos enlaces; nombres como «article» o «sidebar» en la página cuentan a favor o en contra de una parte). Si falta algo, desmarca la opción para tomar la página entera. **Imágenes** obtiene las imágenes del artículo (las muy grandes se reducen a 1600 píxeles de ancho). Para una página de internet, las imágenes se obtienen solo de direcciones públicas: una que esté en tu propio equipo o en una red privada (incluso mediante una redirección) se omite, y el resultado indica cuántas. Una página que procede de tu propio equipo o red se obtiene con todas sus imágenes.

**Importar como**:

- **Documento de texto** (lo habitual): los títulos, párrafos, citas, listas, tablas (una línea por fila), código de programa, leyendas, negrita, cursiva y enlaces se convierten en un documento normal que puedes editar, anotar, dividir y compilar.
- **Documento de página web**: una copia de solo lectura del artículo con sus imágenes, para consulta. No se cuenta ni se compila.

**Ver un documento de página web** (0.81.3). El editor muestra la página como texto sencillo con formato, con su dirección y dos botones encima. **Abrir la página guardada en el navegador** muestra la página tal como se guardó, en tu navegador web: {{APP}} pone una copia en tu carpeta Descargas (*Quillbinder web pages/<title>*), porque un navegador instalado como paquete aislado (Firefox en Kubuntu, por ejemplo) no puede leer las carpetas propias de {{APP}}; la línea bajo los botones indica dónde está la copia y el siguiente clic la reemplaza. En Windows la copia se hace en cambio en una carpeta temporal y se elimina al cerrar {{APP}}, de modo que no se escribe nada fuera de la carpeta del programa. **Abrir la dirección original** abre la dirección de la página, para que veas la página tal como es hoy; el botón está desactivado cuando la página no tiene dirección. Las páginas web que guardó Scrivener (un archivo *.mht* cada una) se muestran de la misma manera; un navegador como Firefox no puede abrir por sí mismo los archivos *.mht*, de modo que la copia es HTML sin más con sus imágenes al lado.

En cualquier caso, la dirección de la página se convierte en un **marcador** del documento (inspector, Marcadores), la descripción breve de la página se convierte en su **sinopsis** y las **notas** registran la dirección, el sitio, el autor, la fecha de publicación y cuándo la obtuviste.

**Arrastrar un enlace** desde un navegador web a una carpeta del archivador, al corcho o al esquema importa la página directamente, con las opciones que elegiste la última vez en la ventana.

Conviene saber: la página se obtiene como lo haría un navegador, pero sin ejecutar sus scripts. Las páginas que construyen su texto con un script suelen llevarlo también en sus datos, y entonces se usa (sin formato ni imágenes; un mensaje lo indica). Las páginas tras un inicio de sesión o un muro de pago dan lo que muestran a un visitante que no ha iniciado sesión. Las páginas seguras (https) necesitan OpenSSL 3 en tu sistema (forma parte de todo Ubuntu). Una dirección web que lleva a un archivo PDF no se importa de esta manera: descarga el archivo y usa **Archivo > Importar > Archivos…**.

**Guiones.** Los archivos Fountain (*.fountain*) y Final Draft (*.fdx*) se importan como guiones: los párrafos reciben los elementos del formato de guion de tu proyecto, el documento está en modo guion y la portada del archivo pasa a las notas del documento. Se conservan el texto en negrita, cursiva y subrayado, las notas y los saltos de página forzados; no se conservan los números de escena, las marcas de revisión ni el aspecto de dos columnas del diálogo doble (las dos intervenciones se suceden). **Importar y dividir…** con **Encabezados de escena** corta un guion en un documento por escena, con el nombre de su encabezado.

### Importar un proyecto de Scrivener

**Archivo > Importar > Proyecto de Scrivener…** lee un proyecto de Scrivener 3 o 2 (la carpeta *.scriv*; elige el archivo *.scrivx* de su interior) y crea a partir de él un nuevo proyecto de {{APP}}. El proyecto de Scrivener solo se lee, nunca se modifica. Desde 0.92.0, el informe posterior a la importación también nombra cada archivo que no se pudo copiar (una imagen, un PDF, un archivo de audio, la imagen de una ficha, un mapa mental), y ese archivo no se cuenta como importado; antes se omitía sin decir nada.

La importación trae: la estructura del archivador, los textos, las sinopsis, las notas, las etiquetas, los estados, las palabras clave, los metadatos personalizados, los tipos de sección, los indicadores de incluir en la compilación, las instantáneas, los comentarios y las notas al pie, las anotaciones en línea, los archivos de investigación (imágenes, PDF y otros), las colecciones, los marcadores del proyecto y los objetivos. Los ajustes de compilación no se importan; configúralos una vez en la ventana de compilación. Elige si el nuevo proyecto guarda su texto como Markdown o como RTF.

Títulos y estilos: Scrivener marca el texto con estilo dentro de sus archivos con códigos como `<$Scr_Ps::0><$Scr_H::2>`. No son texto; {{APP}} los quita. Un título recibe **Título 1** a **Título 6**, y un párrafo con uno de los estilos de párrafo de Scrivener recibe el estilo de tu proyecto con el mismo nombre (**Título**, **Cita en bloque**; el **Centered Text** de Scrivener se convierte en **Centrado**). Desde 0.81.3, el texto con uno de los estilos de carácter de Scrivener recibe el estilo de carácter de tu proyecto con ese nombre, y un estilo de Scrivener para el que tu proyecto no tiene ningún estilo con ese nombre se convierte en un estilo del proyecto, con el aspecto que le da Scrivener. Los proyectos importados por versiones anteriores a 0.14 conservaron los códigos como texto; {{APP}} los quita cuando se abre un proyecto así y lo indica en la barra de estado. Los marcadores del proyecto no se importaban antes de 0.14; para obtenerlos, importa de nuevo el proyecto de Scrivener.

Si algo no se trasladó correctamente, el proyecto original no se ha tocado y puedes copiar el texto directamente desde Scrivener.

### Trabajar en formato Scrivener

En lugar de hacer una copia, {{APP}} también puede trabajar directamente sobre un proyecto de Scrivener 3, para que puedas continuar después en Scrivener (por ejemplo en otro equipo).

- **Abrir**: **Archivo > Abrir proyecto…** (**Abrir otro…** en la ventana de inicio) y elige el archivo *.scrivx* dentro de la carpeta *.scriv*. La primera vez {{APP}} pregunta: **Trabajar en formato Scrivener** (tus cambios se guardan en el *.scriv*), **Convertir…** (se crea a partir de él un proyecto propio de {{APP}}, como con la importación anterior) o **Cancelar**. Después, el proyecto se abre sin la pregunta, también desde **Archivo > Abrir reciente**.
- **Proyectos nuevos**: elige **Formato: proyecto de Scrivener 3 (.scriv)** en la ventana Nuevo proyecto.
- **Guardar**: cada guardado (también el automático) escribe en el *.scriv*. Solo se escribe lo que cambiaste; los documentos que no tocaste conservan los archivos que escribió Scrivener. El título de la ventana dice *(formato Scrivener)*, y **Archivo > Mostrar el proyecto en el gestor de archivos** y las copias de seguridad toman la carpeta *.scriv*.
- **Proyectos grandes** (desde 0.60.5): el guardado automático mientras escribes pone en el *.scriv* solo los textos, notas y sinopsis que cambiaste (y el archivador, cuando cambió), de modo que sigue siendo breve incluso con cientos de miles de palabras. Todo lo demás, como el índice de búsqueda de Scrivener, las instantáneas y las imágenes, sigue cuando cambias a otro programa, usas **Archivo > Guardar** (**Ctrl+S**) o cierras el proyecto; por eso, antes de abrir el proyecto en Scrivener, ciérralo aquí o cambia antes a otro programa.
- **Un programa a la vez**: si Scrivener tiene abierto el proyecto (o no se cerró correctamente), {{APP}} lo indica y pregunta antes de abrirlo. Cierra el proyecto en un programa antes de abrirlo en el otro. Cuando el proyecto se modificó en Scrivener (o por un servicio de sincronización) desde la última vez que lo tuviste abierto aquí, {{APP}} lo lee de nuevo e indica en la barra de estado cuántos documentos cambiaron; un documento modificado entretanto en Scrivener nunca se sobrescribe con un texto más antiguo.

Qué pasa al *.scriv*: el archivador, los textos (como RTF, con sus imágenes), las sinopsis, las notas (también de imágenes, PDF, audio y vídeo, desde 0.81.3), las etiquetas, los estados, los tipos de sección, **Incluir en la compilación**, los marcadores (del proyecto y de cada documento, también a direcciones web y archivos), los objetivos del proyecto con la fecha límite, los objetivos de documento, el historial de escritura, el corcho (la disposición de cada carpeta y los lugares de las fichas colocadas libremente), el título, el autor y el idioma del archivo compilado (en los ajustes de compilación de Scrivener), las instantáneas, los comentarios y las notas al pie, las anotaciones y notas al pie en línea, los estilos de párrafo y de carácter, las palabras clave, los campos de metadatos personalizados y sus valores, las colecciones, las búsquedas guardadas (colecciones de búsqueda, desde 0.90.0), las imágenes de las fichas (desde 0.90.0), las imágenes, los PDF, el audio y el vídeo, las páginas web, los iconos (por los nombres de Scrivener; tus propios iconos en su carpeta *Icons*) y los mapas mentales (como un archivo adicional junto a los archivos del documento, que Scrivener deja en paz). Todo se escribe en la forma que usa Scrivener 3 para Windows:

- Las palabras clave, los metadatos personalizados y las colecciones conservan los números y nombres de Scrivener (desde 0.81.2), de modo que son los mismos en ambos programas.
- Estilos (desde 0.81.3): los estilos de un texto que cambias se marcan en él como los marca Scrivener, de modo que Scrivener muestra los mismos estilos. Un estilo que creaste en {{APP}} se añade a los estilos del proyecto de Scrivener la primera vez que un texto lo usa, con su aspecto.
- Las anotaciones y notas al pie en línea (desde 0.81.3) siguen en línea, las anotaciones con su color (antes, un texto que cambiabas se las daba a Scrivener como comentarios y notas al pie enlazados).
- Páginas web (desde 0.81.3): una página que guardaste en {{APP}} entra como la página web guardada propia de Scrivener (un archivo *.mht* con sus imágenes) y conserva su dirección. Otros archivos para los que Scrivener no tiene un tipo entran como documentos de texto que nombran el archivo, con el archivo al lado; {{APP}} los lee de vuelta tal como eran.
- El audio, el vídeo y las páginas web que las versiones anteriores a 0.81.2 convertían en esos documentos de texto vuelven a ser documentos propios de Scrivener de audio, vídeo o página web con el siguiente guardado.
- Los textos (desde 0.90.0) se escriben de vuelta con todo lo que Scrivener conserva en ellos: sangrías derechas por debajo de cero, listas con su numeración, el subrayado de los enlaces, imágenes enlazadas a un archivo, fuentes nombradas por su variante (como *Cambria-BoldItalic*) y el lugar de una imagen cuyos datos faltan. Los comentarios y las notas al pie conservan sus ID, si su recuadro está plegado y su propio formato mientras no los cambies.
- El corcho (desde 0.90.0): las fichas que colocas libremente conservan su lugar en ambos programas. Scrivener guarda un lugar en anchos de ficha; {{APP}} lo convierte en su propio tamaño de ficha, de modo que la disposición sigue igual con el tamaño con que cada programa muestre las fichas. Cada carpeta conserva su disposición (libre o en cuadrícula), como en Scrivener: elegir **Libre** o la cuadrícula para el corcho de una carpeta se recuerda para esa carpeta.
- Etiquetas y estados (desde 0.90.0): la etiqueta y el estado predeterminados de los documentos nuevos, y los nombres que Scrivener da a «No Label» y «No Status», se quedan como los tiene el proyecto.
- Búsquedas guardadas (desde 0.90.0): una búsqueda guardada de Scrivener es una colección de búsqueda en {{APP}}, y una colección de búsqueda que creas o cambias pasa al *.scriv* como búsqueda guardada. La lista de resultados de búsqueda recientes de Scrivener sigue siendo propia de Scrivener. Se comprobó con una búsqueda guardada de cualquiera de las palabras; cómo nombra Scrivener los demás tipos (todas las palabras, la frase exacta, una expresión regular, solo documentos incluidos en la compilación o excluidos de ella) aún no se ha visto en un proyecto real, de modo que una búsqueda guardada de esos tipos puede salir en Scrivener como una búsqueda de todas las palabras. Dónde buscan en los documentos una búsqueda no forma parte de lo que pasa: una búsqueda guardada leída de Scrivener busca en todas partes.
- La imagen de una ficha (desde 0.90.0): una ficha que muestra una imagen en lugar de su sinopsis la conserva en ambos programas; quitarla o ponerla en {{APP}} llega a Scrivener.

Lo que se queda solo del lado de {{APP}} (en una copia de trabajo en *~/.local/share/{{APP}}/ScrivenerProjects*): los formatos de guion, los ajustes de compilación (salvo el título, el autor y el idioma), los objetos (sección 7) y la disposición de la ventana; lo que el *.scriv* ya tenga de esto se deja en él tal como está.

La primera vez que se abre o se importa un proyecto de Scrivener, su archivador muestra las carpetas que estaban abiertas en Scrivener y los documentos seleccionados allí (0.90.0; {{APP}} nunca escribe los ajustes de ventana propios de Scrivener). La primera vez que abres un proyecto en formato Scrivener con 0.81.2 o posterior, {{APP}} lee de nuevo el *.scriv* y une las palabras clave, los metadatos personalizados y las colecciones de su copia de trabajo con los del proyecto, de modo que no se pierde nada de lo que hiciste con una versión anterior; el siguiente guardado los pone en el *.scriv*. La primera apertura con 0.90.0 o posterior hace lo mismo con los marcadores, los objetivos de documento, la fecha límite, el historial de escritura y el corcho.

Si un guardado en el *.scriv* se interrumpió (un fallo, un disco lleno) después de que se escribieran documentos nuevos en la copia de trabajo, la siguiente apertura conserva esos documentos y lo indica; el siguiente guardado los pone en el *.scriv*. Los documentos que se eliminaron en Scrivener siguen eliminados.

**Archivo > Guardar como…** (**Ctrl+Shift+S**) guarda el proyecto abierto con otro nombre o en otro lugar, y es la manera en que un proyecto cambia de formato. En el diálogo, escribe el nombre y elige el lugar; el **tipo de archivo** decide el formato: *Proyecto de Scrivener 3 (\*.scriv)* o *Proyecto de {{APP}} (\*.qbx)* (un nombre que termina en *.scriv* o *.qbx* decide por sí solo). Un proyecto es una carpeta: el programa la crea a partir del nombre (*Name.scriv*, o *Name.quillbinder* con *Name.qbx* dentro). Un nombre que ya existe se rechaza.

- A partir de entonces la ventana trabaja con el nuevo proyecto y todo guardado posterior va allí. El título del proyecto es el nombre que escribiste. Los cambios que no habías guardado todavía pasan solo al nuevo proyecto.
- El original se queda en el disco tal como estaba en ese momento (consérvalo como copia o elimínalo); también está en **Archivo > Abrir reciente**, debajo del nuevo proyecto. Su archivo de bloqueo se quita cuando la ventana lo abandona, y el nuevo proyecto recibe el suyo.
- El nuevo proyecto es un proyecto propio, con sus propias **copias de seguridad**: empieza una serie nueva, y las copias hechas hasta ahora se quedan con el original (sección 14, Copias de seguridad). Un proyecto en formato Scrivener recibe para esto un nuevo identificador en su *.scrivx*.
- Guardar como funciona igual de un formato al mismo formato (un nombre o lugar nuevo) y al otro (el cambio que hacía hasta 0.92 el menú «Formato del proyecto»).

**Archivo > Exportar > Proyecto de Scrivener…** escribe una **copia** en formato Scrivener, y la entrada que está debajo en el mismo menú, **Proyecto de {{APP}}…**, una copia en el formato propio de {{APP}}; ambas dejan la ventana en el original: no cambia nada en el proyecto con el que trabajas (antes de 0.93.0 eran «Guardar una copia como ...» en el menú «Formato del proyecto»).

### Exportar documentos

**Archivo > Exportar > Archivos…** guarda los documentos seleccionados para otros programas. Usa Compilar para un manuscrito terminado; usa Exportar para entregar documentos sueltos a alguien o para conservar una copia fuera de {{APP}}.

1. En el archivador, selecciona los documentos y carpetas que quieres exportar.
2. Elige **Archivo > Exportar > Archivos…**.
3. **Exportar archivos de texto como**: texto enriquecido (.rtf), Word (.docx), Markdown (.md), texto sin formato (.txt), Fountain o Final Draft (para guiones, Fountain viene preseleccionado). Las imágenes, los archivos PDF, las páginas web, el audio y el vídeo se exportan tal cual; el formato solo cuenta entonces para sus notas.
4. **Metadatos**: **Exportar metadatos** escribe un archivo de texto junto a cada documento, «*name* (metadata).txt», con su título, tipo, etiqueta, estado, palabras clave, incluir en la compilación, tipo de sección, recuento de palabras, objetivo, fechas de creación y de cambio, metadatos personalizados y marcadores. **Exportar sinopsis** le añade la sinopsis (o escribe «*name* (synopsis).txt» cuando no se exportan los metadatos).
5. **Opciones**: **Incluir subdocumentos** (las carpetas se convierten en carpetas), **Exportar notas** («*name* (notes)» en el formato del paso 3; texto sin formato para guiones), **Exportar comentarios y anotaciones en línea** (desmárcalo para entregar el texto limpio; las palabras comentadas se quedan, las notas al pie siempre se quedan), **Exportar instantáneas** (una carpeta «*name* (snapshots)» con un archivo por instantánea, con el nombre formado por la fecha y el título).
6. **Ubicación**: la carpeta donde guardar. Cuando se seleccionan varios documentos, van a una carpeta nueva cuyo nombre indicas (se propone el título del proyecto; si la carpeta existe, se añade un número).
7. Haz clic en **Exportar**. La carpeta se abre en el gestor de archivos.

Las opciones se recuerdan para la siguiente exportación.

La forma rápida: **arrastra documentos fuera del archivador** al gestor de archivos o al escritorio. Los documentos de texto se guardan como archivos RTF (o Word, Markdown o texto sin formato: **Archivo > Opciones > Comportamientos**), las imágenes y otros archivos tal cual, y una carpeta se convierte en una carpeta con sus documentos dentro. Si el gestor de archivos pregunta si mover o copiar, cualquiera de las dos cosas vale: los documentos se quedan en tu proyecto.

### Comparar y transferir: un archivo de Word o LibreOffice y el proyecto

Cuando un manuscrito vuelve de un editor, o seguiste trabajando en él en Word o LibreOffice, **Archivo > Comparar y transferir > Comparar con un archivo…** encuentra lo que difiere entre ese archivo y tu proyecto y toma el lado que elijas.

1. Elige el archivo: un archivo de Word (*.docx*) o de LibreOffice (*.odt*). Los antiguos archivos de Word (*.doc*), RTF, de texto sin formato y Markdown también se pueden comparar, pero solo se leen: su texto puede pasar al proyecto; el archivo en sí no se puede cambiar.
2. {{APP}} compara el archivo con **lo que Compilar hace del proyecto**, no con el proyecto entero: los documentos del grupo de compilación (el Borrador, salvo que se elija otro grupo en la ventana de compilación) que tienen **Incluir en la compilación**, en el orden del archivador. El material preliminar y final, la carpeta Investigación y los documentos excluidos de Compilar no cuentan. Si se encuentra en él menos del 65 % del texto del archivo, {{APP}} lo indica y pregunta si comparar de todos modos (**Comparar de todos modos**); el archivo puede pertenecer a otro proyecto, o Compilar omite lo que el archivo tiene. Una palabra cuenta como encontrada cuando está en una serie de cuatro palabras que el texto compilado también tiene, de modo que unas pocas palabras cambiadas no hacen que un archivo no esté relacionado.
3. La ventana **Comparar y transferir** muestra las diferencias. Elige el lado que se actualiza:
   - **Actualizar el proyecto desde el archivo**: los cambios se hacen en tus documentos. Antes, cada documento cambiado recibe una instantánea, «Antes de Comparar y transferir (*nombre del archivo*)» (**Documentos > Instantáneas**), para que siempre puedas volver atrás. El texto que solo está en el archivo se convierte en un documento nuevo (con el nombre de su título en el archivo), colocado después del documento anterior.
   - **Actualizar el archivo desde el proyecto**: el archivo que elegiste se queda como está. {{APP}} escribe una copia a su lado, «*name* (Quillbinder).docx» (con un número cuando ese nombre ya existe). En un archivo de Word, cada cambio es un **cambio con control de cambios** del autor «Quillbinder»: abre la copia en Word o LibreOffice y acepta o rechaza allí cada cambio (en Word, en la pestaña Revisar; en LibreOffice, con Editar, Registrar cambios, Gestionar). Una copia de LibreOffice (*.odt*) recibe los cambios directamente, sin marcas.
4. En la vista, el texto **verde** entra, el texto **rojo** tachado sale y el texto **azul** recibe la negrita, la cursiva o el subrayado del otro lado. Los párrafos grises son texto sin cambios alrededor de un cambio, para mostrar dónde está. Cada lugar tiene un enlace **aceptar** u **omitir**; un clic lo alterna.
5. Haz clic en **Transferir ... al proyecto** o **Escribir ... en una copia del archivo**. Un mensaje indica lo que se hizo; para el archivo, **Abrir el archivo** abre la copia.

**Por capítulo** (**Archivo > Comparar y transferir > Por capítulo**, también una casilla en la ventana) enumera los documentos a la izquierda, cada uno con una casilla y lo que difiere («2 lugar(es)», «solo en el archivo», «no está en el archivo»). Desmarca un documento para dejarlo como está; un clic muestra sus cambios a la derecha. Sin Por capítulo, todos los cambios se muestran uno debajo de otro.

**Paso a paso** (**Archivo > Comparar y transferir > Paso a paso**, también en la ventana) muestra un cambio cada vez: **Aceptar** u **Omitir**, o vuelve **Atrás**. Al final la ventana indica cuántos cambios se aceptarán; entonces haz clic en Transferir. Ambas opciones se recuerdan para la próxima vez.

Qué se compara: las palabras, y la negrita, la cursiva y el subrayado. Cada párrafo del archivo se empareja con el documento que tiene sus palabras, de modo que una escena se encuentra también donde se movió en el archivo (entonces se marca «en otro lugar del archivo»; el orden del archivador no cambia). Los títulos del archivo que no son texto de un documento, como los títulos de capítulo creados por Compilar, no se comparan.

Qué está marcado al principio: cada cambio dentro de un documento. El texto que solo está en un lado se marca cuando tiene al menos 100 palabras (un capítulo nuevo del archivo se convierte en un documento nuevo; una escena que falta en el archivo se añade a la copia). Un documento entero nunca se mueve a la Papelera, y una parte entera nunca se saca del archivo, salvo que lo marques.

Límites: los párrafos nuevos toman el aspecto del párrafo contiguo. El texto de las tablas se compara como los demás párrafos (una tabla en sí nunca se saca) y los campos cuentan con el texto que muestran. Los encabezados y pies de página, los cuadros de texto y el texto de las notas al pie y comentarios del archivo no se comparan y se quedan como están; los comentarios y notas al pie del proyecto se quedan en el proyecto y no se escriben en el archivo. Si el archivo cambió después de la comparación, la copia no se escribe; compara de nuevo.

### Imprimir

**Archivo > Imprimir documento actual…** (**Ctrl+P**) imprime lo que muestra el editor actual:

- **Un texto** (un documento, o varios en el texto combinado): el texto con su formato e imágenes, el título de cada documento sobre su texto y las notas al pie al final de la página. Con el teclado en un documento de apoyo, se imprime el documento de apoyo.
- **El corcho**: las fichas en el orden mostrado, tantas en una hoja como quepan (con líneas para cortar), o una ficha en cada hoja para cartulina de fichas.
- **El esquema**: las filas y columnas como las muestra el esquema, con los títulos de las columnas en la parte superior de cada página.
- **Una imagen** (tan grande como permita la página), **un archivo PDF** (sus páginas) o **una página web** (su texto).

La ventana de impresión elige la impresora, las páginas y el número de copias. **Selección** imprime solo el texto seleccionado, o solo las fichas o filas del esquema seleccionadas. Para guardar la impresión como archivo PDF, elige **Imprimir a archivo (PDF)** como impresora.

**Archivo > Vista previa de impresión…** muestra primero las páginas; el botón de impresora de la vista previa las imprime, **Cerrar** (o **Esc**) cierra la vista previa.

**Archivo > Ajustes de impresión…** decide cómo sale todo en el papel. La ventana se abre en la pestaña de lo que muestra el editor; **Vista previa…** muestra el resultado antes de hacer clic en **Aceptar**. Los ajustes pertenecen al proyecto; marca **Usar estos ajustes también para los proyectos nuevos** para convertirlos en lo habitual para los proyectos nuevos.

- **Página**: el tamaño y la orientación del papel (el botón **Papel de la vista de página** toma el papel y los márgenes de **Ver > Edición de texto > Tamaño y márgenes de página…**), los márgenes y el encabezado y el pie de página, cada uno con una parte izquierda, una central y una derecha. Marcadores de posición: `<$title>` (lo que se imprime), `<$projecttitle>`, `<$author>`, `<$date>`, `<$p>` (el número de página) y `<$pages>` (el número de páginas). Desmarca **Encabezado y pie también en la primera página** para una primera página limpia.
- **Texto**: lo que se imprime con el texto (el título de cada documento, la sinopsis debajo, las notas del documento después del texto, los comentarios y las anotaciones en línea como notas al final de la página, las notas al pie al final del texto en lugar de al final de cada página, los resaltados); las fuentes (como en el editor, o **Una sola fuente** de tu elección); y el interlineado (como en el editor, sencillo, 1,5 líneas o doble, con espacio para correcciones). Los documentos de guion conservan su disposición de guion.
- **Fichas**: el tamaño de la ficha (3 x 5 in, 4 x 6 in, 5 x 8 in, A7, A6 o un tamaño propio), tantas fichas en una hoja como quepan o una ficha en cada hoja, lo que muestra cada ficha (título, sinopsis, las primeras líneas del texto cuando no hay sinopsis, imágenes, el color de la etiqueta como una franja arriba, estado, palabras clave, las líneas de una ficha, una línea alrededor de cada ficha) y la fuente.
- **Esquema**: las sinopsis (como las muestra el esquema, siempre o nunca), también las filas dentro de los grupos cerrados, los títulos de las columnas en la parte superior de cada página, una línea entre filas, un punto del color de la etiqueta y la fuente. Con muchas columnas, **Horizontal** en la pestaña **Página** les da más espacio.

Imprimir documento actual imprime un documento tal como trabajas en él. Para un manuscrito terminado con el aspecto de un formato de compilación, compílalo (sección 13) e imprime el archivo PDF o de Word.

---

## 15. Copias de seguridad

{{APP}} hace una copia de seguridad (un archivo ZIP de todo el proyecto) cada vez que cierras un proyecto y conserva las cinco últimas por proyecto. Cámbialo en **Archivo > Opciones > Copia de seguridad**, donde también puedes elegir la carpeta de copias de seguridad (lo mejor es otra unidad, o una carpeta que copie tu programa de sincronización).

Desde 0.91, las copias de seguridad de todos los proyectos están directamente en la carpeta de copias de seguridad, cada una con el nombre de su proyecto, la fecha y la hora. Desde 0.92.0, el nombre lleva además el **ID** del proyecto, cinco caracteres tras un «#» (*My Novel #K7T2M bak 2026-11-20 15-40-00.zip*). El ID pertenece al proyecto, no al equipo: es el mismo en cada equipo en el que se abre el proyecto y se conserva cuando cambias el nombre del proyecto o mueves su carpeta. Las copias de seguridad de un proyecto son los archivos que llevan su título y su ID, y solo ellas se cuentan y se limpian para él, de modo que dos proyectos con el mismo título conservan cada uno sus propias copias y su propio número de copias conservadas. Una copia o un proyecto nuevo que crea el propio programa es un proyecto propio con su propio ID: **Archivo > Guardar como…** y la entrada Proyecto de {{APP}} de **Archivo > Exportar**, un proyecto restaurado con **Restaurar copia de seguridad** (que también recibe un título propio) y un proyecto creado a partir de una plantilla propia. Una copia hecha fuera del programa (una carpeta que copiaste a mano, una copia de seguridad que descomprimiste a mano) tiene el ID de su original: dale a esa copia un título propio. Un proyecto en formato Scrivener toma su ID del identificador que Scrivener guarda en el *.scrivx*. Si tu carpeta de copias de seguridad está en el OneDrive de una organización, es posible que su administrador haya prohibido «#» en los nombres de archivo; OneDrive para uso doméstico lo permite. Las copias de seguridad hechas por una versión anterior a 0.92.0 no tienen ID en su nombre: la limpieza nunca las elimina, **Restaurar copia de seguridad** las sigue enumerando y las eliminas tú mismo cuando ya no las necesites. Cuando cambias el título de un proyecto, las copias hechas con el título anterior se quedan hasta que las elimines. Las versiones 0.70 a 0.90 guardaban las copias de seguridad de cada proyecto en una carpeta propia dentro de la carpeta de copias de seguridad (*My Novel 1a2b3c4d*); esa carpeta se deja como está, ya no se añade ni se elimina nada allí y **Restaurar copia de seguridad** sigue enumerando sus copias. Una copia de seguridad se ejecuta junto a la ventana: tras un momento, una ventana de progreso muestra cómo va y puede cancelarla. Quién tiene abierto el proyecto (el archivo de bloqueo con el nombre del equipo y de la sesión) no está en la copia de seguridad.

- Con **Hacer copia de seguridad en cada guardado manual** (**Archivo > Opciones > Copia de seguridad**), cada **Archivo > Guardar** (**Ctrl+S**) hace también una copia de seguridad. Dos copias dentro del mismo segundo reciben un número («... (2)»). Cuando esa copia no se puede hacer, está incompleta o tiene que esperar fuera de la carpeta de copias de seguridad, un aviso junto al texto lo indica (0.92.0; antes solo lo hacía la barra de estado, durante unos segundos). Tu texto se guarda en cualquier caso. El aviso llega una vez por causa hasta que cierres el programa; la barra de estado lo indica en cada guardado.
- **La línea de mensajes** bajo el archivador muestra mensajes breves de uno en uno, durante 5 segundos (10 con un enlace) o hasta que hagas clic en ella; los demás esperan. Los complementos también la usan (por ejemplo «Modelo descargado, uso de RAM mínimo» cuando Traducción y corrección o Transcripción de voz liberó su modelo tras 15 minutos sin uso; el tiempo está en sus opciones). **Ayuda > Solución de problemas y desarrollo > Información del sistema** indica la memoria que mantiene el programa.
- **Complementos que no están instalados.** **Edición > Herramientas de escritura** enumera «Traducir y corregir» y «Transcripción de voz» mientras el complemento falta o está desactivado; cada uno muestra un aviso con un botón que abre **Ayuda > Complementos y actualizaciones > Gestor de complementos**.
- **Archivo > Copia de seguridad > Hacer copia de seguridad ahora** hace una copia de inmediato. Desde 0.94.0, el éxito se comunica en la línea de mensajes bajo el archivador («Copia de seguridad guardada.» con el enlace **Mostrar carpeta**, que abre la carpeta de la copia; la información emergente nombra el archivo; un mensaje con enlace permanece 10 segundos); ningún cuadro espera a que pulses Aceptar. Solo aparece un cuadro cuando algo falló, o cuando la copia se hace pero algunos archivos no se pudieron leer o no se pudo eliminar una copia más antigua.
- **Una copia de seguridad que un apagado o un comando kill impidió hacer** queda anotada y se hace en la siguiente apertura (0.81). Desde 0.93.0, la nota permanece hasta que se hace esa copia (en la carpeta de copias de seguridad, o esperando a su lado como se describió antes): una apertura cuya copia falla deja la nota para la siguiente apertura. **Una copia más antigua que la limpieza no puede eliminar** (un archivo bloqueado o protegido) se queda, y un mensaje nombra el archivo y el motivo una vez; elimínala a mano. Mientras sigues escribiendo, este mensaje es un pequeño aviso junto al texto que no toma ninguna tecla.
- **Hacer copia de seguridad en…** guarda una copia donde elijas, por ejemplo en una memoria USB.
- **Restaurar copia de seguridad…** (0.70) enumera las copias de seguridad del proyecto abierto con su fecha, tamaño y si están completas; elige una y **Restaurar y abrir**: se descomprime en una carpeta nueva junto al proyecto, *… (restored 2026-11-10 14-03)*, su título dice «(restaurado …)» y se abre allí. El proyecto que tenías abierto no cambia. La lista contiene las copias con el ID de este proyecto, las que no tienen ID (hechas antes de 0.92.0) y las de otro proyecto con el mismo título; el nombre de archivo en la lista muestra cuál es cuál, de modo que nada queda fuera de alcance. Desde 0.93.0 también enumera las copias que esperan en la carpeta de trabajo propia del programa (hechas mientras no se podía acceder a la carpeta de copias de seguridad; la siguiente copia en la carpeta de copias de seguridad las pone allí): están marcadas «en espera (aún no está en la carpeta de copias de seguridad)» y se restauran de la misma manera. Desde 0.81, una restauración de cualquier tamaño funciona (copias de 2 GB y más, archivos de más de 1 GB como vídeos, más de 65 535 archivos), cada archivo se comprueba al escribirse, una restauración que no cabe en la unidad no se inicia, una ventana de progreso puede cancelarla y, cuando no se pueden restaurar archivos, el mensaje nombra cada uno con el motivo (un disco lleno, una entrada dañada). Desde 0.93.0, el mensaje tiene tres formas: **restaurado** (todo está allí), **restaurado de forma incompleta** (así titulado, con un signo de advertencia, con los archivos que faltan nombrados con el motivo: mira el proyecto restaurado antes de fiarte de él) y **no restaurado**; y cuando el título del proyecto restaurado no se pudo marcar como «(restaurado …)», el mensaje lo indica.
- **Mover las copias antiguas a la carpeta de copias de seguridad…** (0.93) mueve las copias de seguridad que las versiones 0.70 a 0.90 guardaban en una carpeta propia dentro de la carpeta de copias de seguridad (*My Novel 1a2b3c4d*) a la propia carpeta de copias de seguridad, con sus nombres, para que estén junto con las demás. Está activo solo mientras el proyecto abierto tiene una carpeta así con copias. Pregunta primero e indica cuántas hay; una copia cuyo nombre ya está en la carpeta de copias de seguridad nunca se sobrescribe y se queda donde estaba; el informe indica lo que se movió y lo que se dejó (con el motivo), y la carpeta antigua se elimina cuando no queda nada en ella. Estas copias no tienen ID en su nombre, de modo que la limpieza nunca las cuenta ni las elimina, como antes; **Restaurar copia de seguridad** las enumera en cualquiera de los dos lugares.
- **Mostrar carpeta de copias de seguridad** abre la carpeta de copias de seguridad.

**Dónde se arma una copia de seguridad (desde 0.90.3).** Una copia se empaqueta y se comprueba en una carpeta propia del programa, *~/.local/share/{{APP}}/backup-work* (en Windows *data\backup-work* en la carpeta del programa), y solo el archivo terminado se pone en la carpeta de copias de seguridad, de una sola vez y con su nombre definitivo: se mueve allí cuando ambas carpetas están en la misma unidad; si no, se copia directamente con su nombre definitivo y se compara con el original. Así, una carpeta de copias de seguridad vigilada por un programa de sincronización, o que está en una memoria USB o en una unidad de red, nunca contiene una copia a medio escribir ni un archivo temporal, y una copia interrumpida nunca parece una terminada. (Antes de 0.90.3 la copia se escribía dentro de la carpeta de copias de seguridad como *… .zip.part* y se renombraba allí.) La carpeta de trabajo está vacía entre copias, salvo una pequeña nota de la carpeta de copias de seguridad a la que sirve.

**Cuando la carpeta de copias de seguridad está fuera de alcance** (una unidad que no está conectada, una carpeta que no se puede crear ni escribir, falta de espacio), la copia se hace de todos modos y espera en la carpeta de trabajo. Un mensaje lo indica y nombra ambas carpetas, y la siguiente copia del proyecto pone las copias que esperan en su sitio antes de la suya. Esperan tantas como copias se conservan. **Hacer copia de seguridad en…** en una carpeta en la que no se puede escribir solo lo indica con el motivo; elige otra carpeta.

**Copias hechas por un programa de sincronización.** Solo los archivos con el nombre con que {{APP}} nombra sus copias de seguridad (*Title #K7T2M bak 2026-11-20 07-56-13.zip*, con «(2)» o «(incomplete)» donde corresponda) cuentan como copias de seguridad y la limpieza puede eliminarlos. Los demás archivos de la carpeta de copias de seguridad de un proyecto se dejan en paz y no cuentan: por ejemplo las copias que hace un programa de sincronización cuando tiene problemas con un archivo (*… -MyComputer-safeBackup-0001.zip*, *… (conflicted copy).zip*), o una copia que hiciste a mano. (Antes de 0.90.3, esas copias contaban como copias de seguridad de su fecha, y varias de ellas podían desplazar a las verdaderas copias más antiguas.) **Restaurar copia de seguridad…** todavía las enumera, de modo que puedes restaurar desde una copia; elimina tú mismo las copias que no necesites.

Los proyectos grandes (de más de 4 GB, por ejemplo con audio y vídeo) también se copian correctamente (desde la versión 0.20). Un archivo que no se puede leer (por ejemplo por sus permisos, o un archivo de una carpeta sincronizada que aún no está en este equipo) se omite; la copia se hace de todos modos, pero se llama *… (incomplete).zip*, un mensaje nombra los archivos que faltan (también para las copias automáticas al abrir o cerrar un proyecto, 0.65), y no se elimina por ello ninguna copia más antigua: el número de copias que se conservan cuenta solo las copias completas. Cuáles son las copias más antiguas lo indican la fecha y la hora de sus nombres, de modo que cambiar el nombre del proyecto no cambia cuáles se conservan (0.81; antes, un título nuevo que se ordenaba antes que el antiguo hacía que cada copia nueva contara como la más antigua, y se eliminaba de inmediato). Después de la limpieza, el programa comprueba que la copia nueva sigue allí. De las copias incompletas, se quedan las más recientes hasta el número que se conserva (0.81). Una carpeta del proyecto que es un enlace a una carpeta de otro lugar se copia con sus archivos (0.81); un enlace que no apunta a ninguna parte, o que vuelve al proyecto, hace que la copia sea incompleta. Si el programa termina por una señal (un apagado que no espera, un comando kill) mientras **Hacer una copia de seguridad al cerrar un proyecto** está activado, no hay tiempo para la copia; la próxima vez que se abra el proyecto, se hace la copia y la barra de estado lo indica (0.81). El proyecto se guarda antes de cada copia; si ese guardado falla (un disco lleno, una carpeta en la que no se puede escribir), no se hace ninguna copia y un mensaje dice por qué, de modo que una copia nunca carece de tus últimos cambios sin que lo sepas. La copia de seguridad de un proyecto en formato Scrivener contiene el *.scriv* y, junto a él, la copia de trabajo de {{APP}} (*… - working copy of {{APP}}*), con las palabras clave, los estilos, los objetos y otras cosas que solo {{APP}} conoce.

**Para restaurar una copia de seguridad a mano** (por ejemplo en otro equipo): abre la carpeta de copias de seguridad, descomprime el archivo ZIP (clic derecho > Extraer) y abre el proyecto que contiene con **Archivo > Abrir proyecto**.

---

## 16. Opciones

**Archivo > Opciones** (**Ctrl+,**) tiene arriba una fila de iconos para las secciones, a la izquierda una lista de páginas (o pestañas) y a la derecha las opciones. No cambia nada hasta que hagas clic en **Aceptar** o **Aplicar**. Abajo a la izquierda, **Gestionar** exporta a un archivo el perfil en uso e importa uno (**Exportar perfil...**, **Importar perfil...**, por ejemplo para llevar tus ajustes a otro equipo; un archivo antiguo de opciones (*.qboptions*) se puede importar como perfil; véase «Perfiles» más abajo), abre **Gestionar perfiles...**, guarda el aspecto actual como un archivo de tema (un archivo que ya existe solo se reemplaza cuando el nuevo se escribió por completo; si falla la escritura, el archivo antiguo se queda como estaba y el mensaje lo indica) y enumera los temas; **Valores predeterminados** devuelve todas las opciones a lo habitual (tus proyectos recientes, tus propios temas, disposiciones, barras de herramientas y atajos se conservan).

**Dónde se guarda cada opción.** Apunta a una opción para ver dónde se guarda su valor: su información emergente indica si el proyecto abierto la guarda o si la guarda el programa para todos los proyectos, y si ese lugar es **fijo** (un ajuste que solo puede guardar el programa, como el idioma) o **depende del ajuste** **Ajustes para** de arriba (y, en la página Apariencia, **Mostrar y cambiar el aspecto de**). (0.90.1 mostraba una P verde o una A azul delante de cada opción; 0.90.2 lo deja a la información emergente.) En una pantalla pequeña las páginas se desplazan, de modo que Aceptar, Cancelar y Aplicar siguen a la vista.

Desde 0.70, la ventana se abre para **Todos los proyectos** (los ajustes del programa); elige **Este proyecto** arriba para fijar algo solo para el proyecto abierto. Los ajustes propios de un proyecto pueden hacer el guardado más seguro, pero nunca menos seguro: cuenta el retardo de guardado automático más corto, una copia de seguridad que hace el programa sigue activa, se conserva el mayor número de copias y la salida automática es solo del programa. El último guardado llega como máximo 10 segundos después de un cambio, sea cual sea el retardo del guardado automático.

- **General**: **Inicio** (reabrir el último proyecto, los diálogos de archivo del escritorio, el **Idioma** del programa, desde el siguiente inicio: English, Deutsch, Español, Français, Português (Brasil), Italiano, Русский, Bahasa Indonesia, Türkçe, Polski o Nederlands (los nueve nuevos son traducciones automáticas, marcadas así en la lista hasta que un hablante las revise; el manual, la guía de inicio rápido y el proyecto de bienvenida siguen el idioma, en inglés donde un idioma no los tiene; el primer inicio toma el idioma del sistema cuando es uno de estos, y la página del primer inicio ofrece la misma lista; **Agrupar los dígitos de los números grandes**: recuentos de palabras, objetivos y estadísticas con un separador entre los millares, como escribe los números tu sistema (44,825 o 44.825), o 44825 si se desmarca); **Guardado** (intervalo de guardado automático, el número de proyectos recientes, instantáneas con cada Archivo > Guardar); **Información del autor** (tu nombre, dirección, teléfono y correo electrónico para las portadas, usados por los marcadores de posición `<$author>`, `<$street>`, `<$city>`, `<$state>`, `<$zip>`, `<$country>`, `<$phone>`, `<$email>`, `<$address>`); las opciones de las herramientas de escritura (**Bloc de notas rápidas**); **Plantillas compartidas** (otra carpeta de plantillas de proyecto, por ejemplo una sincronizada entre tus equipos; sus plantillas aparecen en Archivo > Nuevo proyecto); **Citas** (el programa que inicia **Formato > Bibliografía/citas...**, por ejemplo Zotero o JabRef); **Separadores** (lo que va entre los documentos unidos y entre las notas del bloc de notas rápidas añadidas a un documento: un solo retorno, una línea vacía, un salto de página, nada o un texto propio); **Salida automática** (guardar y cerrar tras un número de minutos sin entrada de teclado ni de ratón); **Avisos** (**Mostrar la consola de registro interna** muestra los mensajes del programa de esta sesión, **Restablecer todos los avisos** vuelve a hacer las preguntas respondidas con «No volver a preguntar»).
- **Edición**: **Opciones** (recargar el último zoom de los editores, el zoom de texto predeterminado de cada zona, centímetros o pulgadas y el paso de la regla, la tecla Insert para sobrescribir, un formato de fecha propio para Insertar > Fecha actual, lo que muestra el recuento en directo, anotaciones en línea sin color, comentarios y notas al pie que se detienen antes de la puntuación, un punto de inserción en bloque); **Formato** (el formato y los estilos de los proyectos nuevos); **Revisiones** (los cinco colores de Formato > Modo de revisión, de la paleta compartida).
- **Comportamientos**: **Modo de composición** (pantalla, ventana principal apartada, Esc); **Enlaces a documentos** (dónde se abren los enlaces a documentos, enlaces de retorno); **Doble clic** (un doble clic en el corcho crea una ficha); **Arrastrar y soltar** (documentos soltados sobre documentos de texto, el formato del texto arrastrado a otros programas); **Carpetas y archivos** (las carpetas se abren como corcho; un texto con subdocumentos se abre como una carpeta); **Reproducción** de audio y vídeo (sección 6); **Referencia rápida** (los paneles se reabren con su proyecto); **Tecla Return** (Return termina la edición de una sinopsis; Return crea una ficha o fila nueva); **Instantáneas** (un mensaje cuando se toman instantáneas).
- **Apariencia**: el tema y once partes de la ventana (Interfaz general, Archivador, Modo de composición, Corcho, Fichas, Inspector y notas, Editor principal, Esquema, Texto combinado, Barras de progreso de objetivos, Marcas de texto), cada una con sus opciones, fuentes y colores, y **Colores estándar** (desde 0.95: las seis etiquetas con las que empiezan los proyectos nuevos, los seis colores de la paleta del color de texto y de la del resaltado, y los colores con los que empiezan los dos botones; **Valores predeterminados** devuelve los estándar del programa; los proyectos existentes conservan sus etiquetas). Véase «Temas, colores y fuentes» más abajo. **Interfaz general** también tiene la ruta completa del proyecto en la barra de título y el tamaño de los iconos de las barras de herramientas.
- **Correcciones**: correcciones al escribir (sección 4) y **Ortografía** (el diccionario, palabras con números, el subrayado, la lista personal de palabras).
- **Teclado**: el atajo de cada comando (sección 19).
- **Compartir**: **Importar** (comentarios, notas al pie, estilos, la codificación del texto sin formato, archivos HTML; sección 14), **Exportar** (el formato de archivo y las notas de Archivo > Exportar > Archivos), **Conversión** (cómo se leen los archivos *.doc*).
- **Copia de seguridad**: cuándo hacer copias (al abrir, al cerrar, con cada Archivo > Guardar), cuántas conservar (una instalación nueva empieza con las 25 más recientes; el campo contiene solo el número, las palabras lo rodean: «Conservar las [25] copias de seguridad más recientes de cada proyecto») y dónde.
- **Extras**, para las adiciones propias de {{APP}}: **Mapa mental** (las opciones de los tableros, la forma y el aspecto de las notas nuevas, el fondo de los tableros sin un color propio en los temas claros y oscuros, el tamaño de los iconos en notas y líneas), **Objetos** (si los proyectos nuevos reciben tus propias plantillas de objetos o las habituales), **Complementos y plugins** (los formatos de compilación y las herramientas de escritura instalados, las carpetas de plugins), **Informes de fallos** (tras cuántos segundos sin respuesta se escribe un informe de bloqueo, cuántos informes se conservan).
- **Voz** (con el complemento Transcripción de voz): el modelo, el dispositivo, los idiomas, el micrófono y las pausas (véase la sección 10, *Transcripción de voz*). Las herramientas de escritura de complementos posteriores también pueden traer una sección propia.

Otras ventanas del programa pueden abrir las opciones en una página, por ejemplo la página Reproducción desde el reproductor multimedia.

Las actualizaciones y los complementos tienen sus propias ventanas: **Ayuda > Complementos y actualizaciones > Instalar actualización o complemento…** y **Ayuda > Complementos y actualizaciones > Gestor de complementos…** (sección 17).

Los ajustes específicos de un proyecto (etiquetas, estado, tipos de sección, el aspecto del texto, metadatos personalizados, carpetas especiales, título y autor para compilar) están en **Proyecto > Ajustes del proyecto**.

### Ajustes para un solo proyecto

Arriba en **Archivo > Opciones**, **Ajustes para:** elige dónde se guarda lo que cambias:

- **Todos los proyectos (el programa)**: los ajustes del programa, para todos los proyectos. Aquí es donde se abre la ventana.
- **Este proyecto**: el proyecto abierto guarda lo que cambias aquí como un ajuste propio (en *settings/options.ini* de su carpeta). Se aplica mientras ese proyecto está abierto; lo que el proyecto no fija viene de los ajustes del programa. El título de la ventana dice «Opciones: este proyecto» y la línea bajo la elección indica cuántos ajustes propios tiene el proyecto (haz clic en el número para verlos). **Usar los ajustes del programa** los quita todos; los ajustes del programa vuelven a aplicarse al proyecto. Un cambio hecho fuera de las opciones, por ejemplo un atajo cambiado en la ventana del teclado o una barra de herramientas organizada, va al proyecto cuando el proyecto ya tiene un valor propio para él, y al programa en caso contrario.

Unos pocos ajustes pertenecen solo al programa y no se muestran para un proyecto: los proyectos recientes, reabrir el último proyecto, los diálogos de archivo del escritorio, los lugares de las ventanas y las disposiciones, los tiempos de los informes de fallos y el aspecto de los proyectos nuevos. El aspecto de un proyecto siempre ha sido propio: la página **Apariencia** muestra el aspecto del proyecto abierto o el aspecto de los proyectos nuevos (**Mostrar y cambiar el aspecto de** en su parte superior); con **Este proyecto** elegido arriba, muestra solo el aspecto del proyecto. **Valores predeterminados** y **Gestionar > Importar perfil…** actúan solo sobre los ajustes del programa, de modo que elige **Todos los proyectos** para ellos.

**Qué ajustes van primero** (**General > Inicio**, para todos los proyectos): si está marcado (lo habitual), van primero los ajustes propios de un proyecto y el programa rellena el resto. Si está desmarcado, siempre se aplican los ajustes del programa: los ajustes propios de los proyectos no se usan (se quedan en los proyectos para más adelante) y cada proyecto muestra el aspecto de los proyectos nuevos.

**Proyecto > Guardar los ajustes del proyecto en un archivo…** incluye las opciones propias del proyecto (la sección **Opciones de este proyecto**), de modo que se pueden llevar a otro proyecto.

### Temas, colores y fuentes

Un tema fija los colores y, si lo quiere, las fuentes de cada zona de la ventana. Elige uno en **Ver > Temas**, o con el botón de tema de la barra de herramientas Temas (activa la barra en **Ver > Barras de herramientas**); el cambio surte efecto de inmediato, sin reiniciar. Hay nueve temas integrados:

| Tema | Aspecto |
|-------|------|
| Tema oscuro de Shelin | ventana y archivador oscuros, un papel cálido con texto negro, Cinzel para los títulos del texto combinado (desde 0.95; está al principio de la lista y se puede eliminar en Gestionar temas; entonces no vuelve tras las actualizaciones) |
| Predeterminado (seguir el escritorio) | los colores de tu escritorio (el esquema de colores de KDE) |
| Claro | ventana gris claro, papel blanco |
| Modo oscuro | gris oscuro, con fichas oscuras |
| Sepia | crema cálido y marrón |
| Pizarra | ventana gris, papel blanco sobre una mesa gris |
| Medianoche | azul oscuro con un acento ámbar |
| Océano | azul verdoso claro |
| Alto contraste | blanco y amarillo sobre negro |

Los complementos pueden traer más temas (el complemento «Temas adicionales» tiene Noche del norte, Bosque, Grafito, Papel cálido y Amanecer rosa).

**El botón de tema** (0.90.2, cambiado en 0.91 y 0.94). Un solo botón, **Tema**, en la barra de herramientas Temas, que al principio está desactivada (**Ver > Barras de herramientas > Temas** la muestra; antes de 0.94, cuatro botones y una lista ocupaban un tercio de la barra de herramientas principal). Su menú enumera:

- **Claro** y **Oscuro**: un clic muestra ese tema de inmediato, tal cual.
- **Mi aspecto 1** y **Mi aspecto 2** guardan dos aspectos propios. Al elegir uno vacío (el menú dice *vacío*), se guarda en él el aspecto que ves ahora (el tema con cada color y fuente que cambiaste encima); al elegir uno lleno, se recupera su aspecto. **Guardar el aspecto actual como Mi aspecto 1** y **...como Mi aspecto 2**, al final del menú, guardan allí el aspecto que ves ahora (preguntan antes de reemplazar un aspecto). Los dos aspectos son temas normales: están en la lista de abajo y en **Ver > Temas**, donde también se pueden exportar o eliminar (uno eliminado deja su entrada vacía).
- **Todos los temas**, debajo, con el que está en uso marcado. Elegir uno hace lo que hace **Ver > Temas**: el proyecto abierto y los proyectos nuevos lo usan, y la pregunta de si conservar tus propios cambios encima llega como antes.
- **Tu propio aspecto nunca se pierde por una elección.** El menú muestra un tema tal cual, de modo que lo que ves cambia de verdad. Si hay colores o fuentes propios encima del tema mostrado, se perderían con eso, así que se guardan antes: en un Mi aspecto vacío sin preguntar (la barra de estado indica dónde) y, cuando ambos contienen un aspecto, {{APP}} pregunta si guardar en Mi aspecto 1, en Mi aspecto 2 o cambiar sin guardar.

Como todo control de barra de herramientas, el botón de tema se puede mover o quitar en **Ver > Barras de herramientas > Personalizar las barras de herramientas**, y toda la barra Temas se puede activar o desactivar en **Ver > Barras de herramientas**.

**La página en los temas oscuros** (0.70). La página habitual de {{APP}} es un papel verde oliva claro junto a una mesa gris. Con un tema oscuro, la página es en cambio de un gris muy oscuro con texto blanco (o la propia página oscura del tema, si la tiene), de modo que un tema oscuro no deja una página brillante en un marco oscuro. Un color de página que elegiste tú mismo (clic derecho > Apariencia, o Archivo > Opciones > Apariencia) se queda en todos los temas.

**Cambiar colores y fuentes.** **Archivo > Opciones > Apariencia** (o **Ver > Temas > Colores y fuentes…**) enumera a la izquierda las partes de la ventana. Cada una muestra sus opciones, sus fuentes y sus colores (un clic en un color muestra la paleta compartida: los colores de etiqueta, los seis colores estándar y **Otro...**):

| Parte | Fuentes | Colores |
|------|-------|--------|
| Interfaz general | menús, botones y paneles | ventana, Fondo del inspector (las listas y campos de texto del inspector), filas alternas, texto, botones, selección, enlaces, información emergente, bordes, texto atenuado |
| Archivador | archivador | fondo, títulos |
| Modo de composición | (ninguna) | fondo, papel, texto |
| Corcho | (ninguna) | corcho, sello de estado |
| Fichas | títulos de las fichas, texto de las fichas | ficha, título, texto, línea bajo el título, líneas de pauta |
| Inspector y notas | sinopsis | Franjas del inspector (la fila de pestañas, los títulos, la fila Etiqueta y Estado), etiquetas, ficha y texto de la sinopsis, notas y su texto |
| Editor principal | título de la barra de encabezado | papel, texto, el espacio junto a la columna de texto, la mesa alrededor de las páginas (solo se muestra en la vista de página), la barra de encabezado (normal, editor activo, fijado en su lugar) |
| Esquema | esquema | fondo, filas alternas, texto |
| Texto combinado | títulos en el texto combinado | (ninguno) |
| Barras de progreso de objetivos | (ninguna) | sesión, borrador, objetivo alcanzado |
| Marcas de texto | (ninguna) | enlaces en el texto, resultados de búsqueda, subrayado de ortografía, las marcas de Mostrar invisibles |

**Cada proyecto conserva su propio aspecto.** El tema del archivador, el editor, el corcho, el esquema, el inspector y el modo de composición, tus propios colores y fuentes para ellos, el ancho de la columna de texto y el borde junto al texto pertenecen al proyecto: cambiarlos en un proyecto deja los demás como están, y una actualización de {{APP}} nunca cambia el aspecto de un proyecto existente. La lista **Mostrar y cambiar el aspecto de** en la parte superior de **Archivo > Opciones > Apariencia** elige de quién muestra y cambia el aspecto la página: **Este proyecto** (el abierto; se elige primero) o **Proyectos nuevos** (el aspecto con el que empieza un proyecto nuevo). Alternar entre los dos conserva lo que cambiaste en cada uno hasta que pulses **Aplicar** o **Aceptar**. Los colores de los menús y paneles (**Interfaz general**) y la fuente de los menús, botones y paneles son los mismos en todos los proyectos. Un proyecto abierto por primera vez con la versión 0.40.2 o posterior conserva el aspecto que tenía hasta entonces. **Ver > Temas** fija a la vez el tema del proyecto abierto y de los proyectos nuevos.

**Menús del clic derecho.** Dondequiera que hagas clic derecho, el menú tiene un submenú **Apariencia** con los colores y las fuentes de lo que hay bajo el ratón: en el texto y en el papel junto a él, el papel, el texto, el espacio junto a la columna, los enlaces, los resultados de búsqueda, el subrayado de ortografía, el ancho de la columna y el borde; en la barra de encabezado sobre un editor, sus colores y su fuente; en el archivador, su fondo, títulos, fuente, sangría y colores de etiqueta; en el corcho, el corcho, las fichas y sus fuentes, los sellos de estado y el ancho de las fichas; en el esquema, su fondo, filas, texto y fuente; en el inspector y sus notas, las franjas, etiquetas, ficha de la sinopsis y notas, y el Fondo del inspector, que es el mismo en todos los proyectos; en el modo de composición, su fondo, papel y texto. Un color muestra la paleta compartida y **El color del tema**; **Opciones de apariencia...** abre esta página de las opciones. Como las opciones, estos menús cambian el aspecto del proyecto abierto (la primera línea del submenú lo indica) o, sin ningún proyecto, el aspecto de los proyectos nuevos.

Haz clic en un color para cambiarlo, en un botón de fuente para elegir una fuente. La pequeña imagen de arriba muestra el resultado antes de que pulses **Aplicar** o **Aceptar**. Tus cambios se asientan sobre el tema: una entrada cambiada se muestra en negrita, el botón **↺** a su lado se la devuelve al tema y **Restablecer colores y fuentes al tema** lo hace con todas. Un color que no has cambiado y que el tema no fija sigue a los demás; por ejemplo, el archivador usa el color de las listas hasta que le das uno propio. El **Fondo del inspector** colorea solo las listas y campos de texto del inspector; los campos de las barras de herramientas, la regla y las listas de las opciones tienen el color de la ventana y los menús.

La fuente y el tamaño del texto que escribes no forman parte del tema: pertenecen al proyecto (**Proyecto > Ajustes del proyecto > Formato**, sección 4).

**Temas propios.** **Guardar como tema…** en las opciones (o **Ver > Temas > Guardar el aspecto actual como tema…**) guarda el tema junto con tus cambios con un nombre nuevo. Aparece entonces en **Ver > Temas** como los demás. **Ver > Temas > Gestionar temas…** enumera todos los temas: **Usar este tema**, **Duplicar…** (también para los temas integrados, para obtener un punto de partida propio), **Cambiar nombre…** y **Eliminar** (solo tus propios temas), **Exportar…** (escribe un archivo *.qbtheme* que puedes compartir) e **Importar…** (añade un archivo así).

Cuando eliges otro tema teniendo cambios propios, {{APP}} pregunta si conservarlos con el nuevo tema o usar el tema tal cual.

Los enlaces de tu texto se guardan con un solo azul; sobre papel oscuro se muestran en un azul más claro (**Editor principal > Enlaces en el texto**). El texto al que tú diste color conserva su color en todos los temas.

### Perfiles

Un tema es solo un aspecto: colores y fuentes. Un **perfil** (desde 0.95) conserva todo lo demás de los ajustes del programa: el idioma, todas las opciones, las barras de herramientas y sus posiciones, el tamaño y la posición de la ventana, los paneles mostrados, los divisores, el conjunto de teclado y tus atajos, las copias de seguridad y su carpeta, las búsquedas de actualizaciones, los colores estándar, las disposiciones guardadas de **Ver > Disposición**, y nombra su tema, de modo que cambiar de perfil cambia también el aspecto. Tu nombre y dirección, el nombre de este equipo, los proyectos recientes y los lugares del proyecto de bienvenida no forman parte de un perfil.

- **Cambiar.** **Ver > Perfiles** enumera los perfiles; un clic cambia de inmediato, sin reiniciar (el idioma es lo único que necesita un reinicio; el programa lo avisa). El perfil que dejas se guarda antes. La barra de herramientas **Perfiles** (**Ver > Barras de herramientas > Perfiles**, desactivada al principio) muestra el perfil en uso; un clic en su botón enumera los perfiles. Los ajustes propios de un proyecto (**Ajustes para: Este proyecto**) siguen aplicándose por encima del perfil. El proyecto abierto muestra el aspecto del perfil nuevo, como cuando eliges un tema en **Ver > Temas**.
- **Predeterminado.** Un programa que ya se había usado toma sus ajustes como el perfil **Predeterminado** al actualizarse a 0.95, de modo que no se pierde nada. Una instalación nueva empieza con los ajustes estándar del programa, en el tema claro o el oscuro según esté configurado tu sistema, cuando el sistema lo indica. La página del primer inicio rellena **Predeterminado**. **Predeterminado** no se puede eliminar.
- **Un perfil nuevo** a partir de los ajustes tal como están ahora: **Ver > Perfiles > Guardar los ajustes como perfil nuevo…** (o **Gestionar perfiles… > Guardar los ajustes como perfil nuevo…**). El perfil nuevo pasa a ser el que está en uso. **Gestionar perfiles…** también renombra, copia y elimina perfiles (el perfil en uso no se puede eliminar; cambia antes).
- **Shelin** es un perfil que viene con el programa. Está al principio de la lista, establece Sitka para el texto nuevo y su propio tema, y se puede eliminar; entonces no vuelve tras las actualizaciones. Las fuentes que nombra y que no están instaladas (Cinzel, Sitka) se sustituyen por otras. Su elección del motor de reproducción necesita el complemento Formatos adicionales de vídeo y audio; las elecciones de modelos de las herramientas de escritura se aplican solo donde esos complementos y modelos están instalados.
- **Tamaños en otra pantalla.** Los anchos del archivador, el editor y el inspector se guardan como proporciones de la ventana; un perfil creado en una pantalla grande muestra las mismas proporciones en una pantalla más pequeña o en una tableta (las columnas laterales conservan un ancho legible), y la ventana se lleva dentro de la pantalla.
- **Exportar e importar.** **Gestionar perfiles… > Exportar…** (o **Archivo > Opciones > Gestionar > Exportar perfil…**) escribe el perfil en un solo archivo (*.qbprofile*) con sus disposiciones y, si es uno de los tuyos, su tema. **Importar…** añade ese archivo como perfil nuevo (no se cambia a él). Un archivo puede nombrar programas, carpetas, archivos o direcciones; como con otros archivos que vienen de otra persona, se te pregunta antes de adoptarlos (una carpeta que no existe en este equipo no serviría de nada). Un archivo antiguo de opciones (*.qboptions*) también se puede importar como perfil.

**Proyecto > Guardar los ajustes del proyecto en un archivo…** se queda como estaba.

---

## 17. Actualizaciones y complementos

**Ayuda > Solución de problemas y desarrollo > Guía del desarrollador: actualizaciones y complementos** explica, para quienes compilan {{APP}} o escriben complementos, cómo funcionan las actualizaciones y los complementos y qué herramientas los crean; también contiene lo que necesita un amigo con inclinación técnica para configurar las cosas (motores multimedia, plugins de formatos de compilación, archivos de paquete, de tema, de disposición y de formato de guion, los archivos dentro de la carpeta de un proyecto) y las variables de entorno que lee el programa.

{{APP}} es un único archivo de programa (el AppImage). Las actualizaciones y las extensiones llegan como pequeños archivos de paquete que terminan en **.qbpkg**. Hay tres tipos, y todos se instalan de la misma manera:

- **Actualización del programa**: una versión más reciente del propio {{APP}}.
- **Complemento**: una extensión que puedes activar y desactivar, como temas de color, plantillas de proyecto, formatos de compilación, diccionarios, herramientas de escritura (sección 10), la reproducción de audio y vídeo y sus formatos adicionales (sección 6), importadores de archivos antiguos de Word y de páginas web (sección 14).

Las actualizaciones del programa y los complementos están en la página **Releases** de la página del proyecto (sección 1), junto al AppImage.
- **Paquete de actualización**: un archivo que contiene una actualización del programa y varios complementos, que se instalan juntos.

### Buscar actualizaciones

Desde 0.92.0, {{APP}} puede buscar actualizaciones por sí mismo. Es lo único para lo que el programa contacta con internet por su cuenta, y puedes desactivarlo.

- **Ayuda > Complementos y actualizaciones > Buscar actualizaciones al iniciar** es el interruptor (activado al principio). **Ayuda > Complementos y actualizaciones > Buscar actualizaciones ahora** busca de inmediato, diga lo que diga el interruptor.
- **El primer inicio** lo explica en su página, antes de contactar con nada: una casilla *Buscar actualizaciones al iniciar el programa* (marcada) con la nota de que no se envía nada sobre ti; un **Aceptar** termina la página. Un programa que se inició antes de 0.94.0 y no ha visto la nota la muestra como un cuadro con **Aceptar** y **Desactivar**.
- **Qué ocurre**: unos segundos después del inicio, como máximo una vez al día, el programa obtiene un pequeño archivo de la página del proyecto (*shelin-boop.github.io/Quillbinder_Collection/updates-1.json*) y la firma de ese archivo. La petición nombra el programa («{{APP}}») y nada más: ni versión, ni sistema, ni identificador, ni cookie. Como en toda conexión, el servidor (GitHub) ve tu dirección de internet y la hora. El archivo enumera la versión más reciente del programa y de los complementos del proyecto; tu programa lo compara con lo instalado, en tu equipo. El archivo solo cuenta cuando lleva la firma de la clave de publicación del proyecto, y un archivo más antiguo que uno ya visto se ignora.
- **Cuando hay una actualización**, una ventana la nombra: el programa o el complemento, tu versión y la nueva, qué hay de nuevo y su tipo. Una actualización **funcional** tiene tres botones: **Descargar e instalar**, **Ignorar esta versión** (no más recordatorios para esa versión) y **Posponer**. Posponer espera más cada vez: 1 día, 1 semana, 2 semanas, 4 semanas; después hay un último recordatorio y tras él terminan los recordatorios de esa versión. Una actualización **recomendada (de seguridad)** tiene solo dos botones, **Descargar e instalar** y **Posponer**: no se puede ignorar y, tras el cuarto aplazamiento, vuelve cada 4 semanas. Cuando hay ambos tipos, una ventana los enumera y sigue las reglas de seguridad. La ventana aparece junto a tu trabajo y no toma el teclado.
- **Descargar e instalar** obtiene los archivos que nombra la lista, comprueba cada uno con el tamaño y la suma de verificación de la lista firmada y lo instala: un complemento o una actualización del programa como lo hace **Ayuda > Complementos y actualizaciones > Instalar actualización o complemento** (solo paquetes con la firma del proyecto). Después ofrece volver a iniciar el programa.
- **Cuando la nueva versión necesita un AppImage nuevo** (otras bibliotecas que las de tu AppImage), {{APP}} lo descarga, lo pone en el lugar del AppImage que iniciaste, esté donde esté y con el mismo nombre de archivo, conserva el que tenías a su lado como *…AppImage.before-update* y se inicia de nuevo. Elimina el archivo conservado cuando la nueva versión te funcione. Si no se puede escribir en la carpeta de tu AppImage, el archivo nuevo va a tu carpeta Descargas y un mensaje dice qué hacer.
- **En Windows** (0.92.0), una versión así llega como su programa de instalación. {{APP}} lo descarga en su propia carpeta (*data\update-setup*), lo comprueba con la lista firmada del proyecto y pregunta. Con **Cerrar y actualizar ahora**, {{APP}} se cierra y el programa de instalación descomprime la nueva versión en la carpeta que usas, sin sus páginas, y la inicia; tus ajustes, complementos y todo lo demás de *data* se quedan. Con **Más tarde** no cambia nada y la actualización se vuelve a ofrecer. Si Windows no inicia el programa de instalación (no lleva ningún certificado de firma de código), un mensaje nombra el archivo, y también puedes descargar la versión actual desde la página del proyecto.
- **Cuando no se puede acceder a la dirección**, el programa lo intenta dos veces, con 5 segundos de diferencia, y después muestra un pequeño texto en la barra del pie del primer editor, en el lugar del texto de guardado, que aparece y se desvanece dos veces. Ninguna ventana, y como máximo una vez al día. (**Buscar actualizaciones ahora** responde en cambio con un mensaje.)
- **Desactivado**, el programa no contacta con nada, como antes de 0.92.0. Para equipos configurados para otras personas, la variable de entorno `QB_NO_UPDATE_CHECK=1` impide que se ejecute la búsqueda automática diga lo que diga el interruptor.
- Un AppImage de una versión anterior a 0.92.0 que ejecuta una actualización del programa más reciente no puede hacer conexiones seguras; en ese caso la búsqueda lo indica cuando se lo pides y calla en caso contrario. Descarga una vez el AppImage actual.
- Lo que el programa recuerda de esto (el interruptor, la hora de la última búsqueda, lo que ignoraste o pospusiste) está en *updates-state.json* en la carpeta de datos del programa.

### Instalar un paquete

1. Elige **Ayuda > Complementos y actualizaciones > Instalar actualización o complemento…** y selecciona el archivo .qbpkg, o haz doble clic en el archivo en tu gestor de archivos.
2. {{APP}} muestra lo que contiene el paquete y qué hay de nuevo. Haz clic en **Instalar**.
3. Si el paquete contiene una actualización del programa, {{APP}} pide reiniciar. Tu proyecto abierto se guarda y se vuelve a abrir.

{{APP}} comprueba cada paquete antes de instalarlo: los archivos dañados y los paquetes hechos para otra versión se rechazan. Una actualización del programa se inicia una vez de forma invisible como prueba; si no se ejecuta en tu equipo, no se instala. Las partes de un paquete de actualización se instalan juntas: cuando una de ellas no se puede instalar, no se instala ninguna (0.65).

**Firmas** (0.65). Cada paquete de la página de releases del proyecto está firmado con la clave del proyecto, y {{APP}} lleva la mitad pública de esa clave. La ventana de instalación dice **Firmado por el proyecto Quillbinder** para un paquete así: es el paquete que publicó el proyecto, sin cambios.

- Un paquete cuya firma no coincide (cambiado después de firmarse, o dañado) nunca se instala.
- Una **actualización del programa** solo se instala con la firma del proyecto.
- Un **complemento** sin ella (uno que hiciste tú u otra persona) solo se instala tras una advertencia clara, con **Cancelar** como opción predeterminada. En la línea de comandos (`--install <file>`), un paquete sin la firma del proyecto solo se instala cuando añades `--allow-unsigned`.

**Claves de complemento** (0.91.3). Los complementos oficiales, también los escritos por otros autores, pueden firmarse con una clave propia en lugar de la clave del proyecto. El proyecto avala esa clave con la suya propia, en un certificado que viene dentro del paquete y que permite a la clave firmar complementos y nada más. La ventana de instalación nombra entonces la clave: *Firmado con la clave de complemento «Quillbinder official add-ons», que el proyecto Quillbinder avala*.

- Una **actualización del programa** firmada con una clave de complemento se rechaza como una sin firmar: las actualizaciones del programa necesitan la clave propia del proyecto.
- Una clave de complemento que el proyecto no avala cuenta como ninguna firma: el complemento solo se instala tras la advertencia.
- Los complementos firmados con la clave propia del proyecto, como lo estaban todos hasta 0.91.2, siguen siendo de confianza. Desde 0.91.3, los complementos propios del proyecto se firman también con la clave de complemento.
- Las versiones hasta 0.91.2 no conocen las claves de complemento y advierten antes de instalar un complemento firmado con una: instala primero la actualización del programa (está firmada con la clave propia del proyecto) y después los complementos.

**La ventana que pregunta antes de instalar un complemento** (0.94.0) dice primero, en una o dos frases, qué hace el complemento. Las líneas técnicas (las bibliotecas que trae, el motor de voz o de gráficos, el entorno de ejecución para el que se compiló) están en **Detalles**, que está plegado hasta que lo abres.

**Comprobar tú mismo una descarga.** Cada release contiene también *SHA256SUMS.txt* (la suma de verificación de cada archivo), su firma *SHA256SUMS.txt.sig* y la clave pública *quillbinder-signing-public.pem*. En una terminal, en la carpeta con las descargas:

    openssl pkeyutl -verify -pubin -inkey quillbinder-signing-public.pem -rawin -in SHA256SUMS.txt -sigfile SHA256SUMS.txt.sig
    sha256sum -c --ignore-missing SHA256SUMS.txt

El primer comando responde *Signature Verified Successfully*, el segundo *OK* para cada archivo que descargaste. Para estar seguro de que el propio archivo de clave es el del proyecto, compara sus 32 bytes (`openssl pkey -pubin -in quillbinder-signing-public.pem -outform DER | tail -c 32 | od -An -tx1`) con la clave del código fuente del programa (*src/core/Signing.cpp* y *packaging/quillbinder-signing-public.pem*): empiezan por *bc 3c 58 00 08 3f 66 e6*.

### El Gestor de complementos

**Ayuda > Complementos y actualizaciones > Gestor de complementos…** muestra lo que está instalado.

- **Versión del programa**: la versión integrada en tu AppImage y cada actualización instalada. La marcada como *en ejecución ahora* es la que se usa. Para volver a una versión anterior, selecciónala y haz clic en **Usar esta versión**, y luego reinicia. Las actualizaciones instaladas que ya no necesites se pueden quitar.
- **Complementos**: desmarca un complemento para desactivarlo sin quitarlo; **Quitar complemento** lo elimina. Los temas, las plantillas y los formatos de compilación de un complemento aparecen en los lugares habituales: **Ver > Temas**, la ventana **Nuevo proyecto** y la ventana **Compilar**.
- Cuando una elección hecha aquí no se puede escribir (un disco lleno, una carpeta en la que no se puede escribir), un mensaje lo indica con el nombre del archivo (0.92.0). Antes, la ventana mostraba la elección como hecha y se deshacía en el siguiente inicio sin decir nada. Lo mismo vale para una instalación: un complemento o una versión del programa cuya entrada no se pudo escribir se notifica como no instalado, con lo que hay que hacer. Cuando una instalación falla a medias y no se puede restituir la versión instalada antes, el mensaje nombra la carpeta en la que está y el nombre que hay que dar a esa carpeta.

### Si una actualización no se inicia

Si una actualización del programa no se inicia dos veces seguidas, {{APP}} la desactiva por sí mismo e inicia la versión integrada en el AppImage, con un mensaje que te lo indica. Nada de tus proyectos se ve afectado por esto.

### Cuando hace falta un AppImage nuevo

Las actualizaciones del programa se ejecutan con las bibliotecas que hay dentro de tu AppImage. De vez en cuando una actualización mayor necesita un AppImage nuevo; la ventana de instalación te dice cuándo un paquete no encaja con el tuyo. Reemplaza el archivo AppImage antiguo por el nuevo; tus ajustes, complementos y proyectos se quedan como están.

0.80.0 es una de esas versiones: su AppImage lleva un Qt nuevo (la compilación propia de Qt 6.12 de The Qt Company en lugar del Qt 6.4.2 de Ubuntu). Los complementos con código de programa (*Media Playback*, *Extra Video and Audio Formats*, *Speech Transcription*, las herramientas de escritura y los importadores) deben instalarse de nuevo en sus versiones para 0.80.0 (el paquete de actualización y **media-all-formats.qbpkg** de la página Releases los traen todos); hasta entonces, el Gestor de complementos marca los antiguos como compilados para un programa más antiguo y su código no se carga. La versión completa de 0.80.0 trae todos los complementos.

0.93.0 es otra (Linux): su AppImage lleva Qt 6.12.1, cuyas bibliotecas integradas (zlib, HarfBuzz, PCRE2, libpng, libjpeg-turbo, FreeType y otras) son más nuevas y tienen correcciones de seguridad. Una actualización del programa no puede traerlas, de modo que la búsqueda de actualizaciones ofrece el nuevo AppImage. Los complementos con código de programa del AppImage anterior (los enumerados arriba) se instalan de nuevo en sus versiones para 0.93.0 de la misma manera, y los complementos multimedia llevan ahora FFmpeg 9.1.

Un AppImage nuevo trae también una versión propia del programa (por ejemplo el AppImage 0.20.0). Inicia esa versión, aunque hayas instalado actualizaciones en el AppImage antiguo: las actualizaciones más antiguas se desactivan y siguen enumeradas en **Ayuda > Complementos y actualizaciones > Gestor de complementos…**, donde puedes quitarlas. Si el archivo nuevo tiene otro nombre o está en otra carpeta que el antiguo, elige una vez **Ayuda > Añadir al menú de aplicaciones**, para que la entrada del menú y los proyectos con doble clic usen el archivo nuevo.

---

## 18. El dialecto de Markdown

No necesitas saber esto para usar {{APP}}; el editor muestra texto con formato. Solo importa si abres los archivos de texto con otro programa. {{APP}} escribe Markdown con estas reglas:

- **Cada línea es un párrafo**, como en un procesador de textos. Una línea vacía es un párrafo vacío.
- Títulos: `# ` a `###### ` al principio de la línea.
- Cita en bloque: `> `. Párrafo centrado: `->text<-`. Cualquier otro estilo de párrafo: `::Style Name:: text`.
- Estilos de carácter (desde la versión 0.4): `<span class="Style Name">text</span>`. En el texto solo se escriben los nombres de los estilos. El aspecto de un estilo se guarda una sola vez, en el archivo del proyecto, de modo que un estilo cambiado no requiere ningún cambio en los documentos.
- Listas: `- ` y `1. `; dos espacios por nivel de anidamiento.
- `**bold**`, `*italic*`, `~~strikethrough~~`, `==highlight==`, `<u>underline</u>`, `<sup>superscript</sup>`, `<sub>subscript</sub>`.
- Colores de revisión: `<rev1>…</rev1>` a `<rev5>…</rev5>`.
- Enlaces: `[text](https://example.com)`; enlaces a documentos del proyecto: `[text](qb://ID)`.
- Imágenes: `![](images/picture.png){width=300}` (las imágenes se guardan en la carpeta del documento). La descripción de una imagen (0.62) es el texto entre corchetes: `![A brass lamp](images/lamp.png){width=300}`.
- Tablas (0.62): una línea `:::table` con el diseño entre llaves (JSON), una línea por fila con las celdas entre `|` (los párrafos de una celda separados por `<p/>`, una celda cubierta por una celda combinada se deja vacía, un `|` en el texto se escribe `\|`) y una línea `:::` al final.
- Una imagen junto al texto o centrada (0.62): `:::figure {"pos":1,"width":168}` (1 izquierda, 2 derecha, 3 centrada), la línea de la imagen, las líneas de la leyenda y `:::`.
- Un salto de línea dentro de un párrafo: `<br>`.
- Fuentes aplicadas a mano (desde la versión 0.3): `<span style="font-family:'DejaVu Sans'; font-size:14pt; color:#aa0000">text</span>`. Las declaraciones son CSS normal, de modo que otros programas de Markdown que admiten HTML también las muestran. Entradas posibles: `font-family`, `font-size` (pt), `font-weight` (100 a 900), `color`, `letter-spacing` (em), `font-variant:small-caps`, `text-transform:uppercase`.
- Formato de párrafo aplicado a mano (desde la versión 0.3): una lista entre llaves al principio de la línea, antes de cualquier otro prefijo: `{: align=right left=10mm right=5mm first=0mm before=6pt after=12pt line=1.5 tabs=20mm,50mmR} text`. `line` es un múltiplo (`1.5`), una altura exacta (`14pt`) o un mínimo (`14pt+`). Las tabulaciones terminan en `R` (derecha), `C` (centrada) o `D` (punto decimal); sin letra significa izquierda. Solo se escriben las entradas que fijaste.
- Los elementos de guion (desde la versión 0.10) son estilos de párrafo con el nombre del elemento: `::Script > Scene Heading:: INT. LIGHTHOUSE - NIGHT`. El aspecto de un elemento se guarda una sola vez, en el archivo del proyecto, con el formato de guion.
- Un salto de página antes de un párrafo (de guiones importados y archivos RTF, desde la versión 0.10): `{: page=before} text`.
- Las versiones anteriores a 0.62 muestran las tablas y las figuras como líneas de texto (`:::table` ...). Si vuelves a una versión más antigua, no edites documentos con tablas o imágenes junto al texto.
- La versión 0.2 muestra ambos marcadores como texto sin formato. Si vuelves a la versión 0.2 con el Gestor de complementos, no edites documentos que los contengan.
- Las versiones anteriores a 0.4 no conocen tus propios estilos: muestran `<span class=…>` como texto sin formato y tratan los párrafos de tus propios estilos como texto normal. Si vuelves a una versión más antigua, no edites documentos que usen tus propios estilos.
- Las versiones anteriores a 0.10 tratan los elementos de guion como texto normal y quitan los nombres de los elementos de un guion en cuanto se ha mostrado en el editor y se guarda el proyecto. Si vuelves a una versión más antigua, no abras allí tus guiones (véase la sección 20).
- Una barra invertida convierte el siguiente carácter en texto normal: `\*` es un asterisco, no cursiva.
- Las notas siguen la convención de CriticMarkup:
  - `{>>inline annotation<<}`
  - `^[inline footnote]`
  - `{==text==}{>>linked comment<<}`
  - `{==text==}^[linked footnote]`
  - Un comentario puede empezar con un color: `{>>@#ff8800 text<<}`.

**Archivo > Convertir archivo Markdown ⇄ RTF…** lee tanto este dialecto como el Markdown estándar (donde los párrafos se separan con líneas vacías) y, al escribir Markdown, pregunta cuál de los dos quieres.

---

## 19. Atajos de teclado

### Cambiar los atajos

**Archivo > Opciones > Teclado** (o **Ayuda > Atajos de teclado**) enumera cada comando, agrupado por menú, con su atajo y el atajo estándar con el que venía el programa. También se enumeran los estilos del proyecto abierto y las disposiciones, de modo que un estilo o una disposición pueden tener un atajo propio.

1. Busca el comando: desplázate, o escribe una parte de su nombre o un atajo como «Ctrl+K» en el campo de búsqueda.
2. Haz clic en el comando, haz clic en el campo de atajo bajo la lista y pulsa las teclas nuevas.
3. Si otro comando tiene ese atajo, se te pregunta si quieres quitárselo. Ese otro comando se queda entonces sin atajo hasta que le des uno nuevo.

**Sin atajo** quita el atajo del comando seleccionado, **Estándar** le devuelve el que traía y **Todo a estándar** deshace todos tus cambios. Tus cambios se muestran en negrita. Un atajo necesita Ctrl, Alt o Meta, o debe ser una tecla de función, porque las letras sueltas se necesitan para escribir. **Exportar** e **Importar** guardan tu conjunto de atajos como un archivo (*.qbkeys*) y cargan uno, por ejemplo para llevarlo a otro equipo.

Los atajos de los estilos pertenecen al nombre del estilo: «Cita en bloque» tiene su atajo en todos los proyectos que tienen un estilo con ese nombre.

### Atajos y disposición del teclado

Algunos de los atajos estándar son teclas que necesitan **AltGr** en un teclado alemán: **Ctrl+{**, **Ctrl+|** y **Ctrl+}** (alinear a la izquierda, centrar, alinear a la derecha), **Ctrl+[** y **Ctrl+]** (atrás y adelante), **Ctrl+\\** (la lista de elementos de guion) y **Ctrl+=** (subíndice). Desde 0.94.0, el programa mira la disposición del teclado del sistema **en el primerísimo inicio** (cuando no existen ajustes de atajos ni un inicio anterior) y conserva la respuesta. Con una **disposición estadounidense**, o cualquier disposición distinta de la alemana, se queda el conjunto habitual de arriba. Con una **disposición alemana (o austríaca)**, estos comandos reciben teclas que no necesitan AltGr:

| Comando | Conjunto alemán |
|---------|------------|
| Alinear a la izquierda / centrar / alinear a la derecha (también en el mapa mental) | Alt+Shift+L / Alt+Shift+E / Alt+Shift+R |
| Justificar (mapa mental) | Alt+Shift+J |
| Atrás / adelante | Alt+Shift+B / Alt+Shift+F |
| Lista de elementos de guion | Ctrl+Shift+O |
| Subíndice | Ctrl+Shift+D |
| Traer al frente / enviar al fondo (mapa mental) | Ctrl+Shift+PageUp / Ctrl+Shift+PageDown |

Los menús muestran cada tecla como la nombra la disposición. Un programa que se usó antes de 0.94.0 conserva sus atajos como estaban (nada cambia por sí solo), y también lo hace cada atajo que fijaste tú mismo en **Archivo > Opciones > Teclado**: un atajo guardado nunca se reemplaza. Donde el sistema no indica su disposición, se usa el conjunto habitual. (Linux: los ajustes de teclado del escritorio o del servidor X; Windows: el idioma de la disposición del teclado; Android: el idioma del teclado.)

### Los atajos estándar

| Acción | Atajo |
|--------|----------|
| Nuevo proyecto / abrir proyecto | Ctrl+Shift+N / Ctrl+O |
| Guardar / cerrar proyecto / salir | Ctrl+S / Ctrl+W / Ctrl+Q |
| Nuevo documento / nueva carpeta | Ctrl+N / Alt+Shift+N |
| Cambiar nombre / duplicar | F2 / Ctrl+D |
| Mover elemento hacia arriba / abajo / fuera / dentro | Ctrl+Alt+Up / Down / Left / Right |
| Dividir en la selección / con la selección como título | Ctrl+K / Ctrl+Shift+K |
| Unir documentos | Ctrl+Shift+M |
| Texto combinado / corcho / esquema | Ctrl+1 / Ctrl+2 / Ctrl+3 |
| Mostrar u ocultar el archivador / el inspector | Ctrl+Alt+B / Ctrl+Alt+I |
| Modo de composición / pantalla completa | F11 / Ctrl+Shift+F11 |
| Enfocar el archivador / enfocar el editor / otro editor | Alt+1 / Alt+2 / F6 |
| Atrás / adelante | Ctrl+[ / Ctrl+] (disposición alemana: Alt+Shift+B / Alt+Shift+F) |
| Documento anterior / siguiente | Alt+Shift+Up / Alt+Shift+Down |
| Mostrar en el archivador | Ctrl+Shift+R |
| Buscar / reemplazar / siguiente / anterior | Ctrl+F / Ctrl+H / F3 / Shift+F3 |
| Búsqueda en el proyecto | Ctrl+Shift+F |
| Usar la selección para buscar | Ctrl+E |
| Buscar por formato / siguiente / anterior | Ctrl+F3 / Ctrl+G / Ctrl+Shift+G |
| Bloc de notas rápidas (complemento) | Alt+Shift+Return; desde cualquier programa: véase la sección 10 |
| Audio y vídeo (complemento): reproducir / pausa | Ctrl+Return |
| Audio y vídeo: retroceder un poco / avanzar un poco | Ctrl+Alt+, / Ctrl+Alt+. |
| Audio y vídeo: insertar marca de tiempo | Ctrl+Shift+Return |
| Filtrar el corcho o el esquema | Ctrl+F (mientras uno de ellos tiene el teclado) |
| Enlazar con un documento | Ctrl+L |
| Comentario / nota al pie | Shift+F2 / Ctrl+Shift+F2 |
| Anotación en línea / nota al pie en línea | Ctrl+Shift+A / Ctrl+Alt+F |
| Tomar instantánea / con título | Ctrl+5 / Ctrl+Shift+5 |
| Objetivos del proyecto / palabras clave del proyecto | Ctrl+Shift+T / Ctrl+Alt+K |
| Importar archivos / compilar | Ctrl+Shift+I / Ctrl+Shift+E |
| Imprimir documento actual | Ctrl+P |
| Ventana de fuentes / ventana de párrafo | Ctrl+T / Ctrl+Alt+P |
| Tamaño de fuente más grande / más pequeño | Ctrl+> / Ctrl+< |
| Copiar formato / pegar formato | Ctrl+Alt+C / Ctrl+Alt+V |
| Pegar con formato / pegar y adaptar el estilo | Ctrl+Alt+Shift+V / Ctrl+Shift+V |
| Ningún estilo / Título 1 / Título 2 / Título 3 | Ctrl+Alt+0 / Ctrl+Alt+1 / Ctrl+Alt+2 / Ctrl+Alt+3 |
| Los seis estilos siguientes de la lista de estilos (Formato > Estilo > Personalizar la lista de estilos) | Ctrl+Alt+4 ... Ctrl+Alt+9 |
| Ir a la sinopsis / ir a las notas del documento (Navegar > Inspeccionar) | Ctrl+Alt+Y / Ctrl+Alt+N |
| Hacer copia de seguridad ahora | Ctrl+Shift+B |
| Modo guion / lista de elementos de guion | Ctrl+8 / Ctrl+\\ (disposición alemana: Ctrl+8 / Ctrl+Shift+O) |
| En un guion: elemento siguiente / cambiar elemento | Enter / Tab (véase la sección 4, Escritura de guiones) |
| Ampliar / reducir / restablecer zoom | Ctrl++ / Ctrl+- / Ctrl+0 |
| Zoom en la zona bajo el ratón | Ctrl+rueda del ratón |
| Mostrar u ocultar la regla | Ctrl+Alt+R |
| Desplazamiento tipo máquina de escribir | Meta+Ctrl+T |
| Opciones | Ctrl+, |
| Este manual | F1 |

---

## 20. Solución de problemas

### Informes de fallos e información del sistema

Si {{APP}} termina a causa de un error, escribe un **informe de fallo** en tu equipo antes de cerrarse. Si su ventana no responde durante 30 segundos (se bloquea; el tiempo se fija en **Archivo > Opciones > Extras > Informes de fallos**), escribe un **informe de bloqueo** que muestra dónde se ha quedado atascado; si se recupera, el informe indica después de cuánto tiempo. No se envía nada a ningún sitio. La próxima vez que inicies {{APP}} después de un fallo, o después de que hayas tenido que terminar un {{APP}} bloqueado, un mensaje te dice lo que pasó. Si terminó en el plazo de un minuto desde el inicio, el mensaje ofrece **Iniciar sin el último proyecto**, por si el proyecto es la causa.

- **Ayuda > Solución de problemas y desarrollo > Informes de fallos…** enumera los informes de fallo y de bloqueo con su fecha y su tipo de error («bloqueo (recuperado)» cuando {{APP}} siguió funcionando). **Copiar al portapapeles** copia el seleccionado, para que puedas pegarlo en un mensaje a quien se ocupe de {{APP}} por ti; **Abrir carpeta** muestra los archivos; **Eliminar** y **Eliminar todo…** los quitan. Se conservan los 20 informes más recientes (**Archivo > Opciones > Extras > Informes de fallos** cambia el número).
- **Archivo > Opciones > General > Avisos > Mostrar la consola de registro interna** muestra los mensajes del programa y de Qt desde que se inició (los últimos 1000), con los nuevos a medida que llegan; es útil cuando algo no funciona y ningún mensaje dice por qué.
- **Ayuda > Solución de problemas y desarrollo > Información del sistema…** muestra la misma instantánea del programa y del equipo que lleva un informe. Empieza con **Cosas que revisar**: lo que puede causar problemas pronto, como una unidad casi llena o de solo lectura, un proyecto en una carpeta de red, una unidad de copias de seguridad con menos espacio libre del que necesita el proyecto, poca memoria libre, un tiempo de guardado automático largo, las copias automáticas desactivadas o informes recientes de fallos y bloqueos. Después vienen la versión del programa, el entorno de ejecución (el conjunto de bibliotecas dentro del AppImage, con el día de publicación de su Qt y su antigüedad: un entorno antiguo significa que un AppImage más reciente puede traer correcciones de seguridad) y las actualizaciones y complementos instalados, el sistema operativo, el escritorio, las pantallas (desde 0.93.1 con el área que puede usar una ventana en cada una), los dispositivos de entrada (teclado, ratón, pantalla táctil, lápiz) y si se muestra el teclado en pantalla (0.93.1), el procesador y la memoria, las carpetas con su espacio libre, los ajustes de guardado y de copias de seguridad y el proyecto abierto. Usa **Copiar al portapapeles** o **Guardar como…** cuando informes de un problema que no es un fallo.

Un informe nombra archivos, carpetas y documentos, y los comandos y teclas usados poco antes del error (las letras escritas solo se cuentan, nunca se registran). No contiene ningún texto de tus documentos. Tu carpeta personal se escribe como **~**.

Si {{APP}} se detuvo desde fuera o el equipo se apagó sin cerrarlo, no hay informe, pero el siguiente inicio también te lo dice. En ambos casos tu trabajo está guardado hasta el último guardado automático, unos segundos antes del final; mira las instantáneas del documento o la copia de seguridad más reciente si falta algo (secciones 11 y 15).

**«No se puede leer el archivo del proyecto».** Si el archivo del proyecto (*TuTítulo*{{FILE}}) está dañado, por ejemplo por un fallo del equipo mientras se guardaba, {{APP}} ofrece la copia que guarda de él (de la última vez que se abrió el proyecto): **Usar la copia**. El archivo dañado se conserva a su lado (*… .damaged-<date>*). Faltan los cambios en el archivador desde entonces (títulos, orden, etiquetas); los textos están todos, y los documentos creados desde entonces se ponen en la carpeta **Documentos recuperados** de Investigación, con el nombre de su primera línea.

**«El archivo del proyecto contenía caracteres que no están permitidos en él».** Las versiones anteriores a 0.20 podían guardar caracteres de control invisibles (pegados desde otros programas en un título o una sinopsis), después de lo cual el proyecto ya no se abría. {{APP}} los quita al abrir el proyecto y te lo indica; no se cambia nada más. Desde 0.20 nunca se guardan.

**Un documento se muestra vacío, con la nota «No se puede leer el archivo de este documento».** Su archivo de texto existe pero no se puede leer (se cambiaron sus permisos, o la unidad en la que está desapareció). {{APP}} lo muestra vacío y no te deja editarlo, para que el archivo nunca se sobrescriba. Revisa los permisos del archivo o la unidad, y después cierra y vuelve a abrir el proyecto.

**Al pulsar Page Down o Page Up se cerró el programa.** Era un error de las versiones anteriores a 0.19; actualiza a 0.19 o posterior. Hasta entonces, desplázate con el ratón o con la barra de desplazamiento.

**El programa no se inicia al hacer doble clic.** Haz clic derecho en el archivo > Propiedades > Permisos y marca «Es ejecutable» (en Dolphin: «Permitir ejecutar el archivo como programa»). Después vuelve a hacer doble clic.

**Los diálogos de archivo se ven distintos de los de otros programas de KDE.** Activa **Usar los diálogos de archivo del escritorio** en **Archivo > Opciones > General** y reinicia {{APP}}.

**Los iconos de las barras de herramientas se ven mal en un tema oscuro o claro.** {{APP}} elige el conjunto claro u oscuro de los iconos de tu escritorio para que encaje con el tema. Si el escritorio no tiene un conjunto adecuado, usa sus propios iconos. Si algunos iconos siguen sin encajar, elige **Predeterminado (seguir el escritorio)** en **Ver > Temas** y fija los colores en los ajustes propios del escritorio.

**Cambié colores y quiero recuperar los del tema.** **Archivo > Opciones > Apariencia > Restablecer colores y fuentes al tema**, o el botón **↺** junto a un color o una fuente concretos.

**Una actualización del programa no se inició.** {{APP}} desactiva por sí mismo una actualización que falla (sección 17). Puedes intentar instalarla de nuevo, o seguir usando la versión integrada.

**Falta un formato de compilación.** Revisa **Archivo > Opciones > Plugins**: indica qué programa necesita el formato. Instálalo en Discover y reinicia {{APP}}.

**El corrector ortográfico no conoce mi idioma.** Instala en Discover el paquete de diccionario *hunspell* correspondiente y elígelo en **Archivo > Opciones > Edición**.

**El proyecto «parece estar abierto en otra ventana o en otro equipo».** Esto ocurre cuando el proyecto está en una carpeta sincronizada que está abierta en otro lugar, o después de un fallo en otro equipo. Si estás seguro de que no está abierto en ningún otro sitio, elige **Sí**.

**Alt+arrastrar no abre un documento de apoyo.** Algunos escritorios usan Alt+arrastrar para algo propio, como mover ventanas. Usa en su lugar **Navegar > Abrir > en el documento de apoyo** o el menú del clic derecho del documento, o cambia la tecla en los ajustes de ventanas del escritorio.

**Tras volver a una versión anterior del programa, un guion es texto sin formato y sin su disposición.** Las versiones anteriores a 0.10 no conocen los elementos de guion y guardan como texto normal un guion que han mostrado. Vuelve a cambiar a la versión actual (**Ayuda > Complementos y actualizaciones > Gestor de complementos**). El texto está todo; para recuperar la disposición, restaura el documento desde una instantánea o una copia de seguridad hecha antes (secciones 11 y 15), o dale de nuevo a los párrafos sus elementos con el menú emergente del pie.

**El atajo del bloc de notas rápidas no hace nada en otros programas.** Los atajos fijados en **Archivo > Opciones > Teclado** funcionan solo dentro de {{APP}}. Para otros programas, el escritorio debe iniciar el comando (sección 10, *Desde cualquier programa*). Si mueves el AppImage, introduce de nuevo el comando (nombra el lugar del AppImage).

**La cámara del bloc de notas rápidas dice que no puede leer la pantalla.** En Wayland solo puede hacerlo el programa de capturas del propio escritorio. Instala *Spectacle* (`sudo apt install kde-spectacle`) y vuelve a intentarlo.

**Edición > Herramientas de escritura dice «No hay herramientas de escritura instaladas».** Las herramientas son complementos: instala el paquete de la actualización 0.11 o actívalas en **Ayuda > Complementos y actualizaciones > Gestor de complementos**.

**Un archivo de audio o vídeo solo ofrece «Abrir con la aplicación predeterminada».** Reproducir dentro de {{APP}} necesita el complemento *Media Playback* (en el paquete de la actualización 0.12); instálalo o actívalo en **Ayuda > Complementos y actualizaciones > Gestor de complementos**. La línea bajo el botón indica lo que falta.

**Un vídeo se reproduce pero no hay sonido, o un archivo de audio dice «No se encontró ninguna salida de sonido».** {{APP}} reproduce el sonido a través del servidor de sonido del escritorio (PipeWire o PulseAudio). Comprueba que el sonido funciona en otros programas y que la salida correcta está elegida en los ajustes de audio del escritorio; {{APP}} sigue un cambio de salida de inmediato.

**«Este archivo no se puede reproducir aquí: usa H.264 y AAC ...»** El archivo está en un formato con licencia de patentes, que *Media Playback* deja fuera. Instala el complemento **Extra Video and Audio Formats**, o el paquete **media-all-formats.qbpkg**, que lo trae junto con *Media Playback* (ambos en la página Releases de la página del proyecto, sección 1), y reinicia {{APP}}.

**Un archivo no se reproduce, o se reproduce sin imagen.** Algunos tipos no forman parte de la decodificación integrada (vídeo AV1, WMA Pro, WMV 9). Abre el archivo con otro programa (clic derecho en el reproductor: **Abrir con la aplicación predeterminada**) o conviértelo a otro formato.

**Un archivo .doc se añade como archivo, no como texto.** El complemento **Importación de Word 97–2003** no está instalado o está desactivado (**Ayuda > Complementos y actualizaciones > Gestor de complementos**); tras instalarlo, reinicia {{APP}}. Un mensaje tras la importación nombra lo que omitió el complemento. Los archivos de Word 6.0 y 95 necesitan LibreOffice (`sudo apt install libreoffice-writer`).

**Una página web llega vacía o con la parte equivocada.** Desmarca **Solo el artículo** en la ventana Página web para tomar la página entera y elimina lo que no necesites. Las páginas que necesitan un inicio de sesión o que están construidas por completo con scripts no se pueden leer sin un navegador: abre la página en tu navegador, selecciona el texto y arrástralo al archivador (se convierte en un documento nuevo). «No se pueden obtener páginas web seguras» significa que falta OpenSSL 3 (`sudo apt install libssl3`, parte de todo Ubuntu) o que el complemento está dañado (instálalo de nuevo).

**El primer segundo de un archivo es silencioso o el tiempo se detiene al principio.** La salida de sonido necesita un momento para arrancar; con algunos servidores de sonido, más de un segundo. Retrocede un poco (Ctrl+Alt+,) si te perdiste el comienzo.

**Las páginas de la vista de página no coinciden con mi manuscrito impreso.** No pretenden coincidir: la vista de página muestra tu texto en hojas mientras escribes. El formato de compilación decide las páginas del archivo terminado (sección 13).

**«Scrivener tiene abierto este proyecto».** Scrivener deja un archivo de bloqueo en el *.scriv* mientras tiene el proyecto abierto, y tras un fallo. Cierra el proyecto en Scrivener (en todos los equipos) y vuelve a abrirlo aquí; si Scrivener no se está ejecutando en ningún sitio, elige **Sí**.

**Un proyecto en formato Scrivener no muestra mis últimos cambios de Scrivener.** {{APP}} vuelve a leer el *.scriv* solo cuando se abre el proyecto. Ciérralo aquí (**Archivo > Cerrar proyecto**), deja que Scrivener o el servicio de sincronización terminen y ábrelo de nuevo.

**Los menús Insertar, Navegar, Proyecto, Documentos y Formato están en gris.** No hay ningún proyecto abierto. Abre o crea uno (menú Archivo).

**Dónde informar de un problema.** **Ayuda > Notificar un problema…** abre en tu navegador la página de incidencias del proyecto (https://github.com/Shelin-boop/Quillbinder_Collection/issues/new/choose), con un formulario para un problema y otro para un deseo; necesitas una cuenta de GitHub y puedes escribir en inglés o en alemán. Las incidencias son públicas: no adjuntes tu manuscrito ni tu proyecto; **Ayuda > Solución de problemas y desarrollo > Información del sistema…** (**Copiar al portapapeles**) contiene lo que un informe necesita sobre el programa y el equipo. Un **problema de seguridad** no se notifica allí, sino en privado, como se describe en *SECURITY.md* del proyecto (la pestaña Security del repositorio, **Report a vulnerability**).

**El índice de un archivo de Word no tiene números de página.** Los números de página los rellena el procesador de textos cuando actualiza el campo; actualízalo allí.

**Algo salió mal con un documento.** Mira en la pestaña de instantáneas del inspector o restaura una copia de seguridad (sección 15).

## Créditos y licencia

- Desarrollador principal: Claude.AI w. Opus 5.5 Extra

{{APP}} es software libre bajo la licencia Apache 2.0: cualquiera puede usarlo, copiarlo, modificarlo y transmitirlo, con o sin cargo, conservando la licencia y el archivo NOTICE, que nombra el proyecto y su código fuente; sin garantía. Las versiones hasta 0.70.0 se publicaron bajo la licencia BSD de cláusula cero (0BSD). La ventana Acerca de (en el menú Ayuda) muestra la versión y la licencia.

{{APP}} usa partes de otros proyectos (Qt, Hunspell, ICU, los iconos Breeze y las demás bibliotecas que hay dentro del AppImage), cada una con su propia licencia. **Ayuda > Licencias de terceros…** (también un botón de la ventana Acerca de) las enumera con sus textos de licencia completos; los mismos textos están dentro del AppImage, en *usr/share/doc/quillbinder/licenses*. El código fuente de cada una, exactamente en la versión usada, está en la release **Third-party source code** de la página del proyecto (https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/third-party-sources); la ventana nombra el archivo de cada biblioteca, y las notas de licencia de los complementos hacen lo mismo con las suyas.

Scrivener y Scapple son nombres de productos de Literature & Latte. {{APP}} es un programa independiente, no hecho por Literature & Latte ni relacionado con esa empresa; lee y escribe sus formatos de archivo para que tus proyectos puedan pasar de un programa a otro.
