#include "calculadora.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define TOTAL_TESTES_VALIDOS 10
#define TOTAL_TESTES_INVALIDOS 4
#define TOLERANCIA 0.001f

typedef struct {
	const char *posfixa;
	const char *infixa_esperada;
	float valor_esperado;
} CasoTeste;

typedef struct {
	const char *posfixa;
} CasoInvalido;

static int comparar_reais(float valor_obtido, float valor_esperado)
{
	return fabsf(valor_obtido - valor_esperado) <= TOLERANCIA;
}

int main(void)
{
	CasoTeste casos_validos[TOTAL_TESTES_VALIDOS] = {
		{"3 4 + 5 *", "(3+4)*5", 35.0f},
		{"7 2 * 4 +", "7*2+4", 18.0f},
		{"8 5 2 4 + * +", "8+5*(2+4)", 38.0f},
		{"6 2 / 3 + 4 *", "(6/2+3)*4", 24.0f},
		{"9 5 2 8 * 4 + * +", "9+5*(2*8+4)", 109.0f},
		{"2 3 + log 5 /", "log(2+3)/5", 0.139794f},
		{"10 log 3 ^ 2 +", "log(10)^3+2", 3.0f},
		{"45 60 + 30 cos *", "(45+60)*cos(30)", 90.93267f},
		{"0.5 45 sen 2 ^ +", "0.5+sen(45)^2", 1.0f},
		{"2 3 4 / *", "2*(3/4)", 1.5f}
	};
	CasoInvalido casos_invalidos[TOTAL_TESTES_INVALIDOS] = {
		{"1 0 /"},
		{"0 log"},
		{"-1 raiz"},
		{"2 +"}
	};
	int indice;
	int aprovados = 0;
	int aprovados_invalidos = 0;

	for (indice = 0; indice < TOTAL_TESTES_VALIDOS; indice++) {
		char *infixa_obtida = getInFixa((char *)casos_validos[indice].posfixa);
		float valor_obtido = getValor((char *)casos_validos[indice].posfixa);
		int infixa_correta = infixa_obtida != NULL && strcmp(infixa_obtida, casos_validos[indice].infixa_esperada) == 0;
		int valor_correto = !isnan(valor_obtido) && comparar_reais(valor_obtido, casos_validos[indice].valor_esperado);

		printf("Teste %d\n", indice + 1);
		printf("  Posfixa: %s\n", casos_validos[indice].posfixa);
		printf("  Infixa esperada: %s\n", casos_validos[indice].infixa_esperada);
		printf("  Infixa obtida:   %s\n", infixa_obtida != NULL ? infixa_obtida : "NULL");
		printf("  Valor esperado: %.6f\n", casos_validos[indice].valor_esperado);
		printf("  Valor obtido:   %.6f\n", valor_obtido);

		if (infixa_correta && valor_correto) {
			printf("  Resultado: OK\n\n");
			aprovados++;
		} else {
			printf("  Resultado: FALHA\n\n");
		}
	}

	for (indice = 0; indice < TOTAL_TESTES_INVALIDOS; indice++) {
		char *infixa_obtida = getInFixa((char *)casos_invalidos[indice].posfixa);
		float valor_obtido = getValor((char *)casos_invalidos[indice].posfixa);
		int invalido_correto = infixa_obtida == NULL && isnan(valor_obtido);

		printf("Teste invalido %d\n", indice + 1);
		printf("  Posfixa: %s\n", casos_invalidos[indice].posfixa);
		printf("  Infixa obtida: %s\n", infixa_obtida != NULL ? infixa_obtida : "NULL");
		printf("  Valor obtido: %.6f\n", valor_obtido);

		if (invalido_correto) {
			printf("  Resultado: OK\n\n");
			aprovados_invalidos++;
		} else {
			printf("  Resultado: FALHA\n\n");
		}
	}

	printf("Resumo validos: %d/%d testes aprovados.\n", aprovados, TOTAL_TESTES_VALIDOS);
	printf("Resumo invalidos: %d/%d testes aprovados.\n", aprovados_invalidos, TOTAL_TESTES_INVALIDOS);
	return aprovados == TOTAL_TESTES_VALIDOS && aprovados_invalidos == TOTAL_TESTES_INVALIDOS ? 0 : 1;
}