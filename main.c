#include <ncurses.h>
#include <locale.h>
#include <string.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 43

#define GRID_W 16
#define GRID_H 1
#define CELL_WIDTH 4
#define CELL_HEIGHT 2

#define GRID_ORIGIN_X 13
#define GRID_ORIGIN_Y 7

#define MAX_VALUE 32768

typedef enum {
    OP_NONE,
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_NOT,
    OP_SHIFT_LEFT,
    OP_SHIFT_RIGHT
} opt_t;

unsigned int calc_result(unsigned int curr, unsigned int opt, opt_t op)
{
    switch (op)
    {
    case OP_AND: return curr & opt;
    case OP_OR: return curr | opt;
    case OP_XOR: return curr ^ opt;

    // NOTE: this operation not calculate using opt value
    case OP_NOT: return ~curr;

    case OP_SHIFT_LEFT: return curr << 1;
    case OP_SHIFT_RIGHT: return curr >> 1;
    default: return 0;
    }
}

void draw_border(const char *title)
{
    for (int x = 1; x < SCREEN_WIDTH; ++x)
    {
        mvaddch(2, x, ACS_HLINE);
        mvaddch(SCREEN_HEIGHT, x, ACS_HLINE);
    }
    for (int x = 1; x < SCREEN_WIDTH; ++x)
    {
        mvaddch(28, x, ACS_HLINE);
        mvaddch(SCREEN_HEIGHT, x, ACS_HLINE);
    }
    for (int y = 1; y < SCREEN_HEIGHT; ++y)
    {
        mvaddch(y + 1, 0, ACS_VLINE);
        mvaddch(y + 1, SCREEN_WIDTH, ACS_VLINE);
    }

    mvaddch(2, 0, ACS_ULCORNER);
    mvaddch(2, SCREEN_WIDTH, ACS_URCORNER);
    mvaddch(SCREEN_HEIGHT, 0, ACS_LLCORNER);
    mvaddch(SCREEN_HEIGHT, SCREEN_WIDTH, ACS_LRCORNER);

    unsigned long title_len = strlen(title);
    unsigned long title_x = (SCREEN_WIDTH - title_len) / 2;

    mvprintw(2, (int)title_x - 2, "┤ %s ├", title);
    mvprintw(4, 4, "Imagine using 16bit processor.");
    mvprintw(6, 4, "- Computer read binary value, base-10 (1 & 0).");
    mvprintw(7, 4,
             "- We also have hexadicimal value, base-16 (0123456789ABCDEF).");
    mvprintw(8, 4,
             "- One thing to remember, all hardware counting from 0 not 1 !");
}

void draw_grid(int pos_y, const char *variable, unsigned int value)
{
    // draw upper
    mvaddch(GRID_ORIGIN_Y + pos_y, GRID_ORIGIN_X, ACS_ULCORNER);

    for (int i = 0; i < GRID_W; ++i)
    {
        int midline_x = GRID_ORIGIN_X + i * CELL_WIDTH;
        mvaddch(GRID_ORIGIN_Y + pos_y, midline_x + 1, ACS_HLINE);
        mvaddch(GRID_ORIGIN_Y + pos_y, midline_x + 2, ACS_HLINE);
        mvaddch(GRID_ORIGIN_Y + pos_y, midline_x + 3, ACS_HLINE);

        if (i < GRID_W - 1)
            mvaddch(GRID_ORIGIN_Y + pos_y,
                    GRID_ORIGIN_X + (1 + i) * CELL_WIDTH, ACS_TTEE);
    }

    mvaddch(GRID_ORIGIN_Y + pos_y, GRID_ORIGIN_X + GRID_W * CELL_WIDTH,
            ACS_URCORNER);

    // draw middle
    for (int i = 0; i < GRID_H; ++i)
    {
        int new_pos_y = GRID_ORIGIN_Y + pos_y + GRID_H + i;
        mvaddch(new_pos_y, GRID_ORIGIN_X, ACS_VLINE);
        mvaddch(new_pos_y, GRID_ORIGIN_X + GRID_W * CELL_WIDTH, ACS_VLINE);

        for (int j = 0; j < GRID_W; ++j)
        {
            mvaddch(new_pos_y, GRID_ORIGIN_X + j * CELL_WIDTH, ACS_VLINE);
        }
    }

    mvaddch(GRID_ORIGIN_Y + pos_y + CELL_HEIGHT, GRID_ORIGIN_X, ACS_LLCORNER);

    // draw bottom
    for (int i = 0; i < GRID_W; ++i)
    {
        int midline_x = GRID_ORIGIN_X + i * CELL_WIDTH;
        mvaddch(GRID_ORIGIN_Y + pos_y + CELL_HEIGHT, midline_x + 1, ACS_HLINE);
        mvaddch(GRID_ORIGIN_Y + pos_y + CELL_HEIGHT, midline_x + 2, ACS_HLINE);
        mvaddch(GRID_ORIGIN_Y + pos_y + CELL_HEIGHT, midline_x + 3, ACS_HLINE);

        if (i < GRID_W - 1)
            mvaddch(GRID_ORIGIN_Y + pos_y + CELL_HEIGHT,
                    GRID_ORIGIN_X + (1 + i) * CELL_WIDTH, ACS_BTEE);
    }

    mvaddch(GRID_ORIGIN_Y + pos_y + CELL_HEIGHT,
            GRID_ORIGIN_X + GRID_W * CELL_WIDTH, ACS_LRCORNER);

    // draw bits
    for (int i = 0; i < GRID_W; i++)
    {
        int bit = (value >> ((GRID_W - 1) - i)) & 1;

        // Position inside cell
        int x = (GRID_ORIGIN_X + CELL_HEIGHT) + (i * CELL_WIDTH);
        int y = GRID_ORIGIN_Y + pos_y + GRID_H;

        mvaddch(y, x, bit ? '1' : '0');
        if (bit == 1) mvaddch(y - 2, x, ACS_DARROW);

        if (i > GRID_W) i = 1;
    }

    mvprintw(GRID_ORIGIN_Y + pos_y + GRID_H, 3, "%s", variable);
}

