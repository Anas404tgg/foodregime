# API Endpoints

| Method | Route | Description |
| --- | --- | --- |
| POST | /api/profile | Enregistre ou met à jour un profil utilisateur dans SQLite |
| GET | /api/plan | Appelle Mistral et renvoie un plan de repas JSON |
| GET | /api/score | Calcule un score nutritionnel en C à partir des macros |
| GET | /api/history | Retourne les 10 derniers plans stockés |
| GET | /api/shopping-list | Génère une liste de courses à partir du dernier plan |

## Paramètres de requête utiles

- `user_id`: identifiant du profil à utiliser.
- `proteins`, `carbs`, `fats`: macros utilisées par `/api/score`.
- `target_calories`: cible calorique optionnelle pour `/api/score`.
