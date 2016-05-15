#ifndef PONG_H
#define PONG_H

#define PONG_TYPE "PONG####"
#ifndef PONG_PATH
#define PONG_PATH "./"
#endif

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480
#define MARGIN        60
#define FIELD_X       WINDOW_WIDTH - 2 * MARGIN
#define FIELD_Y       WINDOW_HEIGHT - 2 * MARGIN
#define BALL_SIZE     4
#define BALL_V        FIELD_X
#define RACKET_Y      30
#define RACKET_X      5
#define RACKET_V      FIELD_Y
#define DIGIT_X       40
#define DIGIT_Y       60
#define DOWN          1
#define UP            -1
#define STILL         0
#define OFF           2
#define QUIT          4
#define X(x)          ((x)+MARGIN)
#define Y(y)          ((y)+MARGIN)

#endif
