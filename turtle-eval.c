
// Turte-Graphics-Compiler:
// Interpreter f�r den Syntaxbaum
//
// Klaus Kusche 2021, 2022

#include <math.h>
#include <stdlib.h>
// F�r ptrdiff_t: Integer in der Gr��e der Differenz zweier Pointer
#include <stddef.h>
#include <assert.h>

#include "turtle.h"

#include "sdlinterf.h"

// RGB-Farbwerte der Default-Farbe, in der gezeichnet wird, 0 ... 100
#define RED 100
#define GREEN 100
#define BLUE 0

// Initiale Koordinatensystem-Gr��e (von -MAX_X/-MAX_Y bis MAX_X/MAX_Y)
// Seitenverh�ltnis automatisch vom SDL-Fenster �bernehmen!
#define MAX_X 20.0
#define MAX_Y ((MAX_X / SDL_X_SIZE) * SDL_Y_SIZE)

// Default-Wert f�r die Verz�gerung in ms nach jedem Zeichnen
#define WALK_DELAY 100
// Default-Wert f�r die Verz�gerung in ms beim Beenden des Programmes
#define END_DELAY 2000

// Speicherpl�tze f�r die vordefinierten globalen Variablen
// (der Wert selbstdefinierter globaler Variablen steht in der Namenstabelle)
double g_dir = 0;        // Richtung (in Grad, wie in Mathe:
// 0 ist nach rechts, gegen den Uhrzeigersinn)
double g_dist = 0;       // Abstand vom Ursprung
double g_x = 0, g_y = 0; // Aktuelle x- und y-Position
double g_args[10];       // argv[1]...argv[9] als double
// g_args[0] bleibt unbenutzt
double g_pi = M_PI;      // die Konstante Pi
double g_max_x = MAX_X, g_max_y = MAX_Y;
// Gr��e des Fensters in Benutzer-Koordinaten
double g_delay = WALK_DELAY;  // Verz�gerung in ms nach jedem Zeichnen
double g_red = RED, g_green = GREEN, g_blue = BLUE; // Farbe des Striches

// Typ eines Eintrags im Stack der lokalen Variablen und Parameter
typedef struct {
    const nameentry_t *name; // Pointer auf den Namenstabellen-Eintrag
    double val;              // Wert dieser Instanz der Variable
} vstack_elem_t;

// Initiale Gr��e des Variablen-Stacks (Anzahl Variablen)
// (wird bei Bedarf mit realloc vergr��ert)
#define STACK_INIT_SIZE 100

// Pointer auf den Anfang (unteres Ende) des Variablen- und Parameter-Stacks
static vstack_elem_t *stack = NULL;
// Pointer auf das aktuelle Top-Element
static vstack_elem_t *stack_top = NULL;
// Pointer hinter das Ende des derzeit angelegten Stacks
static vstack_elem_t *stack_end = NULL;

// Eine Markierung
typedef struct {
    double x, y;  // Koordinaten
    double dir;   // Richtung
} mstack_elem_t;

// Initiale Gr��e des Markierungs-Stacks (Anzahl Markierungen)
// (wird bei Bedarf mit realloc vergr��ert)
#define MARK_INIT_SIZE 100

// Pointer auf die unterste Markierung am Stack (Stack-Anfang, immer 0/0)
static mstack_elem_t *first_mark = NULL;
// Pointer auf die oberste Markierung am Stack
static mstack_elem_t *top_mark = NULL;
// Pointer hinter das Ende des derzeit angelegten Stacks
static mstack_elem_t *end_marks = NULL;

// Source-Position Zeile 1 / Spalte 1 f�r Fehlermeldungen beim Initialisieren
static const srcpos_t startpos = {1, 1};


// Hauptfunktionen des Interpreters

// Rechne eine Rechnung aus
static double expr(const treenode_t *t);

// Rechne eine Bedingung aus
static bool cond(const treenode_t *t);

// Arbeite eine Liste von Statements ab
static void slist(const treenode_t *t);

// Allgemeine Hilfsfunktionen

// Mach aus Winkelgrad Bogenma�
static double to_rad(double x);

// Normalisiere die Richtung auf 0...360
static double to_range(double x);

