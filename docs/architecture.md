# Architecture

## Vue d'ensemble

```text
+--------------------+        HTTP/JSON        +--------------------------+
| frontend-web       | <---------------------> | backend C (8080)         |
| Next.js + Tailwind |                         | libmicrohttpd + cJSON    |
+--------------------+                         | SQLite + libcurl         |
        |                                       +-----------+--------------+
        |                                                   |
        | React Native / Expo                                 | SQLite
        v                                                   v
+--------------------+                         +--------------------------+
| frontend-mobile    |                         | users / meal_plans /     |
| Expo + RN          |                         | favorite_recipes         |
+--------------------+                         +--------------------------+
```

## Composants

- Le backend C centralise l'API REST, le calcul nutritionnel et la persistance SQLite.
- Le client Mistral construit les requêtes du plan de repas et renvoie la réponse brute.
- Le client Supabase synchronise les plans lorsque les clés d'environnement sont disponibles.
- Le frontend web consomme l'API avec `fetch()` et affiche les résultats dans une interface Tailwind.
- Le frontend mobile reprend les mêmes écrans avec React Navigation et AsyncStorage.

## Flux principal

1. L'utilisateur envoie son profil depuis le frontend.
2. Le backend enregistre le profil dans SQLite.
3. `GET /api/plan` récupère le profil, appelle Mistral et sauvegarde le plan.
4. `GET /api/history` et `GET /api/shopping-list` relisent les données sauvegardées.
5. Les plans peuvent être synchronisés vers Supabase à partir des variables d'environnement.
