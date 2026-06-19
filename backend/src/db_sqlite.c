#define _POSIX_C_SOURCE 200809L

#include "db_sqlite.h"

#include <stdlib.h>
#include <string.h>

#include <cjson/cJSON.h>
#include <sqlite3.h>

/* Garde la connexion SQLite globale ouverte pour tout le processus. */
static sqlite3 *g_db = NULL;

/* Exécute une requête SQL simple sans retour de lignes. */
static int execute_sql(const char *sql)
{
    char *error_message = NULL;
    int rc;

    if (g_db == NULL)
    {
        return -1;
    }

    rc = sqlite3_exec(g_db, sql, NULL, NULL, &error_message);
    if (error_message != NULL)
    {
        sqlite3_free(error_message);
    }

    return rc == SQLITE_OK ? 0 : -1;
}

/* Duplique un texte SQLite en chaîne C gérée par malloc. */
static char *copy_sqlite_text(const unsigned char *text)
{
    const char *source = text != NULL ? (const char *)text : "";
    return strdup(source);
}

/* Construit une structure UserProfile à partir d'une ligne SQLite. */
static UserProfile *profile_from_stmt(sqlite3_stmt *statement)
{
    UserProfile *profile = (UserProfile *)calloc(1, sizeof(UserProfile));

    if (profile == NULL)
    {
        return NULL;
    }

    profile->id = sqlite3_column_int(statement, 0);
    profile->name = copy_sqlite_text(sqlite3_column_text(statement, 1));
    profile->age = sqlite3_column_int(statement, 2);
    profile->weight = (float)sqlite3_column_double(statement, 3);
    profile->height = (float)sqlite3_column_double(statement, 4);
    profile->goal = copy_sqlite_text(sqlite3_column_text(statement, 5));
    profile->restrictions = copy_sqlite_text(sqlite3_column_text(statement, 6));
    profile->budget = (float)sqlite3_column_double(statement, 7);

    if (profile->name == NULL || profile->goal == NULL || profile->restrictions == NULL)
    {
        free_user_profile(profile);
        return NULL;
    }

    return profile;
}

/* Initialise la base et crée les tables métier. */
int db_init(const char *db_path)
{
    const char *schema_sql =
        "PRAGMA foreign_keys = ON;"
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "age INTEGER,"
        "weight REAL,"
        "height REAL,"
        "goal TEXT,"
        "restrictions TEXT,"
        "budget REAL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");"
        "CREATE TABLE IF NOT EXISTS meal_plans ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER REFERENCES users(id),"
        "plan_json TEXT NOT NULL,"
        "nutrition_score REAL,"
        "generated_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "synced_supabase INTEGER DEFAULT 0"
        ");"
        "CREATE TABLE IF NOT EXISTS favorite_recipes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER REFERENCES users(id),"
        "recipe_json TEXT NOT NULL,"
        "saved_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";

    if (g_db != NULL)
    {
        db_close();
    }

    if (sqlite3_open(db_path, &g_db) != SQLITE_OK)
    {
        g_db = NULL;
        return -1;
    }

    if (execute_sql(schema_sql) != 0)
    {
        db_close();
        return -1;
    }

    return 0;
}

/* Ferme la connexion SQLite globale. */
void db_close(void)
{
    if (g_db != NULL)
    {
        sqlite3_close(g_db);
        g_db = NULL;
    }
}

/* Libère les allocations associées à un profil utilisateur. */
void free_user_profile(UserProfile *profile)
{
    if (profile == NULL)
    {
        return;
    }

    free(profile->name);
    free(profile->goal);
    free(profile->restrictions);
    free(profile);
}

