#include <stdio.h>
#include <stdlib.h>

int is_program_character(char c) {
    switch (c) {
        case '<':
        case '>':
        case '+':
        case '-':
        case '[':
        case ']':
        case '.':
        case ',':
            return 1;

        default:
            return 0;
    }
}
void emit_crash() {
    printf("void crash() {\n");

    printf("\tfprintf(stderr, \"Brainfuck program crashed\");\n");
    printf("\texit(1);\n");

    printf("}\n");
}

void emit_expand_tape() {
    printf("void expand_tape(uchar **tape, int *tape_len) {\n");

    printf("\t*tape_len *= 2;\n");
    printf("\t*tape = (uchar *) realloc(*tape, *tape_len);\n");

    // TODO zero right half of tape
    printf("}\n");
}

void emit_helpers() {
    emit_crash();
    emit_expand_tape();
}

void emit_program(char *progbuf, int n_instr) {
    printf("\t\n");
    printf("\tint tape_len = 8;\n");
    printf("\tint tape_pos = 0;\n");
    printf("\tuchar *tape = (uchar *) calloc(tape_len, 1);\n");
    printf("\tint tmp_char = 0;\n");
    printf("\t\n");

    int nesting = 0;
    for (int i = 0; i < n_instr; i++) {
        char instr = progbuf[i];

        switch (instr) {
            case '<':
                printf("\tif (tape_pos == 0) { crash(); }\n");
                printf("\ttape_pos--;\n");
                break;
            case '>':
                printf("\ttape_pos++;\n");
                printf("\tif (tape_pos == tape_len) { expand_tape(&tape, &tape_len); }\n");
                break;
            case '+':
                printf("\ttape[tape_pos] += 1;\n");
                break;
            case '-':
                printf("\ttape[tape_pos] -= 1;\n");
                break;
            case '[':
                printf("\twhile (tape[tape_pos]) {\n");
                nesting++;
                break;
            case ']':
                printf("\t}\n");
                nesting--;
                break;
            case '.':
                printf("\tputchar(tape[tape_pos]);\n");
                break;
            case ',':
                printf("\ttmp_char = getchar();\n");
                printf("\tif (tmp_char == EOF) { crash(); }\n");
                printf("\ttape[tape_pos] = tmp_char;\n");
                break;

            default:
                fprintf(stderr, "Error outputting C program");
                exit(1);
        }
        printf("\n");
    }
    if (nesting != 0) {
        fprintf(stderr, "Uneven nesting in brainfuck program: +%d\n", nesting);
    }
}

/*

Reads program from stdin, emits C source code to stdout

Usage:
cat examples/email.bf | bfc > email.c && cc -o email.out email.c && ./email.out

*/
int main(int argc, char* argv[]) {
    int progbuf_size = 128;
    int n_instr = 0;
    char *program = (char *) malloc(progbuf_size);

    int c;
    while ((c = getchar()) != EOF) {
        if (!is_program_character(c)) {
            continue;
        }

        n_instr++;
        if (n_instr == progbuf_size) {
            progbuf_size *= 2;
            program = (char *) realloc(program, progbuf_size);
        }
        program[n_instr - 1] = c;
    }

    fprintf(stderr, "%d instructions\n%d program buffer size\n", n_instr, progbuf_size);

    printf("#include <stdio.h>\n");
    printf("typedef unsigned char uchar;\n");
    emit_helpers();
    printf("void main() {\n");
    emit_program(program, n_instr);
    printf("}\n");

    free(program);
    return 0;
}
