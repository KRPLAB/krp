#include "../include/band_matrix_dia.h"
#include "../include/timer.h"
#include <stdlib.h>
#include <stdio.h>

// As funções auxiliares (dot_product, update_d) permanecem IGUAIS
static double dot_product(const double *a, const double *b, int n) {
    double s = 0.0;
    for (int i = 0; i < n; i++) s += a[i] * b[i];
    return s;
}

// Passo do CG (recebe matriz_banda_dia)
double *gradiente_conjugado_dia(const struct matriz_banda_dia *a, const double *x,
                                double *r, const double *d) {
    double *z = matriz_banda_vetor_dia(a, d);
    if (!z) return NULL;

    double r_dot = dot_product(r, r, a->n);
    double d_dot_z = dot_product(d, z, a->n);
    if (d_dot_z == 0.0) {
        free(z);
        return NULL;
    }
    double alpha = r_dot / d_dot_z;

    double *x_new = malloc(a->n * sizeof(double));
    if (!x_new) {
        free(z);
        return NULL;
    }
    for (int i = 0; i < a->n; i++)
        x_new[i] = x[i] + alpha * d[i];

    for (int i = 0; i < a->n; i++)
        r[i] -= alpha * z[i];

    free(z);
    return x_new;
}

void update_d(double *d, const double *r, double beta, int n) {
    for (int i = 0; i < n; i++)
        d[i] = r[i] + beta * d[i];
}

// Resolvedor CG principal
int cg_solve_dia(const struct matriz_banda_dia *a, const double *b, double *x,
                 double tol, int max_iter) {
    int n = a->n;
    int iter = 0;

    double *Ax = matriz_banda_vetor_dia(a, x);
    if (!Ax) return -1;

    double *r = malloc(n * sizeof(double));
    double *d = malloc(n * sizeof(double));
    if (!r || !d) {
        free(Ax); free(r); free(d);
        return -1;
    }
    for (int i = 0; i < n; i++) {
        r[i] = b[i] - Ax[i];
        d[i] = r[i];
    }
    free(Ax);

    double aux = dot_product(r, r, n);
    double tol2 = tol * tol;

    while (aux > tol2 && iter < max_iter) {
        double r_dot_old = aux;

        double *x_new = gradiente_conjugado_dia(a, x, r, d);
        if (!x_new) {
            free(r); free(d);
            return -1;
        }
        for (int i = 0; i < n; i++)
            x[i] = x_new[i];
        free(x_new);

        aux = dot_product(r, r, n);
        iter++;

        if (aux <= tol2) break;

        double beta = aux / r_dot_old;
        update_d(d, r, beta, n);
    }

    free(r);
    free(d);
    return iter;
}