#ifndef APPLICATION_H
#define APPLICATION_H


#include <stdarg.h>
////////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////////

/***
 * list of supported applications and there actions
 */
typedef struct application {
    const char id[9];
    const char desc[512];
    void (*app)(char *, char *, int); // action to do, take the message, content and lookup_flag
} application;



void sendappmessage_all(const char *type, const char *format, ...);


#endif /* APPLICATION_H */
