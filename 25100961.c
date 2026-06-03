#include "calculadora.h"

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ITENS_PILHA 256
#define TAMANHO_TOKEN 64

typedef enum {
	NO_VALOR = 0,
	NO_SOMA,
	NO_SUBTRACAO,
	NO_MULTIPLICACAO,
	NO_DIVISAO,
	NO_MODULO,
	NO_POTENCIA,
	NO_FUNCAO
} TipoNo;

typedef struct {
	char texto[512];
	int precedencia;
	TipoNo tipo;
	double valor;
} NoInfixo;

static int eh_token_numero(const char *token)
{
	char *fim_numero;

	if (token == NULL || *token == '\0') {
		return 0;
	}

	strtof(token, &fim_numero);
	return *fim_numero == '\0';
}

static int eh_funcao_unaria(const char *token)
{
	return strcmp(token, "sen") == 0 || strcmp(token, "cos") == 0 ||
		   strcmp(token, "tg") == 0 || strcmp(token, "log") == 0 ||
		   strcmp(token, "raiz") == 0;
}

static int eh_operador_binario(const char *token)
{
	return strcmp(token, "+") == 0 || strcmp(token, "-") == 0 ||
		   strcmp(token, "*") == 0 || strcmp(token, "/") == 0 ||
		   strcmp(token, "%") == 0 || strcmp(token, "^") == 0;
}

static int precedencia_operador(const char *token)
{
	if (strcmp(token, "+") == 0 || strcmp(token, "-") == 0) {
		return 1;
	}
	if (strcmp(token, "*") == 0 || strcmp(token, "/") == 0 || strcmp(token, "%") == 0) {
		return 2;
	}
	if (strcmp(token, "^") == 0) {
		return 3;
	}
	return 4;
}

static TipoNo tipo_operador(const char *token)
{
	if (strcmp(token, "+") == 0) {
		return NO_SOMA;
	}
	if (strcmp(token, "-") == 0) {
		return NO_SUBTRACAO;
	}
	if (strcmp(token, "*") == 0) {
		return NO_MULTIPLICACAO;
	}
	if (strcmp(token, "/") == 0) {
		return NO_DIVISAO;
	}
	if (strcmp(token, "%") == 0) {
		return NO_MODULO;
	}
	if (strcmp(token, "^") == 0) {
		return NO_POTENCIA;
	}
	return NO_FUNCAO;
}

static double para_radianos(double graus)
{
	return graus * 3.14159265358979323846 / 180.0;
}

static int tokenizar_expressao(const char *expressao, char tokens[][TAMANHO_TOKEN], int *quantidade)
{
	char buffer[512];
	char *token_atual;
	int total = 0;

	if (expressao == NULL || quantidade == NULL) {
		return 0;
	}

	strncpy(buffer, expressao, sizeof(buffer) - 1);
	buffer[sizeof(buffer) - 1] = '\0';

	token_atual = strtok(buffer, " \t\r\n");
	while (token_atual != NULL) {
		if (total >= MAX_ITENS_PILHA || strlen(token_atual) >= TAMANHO_TOKEN) {
			return 0;
		}

		strcpy(tokens[total], token_atual);
		total++;
		token_atual = strtok(NULL, " \t\r\n");
	}

	*quantidade = total;
	return total > 0;
}

static int precisa_parenteses_esquerda(const NoInfixo *filho, TipoNo tipo_pai, int precedencia_pai)
{
	if (filho->precedencia < precedencia_pai) {
		return 1;
	}

	if (tipo_pai == NO_POTENCIA && filho->precedencia == precedencia_pai) {
		return 1;
	}

	return 0;
}

static int precisa_parenteses_direita(const NoInfixo *filho, TipoNo tipo_pai, int precedencia_pai)
{
	if (filho->precedencia < precedencia_pai) {
		return 1;
	}

	if (filho->precedencia > precedencia_pai) {
		return 0;
	}

	switch (tipo_pai) {
	case NO_SUBTRACAO:
	case NO_DIVISAO:
	case NO_MODULO:
		return 1;
	case NO_MULTIPLICACAO:
		return filho->tipo == NO_DIVISAO || filho->tipo == NO_MODULO;
	case NO_POTENCIA:
		return 0;
	default:
		return 0;
	}
}

