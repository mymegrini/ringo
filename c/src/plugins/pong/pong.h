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
#define RACKET_Y      30
#define RACKET_X      5
#define DIGIT_X       40
#define DIGIT_Y       60
#define UP            30
#define DOWN          -30
#define X(x)          ((x)+MARGIN)
#define Y(y)          ((y)+MARGIN)

#endif
