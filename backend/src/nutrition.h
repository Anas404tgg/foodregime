#ifndef NUTRITION_H
#define NUTRITION_H

/* Déclare les fonctions nutritionnelles utilisées par le serveur. */
float calculate_bmi(float weight_kg, float height_cm);
float calculate_tmb(int age, float weight, float height, char *gender);
float calculate_daily_calories(float tmb, char *goal);
int score_meal(float proteins, float carbs, float fats, float target_calories);

#endif
