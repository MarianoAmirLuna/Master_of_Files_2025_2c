## Master of Files
Este proyecto es una simulación de un sistema operativo distribuido enfocado en la
gestión eficiente de archivos y memoria. El sistema permite procesar consultas
complejas sobre archivos mediante una arquitectura de módulos interconectados que
emulan el comportamiento de un Kernel, un File System y un Worker.

## El Concepto del "Worker"
En la arquitectura tradicional de computadoras, la CPU (procesamiento) y la Memoria RAM (almacenamiento temporal) son componentes físicamente separados. En este proyecto, hemos unificado ambos conceptos en un único módulo denominado Worker.

- Como `CPU`: El Worker es el motor de ejecución. Implementa un ciclo de instrucción (fetch-decode-execute-check_interrupt) para procesar un set de instrucciones personalizadas.

- Como `Memoria`: Gestiona un espacio de direccionamiento propio mediante Paginación, administrando "frames" de memoria física y una tabla de páginas global para mapear datos lógicos a direcciones reales.

Esta unificación permite simular cómo un procesador interactúa directamente con la jerarquía de memoria y cómo se sincroniza con un almacenamiento persistente externo.

## Guía de Despliegue

1. Preparación del Entorno
Para garantizar un entorno controlado y fiel a los sistemas de servidor reales:

    1.1 Inicia una máquina virtual con Ubuntu Server (sin interfaz gráfica).

    1.2 Obtén la dirección IP de la máquina ejecutando el comando:
```bash
ifconfig
```
> [!NOTE]
> Busca el valor inet dentro de la interfaz de red activa.

    1.3 Conéctate a la máquina mediante un cliente SSH como PuTTY usando la IP obtenida.

2. Instalación y Dependencias
El proyecto utiliza las bibliotecas estándar de la cátedra para manejo de hilos, sockets y estructuras de datos comunes.

    2.1 Asegúrate de tener instaladas las [so-commons-library].

    2.2 Accede a la carpeta de despliegue:

```bash
cd so-deploy
```

    2.3 Clona el repositorio del proyecto en esa ubicación.

3. Configuración del Sistema
Antes de compilar, debemos "decirle" a cada módulo dónde se encuentran desplegados sus compañeros. Se utiliza un script [so-deploy] de la cátedra que modifica los archivos .config de manera automática: 

```bash
# Ejemplo de configuración de IPs
./config IP_MASTER=<IP_DE_MODULO_MASTER>
./config IP_STORAGE=<IP_DE_MODULO_STORAGE>
```

> [!NOTE]
> Puedes configurar cualquier parámetro presente en los archivos .config, como retardos de memoria o algoritmos de planificación.

4. Compilación
Ejecuta el script de compilación automática que limpia binarios antiguos y genera los nuevos ejecutables en todos los módulos:

```bash
./bin/query_control [archivo_config] [archivo_query] [prioridad]
```

- `archivo_config`: Define el comportamiento del módulo (niveles de Log, retardos de respuesta, algoritmos de reemplazo como LRU/Clock-M, etc.).

- `archivo_query`: Un script con las instrucciones que el Worker debe ejecutar.

- `prioridad`: Un valor numérico que el Kernel (Master) utiliza para su algoritmo de planificación de procesos.

## Ejecución y Pruebas
Para poner a prueba como se levanta un modulo del sistema se utilizará el módulo Query Control, que actúa como la interfaz de entrada de los procesos, en este proyecto llamados queries.
Comando de ejecución:

```bash
./2_compile_all.sh
```

## Set de Instrucciones Soportadas:

El Worker es capaz de interpretar y ejecutar las siguientes operaciones lógicas:

| Instrucción | Descripción |
| :--- | :--- |
| **CREATE** [FILE]:[TAG] | Crea un nuevo archivo con una tag identificadora. |
| **WRITE** [FILE]:[TAG] [DIR] [DATA] | Escribe datos en una dirección lógica específica. |
| **READ** [FILE]:[TAG] [DIR] [SIZE] | Lee una cantidad de bytes desde una dirección lógica. |
| **TRUNCATE** [FILE]:[TAG] [SIZE] | Modifica el tamaño de un archivo en el almacenamiento. |
| **FLUSH** [FILE]:[TAG] | Sincroniza las páginas sucias de memoria con el File System. |
| **COMMIT** [FILE]:[TAG] | Asegura la persistencia de los cambios realizados.|
| **DELETE** [FILE]:[TAG] | Elimina el archivo y libera sus marcos de memoria. |

