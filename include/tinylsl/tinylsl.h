#pragma once

typedef struct {} tinylsl_confit_t;

typedef struct {
    tinylsl_confit_t config;
} tinylsl_t;

int tinylsl_create(tinylsl_t lsl, const tinylsl_confit_t config);
