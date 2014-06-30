#ifndef IO_SERVER_CONFIG
#define	IO_SERVER_CONFIG


// 8e1, see termios.h for more information
#define RS_FLAGS CS8 | CREAD | CLOCAL | PARENB
// speed
#define RS_SPEED B38400

// TCP server port
#define IO_TCP_SERVER_PORT (2002)

#endif