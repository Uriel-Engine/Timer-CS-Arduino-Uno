# Arduino Counter-Strike Timer

Un **Temporizador inspirado en Counter-Strike** construido con un **Arduino Uno**, utilizando una **pantalla LCD 16x2 con interfaz I2C**, un **teclado matricial 4x4**, un **DFPlayer Mini** para los efectos de sonido y un **LED sincronizado con la cuenta regresiva**.

> Este proyecto está destinado únicamente para fines de entretenimiento, Escape Rooms, creación de props y propósitos educativos.

---

# 📸 Características

- ⏱ Temporizador inspirado en Counter-Strike
- ⌨️ Interfaz mediante teclado matricial 4x4
- 🔊 Efectos de sonido utilizando DFPlayer Mini
- 💡 LED sincronizado con los beeps de la cuenta regresiva
- 🔑 Contraseña configurable de 4 dígitos
- ⏱ Tiempo de cuenta regresiva configurable (HH:MM:SS)
- 🚫 Tiempo límite para ingresar la contraseña
- ❌ Penalización por contraseña incorrecta
- 🏆 Estados de victoria y derrota
- 📟 Interfaz sencilla mediante pantalla LCD 16x2
- 📄 Todo el proyecto contenido en un único archivo `.ino`

---

# 🧰 Hardware

|      Componente       | Cantidad |
|-----------------------|---------:|
| Arduino Uno           |     1    |
| LCD 16x2 I2C          |     1    |
| Teclado Matricial 4x4 |     1    |
| DFPlayer Mini         |     1    |
| Bocina                |     1    |
| LED                   |     1    |
| Resistencia de 220Ω   |     1    |
| Tarjeta MicroSD       |     1    |

---

# 🔌 Conexiones

## LCD I2C

| LCD  | Arduino |
|------|----------|
| VCC  | 5V       |
| GND  | GND      |
| SDA  | A4       |
| SCL  | A5       |

---

## DFPlayer Mini

| DFPlayer | Arduino |
|-----------|----------|
| VCC       | 5V       |
| GND       | GND      |
| RX        | D11      |
| TX        | D10      |
| SPK1      | Bocina   |
| SPK2      | Bocina   |

---

## Teclado Matricial

| Pin del Keypad | Arduino |
|-------------|------------|
| 1           | 2          |
| 2           | 3          |
| 3           | 4          |
| 4           | 5          |
| 5           | 6          |
| 6           | 7          |
| 7           | 8          |
| 8           | 9          |

---

## LED

|   LED  | Arduino |
|--------|---------|
| Ánodo  | D12     |
| Cátodo | GND     |

---

# 🎮 Controles

## Menú Principal

```
A → Iniciar / Plantar
B → Configuración
```

---

## Configuración

```
A → Contraseña
B → Tiempo
```

---

## Teclas Generales

| Tecla |       Función       |
|-------|---------------------|
| 0-9   | Introducir números  |
| *     | Cancelar / Regresar |
| #     | Aceptar / Confirmar |

---

# ⚙ Configuración Predeterminada

Contraseña

```
1234
```

Tiempo de cuenta regresiva

```
00:01:50
```

Estos valores se restauran automáticamente cada vez que el Arduino se enciende.

---

# ⏱ Funcionamiento

1. Selecciona **Iniciar / Plantar**.
2. Se reproduce el sonido **"Bomb Planted"**.
3. Comienza la cuenta regresiva.
4. Cada segundo:
   - Se actualiza la pantalla LCD.
   - El LED parpadea.
   - Se reproduce el sonido del beep.
5. Presiona **#** para ingresar la contraseña.
6. Introduce el código correcto antes de que el tiempo llegue a cero.

---

## Contraseña Correcta

- La cuenta regresiva se detiene.
- Se reproduce el sonido de victoria.
- Se muestra la pantalla de victoria.

---

## Contraseña Incorrecta

Si la contraseña ingresada es incorrecta:

- Se muestra el mensaje **"WRONG PASSWORD"**.
- Si quedan más de **15 segundos**, el temporizador se reduce inmediatamente a **15 segundos**.
- Después de 5 segundos vuelve la pantalla de la cuenta regresiva.
- El jugador puede volver a intentarlo hasta que el tiempo llegue a cero.

---

## Tiempo Límite para la Contraseña

Si durante **15 segundos** no se presiona ninguna tecla mientras se está ingresando la contraseña:

- Se cancela automáticamente el ingreso del código.
- Se regresa a la pantalla de la cuenta regresiva.
- El temporizador continúa funcionando normalmente.

---

# 🔊 Archivos de Audio

Copia estos archivos en la raíz de la tarjeta MicroSD en el DFPlayer mini.

| Archivo  | Descripción         |
|----------|---------------------|
| 0001.mp3 | Beep                |
| 0002.mp3 | Bomb Planted        |
| 0003.mp3 | Victoria            |
| 0004.mp3 | Explosión / Derrota |

---

# 📁 Estructura del Proyecto

```
Timer_CS.ino
README.md
/
└── Tarjeta SD
    ├── 0001.mp3
    ├── 0002.mp3
    ├── 0003.mp3
    └── 0004.mp3
```

---

# 📜 Licencia

Este proyecto se distribuye bajo la licencia MIT.

Este proyecto es una recreación electrónica DIY no oficial realizada únicamente con fines educativos y de entretenimiento.