// Mach aus der double-Farbe 0...100 eine int-Farbe 0...255
static int to_color(double x);

// Mach aus unseren Koordinaten (0/0 ist Bildmitte, +y ist oben)
// Fensterkoordinaten (0/0 ist links oben)
// Ergebnis wird in xp bzw. yp gespeichert
// Returnwert true bei Erfolg, false wenn offscreen
static bool to_x_pixel(double x, int *xp);

static bool to_y_pixel(double y, int *yp);

// Geh von der aktuellen Position zu Position x/y
// t ist keyw_walk oder keyw_jump ==> mit / ohne Strich zeichnen
static void walk(double x, double y, type_t t);

// "gleich +- Rundungsfehler"-Vergleich
static bool eps_equal(double x, double y);

// Beende das Programm normal (END_DELAY warten, Grafik schlie�en)
static void end_prog(void);

// Hilfsfunktionen f�r Variablen

// Liefert einen Pointer auf den Wert der Variablen t
// Bei lokalen Variablen:
// Eine Instanz von t im aktuellen Stack-Frame muss bereits existieren
// ro: Wird die Variable im Aufrufer nur gelesen oder auch geschrieben?
static double *var_ptr(const treenode_t *t, bool ro);

// Rechnet exp aus und speichert das Ergebnis in der Variable t
// Liefert einen Pointer auf ihren Wert
// Bei lokalen Variablen:
// Gibt es die Variable schon im lokalen Stackframe,
// wird die bestehende Instanz verwendet, sonst eine neue angelegt
static double *set_var(const treenode_t *t, const treenode_t *exp);

// Hilfsfunktionen f�r Funktionen & Stack der Variablen und Parameter

// Initialisiert den Stack
static void init_stack(void);

// Legt einen Eintrag auf den Stack
// Macht den Stack bei Bedarf gr��er
// Mit var gleich NULL auch f�r den Funktions-Trenn-Eintrag verwendet
static void push(const nameentry_t *var, double val, const srcpos_t *pos);

// Anzahl der Parameter einer Funktion
static int param_cnt(const funcdef_t *f);

// Pr�ft, ob die Argument-Anzahl im Knoten t gleich expected ist
// Wenn nicht: Fehlermeldung ausgeben und Programm beenden
static void check_arg_cnt(const treenode_t *t, int expected);

// Beginnt einen Funktions- oder Pfadaufruf:
// - Variablen-Stackframe anlegen
// - Parameter ausrechnen und anlegen
static void start_call(const funcdef_t *f, const treenode_t *t);

// R�umt den Var-Stack am Ende eines Funktions- oder Pfadaufrufs wieder auf:
// Entfernt die lokalen Variablen und Parameter vom Stack
static void end_call(void);

// F�hrt einen Calc-Funktions-Aufruf aus
static double fcall(const treenode_t *t);

// F�hrt einen Pfad-Funktions-Aufruf aus
static void pcall(const treenode_t *t);

// Stack der Positions-Marken

// Initialisiert den Stack
static void init_marks(void);

// Speichert die aktuelle Position auf dem Stack
static void push_mark(const srcpos_t *pos);

// Entfernt die oberste Marke vom Stack
// (au�er wir sind schon bei der letzten Markierung am Stack)
static void pop_mark(void);


// Allgemeine Hilfsfunktionen

// Mach aus Winkelgrad Bogenma�
static double to_rad(double x) {
    return (x * (2 * M_PI / 360));
}

// Normalisiere die Richtung auf 0...360
static double to_range(double x) {
    x = fmod(x, 360);
    if (x < 0) { x += 360; }
    return x;
}

// Mach aus der double-Farbe 0...100 eine int-Farbe 0...255
static int to_color(double x) {
    if (x <= 0) return 0;
    if (x >= 100) return 255;
    return lround(x / 100 * 255);
}

// Mach aus unseren double-Koordinaten (0/0 ist Bildmitte, +y ist oben)
// int-Fensterkoordinaten (0/0 ist links oben)
// Ergebnis wird in xp bzw. yp gespeichert
// Returnwert true bei Erfolg, false wenn offscreen
static bool to_x_pixel(double x, int *xp) {
    *xp = lround((SDL_X_SIZE - 1) * (g_max_x + x) / (2 * g_max_x));
    return ((*xp >= 0) && (*xp < SDL_X_SIZE));
}

