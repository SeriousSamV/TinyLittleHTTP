//
// Created by Samuel Vishesh Paul on 28/10/24.
//

#include "tiny_http_server_lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum parse_http_request_status {
    PARSE_OK = 0,
    PARSE_E_REQ_IS_NULL = -11,
    PARSE_E_MALFORMED_HTTP_HEADER = 1,
    PARSE_E_ALLOC_MEM_FOR_HEADERS = 2,
    PARSE_E_HTTP_METHOD_NOT_SUPPORTED = 3,
    PARSE_E_MALFORMED_HTTP_REQUEST_LINE = 4,
    PARSE_E_HTTP_VERSION_NOT_SUPPORTED = 5,
};


/**
 * Frees the memory allocated for the given HTTP request and its components.
 *
 * @param http_request The pointer to the HTTP request to be destroyed.
 */
void destroy_http_request(http_request *http_request) {
    if (http_request == nullptr) {
        fprintf(stderr, "http_request is already null");
        fflush(stderr);
        return;
    }
    if (http_request->body != nullptr) {
        free(http_request->body);
        http_request->body = nullptr;
        http_request->body_len = 0;
    }
    if (http_request->headers != nullptr) {
        for (size_t i = 0; i < http_request->headers_cnt; i++) {
            free(http_request->headers[i]);
            http_request->headers[i] = nullptr;
        }
        http_request->headers = nullptr;
        http_request->headers_cnt = 0;
    }
    free(http_request->url);
    http_request->url = nullptr;
    free(http_request);
}

enum render_http_response_status render_http_response(
    const http_response *http_response,
    uint8_t **out_response_octets,
    size_t *out_response_len) {
    if (http_response == nullptr) {
        fprintf(stderr, "http_response is already null");
        fflush(stderr);
        *out_response_len = 0;
        return RENDER_E_RESPONSE_OBJ_IS_NULL;
    }
    if (out_response_len == nullptr) {
        fprintf(stderr, "out_response_len is null");
        fflush(stderr);
        return RENDER_E_OUT_PARAM_ADDR_IS_NULL;
    }
    *out_response_octets = calloc(
        // TODO: have a way to predict the size more accurately to avoid over or under allocation
        http_response->body_len + http_response->headers_cnt * 2000 + 32,
        sizeof(uint8_t));
    if (*out_response_octets == nullptr) {
        fprintf(stderr, "cannot alloc mem for out_response_octets");
        fflush(stderr);
        *out_response_len = 0;
        return RENDER_E_MEM_ALLOC_FAILED;
    }

    size_t octets_written = 0;

    // region status line
    // Status-Line:
    // "HTTP/" 1*DIGIT "." 1*DIGIT SP 3DIGIT SP *<TEXT, excluding CR, LF>
    // "HTTP/<http_version><SP><http response status><SP><reason phrase><CR><LF>"

    // region "HTTP/x.y" part
    strcpy((char *) *out_response_octets, "HTTP/"); // "HTTP/"
    octets_written += 5;
    if (http_response->version == HTTP_1_0) {
        strcpy((char *) *out_response_octets + octets_written, "1.0");
        octets_written += 3;
    } else {
        fprintf(stderr, "unsupported HTTP version");
        fflush(stderr);
        *out_response_len = 0;
        free(*out_response_octets);
        *out_response_octets = nullptr;
        return RENDER_E_HTTP_VERSION_NOT_SUPPORTED;
    }
    // endregion "HTTP/x.y" part
    strcpy((char *) *out_response_octets + octets_written, " ");
    octets_written += 1;
    // region http status
    snprintf((char *) *out_response_octets + octets_written, 5, "%d ", http_response->status_code);
    octets_written += 4;
    // endregion http status
    // region reason phrase
    const size_t reason_phrase_len = strnlen((char *) http_response->reason_phrase, 128);
    strncpy((char *) *out_response_octets + octets_written,
            (const char *) http_response->reason_phrase,
            reason_phrase_len);
    octets_written += reason_phrase_len;
    // endregion reason phrase
    strncpy((char *) *out_response_octets + octets_written, "\r\n", 2);
    octets_written += 2;
    // endregion status line

    if (http_response->headers != nullptr && http_response->headers_cnt > 0) {
    }

    *out_response_len = octets_written;
    return RENDER_OK;
}

