#ifndef GLOBAL_H
#define GLOBAL_H
#include <ti/getcsc.h>

extern const uint8_t inputTextColor;
extern const uint8_t inputBackgroundColor;
extern const uint8_t backgroundColor;

typedef enum {
    Character_Power = '^',
    Character_PI = (char)128,
    Character_RightArrow = (char)129,
    Character_PlaceHolder = (char)130,
    Character_SecondCursorIdentifier = (char)131,
    Character_SecondAlphaCursorIdentifier = (char)132,
    Character_AlphaCursorIdentifier = (char)133,
    Charcater_Top_Open_Parenthesis = (char)134,
    Charcater_Bottom_Open_Parenthesis = (char)135,
    Charcater_Top_Close_Parenthesis = (char)136,
    Charcater_Bottom_Close_Parenthesis = (char)137,
    Charcater_Top_Open_Bracket = (char)138,
    Charcater_Bottom_Open_Bracket = (char)139,
    Charcater_Top_Close_Bracket = (char)140,
    Charcater_Bottom_Close_Bracket = (char)141,
    Character_Degree = (char)142,
    Character_Minute = (char)143,
    Character_Second = (char)144,
    Character_Delete = (char)145,
    
    // character representaion for keys on TI84 calc
    // technacally not neccesary but for ease of mind
    Character_Left = (char)146,
    Character_Right = (char)147,
    Character_Up = (char)148,
    Character_Down = (char)149,
    Character_Fraction = (char)150,
    Character_Clear = (char)151,
    Character_Sin = (char)152,
    Character_Cos = (char)153,
    Character_Tan = (char)154,
    Character_InvSin = (char)155,
    Character_InvCos = (char)156,
    Character_InvTan = (char)157,
    Character_Csc = (char)158,
    Character_Sec = (char)159,
    Character_Cot = (char)160,
    Character_InvCsc = (char)161,
    Character_InvSec = (char)162,
    Character_InvCot = (char)163,
    Character_Root = (char)164,
    Character_SquareRoot = (char)165,
    Character_LogBase = (char)166,
    Character_Log = (char)167,
    Character_Ln = (char)168,
    Character_Square = (char)169,
    Character_Reciprocal = (char)170,
} CharacterCode;

extern const char version[10];

extern bool preferARCsyntax; // arcsin(x) > sin^-1(x)
extern bool preferDMSsyntax; // 12°20'42" > 12.345°
extern bool radianMode; // rads > degress





#endif