#include <errno.h>
#include <math.h>
#include <signal.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <asm-generic/ioctls.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <emgc.h>

#include "general.h"
#include "../emsh.h"
#include "../environment.h"
#include "../helpers.h"

// Struct for holding cursor information
struct Cursor {
    unsigned int row;
    unsigned int row_tot;
    unsigned short col;
    unsigned short col_home;
    unsigned int index;
};
// Struct for holding input buffer and related information
struct InBuf {
    char *buf;
    unsigned int buf_size;
    unsigned int buf_len;
    unsigned int no_chars;
};

// Globals
const char *PS1;
const char *PS2;
const char *IFS;

// Local  
// Returns terminal width in number of columns
void terminal_get_size(struct winsize *term){
    if (ioctl(STDERR_FILENO, TIOCGWINSZ, term) == -1){
        perror("emsh: could not get terminal size");
        term->ws_row = 0;
    }
}

// Local  
// Return local code for escape sequence seq
int escape_sequence_parse(char *seq){
    if (strcmp(seq, "\033[1~") == 0 || strcmp(seq, "\033[H") == 0){ // Home
        return 1;
    }
    if (strcmp(seq, "\033[2~") == 0){ // Insert
        return 2;
    }
    if (strcmp(seq, "\033[3~") == 0 || strcmp(seq, "\033[P") == 0){ // Delete
        return 3;
    }
    if (strcmp(seq, "\033[4~") == 0 || strcmp(seq, "\033[F") == 0){ // End
        return 4;
    }
    if (strcmp(seq, "\033[C") == 0 || strcmp(seq, "\033OC") == 0){ // Arrow right
        return 67;
    }
    if (strcmp(seq, "\033[D") == 0 || strcmp(seq, "\033OD") == 0){ // Arrow left
        return 68;
    }
    for (int i = strlen(seq) - 1; i >= 0; i--){
        seq[i + 1] = seq[i];
    }
    seq[0] = '^';
    seq[1] = '[';
    return 0;
}

// Local  
// Clear row, write s to screen, move cursor back curs_offset steps
void stdin_reprint(struct InBuf *input, struct Cursor *cursor){
    curs_home();
    erase_scrfw();
    fprintf(stderr, "%s", input->buf);
    if (cursor->col == 1 && cursor->row > 1){
        fprintf(stderr, "\n");
    }
    if (cursor->row != cursor->row_tot){
        curs_up(cursor->row_tot - cursor->row);
    }
    curs_goto_col(cursor->col);
}

// Local  
// Set terminal to raw mode  
// term_cur is a struct storing settings for the terminal before going raw  
// Returns 0 on success and -1 on failure
int terminal_set_raw(struct termios *term_cur){
    if (tcgetattr(STDIN_FILENO, term_cur) == -1){
        perror("emsh: could not get terminal attributes");
        return -1;
    }
    struct termios term_raw = *term_cur;
    term_raw.c_lflag &= ~(ICANON | ECHO | ISIG);
    term_raw.c_cc[VMIN] = 1;
    term_raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &term_raw) == -1){
        perror("emsh: could not set terminal attributes");
        return -1;
    }
    return 0;
}

// Local  
// Restore terminal from raw mode  
// term_info is a struct with the requested settings  
// Returns 0 on success and -1 on failure
void terminal_restore(const struct termios *term_info){
    if (tcsetattr(STDIN_FILENO, TCSANOW, term_info) == -1){
        char **tmp = gc_malloc('l', 1 * sizeof(char));
        *tmp = string_copy("reset", 'l');
        program_exec(1, tmp);
        perror("emsh: could not set terminal attributes\n");
        flag.failure = true;
        return;
    }
    return;
}

// Local  
// Get cursor position  
// Updates col in cursor, returns row
unsigned int cursor_get_pos(struct Cursor *cursor, unsigned short term_w){
    if (term_w == 0){
        cursor->row = 0;
        return 0;
    }
    if (term_w < cursor->col_home || cursor->col_home == 0){
        short temp_col = strlen(PS1) + 1;
        while (temp_col > term_w){
            temp_col -= term_w;
        }
        cursor->col_home = temp_col;
    }
    char pos_response[12];
    int index = 0;
    fprintf(stderr, "\033[6n");
    do {
        pos_response[index] = getc(stdin);
    } while (pos_response[index++] != 'R');
    pos_response[index--] = '\0';
    unsigned int row = 0;
    for (int i = index; i > 0; i--){
        if (pos_response[i] == ';'){
            for (int j = i; j > 0; j--){
                if (pos_response[j] == '['){
                    j++;
                    while (j <= i - 1){
                        row += ((int) pos_response[j] - 48) * pow(10, (i - 1) - j);
                        j++;
                    }
                    break;
                }
            }
            i++;
            int end = strlen(pos_response) - 2;
            cursor->col = 0;
            while (i <= end){
                cursor->col += ((int) pos_response[i] - 48) * pow(10, end - i);
                i++;
            }
            break;
        }
    }
    return row;
}

