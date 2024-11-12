#include <ti/getcsc.h>

const uint8_t TextData[128 * 8 + 8 * 14] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  //   - 0
	0x7E,0x81,0xA5,0x81,0xBD,0xBD,0x81,0x7E,  // ☺ - 1
	0x7E,0xFF,0xDB,0xFF,0xC3,0xC3,0xFF,0x7E,  // ☻ - 2
	0x6C,0xFE,0xFE,0xFE,0x7C,0x38,0x10,0x00,  // ♥ - 3
	0x10,0x38,0x7C,0xFE,0x7C,0x38,0x10,0x00,  // ♦ - 4
	0x38,0x7C,0x38,0xFE,0xFE,0x10,0x10,0x7C,  // ♣ - 5
	0x00,0x18,0x3C,0x7E,0xFF,0x7E,0x18,0x7E,  // ♠ - 6
	0x00,0x00,0x18,0x3C,0x3C,0x18,0x00,0x00,  // • - 7
	0xFF,0xFF,0xE7,0xC3,0xC3,0xE7,0xFF,0xFF,  // ◘ - 8
	0x00,0x3C,0x66,0x42,0x42,0x66,0x3C,0x00,  // ○ - 9
	0xFF,0xC3,0x99,0xBD,0xBD,0x99,0xC3,0xFF,  // ◙ - 10
	0x0F,0x07,0x0F,0x7D,0xCC,0xCC,0xCC,0x78,  // ♂ - 11
	0x3C,0x66,0x66,0x66,0x3C,0x18,0x7E,0x18,  // ♀ - 12
	0x3F,0x33,0x3F,0x30,0x30,0x70,0xF0,0xE0,  // ♪ - 13
	0x7F,0x63,0x7F,0x63,0x63,0x67,0xE6,0xC0,  // ♫ - 14
	0x99,0x5A,0x3C,0xE7,0xE7,0x3C,0x5A,0x99,  // * - 15
	0x80,0xE0,0xF8,0xFE,0xF8,0xE0,0x80,0x00,  // ► - 16
	0x02,0x0E,0x3E,0xFE,0x3E,0x0E,0x02,0x00,  // ◄ - 17
	0x18,0x3C,0x7E,0x18,0x18,0x7E,0x3C,0x18,  // ↕ - 18
	0x66,0x66,0x66,0x66,0x66,0x00,0x66,0x00,  // ‼ - 19
	0x7F,0xDB,0xDB,0x7B,0x1B,0x1B,0x1B,0x00,  // ¶ - 20
	0x3F,0x60,0x7C,0x66,0x66,0x3E,0x06,0xFC,  // § - 21
	0x00,0x00,0x00,0x00,0x7E,0x7E,0x7E,0x00,  // ▬ - 22
	0x18,0x3C,0x7E,0x18,0x7E,0x3C,0x18,0xFF,  // ↨ - 23
	0x18,0x3C,0x7E,0x18,0x18,0x18,0x18,0x00,  // ↑ - 24
	0x18,0x18,0x18,0x18,0x7E,0x3C,0x18,0x00,  // ↓ - 25
	0x00,0x18,0x0C,0xFE,0x0C,0x18,0x00,0x00,  // → - 26
	0x00,0x30,0x60,0xFE,0x60,0x30,0x00,0x00,  // ← - 27
	0x00,0x00,0xC0,0xC0,0xC0,0xFE,0x00,0x00,  // └ - 28
	0x00,0x24,0x66,0xFF,0x66,0x24,0x00,0x00,  // ↔ - 29
	0x00,0x18,0x3C,0x7E,0xFF,0xFF,0x00,0x00,  // ▲ - 30
	0x00,0xFF,0xFF,0x7E,0x3C,0x18,0x00,0x00,  // ▼ - 31
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  //   - 32
	0xC0,0xC0,0xC0,0xC0,0xC0,0x00,0xC0,0x00,  // ! - 33
	0xD8,0xD8,0xD8,0x00,0x00,0x00,0x00,0x00,  // " - 34
	0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00,  // # - 35
	0x18,0x7E,0xC0,0x7C,0x06,0xFC,0x18,0x00,  // $ - 36
	0x00,0xC6,0xCC,0x18,0x30,0x66,0xC6,0x00,  // % - 37
	0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00,  // & - 38
	0x30,0x30,0x30,0x00,0x00,0x00,0x00,0x00,  // ' - 39
	0x30,0x60,0xC0,0xC0,0xC0,0x60,0x30,0x00,  // ( - 40
	0xC0,0x60,0x30,0x30,0x30,0x60,0xC0,0x00,  // ) - 41
	0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00,  // * - 42
	0x00,0x30,0x30,0xFC,0xFC,0x30,0x30,0x00,  // + - 43
	0x00,0x00,0x00,0x00,0x00,0x60,0x60,0xC0,  // , - 44
	0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0x00,  // - - 45
	0x00,0x00,0x00,0x00,0x00,0xC0,0xC0,0x00,  // . - 46
	0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00,  // / - 47
	0x7C,0xCE,0xDE,0xF6,0xE6,0xC6,0x7C,0x00,  // 0 - 48
	0x30,0x70,0x30,0x30,0x30,0x30,0xFC,0x00,  // 1 - 49
	0x7C,0xC6,0x06,0x7C,0xC0,0xC0,0xFE,0x00,  // 2 - 50
	0xFC,0x06,0x06,0x3C,0x06,0x06,0xFC,0x00,  // 3 - 51
	0x0C,0xCC,0xCC,0xCC,0xFE,0x0C,0x0C,0x00,  // 4 - 52
	0xFE,0xC0,0xFC,0x06,0x06,0xC6,0x7C,0x00,  // 5 - 53
	0x7C,0xC0,0xC0,0xFC,0xC6,0xC6,0x7C,0x00,  // 6 - 54
	0xFE,0x06,0x06,0x0C,0x18,0x30,0x30,0x00,  // 7 - 55
	0x7C,0xC6,0xC6,0x7C,0xC6,0xC6,0x7C,0x00,  // 8 - 56
	0x7C,0xC6,0xC6,0x7E,0x06,0x06,0x7C,0x00,  // 9 - 57
	0x00,0xC0,0xC0,0x00,0x00,0xC0,0xC0,0x00,  // : - 58
	0x00,0x60,0x60,0x00,0x00,0x60,0x60,0xC0,  // ; - 59
	0x18,0x30,0x60,0xC0,0x60,0x30,0x18,0x00,  // < - 60
	0x00,0x00,0xFC,0x00,0xFC,0x00,0x00,0x00,  // = - 61
	0xC0,0x60,0x30,0x18,0x30,0x60,0xC0,0x00,  // > - 62
	0x78,0xCC,0x18,0x30,0x30,0x00,0x30,0x00,  // ? - 63
	0x7C,0xC6,0xDE,0xDE,0xDE,0xC0,0x7E,0x00,  // @ - 64
	0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0x00,  // A - 65
	0xFC,0xC6,0xC6,0xFC,0xC6,0xC6,0xFC,0x00,  // B - 66
	0x7C,0xC6,0xC0,0xC0,0xC0,0xC6,0x7C,0x00,  // C - 67
	0xF8,0xCC,0xC6,0xC6,0xC6,0xCC,0xF8,0x00,  // D - 68
	0xFE,0xC0,0xC0,0xF8,0xC0,0xC0,0xFE,0x00,  // E - 69
	0xFE,0xC0,0xC0,0xF8,0xC0,0xC0,0xC0,0x00,  // F - 70
	0x7C,0xC6,0xC0,0xC0,0xCE,0xC6,0x7C,0x00,  // G - 71
	0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0x00,  // H - 72
	0x7E,0x18,0x18,0x18,0x18,0x18,0x7E,0x00,  // I - 73
	0x06,0x06,0x06,0x06,0x06,0xC6,0x7C,0x00,  // J - 74
	0xC6,0xCC,0xD8,0xF0,0xD8,0xCC,0xC6,0x00,  // K - 75
	0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xFE,0x00,  // L - 76
	0xC6,0xEE,0xFE,0xFE,0xD6,0xC6,0xC6,0x00,  // M - 77
	0xC6,0xE6,0xF6,0xDE,0xCE,0xC6,0xC6,0x00,  // N - 78
	0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,  // O - 79
	0xFC,0xC6,0xC6,0xFC,0xC0,0xC0,0xC0,0x00,  // P - 80
	0x7C,0xC6,0xC6,0xC6,0xD6,0xDE,0x7C,0x06,  // Q - 81
	0xFC,0xC6,0xC6,0xFC,0xD8,0xCC,0xC6,0x00,  // R - 82
	0x7C,0xC6,0xC0,0x7C,0x06,0xC6,0x7C,0x00,  // S - 83
	0xFF,0x18,0x18,0x18,0x18,0x18,0x18,0x00,  // T - 84
	0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xFE,0x00,  // U - 85
	0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x38,0x00,  // V - 86
	0xC6,0xC6,0xC6,0xC6,0xD6,0xFE,0x6C,0x00,  // W - 87
	0xC6,0xC6,0x6C,0x38,0x6C,0xC6,0xC6,0x00,  // X - 88
	0xC6,0xC6,0xC6,0x7C,0x18,0x30,0xE0,0x00,  // Y - 89
	0xFE,0x06,0x0C,0x18,0x30,0x60,0xFE,0x00,  // Z - 90
	0xF0,0xC0,0xC0,0xC0,0xC0,0xC0,0xF0,0x00,  // [ - 91
	0xC0,0x60,0x30,0x18,0x0C,0x06,0x02,0x00,  // \ - 92
	0xF0,0x30,0x30,0x30,0x30,0x30,0xF0,0x00,  // ] - 93
	0x10,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00,  // ^ - 94
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,  // _ - 95
	0xC0,0xC0,0x60,0x00,0x00,0x00,0x00,0x00,  // ` - 96
	0x00,0x00,0x7C,0x06,0x7E,0xC6,0x7E,0x00,  // a - 97
	0xC0,0xC0,0xC0,0xFC,0xC6,0xC6,0xFC,0x00,  // b - 98
	0x00,0x00,0x7C,0xC6,0xC0,0xC6,0x7C,0x00,  // c - 99
	0x06,0x06,0x06,0x7E,0xC6,0xC6,0x7E,0x00,  // d - 100
	0x00,0x00,0x7C,0xC6,0xFE,0xC0,0x7C,0x00,  // e - 101
	0x1C,0x36,0x30,0x78,0x30,0x30,0x78,0x00,  // f - 102
	0x00,0x00,0x7E,0xC6,0xC6,0x7E,0x06,0xFC,  // g - 103
	0xC0,0xC0,0xFC,0xC6,0xC6,0xC6,0xC6,0x00,  // h - 104
	0x60,0x00,0xE0,0x60,0x60,0x60,0xF0,0x00,  // i - 105
	0x06,0x00,0x06,0x06,0x06,0x06,0xC6,0x7C,  // j - 106
	0xC0,0xC0,0xCC,0xD8,0xF8,0xCC,0xC6,0x00,  // k - 107
	0xE0,0x60,0x60,0x60,0x60,0x60,0xF0,0x00,  // l - 108
	0x00,0x00,0xCC,0xFE,0xFE,0xD6,0xD6,0x00,  // m - 109
	0x00,0x00,0xFC,0xC6,0xC6,0xC6,0xC6,0x00,  // n - 110
	0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7C,0x00,  // o - 111
	0x00,0x00,0xFC,0xC6,0xC6,0xFC,0xC0,0xC0,  // p - 112
	0x00,0x00,0x7E,0xC6,0xC6,0x7E,0x06,0x06,  // q - 113
	0x00,0x00,0xFC,0xC6,0xC0,0xC0,0xC0,0x00,  // r - 114
	0x00,0x00,0x7E,0xC0,0x7C,0x06,0xFC,0x00,  // s - 115
	0x30,0x30,0xFC,0x30,0x30,0x30,0x1C,0x00,  // t - 116
	0x00,0x00,0xC6,0xC6,0xC6,0xC6,0x7E,0x00,  // u - 117
	0x00,0x00,0xC6,0xC6,0xC6,0x7C,0x38,0x00,  // v - 118
	0x00,0x00,0xC6,0xC6,0xD6,0xFE,0x6C,0x00,  // w - 119
	0x00,0x00,0xC6,0x6C,0x38,0x6C,0xC6,0x00,  // x - 120
	0x00,0x00,0xC6,0xC6,0xC6,0x7E,0x06,0xFC,  // y - 121
	0x00,0x00,0xFE,0x0C,0x38,0x60,0xFE,0x00,  // z - 122
	0x1C,0x30,0x30,0xE0,0x30,0x30,0x1C,0x00,  // { - 123
	0xC0,0xC0,0xC0,0x00,0xC0,0xC0,0xC0,0x00,  // | - 124
	0xE0,0x30,0x30,0x1C,0x30,0x30,0xE0,0x00,  // } - 125
	0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00,  // ~ - 126
	0x00,0x10,0x38,0x6C,0xC6,0xC6,0xFE,0x00,  // △ - 127
    0x00,0x7F,0xA4,0x24,0x24,0x25,0x42,0x00,  // π - 128
    0x00,0x30,0x38,0x3C,0x3C,0x38,0x30,0x00,  // > - 129 // right arrow
    0xDB,0x81,0x00,0x81,0x81,0x00,0x81,0xDB,  //   - 130 // Placeholder character for fractions/exponents etc
    0x00,0x18,0x3C,0x7E,0x18,0x18,0x18,0x00,  //   - 131 // Second Cursor Identifier 
    0x00,0x18,0x24,0x3C,0x24,0x00,0x7E,0x00,  //   - 132 // Alpha + Second Cursor Identifier
    0x00,0x18,0x24,0x24,0x3C,0x24,0x24,0x00,  //   - 133 // Alpha Cursor Identifier
    0x04,0x08,0x08,0x10,0x00,0x00,0x00,0x00,  //   - 134 // top half of parenthesis - open
    0x00,0x00,0x00,0x00,0x10,0x08,0x08,0x04,  //   - 135 // bottom half of parenthesis - open
    0x20,0x10,0x10,0x08,0x00,0x00,0x00,0x00,  //   - 136 // top half of parenthesis - closing
    0x00,0x00,0x00,0x00,0x08,0x10,0x10,0x20,  //   - 137 // bottom half of parenthesis - closing
    0x1C,0x10,0x10,0x10,0x00,0x00,0x00,0x00,  //   - 138 // top half of bracket - open
    0x00,0x00,0x00,0x00,0x10,0x10,0x10,0x1C,  //   - 139 // bottom half of bracket - open
    0x38,0x08,0x08,0x08,0x00,0x00,0x00,0x00,  //   - 140 // top half of bracket - closing
    0x00,0x00,0x00,0x00,0x08,0x08,0x08,0x38,  //   - 141 // bottom half of bracket - closing
};