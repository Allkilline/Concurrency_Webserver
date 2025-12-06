#ifndef __REQUEST_H__
#define __REQUEST_H__


// Parses the URI. Returns 1 if static, 0 if dynamic.
// Fills filename and cgiargs
int request_parse_uri(char *uri, char *filename, char *cgiargs);

// Handles a complete HTTP request on the given socket.
void request_handle(int fd);

#endif // __REQUEST_H__
