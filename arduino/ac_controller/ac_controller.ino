#define KEY_UP           0
#define KEY_RIGHT        1
#define KEY_DOWN         2
#define KEY_LEFT         3
#define KEY_RIGHT_TURN   4
#define KEY_LEFT_TURN    5
#define KEY_START        6
#define KEY_WAIT         7

// TODO(mayah): Change PIN numbers appropriately.
#define PIN_UP           6
#define PIN_RIGHT        7
#define PIN_DOWN         8
#define PIN_LEFT         9
#define PIN_RIGHT_TURN   10
#define PIN_LEFT_TURN    11
#define PIN_START        12

#define PIN_LED          13

#define ARRAY_SIZE(xs)   (sizeof(xs) / sizeof(xs[0]))

static const int OUTPUT_PINS[] = {
    PIN_UP, PIN_RIGHT, PIN_DOWN, PIN_LEFT,
    PIN_RIGHT_TURN, PIN_LEFT_TURN, PIN_START, PIN_LED
};

void setup()
{
    for (int i = 0; i < ARRAY_SIZE(OUTPUT_PINS); ++i)
        pinMode(OUTPUT_PINS[i], OUTPUT);

    for (int i = 0; i < ARRAY_SIZE(OUTPUT_PINS); ++i)
        digitalWrite(OUTPUT_PINS[i], LOW);

    digitalWrite(PIN_LED, HIGH);

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
        digitalWrite(PIN_LED, HIGH);
        delay(wait);
        return;
    }

    int up = ((x >> KEY_UP) & 1) ? HIGH : LOW;
    int right = ((x >> KEY_RIGHT) & 1) ? HIGH : LOW;
    int left = ((x >> KEY_LEFT) & 1) ? HIGH : LOW;
    int down = ((x >> KEY_DOWN) & 1) ? HIGH : LOW;
    int right_turn = ((x >> KEY_RIGHT_TURN) & 1) ? HIGH : LOW;
    int left_turn = ((x >> KEY_LEFT_TURN) & 1) ? HIGH : LOW;
    int start = ((x >> KEY_START) & 1) ? HIGH : LOW;

    digitalWrite(PIN_UP, up);
    digitalWrite(PIN_RIGHT, right);
    digitalWrite(PIN_DOWN, down);
    digitalWrite(PIN_LEFT, left);
    digitalWrite(PIN_RIGHT_TURN, right_turn);
    digitalWrite(PIN_LEFT_TURN, left_turn);
    digitalWrite(PIN_START, start);
    delay(34);
}
