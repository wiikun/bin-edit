#include <stdio.h>
#include <stdbool.h>
#include <curses.h>
#include <string.h>
#include <stdlib.h> 

#define addr(x) (x*3)
#define cursor(x,y) (y*16+x)

FILE* load(char* name,bool write){
    FILE* file = NULL;

    if(write){
        file = fopen(name,"wb");
    } else if(!write){
        file = fopen(name,"rb+");
    }

    if(file == NULL){
        perror("�t�@�C�����J���܂���ł���");
    }
    return file;
}

int save(FILE* file,unsigned char* buf,long fs){
    fseek(file, 0, SEEK_SET);
    fwrite(buf, 1, fs, file);
    fflush(file);
    return 0;
}

int main(int argc, char* argv[]){
    FILE* file = NULL;
    if (argc < 2) {
        fprintf(stderr, "�g����: %s <�t�@�C����> [-w]\n", argv[0]);
        return 1;
    }
    if(argc > 2 && strcmp(argv[2], "-w") == 0){
        file = load(argv[1],true);
    } else {
        file = load(argv[1],false);
    }

    initscr();    /* curses�������A��ʂ����� */
    noecho();     /* ���͕�������ʂɏo���Ȃ� */
    cbreak();     /* ���^�[���L�[�Ȃ��ł����͉\�� */
    keypad(stdscr, TRUE);
    start_color();       // �F�@�\��L����
    use_default_colors(); // �[���̃f�t�H���g�w�i���g��
    init_pair(1, COLOR_WHITE, COLOR_BLUE); // �������{�w�i

    int c;
    int count = 0;  // 16�o�C�g���Ƃɉ��s
    int row = 0;    // ncurses �̍s�Ǘ�
    int idx;

    while ((c = fgetc(file)) != EOF) {
        move(row, count * 3);              // 1�o�C�g = 2�� + ��
        printw("%02X", (unsigned char)c);  // 16�i�\��
        count++;

        if (count >= 16) {                 // 16�o�C�g���Ƃɉ��s
            count = 0;
            row++;
        }

        if (row >= LINES) {                // ��ʂ����ς��ňꎞ��~
            printw("--More--");
            refresh();
            getch();
            clear();
            row = 0;
        }
    }


    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    int key = 0;
    row = 0;
    int column = 0;
    unsigned char* buffer = malloc(filesize);

    bool quit = false;
    if (!buffer) return 1;

// �t�@�C����ǂݍ���
    size_t n = fread(buffer, 1, filesize, file); // 1�o�C�g���Afilesize��
    if (n != filesize) {
        perror("�ǂݍ��݃G���[");
    }

    while (key != 'q' && !quit){
        key = getch();
        switch (key)
        {
        case KEY_RIGHT:
            if (cursor(column+1, row) < filesize && column < 15) column++;
            break;

        case KEY_LEFT:
            if (column > 0) column--;
            break;

        case KEY_UP:
            if (row > 0) row--;
            break;

        case KEY_DOWN:
            if (cursor(column, row+1) < filesize) row++;
            break;

        
        case 'w':
            buffer[cursor(column,row)]++;
            break;  

        case 's':
            buffer[cursor(column,row)]--;
            break;  

        case 'o':
            save(file,buffer,filesize);
            quit = true;
            break;

        default:
            continue;
        }
        idx = cursor(column, row);
        move(row, addr(column));
        attron(COLOR_PAIR(1));
        printw("%02X", buffer[idx]);
        attroff(COLOR_PAIR(1));
        refresh();
    }
    

    getch();
    endwin();     /* curses�I���A�[�������ɖ߂� */
    free(buffer);
    fclose(file);
    return 0;
}
