//
// Created by Samuel Vishesh Paul on 28/10/24.
//

#ifndef TINY_HTTP_SERVER_LIB_H
#define TINY_HTTP_SERVER_LIB_H
#include <stdint.h>
#include <stddef.h>

#define MAX_URL_LENGTH 8000
#define MAX_HTTP_HEADER_NAME_LENGTH 8000
#define MAX_HTTP_HEADER_VALUE_LENGTH 8000

typedef enum http_version {
    HTTP_1_0 = 1,
} http_version;

typedef enum http_method {
    GET = 1,
    HEAD = 2,
    POST = 3,
} http_method;

typedef struct http_header {
    char *name;
    char *value;
} http_header;

typedef struct http_request {
    http_version version;
    http_method method;
    char *url;
    http_header **headers;
    size_t headers_cnt;
    uint8_t *body;
    size_t body_len;
    struct http_response *response;
} http_request;

typedef struct http_response {
    http_version version;
    http_header *headers;
    size_t headers_cnt;
    uint8_t *body;
    size_t body_len;
} http_response;

/**
 * Parses an HTTP request from the given http packet in octets.
 *
 * @param http_packet A pointer to the HTTP packet to parse.
 * @param http_packet_len The length of the HTTP packet.
 *
 * @return A pointer to the parsed HTTP request, or nullptr if parsing fails.
 */
http_request *parse_http_request(const uint8_t *const http_packet, const size_t http_packet_len);

/**
 * Frees the memory allocated for the given HTTP request and its components.
 *
 * @param http_request The pointer to the HTTP request to be destroyed.
 */
void destroy_http_request(http_request *http_request);

#endif //TINY_HTTP_SERVER_LIB_H