static bool to_y_pixel(double y, int *yp) {
    // Achtung: Vorzeichen umdrehen!
    *yp = lround((SDL_Y_SIZE - 1) * (g_max_y - y) / (2 * g_max_y));
    return ((*yp >= 0) && (*yp < SDL_Y_SIZE));
}

// Geh von der aktuellen Position zu Position x/y
// t ist keyw_walk oder keyw_jump ==> mit / ohne Strich zeichnen
static void walk(double x, double y, type_t t) {
    //printf("%f/%f - %f/%f %d\n", g_x, g_y, x, y, t == keyw_walk);
    if (t == keyw_walk) {
        int x1, y1, x2, y2;
        if (to_x_pixel(g_x, &x1) && to_y_pixel(g_y, &y1) &&
            to_x_pixel(x, &x2) && to_y_pixel(y, &y2)) {
            sdlDrawLine(x1, y1, x2, y2,
                        to_color(g_red), to_color(g_green), to_color(g_blue));
            sdlUpdate();
            sdlMilliSleep(g_delay);
        } else {
            printf("Line %.2f/%.2f - %.2f/%.2f is offscreen\n", g_x, g_y, x, y);
        }
    }
    g_x = x;
    g_y = y;
    g_dist = sqrt(x * x + y * y);
}

// "gleich +- Rundungsfehler"-Vergleich
static bool eps_equal(double x, double y) {
    double eps = fabs(x) * 1e-13;
    if (eps < 1e-20) { eps = 1e-20; }
    return fabs(x - y) <= eps;
}

// Beende das Programm normal (END_DELAY warten, Grafik schlie�en)
static void end_prog(void) {
    sdlMilliSleep(END_DELAY);
    sdlExit();
    exit(EXIT_SUCCESS);
}


// Funktionen f�r Variablen

// Liefert einen Pointer auf den Wert der Variablen t
// Bei lokalen Variablen:
// Eine Instanz von t im aktuellen Stack-Frame muss bereits existieren
// ro: Wird die Variable im Aufrufer nur gelesen oder auch geschrieben?
static double *var_ptr(const treenode_t *t, bool ro) {
    nameentry_t *n = t->d.p_name;
    switch (n->type) {
        case name_var:
            // Stack vom Top abw�rts durchsuchen,
            // h�chstens bis zum NULL-Eintrag,
            // der die Variablen und Parameter der aktuellen Funktion begrenzt
            for (vstack_elem_t *p = stack_top; p->name != NULL; --p) {
                if (p->name == n) {
                    return &(p->val);
                }
            }
            code_error(&(t->pos), "Variable %s hat noch keinen Wert\n", n->name);
            break;
        case name_glob:
            return &(n->d.val);
            break;
        case name_pvar_ro:
            assert(ro);
            return n->d.p_val;
            break;
        case name_pvar_rw:
            return n->d.p_val;
            break;
        default:
            assert(false);
    }
}

// Rechnet exp aus und speichert das Ergebnis in der Variable t
// Liefert einen Pointer auf ihren Wert
// Bei lokalen Variablen:
// Gibt es die Variable schon im lokalen Stackframe,
// wird die bestehende Instanz verwendet, sonst eine neue angelegt
static double *set_var(const treenode_t *t, const treenode_t *exp) {
    nameentry_t *n = t->d.p_name;
    double x = expr(exp);
    switch (n->type) {
        case name_var:
            // Stack vom Top abw�rts durchsuchen,
            // h�chstens bis zum NULL-Eintrag,
            // der die Variablen und Parameter der aktuellen Funktion begrenzt
            for (vstack_elem_t *p = stack_top; p->name != NULL; --p) {
                if (p->name == n) {
                    p->val = x;
                    return &(p->val);
                }
            }
            // Die Variable gibt es im Stackframe noch nicht: Anlegen
            push(n, x, &(t->pos));
            return &(stack_top->val);
            break;
        case name_glob:
            n->d.val = x;
            return &(n->d.val);
            break;
        case name_pvar_ro:
            assert(false);  // readonly!
            break;
        case name_pvar_rw:
            *(n->d.p_val) = x;
            return n->d.p_val;
            break;
        default:
            assert(false);
    }
}


