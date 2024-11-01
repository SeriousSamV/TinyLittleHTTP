//
// Created by Samuel Vishesh Paul on 28/10/24.
//

#define DEBUG 1

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../src/tiny_http/tiny_http_server_lib.h"

http_server_settings settings = {
    .max_header_name_length = 256,
    .max_header_value_length = 512,
    .max_body_length = 1024 * 1024 * 8, // 8M
    .max_url_length = 8000 // NOTE: should be ~2000, but meh the standards doesn't have a comment on it, SEO says 2000
};

void test_request_parse_get_root_curl(void) {
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const uint8_t request[] = "GET / HTTP/1.0\r\n"
            "Host: localhost:8085\r\n"
            "User-Agent: curl/8.7.1\r\n"
            "Accept: */*\r\n"
            "\r\n";
    http_request *http_req = parse_http_request(&settings, request, strlen((char *) request));
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
    http_request *http_req = parse_http_request(&settings, request, strlen((char *) request));
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
    http_request *http_req = parse_http_request(&settings, request, strlen((char *) request));
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
    http_request *http_req = parse_http_request(&settings, request, strlen((char *) request));
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

void test_response_render_200_no_body(void) {
    const http_response response = {
        .version = HTTP_1_0,
        .status_code = 200,
        .reason_phrase = (uint8_t *) "OK",
    };
    uint8_t *response_octets = nullptr;
    size_t response_octets_len = 0;
    const enum render_http_response_status response_code = render_http_response(
        &settings, &response, &response_octets, &response_octets_len);
    assert(response_code == RENDER_OK);
    assert(response_octets_len > 0);
    assert(strncmp((char *) response_octets, "HTTP/1.0 200 OK\r\n", 32) == 0);
    free(response_octets);
}

void test_response_render_404_no_body(void) {
    const http_response response = {
        .version = HTTP_1_0,
        .status_code = 404,
        .reason_phrase = (uint8_t *) "Not Found",
    };
    uint8_t *response_octets = nullptr;
    size_t response_octets_len = 0;
    const enum render_http_response_status response_code = render_http_response(
        &settings, &response, &response_octets, &response_octets_len);
    assert(response_code == RENDER_OK);
    assert(response_octets_len > 0);
    assert(strncmp((char *) response_octets, "HTTP/1.0 404 Not Found\r\n", 32) == 0);
    free(response_octets);
}

void test_response_render_200_with_body(void) {
    {
        http_response response = {
            .version = HTTP_1_0,
            .status_code = 200,
            .reason_phrase = (uint8_t *) "OK",
            .headers_cnt = 2,
            .body = (uint8_t *) "{\n    \"key1\": \"value1\",\n    \"key2\": \"value2\"\n}",
            .body_len = 46
        };
        response.headers = malloc(sizeof(struct http_header) * response.headers_cnt);
        response.headers[0].name = "Content-Type";
        response.headers[0].value = "application/json";
        response.headers[1].name = "Content-Length";
        response.headers[1].value = "46";

        uint8_t *response_octets = nullptr;
        size_t response_octets_len = 0;

        const enum render_http_response_status response_code = render_http_response(
            &settings, &response, &response_octets, &response_octets_len);

        assert(response_code == RENDER_OK);
        assert(response_octets_len > 0);

        const char *expected_response = "HTTP/1.0 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: 46\r\n"
                "\r\n"
                "{\n    \"key1\": \"value1\",\n    \"key2\": \"value2\"\n}";
        const size_t expected_response_len = strnlen(expected_response, 2000);
        assert(strncmp((char *)response_octets, expected_response, expected_response_len) == 0);
        assert(strnlen((char *) response_octets, expected_response_len) == response_octets_len);
        free(response_octets);
    }
}

int main() {
    test_request_parse_get_root_curl();
    test_request_post_root_curl();
    test_request_post_root_curl_with_wide_chars();
    test_request_parse_head();

    test_response_render_200_no_body();
    test_response_render_404_no_body();
    test_response_render_200_with_body();

    return EXIT_SUCCESS;
}
