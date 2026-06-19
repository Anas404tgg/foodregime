#ifndef JSON_PARSER_H
#define JSON_PARSER_H

/* Extrait le contenu assistant de la réponse brute Mistral. */
char *extract_mistral_content(const char *raw_json);

/* Extrait le premier fragment JSON valide trouvé dans un texte. */
char *extract_json_fragment(const char *text);

#endif
