#pragma once
#include <types.h>

void cpu_early_init(void);
void cpu_detect(void);
const char* cpu_get_vendor(void);
const char* cpu_get_brand(void);
