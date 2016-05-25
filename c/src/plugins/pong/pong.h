#ifndef PONG_H
#define PONG_H

#define PONG_TYPE "PONG####"
#ifndef PONG_PATH
#define PONG_PATH "./"
#endif

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480
#define MARGIN        60
#define X(x)          ((x)+MARGIN)
#define Y(y)          ((y)+MARGIN)
#define FIELD_W       WINDOW_WIDTH - 2 * MARGIN
#define FIELD_H       WINDOW_HEIGHT - 2 * MARGIN
#define RESULT_W      100
#define RESULT_H      90
#define RESULT1_X     MARGIN + FIELD_W / 4 - RESULT_W / 2
#define RESULT1_Y     MARGIN + FIELD_H / 2 - RESULT_H / 2
#define RESULT2_X     MARGIN + 3 * FIELD_W / 4 - RESULT_W / 2
#define RESULT2_Y     MARGIN + FIELD_H / 2 - RESULT_H / 2
#define BALL_SIZE     4
#define BALL_V        FIELD_W
#define RACKET_Y      30
#define RACKET_X      4
#define RACKET_V      FIELD_H
#define DIGIT_X       40
#define DIGIT_Y       60
#define DOWN          1
#define UP            -1
#define STILL         0
#define OFF           2
#define QUIT          4

#endif
