#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curses.h>
#include <string.h>
#include <locale.h>

#define minus(x) (x*(-1))
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
        perror("ファイルを開けませんでした");
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
        fprintf(stderr, "使い方: %s <ファイル名> [-w]\n", argv[0]);
        return 1;
    }
    if(argc > 2 && strcmp(argv[2], "-w") == 0){
        file = load(argv[1],true);
    } else {
        file = load(argv[1],false);
    }



    initscr();    /* curses初期化、画面も消去 */
    noecho();     /* 入力文字を画面に出さない */
    cbreak();     /* リターンキーなしでも入力可能に */
    keypad(stdscr, TRUE);
    start_color();       // 色機能を有効化
    use_default_colors(); // 端末のデフォルト背景を使う
    init_pair(1, COLOR_WHITE, COLOR_BLUE); // 白文字＋青背景

    mvprintw(LINES - 2, 0, "[arrow] move / w: +1 / s: -1 / a: +16 / d: -16 / o: save&exit / q: quit");


    int c;
    int count = 0;  // 16バイトごとに改行
    int row = 0;    // ncurses の行管理
    int idx;

        //-addオプションがついてる時
    int addsize = 0;

    if(argc > 3 && strcmp(argv[2], "-add") == 0){
        addsize = atoi(argv[3]);
    } else if(argc > 3 && strcmp(argv[2], "-del") == 0){
        addsize = minus(atoi(argv[3]));
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    if(filesize + addsize < 0) {
        endwin();
        return 1;
    }

    for (long i = 0; i < filesize+addsize; i++) {
        c = fgetc(file);
        if(c == EOF) c = 0;
        move(row, count * 3);              // 1バイト = 2桁 + 空白
        printw("%02X", (unsigned char)c);  // 16進表示
        count++;

        if (count >= 16) {                 // 16バイトごとに改行
            count = 0;
            row++;
        }

        if (row >= LINES) {                // 画面いっぱいで一時停止
            printw("--More--");
            refresh();
            getch();
            clear();
            row = 0;
        }
    }

    

    int key = 0;
    row = 0;
    int column = 0;
    unsigned char* buffer = malloc(filesize+addsize);

    if (!buffer) {
        endwin();
        return 1;
    }

    memset(buffer, 0, filesize + addsize);
    bool quit = false;
    

// ファイルを読み込む
    rewind(file); // ← ファイル先頭に戻す
    size_t n = fread(buffer, 1, filesize, file); // 1バイトずつ、filesize個
    if (n != filesize) {
        perror("読み込みエラー");
    }

    while (key != 'q' && !quit){
        key = getch();
        switch (key)
        {
        case KEY_RIGHT:
            if (cursor(column+1, row) < filesize+addsize && column < 15) column++;
            break;

        case KEY_LEFT:
            if (column > 0) column--;
            break;

        case KEY_UP:
            if (row > 0) row--;
            break;

        case KEY_DOWN:
            if (cursor(column, row+1) < filesize+addsize) row++;
            break;

        
        case 'w':
            buffer[cursor(column,row)]++;
            break;  

        case 's':
            buffer[cursor(column,row)]--;
            break;  
        
        case 'a':
            buffer[cursor(column,row)] = (buffer[cursor(column,row)] + 16) %256;
            break;  

        case 'd':
            buffer[cursor(column,row)] = (buffer[cursor(column,row)] - 16 + 256) %256;
            break; 

        case 'o':
            save(file,buffer,filesize+addsize);
            quit = true;
            break;

        default:
            break;
        }
        idx = cursor(column, row);
        move(row, addr(column));
        printw("%02X", buffer[idx]);
        move(row, addr(column)); // カーソルを戻す
        refresh();
    }
    

    getch();
    endwin();     /* curses終了、端末を元に戻す */
    free(buffer);
    fclose(file);
    return 0;
}
