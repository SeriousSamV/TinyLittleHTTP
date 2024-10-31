//
// Created by Samuel Vishesh Paul on 28/10/24.
//

#define DEBUG 1

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../src/tiny_http/tiny_http_server_lib.h"

void test_request_parse_get_root_curl(void) {
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const uint8_t request[] = "GET / HTTP/1.0\r\n"
            "Host: localhost:8085\r\n"
            "User-Agent: curl/8.7.1\r\n"
            "Accept: */*\r\n"
            "\r\n";
    http_request *http_req = parse_http_request(request, strlen((char *) request));
    assert(http_req != nullptr);
    assert(http_req->method == GET);
    assert(http_req->version == HTTP_1_0);
    assert(strncmp(http_req->url, "/", 1) == 0);
    assert(http_req->headers_cnt == 3);
    assert(strncmp(http_req->headers[0]->name , "Host", 255) == 0);
    assert(strncmp(http_req->headers[0]->value, "localhost:8085", 255) == 0);
    assert(strncmp(http_req->headers[1]->name, "User-Agent", 255) == 0);
    assert(strncmp(http_req->headers[1]->value, "curl/8.7.1", 255) == 0);
    assert(strncmp(http_req->headers[2]->name, "Accept", 255) == 0);
    assert(strncmp(http_req->headers[2]->value, "*/*", 255) == 0);
    assert(http_req->body == nullptr);
    assert(http_req->body_len == 0);
    destroy_http_request(http_req);
}

void test_request_post_root_curl(void) {
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const uint8_t request[] = "POST /one/two/three HTTP/1.0\r\n"
            "Content-Type: application/json\r\n"
            "User-Agent: PostmanRuntime/7.42.0\r\n"
            "Accept: */*\r\n"
            "Host: localhost:8085\r\n"
            "Accept-Encoding: gzip, deflate, br\r\n"
            "Content-Length: 68\r\n"
            "\r\n"
            "{\n    \"key1\": \"value1\",\n    \"key2\": \"value2\",\n    \"key3\": \"value3\"\n}";
    http_request *http_req = parse_http_request(request, strlen((char *) request));
    assert(http_req != nullptr);
    assert(http_req->method == POST);
    assert(http_req->version == HTTP_1_0);
    assert(strncmp(http_req->url, "/one/two/three", 255) == 0);
    assert(http_req->headers_cnt == 6);
    assert(strncmp(http_req->headers[0]->name, "Content-Type", 255) == 0);
    assert(strncmp(http_req->headers[0]->value, "application/json", 255) == 0);
    assert(strncmp(http_req->headers[1]->name, "User-Agent", 255) == 0);
    assert(strncmp(http_req->headers[1]->value, "PostmanRuntime/7.42.0", 255) == 0);
    assert(strncmp(http_req->headers[2]->name, "Accept", 255) == 0);
    assert(strncmp(http_req->headers[2]->value, "*/*", 255) == 0);
    assert(strncmp(http_req->headers[3]->name, "Host", 255) == 0);
    assert(strncmp(http_req->headers[3]->value, "localhost:8085", 255) == 0);
    assert(strncmp(http_req->headers[4]->name, "Accept-Encoding", 255) == 0);
    assert(strncmp(http_req->headers[4]->value, "gzip, deflate, br", 255) == 0);
    assert(strncmp(http_req->headers[5]->name, "Content-Length", 255) == 0);
    assert(strncmp(http_req->headers[5]->value, "68", 255) == 0);
    assert(http_req->body_len == 68);
    assert(
        strncmp((char *) http_req->body,
            "{\n    \"key1\": \"value1\",\n    \"key2\": \"value2\",\n    \"key3\": \"value3\"\n}", 68) == 0);
    destroy_http_request(http_req);
}