static int montar_no_binario(NoInfixo *resultado, const NoInfixo *esquerda, const NoInfixo *direita, const char *token)
{
	char texto_esquerda[512];
	char texto_direita[512];
	int precedencia = precedencia_operador(token);
	TipoNo tipo = tipo_operador(token);

	if (precisa_parenteses_esquerda(esquerda, tipo, precedencia)) {
		snprintf(texto_esquerda, sizeof(texto_esquerda), "(%s)", esquerda->texto);
	} else {
		snprintf(texto_esquerda, sizeof(texto_esquerda), "%s", esquerda->texto);
	}

	if (precisa_parenteses_direita(direita, tipo, precedencia)) {
		snprintf(texto_direita, sizeof(texto_direita), "(%s)", direita->texto);
	} else {
		snprintf(texto_direita, sizeof(texto_direita), "%s", direita->texto);
	}

	if (snprintf(resultado->texto, sizeof(resultado->texto), "%s%s%s", texto_esquerda, token, texto_direita) >=
		(int)sizeof(resultado->texto)) {
		return 0;
	}

	resultado->precedencia = precedencia;
	resultado->tipo = tipo;

	if (strcmp(token, "+") == 0) {
		resultado->valor = esquerda->valor + direita->valor;
	} else if (strcmp(token, "-") == 0) {
		resultado->valor = esquerda->valor - direita->valor;
	} else if (strcmp(token, "*") == 0) {
		resultado->valor = esquerda->valor * direita->valor;
	} else if (strcmp(token, "/") == 0) {
		if (direita->valor == 0.0) {
			return 0;
		}
		resultado->valor = esquerda->valor / direita->valor;
	} else if (strcmp(token, "%") == 0) {
		if (direita->valor == 0.0) {
			return 0;
		}
		resultado->valor = fmod(esquerda->valor, direita->valor);
	} else if (strcmp(token, "^") == 0) {
		resultado->valor = pow(esquerda->valor, direita->valor);
	}

	return 1;
}

static int montar_no_unario(NoInfixo *resultado, const NoInfixo *operando, const char *token)
{
	if (snprintf(resultado->texto, sizeof(resultado->texto), "%s(%s)", token, operando->texto) >=
		(int)sizeof(resultado->texto)) {
		return 0;
	}

	resultado->precedencia = 4;
	resultado->tipo = NO_FUNCAO;

	if (strcmp(token, "sen") == 0) {
		resultado->valor = sin(para_radianos(operando->valor));
	} else if (strcmp(token, "cos") == 0) {
		resultado->valor = cos(para_radianos(operando->valor));
	} else if (strcmp(token, "tg") == 0) {
		resultado->valor = tan(para_radianos(operando->valor));
	} else if (strcmp(token, "log") == 0) {
		if (operando->valor <= 0.0) {
			return 0;
		}
		resultado->valor = log10(operando->valor);
	} else if (strcmp(token, "raiz") == 0) {
		if (operando->valor < 0.0) {
			return 0;
		}
		resultado->valor = sqrt(operando->valor);
	}

	return 1;
}

