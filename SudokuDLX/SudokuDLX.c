// Solução de Sudoku usando DLX (Algorítimo X implementado com Dancing Links)
// Este código é uma versão especializada de https://github.com/blynn/dlx 

#include <stdio.h>
#include <malloc.h>
#include <time.h>

#define FALSE	0
#define TRUE	1

// Celula da tabela
typedef struct _cel {
	struct _cel *l, *r, *u, *d;	// left, right, up, down
	union
	{
		struct _cel *c;	// celula normal: link para o cabeçalho da colula
		int ncels;		// cabeçalho de coluna: número de celulas na coluna
	};
	int n;	// "nome" da linha onde está a célula
} CELULA;

// Tabela
#define NCOL	324
#define NLIN	729
CELULA raiz;			// inicio da lista de colunas
CELULA coluna[NCOL];	// cabeçalho das colunas
CELULA *linha[NLIN];	// aponta para primeira celula da linha
int solucao[NLIN];		// linhas escolhidas para a solução
int nlinsol = 0;		// número de linhas na solução

// Diagrama com a solução
char diag[9][9];

// Lista uma coluna p/ depuracao
void dump_col(int c) {
	CELULA * cel;

	printf ("Coluna %d:", c);
	for (cel = coluna[c].d; cel != &coluna[c]; cel = cel->d) {
		int l, c, d;
		l = cel->n / 81;
		c = (cel->n / 9) % 9;
		d = cel->n % 9;
		printf (" [%d,%d,%d]", l, c, d+1);
	}
	printf ("\n");
}

// Lista uma linha p/ depuracao
void dump_lin(int l) {
	CELULA *cel = linha[l];

	printf ("Linha %d:", l);
	do {
		int c = cel->c - coluna;
		printf (" %d", c);
		cel = cel->r;
	} while (cel != linha[l]);
	printf ("\n");
}

// Inicia a tabela vazia
void inittab() {
	int i;

	// colunas
	raiz.l = &coluna[NCOL-1];
	raiz.r = &coluna[0];
	coluna[0].l = &raiz;
	coluna[NCOL-1].r = &raiz;
	for (i = 0; i < NCOL; i++) {
		if (i != 0) {
			coluna[i].l = &coluna[i-1];
		}
		if (i != (NCOL-1)) {
			coluna[i].r = &coluna[i+1];
		}
		coluna[i].u = coluna[i].d = &coluna[i];
		coluna[i].ncels = 0;
	}

	// linhas
	for (i = 0; i < NLIN; i++) {
		linha[i] = NULL;
	}
}

// Cria uma celula na linha l e coluna c
void marca(int l, int c) {
	// aloca a celula
	CELULA *cel = (CELULA *) malloc (sizeof(CELULA));

	// nome da célula é a linha onde ela está
	cel->n = l;

	// acrescenta na coluna
	cel->c = &coluna[c];
	cel->u = coluna[c].u;
	coluna[c].u = cel;
	cel->u->d = cel;
	cel->d = &coluna[c];
	coluna[c].ncels++;

	// acrescenta na linha
	if (linha[l] == NULL) {
		linha[l] = cel;
		cel->l = cel->r = cel;
	} else {
		cel->l = linha[l]->l;
		linha[l]->l = cel;
		cel->r = linha[l];
		cel->l->r = cel;
	}
}

// Cada linha da tabela corresponde a uma possibilidade
// de dígito em uma posição do diagrama
#define LIN(l,c,d) ((l)*81 + (c)*9 + (d))

// Cria a tabela com as restrições
void criatab() {
	int l, c, d;
	int col = 0;
	int bloco;

	// tabela vazia
	inittab();

	// só um número em cada posição
	for (l = 0; l < 9; l++) {
		for (c = 0; c < 9; c++) {
			for (d = 0; d < 9; d++) {
				marca (LIN(l,c,d), col);
			}
			col++;
		}
	}

	// cada número só pode aparecer uma vez em uma linha
	for (d = 0; d < 9; d++) {
		for (l = 0; l < 9; l++) {
			for (c = 0; c < 9; c++) {
				marca (LIN(l,c,d), col);
			}
			col++;
		}
	}

	// cada número só pode aparecer uma vez em uma coluna
	for (d = 0; d < 9; d++) {
		for (c = 0; c < 9; c++) {
			for (l = 0; l < 9; l++) {
				marca (LIN(l,c,d), col);
			}
			col++;
		}
	}

	// cada número só pode aparecer uma vez em um bloco
	for (d = 0; d < 9; d++) {
		for (l = 0; l < 9; l++) {
			for (c = 0; c < 9; c++) {
				bloco = 3*(l/3) + (c/3);
				marca (LIN(l,c,d), col + bloco);
			}
		}
		col += 9;
	}
}

