#define KEY_UP           0
#define KEY_RIGHT        1
#define KEY_DOWN         2
#define KEY_LEFT         3
#define KEY_RIGHT_TURN   4
#define KEY_LEFT_TURN    5
#define KEY_START        6
#define KEY_WAIT         7

// TODO(mayah): Change PIN numbers appropriately.
// #define PIN_UP           6
#define PIN_START        8
#define PIN_DOWN         9
#define PIN_LEFT         10
#define PIN_RIGHT        11
#define PIN_LEFT_TURN    12
#define PIN_RIGHT_TURN   13

#define ARRAY_SIZE(xs)   (sizeof(xs) / sizeof(xs[0]))

static const int OUTPUT_PINS[] = {
//     PIN_UP,
    PIN_RIGHT,
    PIN_DOWN,
    PIN_LEFT,
    PIN_RIGHT_TURN,
    PIN_LEFT_TURN,
    PIN_START
};

void pressButton(int pin, boolean pressed)
{
    if (pressed) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    } else {
        // If we set the pin mode INPUT, the pin will have
        // high impedance. This means the button is not pressed.
        pinMode(pin, INPUT);
    }
}

void setup()
{
    for (int i = 0; i < ARRAY_SIZE(OUTPUT_PINS); ++i)
        pressButton(OUTPUT_PINS[i], false);

    Serial.begin(38400);
    delay(100);
}

void loop()
{
    if (Serial.available() <= 0)
        return;

    int x = Serial.read();
    x &= 0xFF;

    if ((x >> KEY_WAIT) & 1) {
        int wait = x & 0x7F;
        delay(wait);
        return;
    }

    // boolean up = ((x >> KEY_UP) & 1);
    boolean right = ((x >> KEY_RIGHT) & 1);
    boolean left = ((x >> KEY_LEFT) & 1);
    boolean down = ((x >> KEY_DOWN) & 1);
    boolean right_turn = ((x >> KEY_RIGHT_TURN) & 1);
    boolean left_turn = ((x >> KEY_LEFT_TURN) & 1);
    boolean start = ((x >> KEY_START) & 1);

    // pressButton(PIN_UP, up);
    pressButton(PIN_RIGHT, right);
    pressButton(PIN_DOWN, down);
    pressButton(PIN_LEFT, left);
    pressButton(PIN_RIGHT_TURN, right_turn);
    pressButton(PIN_LEFT_TURN, left_turn);
    pressButton(PIN_START, start);
    delay(34);
}