// Funktionen f�r Funktionen

// Initialisiert den Stack
static void init_stack(void) {
    stack = stack_top = malloc(STACK_INIT_SIZE * sizeof(vstack_elem_t));
    mem_check(stack, "die ersten Variablen", &startpos);
    stack_end = stack + STACK_INIT_SIZE;
    // der Stack enth�lt am Boden immer ein NULL-Element
    stack_top->name = NULL;
    stack_top->val = 0;
}

// Legt einen Eintrag auf den Stack
// Macht den Stack bei Bedarf gr��er
// Mit var gleich NULL auch f�r den Funktions-Trenn-Eintrag verwendet
static void push(const nameentry_t *var, double val, const srcpos_t *pos) {
    if (var != NULL) assert(var->type == name_var);
    ++stack_top;
    if (stack_top == stack_end) {
        // Stack ist voll, Gr��e verdoppeln
        ptrdiff_t fill = stack_top - stack;
        ptrdiff_t size = 2 * fill;
        stack = realloc(stack, size * sizeof(vstack_elem_t));
        mem_check(stack, "weitere Variablen", pos);
        stack_top = stack + fill;
        stack_end = stack + size;
    }
    stack_top->name = var;
    stack_top->val = val;
}

// Anzahl der Parameter einer Funktion
static int param_cnt(const funcdef_t *f) {
    for (int i = 0;; ++i) {
        if ((i == MAX_ARGS) || (f->params[i] == NULL)) {
            return i;
        }
    }
}

// Pr�ft, ob die Argument-Anzahl im Knoten t gleich expected ist
// Wenn nicht: Fehlermeldung ausgeben und Programm beenden
static void check_arg_cnt(const treenode_t *t, int expected) {
    for (int actual = 0;; ++actual) {
        if ((actual == MAX_ARGS) || (t->son[actual] == NULL)) {
            if (actual != expected) {
                code_error(&(t->pos), "Die Anzahl der Werte im Auruf (%d) "
                                      "passt nicht zur Anzahl der Parameter (%d)",
                           actual, expected);
            }
            return;
        }
    }
}

// Beginnt einen Funktions- oder Pfadaufruf:
// - Variablen-Stackframe anlegen
// - Parameter ausrechnen und anlegen
static void start_call(const funcdef_t *f, const treenode_t *t) {
    int cnt = param_cnt(f);
    check_arg_cnt(t, cnt);

    // zuerst alle Argumente im *alten* Stack-Frame ausrechnen
    double args[MAX_ARGS];
    for (int i = 0; i < cnt; ++i) {
        args[i] = expr(t->son[i]);
    }
    // Ein Stack-Eintrag mit Namenspointer NULL beginnt ein neues Stack-Frame
    push(NULL, 0, &(t->pos));
    // dann die Parameter auf dem Stack anlegen
    for (int i = 0; i < cnt; ++i) {
        push(f->params[i], args[i], &(t->pos));
    }
}

// R�umt den Var-Stack am Ende eines Funktions- oder Pfadaufrufs wieder auf:
// Entfernt die lokalen Variablen und Parameter vom Stack
static void end_call(void) {
    // Entferne alle Eintr�ge des aktuellen Stack-Frames bis zum NULL-Element
    while (stack_top->name != NULL) {
        --stack_top;                  // vom Stack entfernen
    }
    // Ein Eintrag ohne Namen markiert das Ende des aktuellen Stack-Frames
    assert(stack_top > stack);  // das unterste Element muss bleiben!
    --stack_top;                // Markierungselement auch entfernen
}

// F�hrt einen Calc-Funktions-Aufruf aus
static double fcall(const treenode_t *t) {
    const nameentry_t *n = t->d.p_name;
    switch (n->type) {
        case name_calc: {
            const funcdef_t *fdef = n->d.func;
            start_call(fdef, t);
            if (fdef->body != NULL) slist(fdef->body);
            double result = expr(fdef->ret);
            end_call();
            return result;
            break;
        }
        case name_math_sin:
        case name_math_cos:
        case name_math_tan:
            check_arg_cnt(t, 1);
            return (n->d.math)(expr(t->son[0]) * (2 * M_PI / 360));
            break;
        case name_math_sqrt:
            check_arg_cnt(t, 1);
            return (n->d.math)(expr(t->son[0]));
            break;
        case name_math_rand: {
            check_arg_cnt(t, 2);
            double lower = expr(t->son[0]);
            double upper = expr(t->son[1]);
            double r = rand() / ((double) RAND_MAX);
            return lower + r * (upper - lower);
            break;
        }
        default:
            assert(false);
    }
}

