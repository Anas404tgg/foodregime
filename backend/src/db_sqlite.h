#ifndef DB_SQLITE_H
#define DB_SQLITE_H

/* Décrit le profil utilisateur stocké en base SQLite. */
typedef struct UserProfile
{
    int id;
    char *name;
    int age;
    float weight;
    float height;
    char *goal;
    char *restrictions;
    float budget;
} UserProfile;

/* Initialise la connexion SQLite et crée le schéma si nécessaire. */
int db_init(const char *db_path);

/* Ferme proprement la connexion SQLite globale. */
void db_close(void);

/* Insère ou met à jour un profil utilisateur. */
int db_save_profile(UserProfile *profile);

/* Récupère un profil utilisateur par identifiant ou le dernier profil. */
UserProfile *db_get_profile(int user_id);

/* Libère un profil alloué dynamiquement. */
void free_user_profile(UserProfile *profile);

/* Enregistre un plan de repas dans la table meal_plans. */
int db_save_plan(int user_id, char *plan_json, float score);

/* Retourne les dix derniers plans au format JSON. */
char *db_get_history(int user_id);

/* Retourne le plan le plus récent au format brut JSON texte. */
char *db_get_latest_plan_json(int user_id);

/* Enregistre une recette favorite. */
int db_save_favorite(int user_id, char *recipe_json);

/* Marque un plan comme synchronisé avec Supabase. */
int db_sync_flag(int plan_id);

#endif
