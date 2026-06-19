#define _POSIX_C_SOURCE 200809L

#include "nutrition.h"

#include <string.h>

/* Renvoie la valeur absolue d'un flottant sans dépendre de la bibliothèque mathématique. */
static float absolute_value(float value)
{
    return value < 0.0f ? -value : value;
}

/* Calcule l'IMC à partir du poids et de la taille. */
float calculate_bmi(float weight_kg, float height_cm)
{
    if (weight_kg <= 0.0f || height_cm <= 0.0f)
    {
        return 0.0f;
    }

    float height_m = height_cm / 100.0f;
    return weight_kg / (height_m * height_m);
}

/* Calcule le métabolisme de base avec la formule de Harris-Benedict. */
float calculate_tmb(int age, float weight, float height, char *gender)
{
    int is_male = 0;

    if (gender != NULL && (gender[0] == 'm' || gender[0] == 'M'))
    {
        is_male = 1;
    }

    if (is_male)
    {
        return 66.47f + (13.75f * weight) + (5.003f * height) - (6.755f * age);
    }

    return 655.1f + (9.563f * weight) + (1.850f * height) - (4.676f * age);
}

/* Ajuste les calories quotidiennes selon l'objectif de l'utilisateur. */
float calculate_daily_calories(float tmb, char *goal)
{
    if (goal == NULL)
    {
        return tmb;
    }

    if (strcmp(goal, "weight_loss") == 0)
    {
        return tmb - 500.0f;
    }

    if (strcmp(goal, "muscle_gain") == 0)
    {
        return tmb + 300.0f;
    }

    return tmb;
}

/* Évalue la cohérence d'un repas selon les macros et la cible calorique. */
int score_meal(float proteins, float carbs, float fats, float target_calories)
{
    float calories = (proteins * 4.0f) + (carbs * 4.0f) + (fats * 9.0f);

    if (calories <= 0.0f || target_calories <= 0.0f)
    {
        return 0;
    }

    float energy_gap = absolute_value(calories - target_calories) / target_calories;
    float energy_score = 100.0f - (energy_gap * 100.0f);
    if (energy_score < 0.0f)
    {
        energy_score = 0.0f;
    }

    float protein_ratio = (proteins * 4.0f) / calories;
    float carbs_ratio = (carbs * 4.0f) / calories;
    float fats_ratio = (fats * 9.0f) / calories;

    float protein_score = 100.0f - (absolute_value(protein_ratio - 0.30f) * 200.0f);
    float carbs_score = 100.0f - (absolute_value(carbs_ratio - 0.45f) * 200.0f);
    float fats_score = 100.0f - (absolute_value(fats_ratio - 0.25f) * 200.0f);

    if (protein_score < 0.0f)
    {
        protein_score = 0.0f;
    }

    if (carbs_score < 0.0f)
    {
        carbs_score = 0.0f;
    }

    if (fats_score < 0.0f)
    {
        fats_score = 0.0f;
    }

    float macro_score = (protein_score + carbs_score + fats_score) / 3.0f;
    float final_score = (energy_score * 0.5f) + (macro_score * 0.5f);

    if (final_score < 0.0f)
    {
        final_score = 0.0f;
    }

    if (final_score > 100.0f)
    {
        final_score = 100.0f;
    }

    return (int)(final_score + 0.5f);
}