void test_request_post_root_curl_with_wide_chars(void) {
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const uint8_t request[] = "POST /one/🐌/three HTTP/1.0\r\n"
            "Content-Type: application/json\r\n"
            "User-Agent: PostmanRuntime/7.42.0\r\n"
            "Accept: */*\r\n"
            "Test-Header: 🐍\r\n"
            "Host: localhost:8085\r\n"
            "Accept-Encoding: gzip, deflate, br\r\n"
            "Content-Length: 68\r\n"
            "\r\n"
            "{\n    \"key1\": \"🐌\",\n    \"key2\": \"value2\",\n    \"key3\": \"value3\"\n}";
    http_request *http_req = parse_http_request(request, strlen((char *) request));
    assert(http_req != nullptr);
    assert(http_req->method == POST);
    assert(http_req->version == HTTP_1_0);
    assert(strncmp(http_req->url, "/one/🐌/three", 255) == 0);
    assert(http_req->headers_cnt == 7);
    assert(strncmp(http_req->headers[0]->name, "Content-Type", 255) == 0);
    assert(strncmp(http_req->headers[0]->value, "application/json", 255) == 0);
    assert(strncmp(http_req->headers[1]->name, "User-Agent", 255) == 0);
    assert(strncmp(http_req->headers[1]->value, "PostmanRuntime/7.42.0", 255) == 0);
    assert(strncmp(http_req->headers[2]->name, "Accept", 255) == 0);
    assert(strncmp(http_req->headers[2]->value, "*/*", 255) == 0);
    assert(strncmp(http_req->headers[3]->name, "Test-Header", 255) == 0);
    assert(strncmp(http_req->headers[3]->value, "🐍", 255) == 0);
    assert(strncmp(http_req->headers[4]->name, "Host", 255) == 0);
    assert(strncmp(http_req->headers[4]->value, "localhost:8085", 255) == 0);
    assert(strncmp(http_req->headers[5]->name, "Accept-Encoding", 255) == 0);
    assert(strncmp(http_req->headers[5]->value, "gzip, deflate, br", 255) == 0);
    assert(strncmp(http_req->headers[6]->name, "Content-Length", 255) == 0);
    assert(strncmp(http_req->headers[6]->value, "68", 255) == 0);
    assert(http_req->body_len == 68);
    assert(
        strncmp((char *) http_req->body,
            "{\n    \"key1\": \"🐌\",\n    \"key2\": \"value2\",\n    \"key3\": \"value3\"\n}", 68) == 0);
    destroy_http_request(http_req);
}

void test_request_parse_head(void) {
    const uint8_t request[] = "HEAD /test HTTP/1.0\r\n"
            "Host: localhost:8085\r\n"
            "User-Agent: custom-agent/1.0\r\n"
            "Accept: */*\r\n"
            "\r\n";
    http_request *http_req = parse_http_request(request, strlen((char *) request));
    assert(http_req != nullptr);
    assert(http_req->method == HEAD);
    assert(http_req->version == HTTP_1_0);
    assert(strncmp(http_req->url, "/test", 255) == 0);
    assert(http_req->headers_cnt == 3);
    assert(strncmp(http_req->headers[0]->name, "Host", 255) == 0);
    assert(strncmp(http_req->headers[0]->value, "localhost:8085", 255) == 0);
    assert(strncmp(http_req->headers[1]->name, "User-Agent", 255) == 0);
    assert(strncmp(http_req->headers[1]->value, "custom-agent/1.0", 255) == 0);
    assert(strncmp(http_req->headers[2]->name, "Accept", 255) == 0);
    assert(strncmp(http_req->headers[2]->value, "*/*", 255) == 0);
    assert(http_req->body == nullptr);
    assert(http_req->body_len == 0);
    destroy_http_request(http_req);
}

int main() {
    test_request_parse_get_root_curl();
    test_request_post_root_curl();
    test_request_post_root_curl_with_wide_chars();
    test_request_parse_head();

    return EXIT_SUCCESS;
}
