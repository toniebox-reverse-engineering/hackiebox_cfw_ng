#include <stdio.h>
#include <stdlib.h>

#include "../jsmn_stream.h"

void start_arr(void *user_arg) {
    /* An example of using the user arg / context pointer in a callback */
    int *parser_id = (int *)user_arg;
    printf("Array started. Parser id = %d\n", *parser_id);
}
void end_arr(void *user_arg) {
    printf("Array ended\n");
}
void start_obj(void *user_arg) {
    printf("Object started\n");
}
void end_obj(void *user_arg) {
    printf("Object ended\n");
}
void obj_key(const char *key, size_t key_len, void *user_arg) {
    printf("Object key: %s\n", key);
}
void str(const char *value, size_t len, void *user_arg) {
    printf("String: %s\n", value);
}
void primitive(const char *value, size_t len, void *user_arg) {
    printf("Primitive: %s\n", value);
}

jsmn_stream_callbacks_t cbs = {
    start_arr,
    end_arr,
    start_obj,
    end_obj,
    obj_key,
    str,
    primitive
};
jsmn_stream_parser parser;

int main(void) {
    FILE *infile = fopen("example.json", "r");

    int parser_id = 1;
    jsmn_stream_init(&parser, &cbs, &parser_id);

    size_t read_count;
    int ch;
    while ((ch = fgetc(infile)) != EOF) {
        jsmn_stream_parse(&parser, (char)ch);
    }

    fclose(infile);
    return EXIT_SUCCESS;
}