// Local
// Handler for when the terminal is resized
void terminal_resize_react(struct winsize *term, struct Cursor * cursor, struct InBuf *input){
    if (term->ws_row == 0 || cursor->row == 0){
        flag.failure = true;
        return;
    }
    long temp_no_chars = input->no_chars;
    cursor->row = 1;
    unsigned int temp_row_tot = 1;
    temp_no_chars -= (term->ws_col - (cursor->col_home - 1));
    while (temp_no_chars >= 0){
        temp_row_tot++;
        temp_no_chars -= term->ws_col;
    }
    unsigned int row_real = cursor_get_pos(cursor, term->ws_col);
    for (unsigned int i = cursor->row_tot; i < temp_row_tot; i++){
        if (row_real == term->ws_row){
            fprintf(stderr, "\n");
        }
        else {
            curs_down(1);
            row_real++;
        }
    }
    for (unsigned int i = cursor->row_tot; i > temp_row_tot; i--){
        curs_up(1);
    }
    cursor->row_tot = temp_row_tot;
    unsigned int temp_col = cursor->col_home;
    unsigned int temp_index = 0;
    short temp_row = 1;
    while (temp_index < cursor->index){
        temp_col++;
        temp_index++;
        while (((unsigned char) input->buf[cursor->index] & 0xC0) == 0x80){
            temp_index++;
        }
        if (temp_col > term->ws_col){
            temp_col = 1;
            temp_row++;
        }
    }
    cursor->row = temp_row;
    cursor->col = temp_col;
    if (cursor->row > 1){
        curs_up(cursor->row - 1);
    }
    curs_goto_col(cursor->col_home);
    curs_save();
    stdin_reprint(input, cursor);
}

// Local  
// Functions for handling terminal signals
static volatile sig_atomic_t resized;
void on_winch(int sig){
    (void)sig;
    resized = 1;
}
void terminal_signal_set(){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_winch;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGWINCH, &sa, NULL);
}