// Cobre uma coluna
void cobre_col(CELULA *cab) {
	CELULA *i, *j;

	// Retira a coluna da lista de cabeçalhos
	cab->l->r = cab->r;
	cab->r->l = cab->l;

	// Remove as linhas que cruzam com a coluna removida
	for (i = cab->d; i != cab; i = i->d) {
		for (j = i->r; j != i; j = j->r) {
			// remove a célula da coluna
			j->u->d = j->d;
			j->d->u = j->u;
			// uma célula a menos na coluna
			j->c->ncels--;
		}
	}
}

// Descobre uma coluna
void descobre_col(CELULA *cab) {
	CELULA *i, *j;

	// Recoloca as linhas que cruzam com a coluna da célula removida
	for (i = cab->u; i != cab; i = i->u) {
		for (j = i->l; j != i; j = j->l) {
			// coloca a célula na coluna
			j->u->d = j->d->u = j;
			// uma célula a mais na coluna
			j->c->ncels++;
		}
	}

	// Recoloca a coluna da lista de cabeçalhos
	cab->l->r = cab->r->l = cab;
}

// Seleciona linha correspondente a dígito fornecido
void sel_linha (int l) {
	CELULA *p;
	CELULA *cel = linha[l];
	cobre_col (cel->c);
	for (p = cel->r; p != cel; p = p->r) {
		cobre_col (p->c);
	}
}

// Carrega o problema
int carrega (char *arq) {
	FILE *fp;
	int l, c, d;

	fp = fopen(arq, "rt");
	if (fp == NULL) {
		return FALSE;
	}
	for (l = 0; l < 9; l++) {
		for (c = 0; c < 9; c++) {
			d = fgetc(fp);
			if ((d >= '1') && (d <= '9')) {
				diag[l][c] = d;
				sel_linha (LIN(l,c,d-'1'));
			}
		}
		fgetc(fp);
	}

	return TRUE;
}

// Lista a solução
void lista(){
	int i, j;

	// Decodifica a solução
	for (i = 0; i < nlinsol; i++) {
		int l, c, d;
		l = solucao[i] / 81;
		c = (solucao[i] / 9) % 9;
		d = solucao[i] % 9;
		diag[l][c] = (char) (d + '1');
	}

	// Imprime o diagrama
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			printf ("%c ", diag[i][j]);
		}
		printf ("\n");
	}
}

// Soluciona o problema
void soluciona() {
	CELULA *cel = raiz.r;
	CELULA *i, *j, *k;
	int s = NLIN;

	// Testa matriz vazia
	if (cel == &raiz) {
		// achou a solução
		lista();
		return;
	}

	// Procura a coluna com menos opções
	for (i = raiz.r; i != &raiz; i = i->r) {
		if (i->ncels < s) {
			cel = i;
			s = i->ncels;
		}
	}
	if (s == 0) {
		// Sem opção
		return;
	}

	// Retira a coluna
	cobre_col(cel);

	// Tenta as várias possibilidades que atendem a coluna
	for (j = cel->d; j != cel; j = j->d) {
		solucao[nlinsol++] = j->n;
		for (k = j->r; k != j; k = k->r) {
			cobre_col (k->c);
		}
		soluciona();
		nlinsol--;
		for (k = j->l; k != j; k = k->l) {
			descobre_col (k->c);
		}
	}
}

// Programa principal
int main (int argc, char*argv[]) {
	time_t inicio;
	char *filename = "teste.txt";
	if (argc > 1) {
		filename = argv[1];
	}
	criatab();
	if (carrega(filename)) {
		inicio = time(NULL);
		soluciona();
		printf ("Tempo = %ld\n", time(NULL) - inicio);
	} else {
		printf ("%s não encontrado\n", filename);
	}
	getchar();
	return 0;
}
