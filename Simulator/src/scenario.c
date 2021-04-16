#include "scenario.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "jsmn.h"

char *KEYS[] = { "scenario", "MA", "start", "end", "balises", "balise", "pos", "linked" };

void baliseArray_add_balise(balise_array_t* array, balise_t balise) {
    if (array->used == array->size) {
        array->size *= 2;
        array->array = realloc(array->array, array->size * sizeof(balise_t));
    }
    array->array[array->used++] = balise;
}

char * json_token_tostr(char *js, jsmntok_t *t) {
    js[t->end] = '\0';
    return js + t->start;
}

bool json_token_streq(char *js, jsmntok_t *t, char *s) {
    return (strncmp(js + t->start, s, t->end - t->start) == 0
            && strlen(s) == (size_t) (t->end - t->start));
}

void replace_newline_with_spaces(char* str) {
    char *pch = strstr(str, "\n");
    while(pch != NULL)
    {
        *pch = ' ';
        pch = strstr(str, "\n");
    }
}

bool create_scenario_from(const char* scenario_path, scenario_t* scenario) {
    
    jsmn_parser parser;
    jsmntok_t tokens[128];
    FILE *scenario_file;
    char content[1024] = {0};
    char line[128] = {0};

    scenario->num_linked_balises = 0;
    scenario->balises.array = malloc(sizeof(balise_t));
    scenario->balises.used = 0;
    scenario->balises.size = 1;

    scenario_file = fopen(scenario_path, "r");

    if (scenario_file == NULL) {
        printf("File not found: %s\n", scenario_path);
        return false;
    }

    while (fgets(line, sizeof(line), scenario_file) != NULL) {
        // replace_newline_with_spaces(line);
        strcat(content, line);
    }

    fclose(scenario_file);

    jsmn_init(&parser);

    int r = jsmn_parse(&parser, content, strlen(content), tokens, 128);

    if (r < 0) {
        printf("Failed to parse tokens from scenario\n");
    }

    jsmntok_t *t;
    char *str;
    for (int i = 0; i < r; ++i) {
        t = &tokens[i];
        str = json_token_tostr(content, t);
        
        if (strcmp(str, "MA") == 0) {
            t = &tokens[++i];
            int token_size = t->size;
            for (int j = 0; j < token_size; ++j) {
                t = &tokens[++i];
                str = json_token_tostr(content, t);
                if (strcmp(str, "start") == 0) {
                    t = &tokens[++i];
                    str = json_token_tostr(content, t);
                    scenario->movement_authority.start_position = atoi(str);
                } else if (strcmp(str, "end") == 0) {
                    t = &tokens[++i];
                    str = json_token_tostr(content, t);
                    scenario->movement_authority.end_position = atoi(str);
                } else {
                    printf("ERROR MA\n");
                }
            } 
        } else if (strcmp(str, "balise") == 0) {
            t = &tokens[++i];
            int token_size = t->size;
            balise_t new_balise;
            for (int j = 0; j < token_size; ++j) {
                t = &tokens[++i];
                str = json_token_tostr(content, t);
                if (strcmp(str, "pos") == 0) {
                    t = &tokens[++i];
                    str = json_token_tostr(content, t);
                    new_balise.position = atoi(str);
                } else if (strcmp(str, "link_pos") == 0) {
                    t = &tokens[++i];
                    str = json_token_tostr(content, t);
                    new_balise.linked_position = atoi(str);
                } else if (strcmp(str, "linked") == 0) {
                    t = &tokens[++i];
                    str = json_token_tostr(content, t);
                    if (strcmp(str, "true") == 0 || strcmp(str, "TRUE") == 0) {
                        new_balise.linked = true;
                    } else {
                        new_balise.linked = false;
                    }
                } else if (strcmp(str, "id") == 0) {
                    t = &tokens[++i];
                    str = json_token_tostr(content, t);
                    new_balise.id = atoi(str);
                } else {
                    printf("ERROR balise\n");
                }
            } 

            printf("Add the new balise\n");
            baliseArray_add_balise(&scenario->balises, new_balise);

            if (new_balise.linked) {
                scenario->num_linked_balises++;
            }

        }
    }

    return true;
}

void scenario_cleanup(scenario_t *scenario) {
    free(scenario->balises.array);
}

balise_array_t* scenario_get_linked_balises(const scenario_t* const scenario) {

    balise_array_t *linked_balises = (balise_array_t* )malloc(sizeof(balise_array_t));
    linked_balises->size = scenario->num_linked_balises;
    linked_balises->used = 0;

    for (size_t i = 0; i < scenario->balises.used; ++i) {
        balise_t *balise = &scenario->balises.array[i];
        if (balise->linked) {
            memcpy(&linked_balises->array[linked_balises->used], balise, sizeof(*balise));
            linked_balises->used++;
        }
    }

    return linked_balises;

}
