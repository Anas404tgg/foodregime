#define _POSIX_C_SOURCE 200809L

#include "db_supabase.h"

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cjson/cJSON.h>

/* Stocke une réponse HTTP de Supabase pendant l'appel libcurl. */
typedef struct Buffer
{
    char *data;
    size_t size;
} Buffer;

/* Ajoute les données reçues dans un tampon alloué dynamiquement. */
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

/* Construit le corps JSON envoyé à Supabase. */
static char *build_supabase_payload(char *plan_json)
{
    cJSON *root = cJSON_CreateObject();
    char *payload = NULL;

    if (root == NULL)
    {
        return NULL;
    }

    cJSON_AddStringToObject(root, "plan_json", plan_json != NULL ? plan_json : "");
    payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return payload;
}

/* Synchronise un plan vers Supabase sans jamais coder les secrets en dur. */
int sync_plan_to_supabase(char *plan_json, char *supabase_url, char *supabase_key)
{
    char *resolved_url = supabase_url != NULL ? supabase_url : getenv("SUPABASE_URL");
    char *resolved_key = supabase_key != NULL ? supabase_key : getenv("SUPABASE_KEY");
    char *payload = NULL;
    char final_url[1024];
    CURL *curl;
    CURLcode curl_result;
    long http_code = 0;
    struct curl_slist *headers = NULL;
    Buffer response_buffer = {0};

    if (plan_json == NULL || resolved_url == NULL || resolved_key == NULL)
    {
        return -1;
    }

    payload = build_supabase_payload(plan_json);
    if (payload == NULL)
    {
        return -1;
    }

    snprintf(final_url, sizeof(final_url), "%s/rest/v1/meal_plans", resolved_url);

    curl = curl_easy_init();
    if (curl == NULL)
    {
        free(payload);
        return -1;
    }

    {
        char apikey_header[1024];
        char authorization_header[1024];

        snprintf(apikey_header, sizeof(apikey_header), "apikey: %s", resolved_key);
        snprintf(authorization_header, sizeof(authorization_header), "Authorization: Bearer %s", resolved_key);
        headers = curl_slist_append(headers, apikey_header);
        headers = curl_slist_append(headers, authorization_header);
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Prefer: return=minimal");
    }

    curl_easy_setopt(curl, CURLOPT_URL, final_url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(payload));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    curl_result = curl_easy_perform(curl);
    if (curl_result == CURLE_OK)
    {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(response_buffer.data);
    free(payload);

    return (curl_result == CURLE_OK && http_code >= 200 && http_code < 300) ? 0 : -1;
}
