#define _POSIX_C_SOURCE 200809L

#include "json_parser.h"

#include <stdlib.h>
#include <string.h>

#include <cjson/cJSON.h>

/* Duplique une chaîne sur une plage donnée. */
static char *duplicate_range(const char *start, const char *end)
{
    size_t length = (size_t)(end - start);
    char *result = (char *)malloc(length + 1);

    if (result == NULL)
    {
        return NULL;
    }

    memcpy(result, start, length);
    result[length] = '\0';
    return result;
}

/* Extrait le contenu assistant depuis la réponse JSON Mistral. */
char *extract_mistral_content(const char *raw_json)
{
    cJSON *root = NULL;
    cJSON *choices = NULL;
    cJSON *choice = NULL;
    cJSON *message = NULL;
    cJSON *content = NULL;
    char *result = NULL;

    if (raw_json == NULL)
    {
        return NULL;
    }

    root = cJSON_Parse(raw_json);
    if (root == NULL)
    {
        return NULL;
    }

    choices = cJSON_GetObjectItemCaseSensitive(root, "choices");
    if (cJSON_IsArray(choices) && cJSON_GetArraySize(choices) > 0)
    {
        choice = cJSON_GetArrayItem(choices, 0);
        message = cJSON_GetObjectItemCaseSensitive(choice, "message");
        content = cJSON_GetObjectItemCaseSensitive(message, "content");
    }

    if (cJSON_IsString(content) && content->valuestring != NULL)
    {
        result = strdup(content->valuestring);
    }
    else if (cJSON_IsObject(content))
    {
        result = cJSON_PrintUnformatted(content);
    }

    cJSON_Delete(root);
    return result;
}

/* Extrait le premier bloc JSON entouré par des accolades ou des crochets. */
char *extract_json_fragment(const char *text)
{
    const char *start = NULL;
    const char *end = NULL;

    if (text == NULL)
    {
        return NULL;
    }

    start = strchr(text, '{');
    if (start == NULL)
    {
        start = strchr(text, '[');
    }

    if (start == NULL)
    {
        return strdup(text);
    }

    end = strrchr(text, '}');
    if (end == NULL || end < start)
    {
        end = strrchr(text, ']');
    }

    if (end == NULL || end < start)
    {
        return strdup(text);
    }

    return duplicate_range(start, end + 1);
}
