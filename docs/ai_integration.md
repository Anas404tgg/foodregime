# AI Integration

## Mistral

Le backend envoie une requête `POST` vers `https://api.mistral.ai/v1/chat/completions` avec :

- `model`: `mistral-small`
- `messages`: un message utilisateur unique contenant le prompt nutritionnel
- `Authorization`: `Bearer ${MISTRAL_API_KEY}`
- `Content-Type`: `application/json`

## Contraintes

- Le délai est limité à 30 secondes.
- Les réponses HTTP 429 sont rejouées jusqu'à 3 fois avec backoff exponentiel.
- La réponse brute est conservée, puis le contenu assistant est extrait et validé comme JSON.

## Variables d'environnement

- `MISTRAL_API_KEY`: clé API Mistral.
- `SUPABASE_URL`: URL du projet Supabase.
- `SUPABASE_KEY`: clé anonyme Supabase.
