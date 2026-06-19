#include <assert.h>
#include <stdio.h>

#include "../src/nutrition.h"

/* Vérifie les calculs de BMI, TMB, calories et score nutritionnel. */
int main(void)
{
    float bmi = calculate_bmi(70.0f, 175.0f);
    float tmb = calculate_tmb(30, 70.0f, 175.0f, "m");
    float calories_loss = calculate_daily_calories(tmb, "weight_loss");
    int score = score_meal(30.0f, 50.0f, 20.0f, 500.0f);

    assert(bmi > 22.8f && bmi < 22.9f);
    assert(tmb > 1700.0f && tmb < 1703.0f);
    assert(calories_loss > 1201.0f && calories_loss < 1202.5f);
    assert(score >= 80 && score <= 100);

    printf("test_nutrition OK\n");
    return 0;
}
