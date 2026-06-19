#ifndef DB_SUPABASE_H
#define DB_SUPABASE_H

/* Synchronise un plan JSON vers la table Supabase meal_plans. */
int sync_plan_to_supabase(char *plan_json, char *supabase_url, char *supabase_key);

#endif
