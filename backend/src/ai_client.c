#define _POSIX_C_SOURCE 200809L

#include "ai_client.h"

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json_parser.h"
#include <cjson/cJSON.h>

/* Stocke une réponse HTTP reçue via libcurl. */
typedef struct Buffer
{
    char *data;
    size_t size;
} Buffer;

/* Ajoute les octets reçus au tampon de réponse. */
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t total_size = size * nmemb;
    Buffer *buffer = (Buffer *)userp;
    char *new_data = (char *)realloc(buffer->data, buffer->size + total_size + 1);

    if (new_data == NULL)
    {
        return 0;
    }

    buffer->data = new_data;
    memcpy(&(buffer->data[buffer->size]), contents, total_size);
    buffer->size += total_size;
    buffer->data[buffer->size] = '\0';
    return total_size;
}

/* Construit le corps JSON attendu par l'API Mistral. */
static char *build_request_body(char *prompt)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *messages = cJSON_CreateArray();
    cJSON *message = cJSON_CreateObject();
    char *json_body = NULL;

    if (root == NULL || messages == NULL || message == NULL)
    {
        cJSON_Delete(root);
        cJSON_Delete(messages);
        cJSON_Delete(message);
        return NULL;
    }

    cJSON_AddStringToObject(root, "model", "mistral-small");
    cJSON_AddItemToObject(root, "messages", messages);
    cJSON_AddStringToObject(message, "role", "user");
    cJSON_AddStringToObject(message, "content", prompt != NULL ? prompt : "");
    cJSON_AddItemToArray(messages, message);

    json_body = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json_body;
}

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/* Attend avec un backoff exponentiel simple entre les essais. */
static void wait_backoff(int attempt)
{
    unsigned int delay_seconds = 1U << attempt;
#ifdef _WIN32
    Sleep(delay_seconds * 1000U);
#else
    sleep(delay_seconds);
#endif
}

/* Appelle Mistral avec authentification Bearer et gestion des retries HTTP 429. */
char *call_mistral_api(char *prompt, char *api_key)
{
    char *request_body = NULL;

    if (prompt == NULL || api_key == NULL || api_key[0] == '\0')
    {
        return NULL;
    }

    request_body = build_request_body(prompt);
    if (request_body == NULL)
    {
        return NULL;
    }

    for (int attempt = 0; attempt < 3; ++attempt)
    {
        CURL *curl = curl_easy_init();
        CURLcode curl_result;
        long http_code = 0;
        struct curl_slist *headers = NULL;
        Buffer response_buffer = {0};
        char authorization_header[1024];

        if (curl == NULL)
        {
            break;
        }

        snprintf(authorization_header, sizeof(authorization_header), "Authorization: Bearer %s", api_key);
        headers = curl_slist_append(headers, authorization_header);
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.mistral.ai/v1/chat/completions");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(request_body));
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_buffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        curl_result = curl_easy_perform(curl);
        if (curl_result == CURLE_OK)
        {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (curl_result == CURLE_OK && http_code >= 200 && http_code < 300)
        {
            free(request_body);
            return response_buffer.data;
        }

        if (http_code == 429 && attempt < 2)
        {
            free(response_buffer.data);
            wait_backoff(attempt);
            continue;
        }

        free(response_buffer.data);
        break;
    }

    free(request_body);
    return NULL;
}
