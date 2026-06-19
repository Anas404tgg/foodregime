#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <cjson/cJSON.h>

#include "../src/json_parser.h"

/* Vérifie l'extraction du contenu JSON depuis une réponse Mistral simulée. */
int main(void)
{
    const char *sample_response =
        "{\"id\":\"chatcmpl-test\",\"choices\":[{\"index\":0,\"message\":{\"role\":\"assistant\",\"content\":\"{\\\"days\\\":[{\\\"day\\\":1,\\\"breakfast\\\":{\\\"name\\\":\\\"Oatmeal\\\",\\\"calories\\\":350,\\\"proteins\\\":12,\\\"carbs\\\":45,\\\"fats\\\":10}}]}\"}}]}";
    char *content = extract_mistral_content(sample_response);
    char *fragment = extract_json_fragment(content);
    cJSON *parsed = NULL;

    assert(content != NULL);
    assert(fragment != NULL);
    assert(strstr(content, "days") != NULL);

    parsed = cJSON_Parse(fragment);
    assert(parsed != NULL);
    assert(cJSON_IsArray(cJSON_GetObjectItemCaseSensitive(parsed, "days")));

    cJSON_Delete(parsed);
    free(content);
    free(fragment);
    printf("test_json_parser OK\n");
    return 0;
}