/**
 *
 * @param headers http_headers from which we've to get the Content-Length
 * @param num_headers number of headers
 * @return the value of `Content-Length` header or error (negative)
 * @retval > 0 is the actual value
 * @retval -1 headers is NULL
 * @retval -2 the `Content-Length` header is not found
 */
ssize_t get_body_size_from_header(const http_header *const *const headers, const size_t num_headers) {
    if (headers == nullptr) return -1;
    for (size_t i = 0; i < num_headers; i++) {
        if (headers[i] == nullptr) continue;
        if (strncmp(headers[i]->name, "Content-Length", 15) == 0) {
            return strtol(headers[i]->value, nullptr, 10);
        }
    }
    return -2;
}

/**
 * Adds the `body` and `body_len` attributes for the `http_request` being parsed
 *
 * @param http_packet http packet stream
 * @param http_packet_len http packet stream length
 * @param request the http_request object being parsed
 * @param ptr the http packet stream scan ptr
 */
enum parse_http_request_status parse_http_request_body(
    const uint8_t *const http_packet,
    const size_t http_packet_len,
    http_request *request,
    const size_t *ptr) {
    if (request == nullptr) {
        fprintf(stderr, "Error: null request\n");
        fflush(stderr);
        return PARSE_E_REQ_IS_NULL;
    }
    if (*ptr < http_packet_len) {
        const ssize_t body_len_from_header = request != nullptr && request->headers != nullptr
                                                 ? get_body_size_from_header(
                                                     (const http_header * const * const) request->headers,
                                                     request->headers_cnt)
                                                 : 0;
        if (body_len_from_header >= 0) {
            request->body_len = body_len_from_header;
        } else {
            request->body_len = http_packet_len - *ptr;
        }
        request->body = (uint8_t *) strndup((char *) http_packet + *ptr, request->body_len);
    }
    return PARSE_OK;
}

enum parse_http_request_status parse_http_request_headers(
    const uint8_t *const http_packet,
    const size_t http_packet_len,
    http_request *request,
    size_t *ptr) {
    if (request == nullptr) return PARSE_E_REQ_IS_NULL;
    for (size_t i = 0; *ptr < http_packet_len; i++) {
        if ((*ptr >= http_packet_len || *ptr + 1 >= http_packet_len)
            || (http_packet[(*ptr)] == '\r'
                && http_packet[*ptr + 1] == '\n')) {
            break;
        }
        http_header *header = calloc(1, sizeof(http_header));
        const size_t header_name_start_ptr = *ptr;
        size_t header_name_len = 0;
        for (int iter_cnt = 0; *ptr < http_packet_len && iter_cnt < MAX_HTTP_HEADER_NAME_LENGTH; (*ptr)++, iter_cnt++) {
            if (http_packet[(*ptr)] == ' ') {
                header_name_len = *ptr - header_name_start_ptr;
                break;
            }
        }
        if (header_name_len == 0) {
            fprintf(stderr, "malformed header");
            fflush(stderr);
            return PARSE_E_MALFORMED_HTTP_HEADER;
        }
        header->name = strndup((char *) http_packet + *ptr - header_name_len, header_name_len - 1);

        for (; *ptr < http_packet_len; (*ptr)++) {
            if (http_packet[(*ptr)] != ' ') {
                break;
            }
        }
        const size_t header_value_start = *ptr;
        size_t header_value_len = 0;
        for (int iter_cnt = 0; *ptr < http_packet_len && iter_cnt < MAX_HTTP_HEADER_VALUE_LENGTH;
             (*ptr)++, iter_cnt++) {
            if (http_packet[(*ptr)] == '\r' && http_packet[*ptr + 1] == '\n') {
                header_value_len = *ptr - header_value_start;
                break;
            }
        }
        header->value = strndup((char *) http_packet + *ptr - header_value_len, header_value_len);
        if (request->headers == nullptr) {
            request->headers = calloc(1, sizeof(http_header *));
            if (request->headers == nullptr) {
                fprintf(stderr, "cannot allocate memory for new headers");
                fflush(stderr);
                return PARSE_E_ALLOC_MEM_FOR_HEADERS;
            }
        } else {
            http_header **new_headers = realloc(request->headers, sizeof(http_header *) * (i + 1));
            if (new_headers == nullptr) {
                fprintf(stderr, "cannot allocate memory for new headers");
                fflush(stderr);
                return PARSE_E_ALLOC_MEM_FOR_HEADERS;
            }
            request->headers = new_headers;
        }
        request->headers[i] = header;
        request->headers_cnt = i + 1;
        *ptr += 2;
    }
    *ptr += 2;
    return PARSE_OK;
}

