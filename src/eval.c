/****************************************
****         A Block of Code         ****
*****************************************

Parsing Emulator for A Block of Code
* Tyler's proposal for syntax/symantics

(C)2014 Erubus Labs
* For internal use only (subject to change)

****************************************/


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "pool.h"

typedef double (*eval_expr_fn)(double, double);
eval_expr_fn eval_op_lut[128];

extern double globals[6];
ASTNode* g_ast_root;

eval_expr_fn eval_get_fn(ASTNode* node) {
    return eval_op_lut[node->data.op];
}

double eval_expr(ASTNode* expr) {
    // Intialize things that can't be
    // declared in switch
    eval_expr_fn op = 0;
    ASTNode* needle = expr;

    // Handle types
    if (!expr) {
        return 0;
    }
    switch (expr->t) {
        case ASTNum:
            return expr->data.num;
        case ASTVar:
            return globals[expr->data.op - 'A'];
        case ASTExp:
            if (expr->data.op == ':') {
                return (*eval_get_fn(expr))(
                    expr->lop->data.op,
                    eval_expr(expr->rop)
                );
            }
            return (*eval_get_fn(expr))(
                eval_expr(expr->lop),
                eval_expr(expr->rop)
            );
        case ASTList:
            for (; needle; needle = needle->rop) {
                eval_expr(needle->data.e);
            }
        default:
            return 0;
    }
}

double eval_default(double x, double y) {
    fprintf(stderr, "ERROR: Unrecognized operator :(\n");
    exit(1);
}

double eval_print(double x, double y) {
    fprintf(stdout, "%g\n", y);
    return y;
}

double eval_assign(double x, double y) {
    return globals[(char)x - 'A'] = y;
}

double eval_add(double x, double y) { return x +  y;  }
double eval_sub(double x, double y) { return x -  y;  }
double eval_mul(double x, double y) { return x *  y;  }
double eval_div(double x, double y) { return x /  y;  }
double eval_pow(double x, double y) { return pow(x,y);}
double eval_mod(double x, double y) {
    return (double)((int)x %  (int)y);
}

double eval_ceq(double x, double y) { return x == y;  }
double eval_clt(double x, double y) { return x <  y;  }
double eval_cgt(double x, double y) { return x >  y;  }
double eval_cle(double x, double y) { return x <= y;  }
double eval_cge(double x, double y) { return x >= y;  }
double eval_cne(double x, double y) { return x != y;  }