// Local  
// Read from terminal char by char  
// Returns number of arguments and populates in_v with arguments
char **input_read_vi(char **in_v, int *in_c){
    // Set up terminal
    struct termios term_info;
    if (terminal_set_raw(&term_info) == -1){
        fprintf(stderr, "emsh: could not run in vi mode");
        option.vi = false;
        return nullptr;
    }
    terminal_signal_set();

    // Init
    struct InBuf input = {.buf_size = 64, .buf_len = 0, .no_chars = 0, .buf = nullptr};
    input.buf = gc_malloc('l', input.buf_size * sizeof(char));
    input.buf[0] = '\0';
    struct winsize term;
    terminal_get_size(&term);
    if (term.ws_row == 0){
        flag.failure = true;
    }
    if (PS1){
        fprintf(stderr, "%s", PS1);
    }
    struct Cursor cursor = {.col = 0, .col_home = 0, .index = 0, .row = 1, .row_tot = 1};
    (void) cursor_get_pos(&cursor, term.ws_col);
    if (cursor.row == 0){
        flag.failure = true;
    }
    char c;
    bool insert_mode = false;
    bool sigint = false;
    curs_save();

    // Read loop
    while (!flag.failure && !sigint && (c = getc(stdin)) != '\n'){
        // Handle input char
        char in_char_buf[16] = {'\0'};
        in_char_buf[0] = c;
        if (in_char_buf[0] == '\033'){ // Escape sequence
            while ((in_char_buf[strlen(in_char_buf) - 1] < '@' || in_char_buf[strlen(in_char_buf) - 1] > '~') || strlen(in_char_buf) <= 2){
                in_char_buf[strlen(in_char_buf)] = getc(stdin);
            }
        }
        else if (in_char_buf[0] >= '\xC2' && in_char_buf[0] <= '\xFF'){ // UTF-8
            in_char_buf[strlen(in_char_buf)] = getc(stdin);
            if (in_char_buf[0] >= '\xE0'){
                in_char_buf[strlen(in_char_buf)] = getc(stdin);
            }
            if (in_char_buf[0] >= '\xF0'){
                in_char_buf[strlen(in_char_buf)] = getc(stdin);
            }
        }
        unsigned int in_char_buf_len = 0;

        // SC for keypress
        switch (in_char_buf[0]){
        case EOF: // End of file / Terminal resize
            if (ferror(stdin) && errno == EINTR){
                clearerr(stdin);
                if (resized){
                    terminal_get_size(&term);
                    terminal_resize_react(&term, &cursor, &input);
                }
                break;
            }
            // TODO: Actual EOF handling
            break;
        case '\034': // SIGQUIT
            // TODO: SIGQUIT and SIGTERM shall be ignored by POSIX.1-2024
            flag.exit = true;
            // fall through
        case '\003': // SIGINT
            input.buf[0] = '\0';
            input.buf_len = 0;
            input.no_chars = 0;
            sigint = true;
            break;
        case '\033': // Escape sequence
            switch (escape_sequence_parse(in_char_buf)){
            case 0: // Not supported
                in_char_buf_len = strlen(in_char_buf);
                break;
            case 1: // Home
                curs_home();
                cursor.col = cursor.col_home;
                cursor.row = 1;
                cursor.index = 0;
                break;
            case 2: // Insert
                if (insert_mode){
                    insert_mode = false;
                }
                else {
                    insert_mode = true;
                }
                break;
            case 3: // Delete
                if (cursor.index != input.buf_len){
                    cursor.index++;
                    cursor.col++;
                    if (cursor.col > term.ws_col){
                        curs_down(1);
                        curs_sol();
                        cursor.col -= term.ws_col;
                        cursor.row++;
                    }
                    else {
                        curs_right(1);
                    }
                    while (((unsigned char) input.buf[cursor.index] & 0xC0) == 0x80){
                        cursor.index++;
                    }
                    goto case_backspace;
                }
                break;
            case 4: // End
                if (input.buf_len == 0 || cursor.index == input.buf_len){
                    break;
                }
                if (cursor.row_tot == 1){
                    curs_goto_col(cursor.col_home + input.no_chars);
                    cursor.col = cursor.col_home + input.no_chars;
                    cursor.index = input.buf_len;
                    break;
                }
                if (cursor.row != cursor.row_tot){
                    curs_down(cursor.row_tot - cursor.row);
                    cursor.row = cursor.row_tot;
                }
                unsigned short col_temp = input.no_chars;
                col_temp -= (cursor.row_tot - 2) * term.ws_col;
                col_temp -= term.ws_col - cursor.col_home;
                curs_goto_col(col_temp);
                cursor.col = col_temp;
                cursor.index = input.buf_len;
                break;
            case 67: // Arrow right
                if (cursor.index < input.buf_len){
                    cursor.index++;
                    cursor.col++;
                    if (cursor.col > term.ws_col){
                        curs_down(1);
                        curs_sol();
                        cursor.col -= term.ws_col;
                        cursor.row++;
                    }
                    else {
                        curs_right(1);
                    }
                    while (((unsigned char) input.buf[cursor.index] & 0xC0) == 0x80){
                        cursor.index++;
                    }
                }
                break;
            case 68: // Arrow left
                if (cursor.index > 0){
                    cursor.index--;
                    cursor.col--;
                    if (cursor.col == 0 && cursor.row != 1){
                        curs_up(1);
                        curs_eol();
                        cursor.col = term.ws_col;
                        cursor.row--;
                    }
                    else {
                        curs_left(1);
                    }
                    while (((unsigned char) input.buf[cursor.index] & 0xC0) == 0x80){
                        cursor.index--;
                    }
                }
                break;
            }
            break;
        // End of escape codes
        case '\b':
        case '\177':
        case_backspace: // Backspace
            if (cursor.index == 0){
                break;
            }
            cursor.index--;
            cursor.col--;
            if (cursor.col == 0 && cursor.row > 1){
                cursor.row--;
                cursor.col = term.ws_col;
            }
            size_t char_size = 1;
            while (((unsigned char) input.buf[cursor.index] & 0xC0) == 0x80){
                cursor.index--;
                char_size++;
            }
            input.buf = string_insert(input.buf, input.buf + cursor.index + char_size, cursor.index, strlen(input.buf + cursor.index + char_size), true, 'l');
            input.buf_len -= char_size;
            input.no_chars--;
            if (((input.no_chars + cursor.col_home - 1) / term.ws_col) + 1 < cursor.row_tot){
                cursor.row_tot--;
            }
            stdin_reprint(&input, &cursor);
            break;
        default:
            in_char_buf_len = strlen(in_char_buf);
            break;
        } // End of keypress SC

        if (in_char_buf_len > 0){
            // Increase buffer allocation if needed
            if (input.buf_len + in_char_buf_len >= input.buf_size - 1){
                input.buf_size *= 2;
                input.buf = gc_realloc(input.buf, input.buf_size * sizeof(char));
            }
            // Add input to buffer
            input.buf = string_insert(input.buf, in_char_buf, cursor.index, in_char_buf_len, false, 'l');
            if (in_char_buf[0] == '^'){
                input.no_chars += in_char_buf_len;
                cursor.col += in_char_buf_len;
            }
            else {
                input.no_chars++;
                cursor.col++;
            }
            input.buf_len += in_char_buf_len;
            cursor.index += in_char_buf_len;
            input.buf[input.buf_len] = '\0';
            // Print in_buf if at end of input string
            if (cursor.index == input.buf_len){
                fprintf(stderr, "%s", in_char_buf);
            }
            // Reset cursor home position if neccessary
            if (input.no_chars > (term.ws_col * cursor.row_tot) - (cursor.col_home - 1)){
                cursor.row_tot++;
                curs_home();
                curs_up(1);
                curs_save();
                curs_down(cursor.row - 1);
                curs_goto_col(cursor.col);
            }
            if (cursor.col > term.ws_col){
                cursor.row++;
                cursor.col -= term.ws_col;
                if (cursor.row > cursor.row_tot){
                    cursor.row_tot++;
                    struct Cursor curs_temp = {0};
                    unsigned int row_real = cursor_get_pos(&curs_temp, term.ws_col);
                    if (cursor.col == 1){
                        fprintf(stderr, "\n");
                    }
                    if (row_real == term.ws_row){
                        curs_home();
                        curs_up(1);
                        curs_save();
                        curs_down(cursor.row - 1);
                        curs_goto_col(cursor.col);
                    }
                }
            }
            // Reprint if not at end of input string
            if (cursor.index != input.buf_len){
                stdin_reprint(&input, &cursor);
            }
        }
    }
    input.buf[input.buf_len] = '\0';
    fprintf(stderr, "\n");

    // Reset term
    terminal_restore(&term_info);
    if (flag.failure){
        return nullptr;
    }
    if (IFS){
        in_v = string_tokenize(in_v, in_c, input.buf, IFS, 'l');
    }
    else {
        in_v = string_tokenize(in_v, in_c, input.buf, " \t\n", 'l');
    }
    in_v[*in_c] = nullptr;
    return in_v;
}