// F�hrt einen Pfad-Funktions-Aufruf aus
static void pcall(const treenode_t *t) {
    const nameentry_t *n = t->d.p_name;
    assert(n->type == name_path);
    const funcdef_t *fdef = n->d.func;
    start_call(fdef, t);
    slist(fdef->body);
    assert(fdef->ret == NULL);
    end_call();
}


// Funktionen f�r den Markierungs-Stack

// Initialisiert den Stack
static void init_marks(void) {
    first_mark = top_mark = malloc(MARK_INIT_SIZE * sizeof(mstack_elem_t));
    mem_check(first_mark, "die ersten Wegmarkierungen", &startpos);
    end_marks = first_mark + MARK_INIT_SIZE;
    // Der Mark-Stack enth�lt am Boden immer die Position 0/0
    top_mark->x = 0;
    top_mark->y = 0;
    top_mark->dir = 0;
}

// Speichert die aktuelle Position auf dem Stack
static void push_mark(const srcpos_t *pos) {
    ++top_mark;
    if (top_mark == end_marks) {
        // Stack ist voll, Gr��e verdoppeln
        ptrdiff_t fill = top_mark - first_mark;
        ptrdiff_t size = 2 * fill;
        first_mark = realloc(first_mark, size * sizeof(mstack_elem_t));
        mem_check(first_mark, "weitere Weg-Markierungen", pos);
        top_mark = first_mark + fill;
        end_marks = first_mark + size;
    }
    top_mark->x = g_x;
    top_mark->y = g_y;
    top_mark->dir = g_dir;
}

// Entfernt die oberste Marke vom Stack
// (au�er wir sind schon bei der letzten Markierung am Stack)
static void pop_mark(void) {
    if (top_mark > first_mark) --top_mark;  // kein pop bei leerem Stack
}


// Rechne eine Rechnung aus
static double expr(const treenode_t *t) {
    assert(t);
    switch (t->type) {
        case name_any:
            return *(var_ptr(t, true));
        case oper_const:
            return t->d.val;
        case oper_neg:
            return -expr(t->son[0]);
        case oper_pow:
            return pow(expr(t->son[0]), expr(t->son[1]));
        case oper_mul:
            return expr(t->son[0]) * expr(t->son[1]);
        case oper_div:
            return expr(t->son[0]) / expr(t->son[1]);
        case oper_add:
            return expr(t->son[0]) + expr(t->son[1]);
        case oper_sub:
            return expr(t->son[0]) - expr(t->son[1]);
        case oper_abs:
            return fabs(expr(t->son[0]));
        case oper_lpar:
            return fcall(t);
        default:
            assert(false);
    }
}

// Rechne eine Bedingung aus
static bool cond(const treenode_t *t) {
    assert(t);
    switch (t->type) {
        case oper_equ:
            return eps_equal(expr(t->son[0]), expr(t->son[1]));
        case oper_nequ:
            return !eps_equal(expr(t->son[0]), expr(t->son[1]));
        case oper_less:
            return expr(t->son[0]) < expr(t->son[1]);
        case oper_lequ:
            return expr(t->son[0]) <= expr(t->son[1]);
        case oper_grtr:
            return expr(t->son[0]) > expr(t->son[1]);
        case oper_gequ:
            return expr(t->son[0]) >= expr(t->son[1]);
        case keyw_not:
            return !cond(t->son[0]);
        case keyw_and:
            return cond(t->son[0]) && cond(t->son[1]);
        case keyw_or:
            return cond(t->son[0]) || cond(t->son[1]);
        default:
            assert(false);
    }
}