enum parse_http_request_status parse_http_request_line_from_packet(
    const uint8_t *const http_packet,
    const size_t http_packet_len,
    http_request *request,
    size_t *ptr) {
    if (request == nullptr) return PARSE_E_REQ_IS_NULL;
    size_t start_uri = 0;
    if (strncmp((char *) http_packet, "GET", 3) == 0) {
        *ptr += 4; // "GET " - 4
        start_uri = 4;
        request->method = GET;
    } else if (strncmp((char *) http_packet, "POST", 4) == 0) {
        *ptr += 5; // "POST" - 5
        start_uri = 5;
        request->method = POST;
    } else if (strncmp((char *) http_packet, "HEAD", 4) == 0) {
        *ptr += 5; // "HEAD" - 5
        start_uri = 5;
        request->method = HEAD;
    } else {
        fprintf(stderr, "right now, only HTTP GET and POST verbs are supported");
        fflush(stderr);
        return PARSE_E_HTTP_METHOD_NOT_SUPPORTED;
    }
#ifdef DEBUG
    printf("request method: %d", request->method);
#endif

    for (int iter_cnt = 0; *ptr < http_packet_len && iter_cnt < MAX_URL_LENGTH; (*ptr)++, iter_cnt++) {
        if (http_packet[(*ptr)] == ' ') {
            request->url = strndup((char *) &http_packet[start_uri], *ptr - start_uri);
            (*ptr)++;
            break;
        }
    }
#ifdef DEBUG
    printf("request url: '%s'", request->url);
#endif
    if (*ptr >= http_packet_len) {
        return PARSE_OK;
    }

    if (strncmp((char *) http_packet + *ptr, "HTTP", 4) == 0) {
        *ptr += 5; // 'HTTP/' - 5
    } else {
        fprintf(stderr, "illegal http packet");
        fflush(stderr);
        return PARSE_E_MALFORMED_HTTP_REQUEST_LINE;
    }
    if (strncmp((char *) http_packet + *ptr, "1.0", 3) == 0) {
        request->version = HTTP_1_0;
        *ptr += 3;
#ifdef DEBUG
        printf("http version: %d", request->version);
#endif
    } else {
        fprintf(stderr, "right now, only HTTP 1.0 is supported");
        fflush(stderr);
        return PARSE_E_HTTP_VERSION_NOT_SUPPORTED;
    }

    if (*ptr >= http_packet_len || *ptr + 2 >= http_packet_len) {
        return PARSE_OK;
    }
    *ptr += 2; // '\r\n'
    return PARSE_OK;
}


/**
 * Parses an HTTP request from the given http packet in octets.
 *
 * @param http_packet A pointer to the HTTP packet to parse.
 * @param http_packet_len The length of the HTTP packet.
 *
 * @return A pointer to the parsed HTTP request, or nullptr if parsing fails.
 */
http_request *parse_http_request(const uint8_t *const http_packet, const size_t http_packet_len) {
    if (http_packet != nullptr && http_packet_len <= 5) {
        fprintf(stderr, "cannot parse http request as it appears empty");
        fflush(stderr);
        return nullptr;
    }
    http_request *request = calloc(1, sizeof(http_request));
    if (request == nullptr) {
        fprintf(stderr, "cannot allocate memory for new http request");
        fflush(stderr);
        return nullptr;
    }
    size_t ptr = 0;

    const enum parse_http_request_status request_line_parse_status =
            parse_http_request_line_from_packet(http_packet, http_packet_len, request, &ptr);
    if (request_line_parse_status != PARSE_OK) {
        destroy_http_request(request);
        return nullptr;
    }

    const enum parse_http_request_status headers_parse_status =
            parse_http_request_headers(http_packet, http_packet_len, request, &ptr);
    if (headers_parse_status != PARSE_OK) {
        destroy_http_request(request);
        return nullptr;
    }

    const enum parse_http_request_status body_parse_status =
            parse_http_request_body(http_packet, http_packet_len, request, &ptr);
    if (body_parse_status != PARSE_OK) {
        destroy_http_request(request);
        return nullptr;
    }

    return request;
}