static int posfixa_para_infixa(const char *expressao, char *saida)
{
	char tokens[MAX_ITENS_PILHA][TAMANHO_TOKEN];
	NoInfixo pilha[MAX_ITENS_PILHA];
	int quantidade = 0;
	int topo = 0;
	int indice;

	if (saida == NULL || !tokenizar_expressao(expressao, tokens, &quantidade)) {
		return 0;
	}

	for (indice = 0; indice < quantidade; indice++) {
		if (eh_token_numero(tokens[indice])) {
			if (topo >= MAX_ITENS_PILHA) {
				return 0;
			}

			snprintf(pilha[topo].texto, sizeof(pilha[topo].texto), "%s", tokens[indice]);
			pilha[topo].precedencia = 5;
			pilha[topo].tipo = NO_VALOR;
			pilha[topo].valor = strtod(tokens[indice], NULL);
			topo++;
			continue;
		}

		if (eh_funcao_unaria(tokens[indice])) {
			NoInfixo resultado;

			if (topo < 1) {
				return 0;
			}

			if (!montar_no_unario(&resultado, &pilha[topo - 1], tokens[indice])) {
				return 0;
			}

			pilha[topo - 1] = resultado;
			continue;
		}

		if (eh_operador_binario(tokens[indice])) {
			NoInfixo resultado;

			if (topo < 2) {
				return 0;
			}

			if (!montar_no_binario(&resultado, &pilha[topo - 2], &pilha[topo - 1], tokens[indice])) {
				return 0;
			}

			pilha[topo - 2] = resultado;
			topo--;
			continue;
		}

		return 0;
	}

	if (topo != 1) {
		return 0;
	}

	strcpy(saida, pilha[0].texto);
	return 1;
}

static int avaliar_posfixa(const char *expressao, float *valor)
{
	char tokens[MAX_ITENS_PILHA][TAMANHO_TOKEN];
	double pilha[MAX_ITENS_PILHA];
	int quantidade = 0;
	int topo = 0;
	int indice;

	if (valor == NULL || !tokenizar_expressao(expressao, tokens, &quantidade)) {
		return 0;
	}

	for (indice = 0; indice < quantidade; indice++) {
		if (eh_token_numero(tokens[indice])) {
			if (topo >= MAX_ITENS_PILHA) {
				return 0;
			}

			pilha[topo] = strtod(tokens[indice], NULL);
			topo++;
			continue;
		}

		if (eh_funcao_unaria(tokens[indice])) {
			double operando;

			if (topo < 1) {
				return 0;
			}

			operando = pilha[topo - 1];

			if (strcmp(tokens[indice], "sen") == 0) {
				pilha[topo - 1] = sin(para_radianos(operando));
			} else if (strcmp(tokens[indice], "cos") == 0) {
				pilha[topo - 1] = cos(para_radianos(operando));
			} else if (strcmp(tokens[indice], "tg") == 0) {
				pilha[topo - 1] = tan(para_radianos(operando));
			} else if (strcmp(tokens[indice], "log") == 0) {
				if (operando <= 0.0) {
					return 0;
				}
				pilha[topo - 1] = log10(operando);
			} else if (strcmp(tokens[indice], "raiz") == 0) {
				if (operando < 0.0) {
					return 0;
				}
				pilha[topo - 1] = sqrt(operando);
			}

			continue;
		}

		if (eh_operador_binario(tokens[indice])) {
			double esquerda;
			double direita;

			if (topo < 2) {
				return 0;
			}

			direita = pilha[topo - 1];
			esquerda = pilha[topo - 2];

			if (strcmp(tokens[indice], "+") == 0) {
				pilha[topo - 2] = esquerda + direita;
			} else if (strcmp(tokens[indice], "-") == 0) {
				pilha[topo - 2] = esquerda - direita;
			} else if (strcmp(tokens[indice], "*") == 0) {
				pilha[topo - 2] = esquerda * direita;
			} else if (strcmp(tokens[indice], "/") == 0) {
				if (direita == 0.0) {
					return 0;
				}
				pilha[topo - 2] = esquerda / direita;
			} else if (strcmp(tokens[indice], "%") == 0) {
				if (direita == 0.0) {
					return 0;
				}
				pilha[topo - 2] = fmod(esquerda, direita);
			} else if (strcmp(tokens[indice], "^") == 0) {
				pilha[topo - 2] = pow(esquerda, direita);
			}

			topo--;
			continue;
		}

		return 0;
	}

	if (topo != 1) {
		return 0;
	}

	*valor = (float)pilha[0];
	return 1;
}

char *getInFixa(char *Str)
{
	static char saida[512];

	if (!posfixa_para_infixa(Str, saida)) {
		return NULL;
	}

	return saida;
}

float getValor(char *Str)
{
	float valor;

	if (!avaliar_posfixa(Str, &valor)) {
		return NAN;
	}

	return valor;
}