eval_expr_fn eval_op_lut[128] = {
    [0]   = &eval_default, // NUL null
    [1]   = &eval_default, // SOH start of header
    [2]   = &eval_default, // STX start of text
    [3]   = &eval_default, // ETX end of text
    [4]   = &eval_default, // EOT end of transmission
    [5]   = &eval_default, // ENQ enquiry
    [6]   = &eval_default, // ACK acknowledge
    [7]   = &eval_default, // BEL bell
    [8]   = &eval_default, // BS  backspace
    [9]   = &eval_default, // HT  horizontal tab
    [10]  = &eval_default, // LF  line feed
    [11]  = &eval_default, // VT  vertical tab
    [12]  = &eval_default, // FF  form feed
    [13]  = &eval_default, // CR  enter / carriage return
    [14]  = &eval_default, // SO  shift out
    [15]  = &eval_default, // SI  shift in
    [16]  = &eval_default, // DLE data link escape
    [17]  = &eval_default, // DC1 device control 1
    [18]  = &eval_default, // DC2 device control 2
    [19]  = &eval_default, // DC3 device control 3
    [20]  = &eval_default, // DC4 device control 4
    [21]  = &eval_default, // NAK negative acknowledge
    [22]  = &eval_default, // SYN synchronize
    [23]  = &eval_default, // ETB end of trans. block
    [24]  = &eval_default, // CAN cancel
    [25]  = &eval_default, // EM  end of medium
    [26]  = &eval_default, // SUB substitute
    [27]  = &eval_default, // ESC escape
    [28]  = &eval_default, // FS  file separator
    [29]  = &eval_default, // GS  group separator
    [30]  = &eval_default, // RS  record separator
    [31]  = &eval_default, // US  unit separator
    [32]  = &eval_default, // ' ' space
    [33]  = &eval_default, // !   exclamation mark
    [34]  = &eval_default, // "   double quote
    [35]  = &eval_default, // #   number
    [36]  = &eval_default, // $   dollar
    [37]  = &eval_mod,     // %   percent
    [38]  = &eval_default, // &   ampersand
    [39]  = &eval_default, // '   single quote
    [40]  = &eval_default, // (   left parenthesis
    [41]  = &eval_default, // )   right parenthesis
    [42]  = &eval_mul,     // *   asterisk
    [43]  = &eval_add,     // +   plus
    [44]  = &eval_default, // ,   comma
    [45]  = &eval_sub,     // -   minus
    [46]  = &eval_default, // .   period
    [47]  = &eval_div,     // /   slash
    [48]  = &eval_default, // 0   zero
    [49]  = &eval_default, // 1   one
    [50]  = &eval_default, // 2   two
    [51]  = &eval_default, // 3   three
    [52]  = &eval_default, // 4   four
    [53]  = &eval_default, // 5   five
    [54]  = &eval_default, // 6   six
    [55]  = &eval_default, // 7   seven
    [56]  = &eval_default, // 8   eight
    [57]  = &eval_default, // 9   nine
    [58]  = &eval_assign,  // :   colon
    [59]  = &eval_default, // ;   semicolon
    [60]  = &eval_clt,     // <   less than
    [61]  = &eval_ceq,     // =   equality sign
    [62]  = &eval_cgt,     // >   greater than
    [63]  = &eval_default, // ?   question mark
    [64]  = &eval_default, // @   at sign
    [65]  = &eval_default, // A
    [66]  = &eval_default, // B
    [67]  = &eval_default, // C
    [68]  = &eval_default, // D
    [69]  = &eval_default, // E
    [70]  = &eval_default, // F
    [71]  = &eval_default, // G
    [72]  = &eval_default, // H
    [73]  = &eval_default, // I
    [74]  = &eval_default, // J
    [75]  = &eval_default, // K
    [76]  = &eval_default, // L
    [77]  = &eval_default, // M
    [78]  = &eval_default, // N
    [79]  = &eval_default, // O
    [80]  = &eval_default, // P
    [81]  = &eval_default, // Q
    [82]  = &eval_default, // R
    [83]  = &eval_default, // S
    [84]  = &eval_default, // T
    [85]  = &eval_default, // U
    [86]  = &eval_default, // V
    [87]  = &eval_default, // W
    [88]  = &eval_default, // X
    [89]  = &eval_default, // Y
    [90]  = &eval_default, // Z
    [91]  = &eval_default, // [   left square bracket
    [92]  = &eval_default, // \   backslash
    [93]  = &eval_default, // ]   right square bracket
    [94]  = &eval_pow,     // ^   caret / circumflex
    [95]  = &eval_default, // _   underscore
    [96]  = &eval_default, // `   grave / accent
    [97]  = &eval_default, // a
    [98]  = &eval_default, // b
    [99]  = &eval_default, // c
    [100] = &eval_default, // d
    [101] = &eval_default, // e
    [102] = &eval_default, // f
    [103] = &eval_cge,     // g
    [104] = &eval_default, // h
    [105] = &eval_default, // i
    [106] = &eval_default, // j
    [107] = &eval_default, // k
    [108] = &eval_cle,     // l
    [109] = &eval_default, // m
    [110] = &eval_default, // n
    [111] = &eval_default, // o
    [112] = &eval_print,   // p
    [113] = &eval_default, // q
    [114] = &eval_default, // r
    [115] = &eval_default, // s
    [116] = &eval_default, // t
    [117] = &eval_default, // u
    [118] = &eval_default, // v
    [119] = &eval_default, // w
    [120] = &eval_default, // x
    [121] = &eval_default, // y
    [122] = &eval_default, // z
    [123] = &eval_default, // {   left curly bracket
    [124] = &eval_default, // |   vertical bar
    [125] = &eval_default, // }   right curly bracket
    [126] = &eval_cne,     // ~   tilde
    [127] = &eval_default  // DEL delete
};
