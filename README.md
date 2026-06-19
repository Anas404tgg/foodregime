# AI Food Planner

AI Food Planner est une application full-stack qui combine un backend C léger, un frontend web Next.js et un frontend mobile Expo pour générer, noter et suivre des plans de repas personnalisés.

## Architecture

```text
[Next.js web] ---- HTTP/JSON ---->
                                 [Backend C]
[Expo mobile] ---- HTTP/JSON ---->  - libmicrohttpd
                                    - SQLite
                                    - libcurl
                                    - cJSON
                                    - Mistral API
                                    - Supabase sync
```

## Installation du backend C

Prérequis : `gcc`, `cmake`, `sqlite3`, `libcurl`, `libmicrohttpd`, `cJSON`.

```bash
cd backend
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

L'exécutable principal est `build/ai_food_planner`.

## Lancer les tests

```bash
cd backend
ctest --test-dir build --verbose
```

Ou via le Makefile :

```bash
cd backend
make
make test
```

## Frontend web

Prérequis : Node.js 20 et npm.

```bash
cd frontend-web
npm install
npm run dev
```

Le frontend utilise `NEXT_PUBLIC_BACKEND_URL` et pointe par défaut vers `http://localhost:8080`.

## Frontend mobile

Prérequis : Node.js 20, Expo CLI et un environnement React Native compatible.

```bash
cd frontend-mobile
npm install
npx expo start
```

## Variables d'environnement

- `MISTRAL_API_KEY`: clé utilisée pour appeler Mistral.
- `SUPABASE_URL`: URL du projet Supabase.
- `SUPABASE_KEY`: clé Supabase anonyme ou de service selon votre stratégie.
- `NEXT_PUBLIC_BACKEND_URL`: URL du backend pour les frontends.

## Endpoints API

| Méthode | Route | Usage |
| --- | --- | --- |
| POST | `/api/profile` | Enregistre un profil utilisateur |
| GET | `/api/plan` | Génère un plan de repas via Mistral |
| GET | `/api/score` | Calcule un score nutritionnel |
| GET | `/api/history` | Retourne l'historique des plans |
| GET | `/api/shopping-list` | Génère la liste de courses |

## Déploiement

- Backend : compiler avec CMake puis déployer l'exécutable sur une machine Linux avec les bibliothèques système requises.
- Frontend web : déployer le dossier `frontend-web` sur Vercel avec `VERCEL_TOKEN`.
- Mobile : publier via le flux Expo habituel après avoir vérifié les appels API.

## Notes techniques

- Tous les secrets doivent provenir des variables d'environnement.
- Les réponses API suivent le format `{"status":"ok|error","data":...}`.
- Les plans générés sont persistés en SQLite et peuvent être synchronisés vers Supabase.
