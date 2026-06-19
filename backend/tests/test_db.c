#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <cjson/cJSON.h>

#include "../src/db_sqlite.h"

/* Vérifie le cycle insertion/lecture avec une base SQLite en mémoire. */
int main(void)
{
    UserProfile profile = {0};
    UserProfile *loaded_profile = NULL;
    char *history_json = NULL;
    cJSON *history = NULL;
    cJSON *plans = NULL;
    int plan_id;

    assert(db_init(":memory:") == 0);

    profile.name = "Test User";
    profile.age = 28;
    profile.weight = 72.5f;
    profile.height = 178.0f;
    profile.goal = "balance";
    profile.restrictions = "no pork";
    profile.budget = 400.0f;

    plan_id = db_save_profile(&profile);
    assert(plan_id > 0);

    loaded_profile = db_get_profile(plan_id);
    assert(loaded_profile != NULL);
    assert(strcmp(loaded_profile->name, "Test User") == 0);
    assert(loaded_profile->age == 28);

    free_user_profile(loaded_profile);

    assert(db_save_plan(plan_id, "{\"days\":[{\"day\":1,\"total_calories\":1800,\"macros\":{\"proteins\":120,\"carbs\":160,\"fats\":60}}]}", 88.0f) > 0);

    history_json = db_get_history(plan_id);
    assert(history_json != NULL);

    history = cJSON_Parse(history_json);
    assert(history != NULL);
    plans = cJSON_GetObjectItemCaseSensitive(history, "plans");
    assert(cJSON_IsArray(plans));
    assert(cJSON_GetArraySize(plans) == 1);

    cJSON_Delete(history);
    free(history_json);
    db_close();

    printf("test_db OK\n");
    return 0;
}
