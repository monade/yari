#include "inputs.h"

__attribute__((import_module("env"), import_name("js_is_key_down")))
extern int js_is_key_down(int key);

void inputs_init() {}

bool is_key_down(int key) {
    return js_is_key_down(key);
}
