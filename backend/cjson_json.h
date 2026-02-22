#ifndef PISCORD_BACKEND_CJSON_H
#define PISCORD_BACKEND_CJSON_H

#include <cJSON.h>
#include <string.h>
#include <stdlib.h>

static int cjson_encode(Piscord *self, char *buf, size_t len, struct JsonField *fields, int num) {
    cJSON *root = cJSON_CreateObject();
    for (int i = 0; i < num; i++) {
        switch (fields[i].type) {
            case PISCORD_JSON_STR_TYPE:
                cJSON_AddStringToObject(root, fields[i].name, (char *)fields[i].target);
                break;
            case PISCORD_JSON_INT_TYPE:
                cJSON_AddNumberToObject(root, fields[i].name, *(int *)fields[i].target);
                break;
            case PISCORD_JSON_BOOL_TYPE:
                cJSON_AddBoolToObject(root, fields[i].name, *(int *)fields[i].target);
                break;
        }
    }
    char *out = cJSON_PrintUnformatted(root);
    strncpy(buf, out, len - 1);
    buf[len - 1] = '\0';
    free(out);
    cJSON_Delete(root);
    return 1; // Success
}

static int cjson_decode_array(Piscord *self, char *buf, int len, int num_objects, int num_fields, struct JsonField *fields_array) {
    cJSON *root = cJSON_Parse(buf);
    if (!root) return 0;

    int count = 0;
    cJSON *array = cJSON_IsArray(root) ? root : NULL;
    if (!array) {
        cJSON_Delete(root);
        return 0;
    }

    cJSON *item = NULL;
    cJSON_ArrayForEach(item, array) {
        if (count >= num_objects) break;
        
        for (int j = 0; j < num_fields; j++) {
            struct JsonField *field = &fields_array[count * num_fields + j];
            
            cJSON *val = item;
            char *name_copy = strdup(field->name);
            char *token = strtok(name_copy, ".");
            while (token != NULL && val != NULL) {
                val = cJSON_GetObjectItemCaseSensitive(val, token);
                token = strtok(NULL, ".");
            }
            free(name_copy);

            if (val) {
                if (cJSON_IsString(val) && field->type == PISCORD_JSON_STR_TYPE) {
                    strncpy((char *)field->target, val->valuestring, field->max - 1);
                    ((char *)field->target)[field->max - 1] = '\0';
                } else if (cJSON_IsNumber(val) && field->type == PISCORD_JSON_INT_TYPE) {
                    *(int *)field->target = val->valueint;
                } else if (cJSON_IsBool(val) && field->type == PISCORD_JSON_BOOL_TYPE) {
                    *(int *)field->target = cJSON_IsTrue(val);
                }
            }
        }
        count++;
    }

    cJSON_Delete(root);
    return count;
}

#endif