/* Insère ou met à jour un profil utilisateur dans SQLite. */
int db_save_profile(UserProfile *profile)
{
    sqlite3_stmt *statement = NULL;
    int rc;
    const char *update_sql = "UPDATE users SET name = ?, age = ?, weight = ?, height = ?, goal = ?, restrictions = ?, budget = ? WHERE id = ?";
    const char *insert_sql = "INSERT INTO users (name, age, weight, height, goal, restrictions, budget) VALUES (?, ?, ?, ?, ?, ?, ?)";

    if (g_db == NULL || profile == NULL || profile->name == NULL)
    {
        return -1;
    }

    if (profile->id > 0)
    {
        rc = sqlite3_prepare_v2(g_db, update_sql, -1, &statement, NULL);
        if (rc != SQLITE_OK)
        {
            return -1;
        }

        sqlite3_bind_text(statement, 1, profile->name, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(statement, 2, profile->age);
        sqlite3_bind_double(statement, 3, profile->weight);
        sqlite3_bind_double(statement, 4, profile->height);
        sqlite3_bind_text(statement, 5, profile->goal != NULL ? profile->goal : "", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(statement, 6, profile->restrictions != NULL ? profile->restrictions : "", -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(statement, 7, profile->budget);
        sqlite3_bind_int(statement, 8, profile->id);
    }
    else
    {
        rc = sqlite3_prepare_v2(g_db, insert_sql, -1, &statement, NULL);
        if (rc != SQLITE_OK)
        {
            return -1;
        }

        sqlite3_bind_text(statement, 1, profile->name, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(statement, 2, profile->age);
        sqlite3_bind_double(statement, 3, profile->weight);
        sqlite3_bind_double(statement, 4, profile->height);
        sqlite3_bind_text(statement, 5, profile->goal != NULL ? profile->goal : "", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(statement, 6, profile->restrictions != NULL ? profile->restrictions : "", -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(statement, 7, profile->budget);
    }

    rc = sqlite3_step(statement);
    sqlite3_finalize(statement);

    if (rc != SQLITE_DONE)
    {
        return -1;
    }

    if (profile->id <= 0)
    {
        profile->id = (int)sqlite3_last_insert_rowid(g_db);
    }

    return profile->id;
}

/* Récupère un profil utilisateur par identifiant ou le dernier profil saisi. */
UserProfile *db_get_profile(int user_id)
{
    const char *sql_with_id = "SELECT id, name, age, weight, height, goal, restrictions, budget FROM users WHERE id = ? LIMIT 1";
    const char *sql_latest = "SELECT id, name, age, weight, height, goal, restrictions, budget FROM users ORDER BY id DESC LIMIT 1";
    sqlite3_stmt *statement = NULL;
    UserProfile *profile = NULL;
    int rc;

    if (g_db == NULL)
    {
        return NULL;
    }

    rc = sqlite3_prepare_v2(g_db, user_id > 0 ? sql_with_id : sql_latest, -1, &statement, NULL);
    if (rc != SQLITE_OK)
    {
        return NULL;
    }

    if (user_id > 0)
    {
        sqlite3_bind_int(statement, 1, user_id);
    }

    if (sqlite3_step(statement) == SQLITE_ROW)
    {
        profile = profile_from_stmt(statement);
    }

    sqlite3_finalize(statement);
    return profile;
}

/* Sauvegarde un plan de repas en base. */
int db_save_plan(int user_id, char *plan_json, float score)
{
    sqlite3_stmt *statement = NULL;
    const char *sql = "INSERT INTO meal_plans (user_id, plan_json, nutrition_score) VALUES (?, ?, ?)";
    int rc;

    if (g_db == NULL || plan_json == NULL)
    {
        return -1;
    }

    rc = sqlite3_prepare_v2(g_db, sql, -1, &statement, NULL);
    if (rc != SQLITE_OK)
    {
        return -1;
    }

    sqlite3_bind_int(statement, 1, user_id);
    sqlite3_bind_text(statement, 2, plan_json, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(statement, 3, score);

    rc = sqlite3_step(statement);
    sqlite3_finalize(statement);

    if (rc != SQLITE_DONE)
    {
        return -1;
    }

    return (int)sqlite3_last_insert_rowid(g_db);
}

/* Construit l'historique des plans au format JSON pour le frontend. */
char *db_get_history(int user_id)
{
    const char *sql_with_user =
        "SELECT id, user_id, plan_json, nutrition_score, generated_at, synced_supabase "
        "FROM meal_plans WHERE user_id = ? ORDER BY generated_at DESC, id DESC LIMIT 10";
    const char *sql_latest =
        "SELECT id, user_id, plan_json, nutrition_score, generated_at, synced_supabase "
        "FROM meal_plans ORDER BY generated_at DESC, id DESC LIMIT 10";
    sqlite3_stmt *statement = NULL;
    cJSON *root = cJSON_CreateObject();
    cJSON *plans = cJSON_CreateArray();
    char *json_text = NULL;
    int rc;

    if (g_db == NULL || root == NULL || plans == NULL)
    {
        cJSON_Delete(root);
        cJSON_Delete(plans);
        return NULL;
    }

    cJSON_AddItemToObject(root, "plans", plans);

    rc = sqlite3_prepare_v2(g_db, user_id > 0 ? sql_with_user : sql_latest, -1, &statement, NULL);
    if (rc != SQLITE_OK)
    {
        cJSON_Delete(root);
        return strdup("{\"plans\":[]}");
    }

    if (user_id > 0)
    {
        sqlite3_bind_int(statement, 1, user_id);
    }

    while ((rc = sqlite3_step(statement)) == SQLITE_ROW)
    {
        cJSON *item = cJSON_CreateObject();
        const unsigned char *plan_text = sqlite3_column_text(statement, 2);
        cJSON *parsed_plan = NULL;

        if (item == NULL)
        {
            continue;
        }

        cJSON_AddNumberToObject(item, "id", sqlite3_column_int(statement, 0));
        cJSON_AddNumberToObject(item, "user_id", sqlite3_column_int(statement, 1));
        cJSON_AddNumberToObject(item, "nutrition_score", sqlite3_column_double(statement, 3));
        cJSON_AddStringToObject(item, "generated_at", (const char *)sqlite3_column_text(statement, 4));
        cJSON_AddNumberToObject(item, "synced_supabase", sqlite3_column_int(statement, 5));

        if (plan_text != NULL)
        {
            parsed_plan = cJSON_Parse((const char *)plan_text);
            if (parsed_plan != NULL)
            {
                cJSON_AddItemToObject(item, "plan", parsed_plan);
            }
            else
            {
                cJSON_AddStringToObject(item, "plan_json", (const char *)plan_text);
            }
        }

        cJSON_AddItemToArray(plans, item);
    }

    sqlite3_finalize(statement);

    if (rc != SQLITE_DONE && rc != SQLITE_ROW)
    {
        cJSON_Delete(root);
        return NULL;
    }

    json_text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json_text;
}

/* Récupère le JSON du dernier plan enregistré. */
char *db_get_latest_plan_json(int user_id)
{
    const char *sql_with_user = "SELECT plan_json FROM meal_plans WHERE user_id = ? ORDER BY generated_at DESC, id DESC LIMIT 1";
    const char *sql_latest = "SELECT plan_json FROM meal_plans ORDER BY generated_at DESC, id DESC LIMIT 1";
    sqlite3_stmt *statement = NULL;
    char *result = NULL;
    int rc;

    if (g_db == NULL)
    {
        return NULL;
    }

    rc = sqlite3_prepare_v2(g_db, user_id > 0 ? sql_with_user : sql_latest, -1, &statement, NULL);
    if (rc != SQLITE_OK)
    {
        return NULL;
    }

    if (user_id > 0)
    {
        sqlite3_bind_int(statement, 1, user_id);
    }

    if (sqlite3_step(statement) == SQLITE_ROW)
    {
        result = copy_sqlite_text(sqlite3_column_text(statement, 0));
    }

    sqlite3_finalize(statement);
    return result;
}

/* Sauvegarde une recette favorite dans la table dédiée. */
int db_save_favorite(int user_id, char *recipe_json)
{
    sqlite3_stmt *statement = NULL;
    const char *sql = "INSERT INTO favorite_recipes (user_id, recipe_json) VALUES (?, ?)";
    int rc;

    if (g_db == NULL || recipe_json == NULL)
    {
        return -1;
    }

    rc = sqlite3_prepare_v2(g_db, sql, -1, &statement, NULL);
    if (rc != SQLITE_OK)
    {
        return -1;
    }

    sqlite3_bind_int(statement, 1, user_id);
    sqlite3_bind_text(statement, 2, recipe_json, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(statement);
    sqlite3_finalize(statement);

    return rc == SQLITE_DONE ? (int)sqlite3_last_insert_rowid(g_db) : -1;
}

/* Marque un plan comme synchronisé avec Supabase. */
int db_sync_flag(int plan_id)
{
    sqlite3_stmt *statement = NULL;
    const char *sql = "UPDATE meal_plans SET synced_supabase = 1 WHERE id = ?";
    int rc;

    if (g_db == NULL || plan_id <= 0)
    {
        return -1;
    }

    rc = sqlite3_prepare_v2(g_db, sql, -1, &statement, NULL);
    if (rc != SQLITE_OK)
    {
        return -1;
    }

    sqlite3_bind_int(statement, 1, plan_id);
    rc = sqlite3_step(statement);
    sqlite3_finalize(statement);

    if (rc != SQLITE_DONE)
    {
        return -1;
    }

    return sqlite3_changes(g_db) > 0 ? 0 : -1;
}