## Ejemplo de un script de Query:

```bash
CREATE LINKIN_PARK:V1
TRUNCATE LINKIN_PARK:V1 1024
WRITE LINKIN_PARK:V1 0 Hybrid_Theory
FLUSH LINKIN_PARK:V1
READ LINKIN_PARK:V1 0 16
COMMIT LINKIN_PARK:V1
END
```

> [!NOTE]
> El comando END actúa como el delimitador de finalización, similar al ; en otros lenguajes

## Tecnologías y Conceptos Aplicados

- **Estrategia de Reemplazo Global**: Implementación de algoritmos como LRU o Clock-M para decidir qué página desalojar de la memoria física cuando esta se encuentra llena

- **Comunicación Asincrónica**: Uso de Sockets TCP/IP y protocolos de serialización propios para el intercambio de paquetes entre módulos.

- **Concurrencia**: Manejo de hilos (pthreads) y semáforos para evitar condiciones de carrera en el acceso a marcos de memoria física.

- **Lenguaje**: C

- **Unidad de Gestión de Memoria**: Implementación de la lógica de una MMU para realizar la traducción de direcciones lógicas a físicas.

- **Paginación Pura con Asignación Fija**: El sistema divide la memoria física en Frames de tamaño fijo definidos en el handshake con el File System. Esto elimina la fragmentación externa, aunque introduce una mínima fragmentación interna en el último marco de cada archivo.

# tp-scaffold

Esta es una plantilla de proyecto diseñada para generar un TP de Sistemas
Operativos de la UTN FRBA.

## Dependencias

Para poder compilar y ejecutar el proyecto, es necesario tener instalada la
biblioteca [so-commons-library] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library
make debug
make install
```

## Compilación y ejecución

Cada módulo del proyecto se compila de forma independiente a través de un
archivo `makefile`. Para compilar un módulo, es necesario ejecutar el comando
`make` desde la carpeta correspondiente.

El ejecutable resultante de la compilación se guardará en la carpeta `bin` del
módulo. Ejemplo:

```sh
cd kernel
make
./bin/kernel
```

## Importar desde Visual Studio Code

Para importar el workspace, debemos abrir el archivo `tp.code-workspace` desde
la interfaz o ejecutando el siguiente comando desde la carpeta raíz del
repositorio:

```bash
code tp.code-workspace
```

## Checkpoint

Para cada checkpoint de control obligatorio, se debe crear un tag en el
repositorio con el siguiente formato:

```
checkpoint-{número}
```

Donde `{número}` es el número del checkpoint, ejemplo: `checkpoint-1`.

Para crear un tag y subirlo al repositorio, podemos utilizar los siguientes
comandos:

```bash
git tag -a checkpoint-{número} -m "Checkpoint {número}"
git push origin checkpoint-{número}
```

## Entrega

Para desplegar el proyecto en una máquina Ubuntu Server, podemos utilizar el
script [so-deploy] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-deploy.git
cd so-deploy
./deploy.sh -r=release -p=utils -p=query_control -p=master -p=worker -p=storage "tp-{año}-{cuatri}-{grupo}"
```

El mismo se encargará de instalar las Commons, clonar el repositorio del grupo
y compilar el proyecto en la máquina remota.

> [!NOTE]
> Ante cualquier duda, pueden consultar la documentación en el repositorio de
> [so-deploy], o utilizar el comando `./deploy.sh --help`.

## Guías útiles

- [Cómo interpretar errores de compilación](https://docs.utnso.com.ar/primeros-pasos/primer-proyecto-c#errores-de-compilacion)
- [Cómo utilizar el debugger](https://docs.utnso.com.ar/guias/herramientas/debugger)
- [Cómo configuramos Visual Studio Code](https://docs.utnso.com.ar/guias/herramientas/code)
- **[Guía de despliegue de TP](https://docs.utnso.com.ar/guías/herramientas/deploy)**

[so-commons-library]: https://github.com/sisoputnfrba/so-commons-library
[so-deploy]: https://github.com/sisoputnfrba/so-deploy