// Arbeite eine Liste von Statements ab
static void slist(const treenode_t *t) {
    assert(t);
    for (; t; t = t->next) {
        switch (t->type) {
            case keyw_walk:
            case keyw_jump:
                if (t->d.walk == keyw_home) {
                    walk(0, 0, t->type);
                    g_dir = 0;
                } else if (t->d.walk == keyw_mark) {
                    walk(top_mark->x, top_mark->y, t->type);
                    g_dir = top_mark->dir;
                    pop_mark();
                } else {
                    double dist = expr(t->son[0]);
                    if (t->d.walk == keyw_back) {
                        dist = -dist;
                    } else {
                        assert(t->d.walk == keyw_walk);
                    }
                    walk(g_x + dist * cos(to_rad(g_dir)),
                         g_y + dist * sin(to_rad(g_dir)),
                         t->type);
                }
                break;
            case keyw_left:
                g_dir += expr(t->son[0]);
                g_dir = to_range(g_dir);
                break;
            case keyw_right:
                g_dir -= expr(t->son[0]);
                g_dir = to_range(g_dir);
                break;
            case keyw_direction:
                g_dir = to_range(expr(t->son[0]));
                break;
            case keyw_clear:
                sdlSetBlack();
                sdlUpdate();
                sdlMilliSleep(g_delay);
                break;
            case keyw_stop:
                // endlos kurze MilliSleep's, damit man das Programm abbrechen kann
                // (ein langes MilliSleep w�re nicht unterbrechbar)
                for (;;) {
                    sdlMilliSleep(100);
                }
                break;
            case keyw_finish:
                end_prog();
                break;
            case keyw_mark:
                push_mark(&(t->pos));
                break;
            case keyw_path:
                pcall(t);
                break;
            case keyw_color:
                g_red = expr(t->son[0]);
                g_green = expr(t->son[1]);
                g_blue = expr(t->son[2]);
                break;
            case keyw_store:
                set_var(t, t->son[0]);
                break;
            case keyw_add:
                *(var_ptr(t, false)) += expr(t->son[0]);
                break;
            case keyw_sub:
                *(var_ptr(t, false)) -= expr(t->son[0]);
                break;
            case keyw_mul:
                *(var_ptr(t, false)) *= expr(t->son[0]);
                break;
            case keyw_div:
                *(var_ptr(t, false)) /= expr(t->son[0]);
                break;
            case keyw_if:
                if (cond(t->son[0])) {
                    slist(t->son[1]);
                } else {
                    if (t->son[2] != NULL) {
                        slist(t->son[2]);
                    }
                }
                break;
            case keyw_do:
                for (int i = expr(t->son[0]); i > 0; --i) {
                    slist(t->son[1]);
                }
                break;
            case keyw_counter: {
                double *p_lvar = set_var(t, t->son[0]);
                if (t->son[2] == NULL) {
                    for (;;) {
                        if (*p_lvar > expr(t->son[1])) break;
                        slist(t->son[4]);
                        *p_lvar += (t->son[3] == NULL) ? 1 : expr(t->son[3]);
                    }
                } else {
                    assert(t->son[1] == NULL);
                    for (;;) {
                        if (*p_lvar < expr(t->son[2])) break;
                        slist(t->son[4]);
                        *p_lvar -= (t->son[3] == NULL) ? 1 : expr(t->son[3]);
                    }
                }
                break;
            }
            case keyw_while:
                while (cond(t->son[0])) {
                    slist(t->son[1]);
                }
                break;
            case keyw_repeat:
                do {
                    slist(t->son[1]);
                } while (!cond(t->son[0]));
                break;
            default:
                assert(false);
        }
    }
}

// Die Hauptfunktion des Evaluators:
// Initialisiere die Verarbeitung und die Grafik
// und arbeite den Syntaxbaum main_tree f�r das Haupprogramm ab
// arg_cnt/arg_val ist der Teil von argc/argv, der in @1, @2, ... geh�rt
// (ohne Programmname und Sourcefile-Name)
void evaluate(const treenode_t *main_tree, int arg_cnt, const char *arg_val[]) {
    for (int i = 0; i < arg_cnt; ++i) {
        g_args[i + 1] = atof(arg_val[i]);  // g_args beginnen bei 1
        // restliche g_args sind schon auf 0 initialisiert
    }

    init_stack();
    init_marks();
    sdlInit();

    slist(main_tree);

    end_prog();
}
