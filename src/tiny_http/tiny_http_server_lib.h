//
// Created by Samuel Vishesh Paul on 28/10/24.
//

#ifndef TINY_HTTP_SERVER_LIB_H
#define TINY_HTTP_SERVER_LIB_H
#include <stdint.h>
#include <stddef.h>

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
    char *path;
    http_header **headers;
    size_t headers_cnt;
    uint8_t *body;
    size_t body_len;
    struct http_response *response;
} http_request;

typedef struct http_response {
    http_version version;
    uint16_t status_code;
    uint8_t *reason_phrase;
    http_header *headers;
    size_t headers_cnt;
    uint8_t *body;
    size_t body_len;
} http_response;

typedef struct http_server_settings {
    size_t max_header_name_length;
    size_t max_header_value_length;
    size_t max_body_length;
    size_t max_url_length;
} http_server_settings;

/**
 * Parses an HTTP request from the given http packet in octets.
 *
 * @param settings
 * @param http_packet A pointer to the HTTP packet to parse.
 * @param http_packet_len The length of the HTTP packet.
 *
 * @return A pointer to the parsed HTTP request, or nullptr if parsing fails.
 */
http_request *parse_http_request(
    const http_server_settings *const settings,
    const uint8_t *const http_packet,
    const size_t http_packet_len);

/**
 * Frees the memory allocated for the given HTTP request and its components.
 *
 * @param http_request The pointer to the HTTP request to be destroyed.
 */
void destroy_http_request(http_request *http_request);

enum render_http_response_status {
    RENDER_OK = 0,
    RENDER_E_MEM_ALLOC_FAILED = -1,
    RENDER_E_RESPONSE_OBJ_IS_NULL = -2,
    RENDER_E_OUT_PARAM_ADDR_IS_NULL = -3,
    RENDER_E_HTTP_VERSION_NOT_SUPPORTED = -4,
};

enum render_http_response_status render_http_response(
    const http_server_settings *const settings,
    const http_response *http_response,
    uint8_t **out_response_octets,
    size_t *out_response_len);

#endif //TINY_HTTP_SERVER_LIB_H
