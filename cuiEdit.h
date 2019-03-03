#ifndef DEF_CUIEDIT_H
#define DEF_CUIEDIT_H

int init(int argc, char *argv[], BUF *buf, SCREEN *scr);
void end(BUF *buf);

void dispBuffer(SCREEN *scr);
void addLineStr(BUF *b);
BUF *insertNewLine(BUF *now);
BUF *keyOperation(BUF *buf, BUF *now, SCREEN *scr, int state);

#endif /* DEF_CUIEDIT_H */