void draw_param_value(int pos_y, const char *param, unsigned int value)
{
    int box_width = 26;

    mvprintw(pos_y, 3, "┌─ %s ──", param);
    for (int i = (int)strlen(param) + 6; i < box_width; i++)
    {
        mvaddch(pos_y, 3 + i, ACS_HLINE);
    }
    mvaddch(pos_y, 3 + box_width, ACS_URCORNER);

    mvprintw(pos_y + 1, 5, "Hex	: 0x%04X", value);
    mvprintw(pos_y + 2, 5, "Decimal	: %i", value);

    mvprintw(pos_y + 3, 3, "└");
    for (int i = 1; i < box_width; i++)
    {
        mvaddch(pos_y + 3, 3 + i, ACS_HLINE);
    }
    mvaddch(pos_y + 3, 3 + box_width, ACS_LRCORNER);
}

void draw_control(opt_t op)
{
    int sy = 10;
    int sx = 41;
    const char *op_names[] = {"", "AND", "OR", "XOR", "NOT", "<<", ">>"};
    const char *op_symbols[] = {"", "&", "|", "^", "~", "<<", ">>"};

    // clang-format off
    mvprintw(sy, sx,      "+-----------------------------------+");
    mvprintw(sy + 1, sx,  "|              CONTROLS             |");
    mvprintw(sy + 2, sx,  "|-----------+------------+----------|");
    mvprintw(sy + 3, sx,  "| a : AND   | x : XOR    | < : <<   |");
    mvprintw(sy + 4, sx,  "| o : OR    | n : NOT    | > : >>   |");
    mvprintw(sy + 5, sx,  "|-----------+------------+----------+");
    mvprintw(sy + 6, sx,  "| Left  : Curr << | Up   : Curr ++  |");
    mvprintw(sy + 7, sx,  "| Right : Curr >> | Down : Curr --  |");
    mvprintw(sy + 8, sx,  "|-----------------+-----------------|");
    mvprintw(sy + 9, sx,  "| j     : Opt <<  | i    : Opt ++   |");
    mvprintw(sy + 10, sx, "| l     : Opt >>  | k    : Opt --   |");
    mvprintw(sy + 11, sx, "|-----------------+-----------------|");
    mvprintw(sy + 12, sx, "| q     : Quit    | r    : Reset    |");
    mvprintw(sy + 13, sx, "+-----------------------------------+");


    mvprintw(sy + 15, 25, "+-------------------------------+");
    mvprintw(sy + 16, SCREEN_WIDTH / 3, "  Active Opt : %s | Symbol : %s ",op_names[op], op_symbols[op]);
    mvprintw(sy + 17, 25, "+-------------------------------+");
    // clang-format on
}

int main(void)
{
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    int ch;
    int first_grid = 24;

    opt_t curr_opt = OP_AND;
    unsigned int curr_value = 0;
    unsigned int opt_value = 0;

    start_color();

    while (1)
    {
        clear();
        draw_border("2AM Bit Vizualizer");

        if (curr_value > MAX_VALUE) curr_value = 1;
        if (opt_value > MAX_VALUE) opt_value = 1;

        unsigned int result_value =
            calc_result(curr_value, opt_value, curr_opt);

        draw_param_value(10, "CURRENT", curr_value);
        draw_param_value(15, "OPT", opt_value);
        draw_param_value(20, "RESULT", result_value);

        draw_control(curr_opt);

        draw_grid(first_grid, "Current", curr_value);
        draw_grid(first_grid + 4, "Operation", opt_value);
        draw_grid(first_grid + 8, "Result", result_value);

        refresh();

        ch = getch();
        if (ch == 'q') break;

        switch (ch)
        {
        case 'a': curr_opt = OP_AND; break;
        case 'o': curr_opt = OP_OR; break;
        case 'x': curr_opt = OP_XOR; break;
        case 'n': curr_opt = OP_NOT; break;
        case '<': curr_opt = OP_SHIFT_LEFT; break;
        case '>': curr_opt = OP_SHIFT_RIGHT; break;

        case KEY_LEFT: curr_value <<= 1; break;
        case KEY_RIGHT: curr_value >>= 1; break;
        case KEY_UP: curr_value++; break;
        case KEY_DOWN: curr_value--; break;

        case 'j': opt_value <<= 1; break;
        case 'l': opt_value >>= 1; break;
        case 'i': opt_value++; break;
        case 'k': opt_value--; break;

        case 'r': curr_value = 0; opt_value = 0;
        }
    }

    endwin();
    return 0;
}
