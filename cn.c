#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "structs.h"
#include "defines.h"
#include "cuiEdit.h"

int main(int argc, char *argv[]){
  int c;
  int pC;
  int pY = -1;
  int x, y;   // cursor position
  int tmp;
  char mes[] = "@editor->OPMode->";
  char filename[256];
  char *delc;
  FILE *fp;
  BUF buf, *now, *next;
  SCREEN scr;
  
  init(argc, argv, &buf, &scr); // 初期化
  now = buf.next;
  
  initscr();
  start_color();
  init_pair(1, COLOR_BLACK, COLOR_GREEN); // status bar
  init_pair(2, COLOR_GREEN, COLOR_GREEN); // space
  init_pair(3, COLOR_RED, COLOR_BLACK);   // OPMODE
  
  getmaxyx(stdscr, scr.height, scr.width);  // screen info
  dispBuffer(&scr); // disp
  set_escdelay(25); // esc delay
  noecho();         // noecho
  cbreak();         // raw

  keypad(stdscr, TRUE); // function key

  // カーソル開始位置に
  move(scr.curY, scr.curX);

  while(1){
    c = getch();    // 1文字入力
    switch(c){
    case 27:  // ESC
      if(pC != -1){
        move(0, 0);
        // status バー の文字を変える
        attrset(COLOR_PAIR(1));
        printw(mes);
        attrset(COLOR_PAIR(2));
        for(int i = 0 ; i < scr.width - strlen(mes)-1; i++){
          printw("_");
        }
        printw("\n");
        /* // 全部赤にする
        for(int i = 1; i < scr.height; i++){
          move(i, 0);
          chgat(scr.width, A_NORMAL, 3, NULL);
        }
        */
        // 現在の行を赤色にする
        
      }
      
      if(pY != scr.curY){
        if(pY != -1){
          move(pY, 0);
          chgat(scr.width, A_NORMAL, 0, NULL);
        }
        move(scr.curY, 0);
        chgat(scr.width, A_NORMAL, 3, NULL);
      }
      pC = -1;
      pY = scr.curY;
      move(scr.curY, scr.curX);
        c = getch();  // コマンド
        switch(c){
        case 'q':   // quit
          goto _end;
        case 's':
        case 'w':   // write and end;
          for(int i = 0; i < 256; i++) filename[i] = '\0';
          attrset(COLOR_PAIR(1));
          move(0, strlen(mes));
          if(c == 'w') printw("w ");
          else if(c == 's') printw("s ");
          echo();
          scanw("%s", filename);
          noecho();
          if(filename[0] == 27){
          }else if(strlen(filename) == 0){
            fp = fopen(scr.filename, "w");
          }else {
            fp = fopen(filename, "w");
            
          }
          if(fp != NULL && filename[0] != 27){
            next = buf.next;
            while(next != &buf){
              for(int i = 0; i < next->cols; i++){
                fputc(next->c[i], fp);
              }
              next = next->next;
              if(next != &buf) fputc('\n', fp);
            }
            fclose(fp);
            if(c == 'w' && filename[0] != 27) goto _end;
            attrset(COLOR_PAIR(1));
            printw(mes);
            attrset(COLOR_PAIR(2));
            for(int i = 0 ; i < scr.width - strlen(mes)-1; i++){
              printw("_");
            }
            attrset(COLOR_PAIR(1));
            move(0, strlen(mes));
            printw("saved");
          }
          break;
        case 'l': // right
          now = keyOperation(&buf, now, &scr, CUR_RIGHT);
          break;
        case 'k': // up
          now = keyOperation(&buf, now, &scr, CUR_UP);
          break;
        case 'j': // down
          now = keyOperation(&buf, now, &scr, CUR_DOWN);
          break;
        case 'h': // left
          now = keyOperation(&buf, now, &scr, CUR_LEFT);
          break;
        default: break;
        }
        switch(c){
        case 27:
        case 'i':
        case 'w':
        case 's':
          pC = 0;
          pY = -1;
          break;
        default:
          //ungetch(c);
          ungetch(27);
          continue;
          break;
        }
      break;
    case KEY_LEFT:
      now = keyOperation(&buf, now, &scr, CUR_LEFT);
      break;
    case KEY_RIGHT:
      now = keyOperation(&buf, now, &scr, CUR_RIGHT);
      break;
    case KEY_UP:
      now = keyOperation(&buf, now, &scr, CUR_UP);
      break;
    case KEY_DOWN:
      now = keyOperation(&buf, now, &scr, CUR_DOWN);
      break;
    case 10:  // ENTER
      now = insertNewLine(now);
      if(scr.fPosX < now->prev->cols){ // もしカーソルより右に文字があれば
        int col = now->prev->cols;
        for(int i = scr.fPosX; i < col; i++){ // 次の行に追加する
          now->c[i-scr.fPosX] = now->prev->c[i];
          now->prev->c[i] = '\0';
          now->cols++;
          now->prev->cols--;
        }
      }
      scr.fPosY++;
      scr.fPosX=0;
      if(scr.curY+1 < scr.height) scr.curY++;
      else scr.top = scr.top->next;
      scr.curX = 7;
      scr.leftCols = 0;
      break;
    case 127: // Backspace
    case KEY_BACKSPACE:
      if(scr.fPosX > 0){  // 消すものがあれば
        if(scr.leftCols > 0){ // 右にスクロールしていたら
          scr.leftCols--;     // 左に1つスクロール
        }else if(scr.curX > 7) scr.curX--;  // カーソルを左に移動
        for(int i = --scr.fPosX; i < now->cols; i++){ // 右にある文字を左に詰める
          now->c[i] = now->c[i+1];
        }
        now->c[now->cols] = '\0';
        now->cols--;
      }else if(now->prev != &buf){  // 前に行があれば
      // もし文字があるなら
        if(now->cols > 0){
          // 前の行末に追加
          for(int i = 0; i < now->cols; i++){
            now->prev->c[now->prev->cols] = now->c[i];
            now->prev->cols++;
          }
        }
        if(scr.curY == 1){  // カーソルが一番上にあるなら一番上の表示行を変える
          scr.top = scr.top->prev;
        }else if(scr.curY > 1) scr.curY--;
        
        if(now->prev->cols > 0){  // 前の行に文字があるなら
          if(now->prev->cols + 7 > scr.width) scr.curX = 7 + now->prev->cols; // カーソル位置を設定
          else scr.curX = scr.width-1;
          scr.fPosX = now->prev->cols;
        }else {
          scr.curX = 7;
          scr.fPosX = 0;
        }
        
        tmp = now->cols;
        //tmp = 0;
        // つなぎかえる
        next = now;
        now->prev->next = now->next;
        now->next->prev = now->prev;
        now = now->prev;
        // いらない行は消す
        free(next->c);
        free(next);
        // 行番号を付け直す
        next = now;
        while(next != &buf){
          next->lineNum = next->prev->lineNum+1;
          next = next->next;
        }
        scr.leftCols = 0;
        if(now->cols > scr.width - 8){
          scr.leftCols = now->cols - scr.width + 8 - tmp;
          scr.fPosX = now->cols - tmp;
          scr.curX = scr.width-1;
          
        }else {
          scr.curX = now->cols + 7-tmp;
          scr.fPosX = now->cols - tmp;
        }
        
      }
      break;
    case 9: // tab
      now = keyOperation(&buf, now, &scr, CUR_TAB);
      break;
    default:
      if(now->cols + 1 < now->maxCols){
        addLineStr(now);
      }
      if(scr.fPosX < now->cols){
        for(int i = now->cols-1; i >= scr.fPosX; i--){
          now->c[i+1] = now->c[i];
        }
      }
      now->c[scr.fPosX++] = (char)c; 
      now->cols++;
      if(scr.curX < scr.width-1){
        scr.curX++;
      }else {
        scr.leftCols++;
      }
    break;
    }
    erase();
    dispBuffer(&scr);
    move(scr.curY, scr.curX);
    pC = c;
    pY = -1;
  }
_end:
  echo();
  nocbreak();
  endwin();
  end(&buf);
  return 0;
}