char **input_read(char **in_v, int *in_c){ // "Main" function
    // TODO: add SIGINT handling
    struct ShVar *sh_var_tmp = sh_var_get("PS1");
    if (sh_var_tmp){
        PS1 = string_copy(sh_var_tmp->value, 'l');
    }
    sh_var_tmp = sh_var_get("PS2");
    if (sh_var_tmp){
        PS2 = string_copy(sh_var_tmp->value, 'l');
    }
    sh_var_tmp = sh_var_get("IFS");
    if (sh_var_tmp){
        IFS = string_copy(sh_var_tmp->value, 'l');
    }
    if (option.vi){
        return input_read_vi(in_v, in_c);
    }
    char *input = gc_malloc('l', 64 * sizeof(char));
    input[0] = '\0';
    char tmp[64];
    if (PS1){
        fprintf(stderr, "%s", PS1);
    }
    do {
        fgets(tmp, 64, stdin);
        input = string_concat(input, tmp, 'l');
    } while (strlen(input) > 0 && input[strlen(input) - 1] != '\n');
    if (strlen(input) == 0){
        input[0] = '\n';
        input[1] = '\0';
    }
    while(input[strlen(input) - 1] == '\n' || input[strlen(input) - 1] == '\r' || input[strlen(input) - 1] == EOF){
        input[strlen(input) - 1] = '\0';
    }
    if (IFS){
        in_v = string_tokenize(in_v, in_c, input, IFS, 'l');
    }
    else {
        in_v = string_tokenize(in_v, in_c, input, " \t\n", 'l');
    }
    in_v[*in_c] = nullptr;
    return in_v;
}