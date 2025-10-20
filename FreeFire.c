#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <locale.h>
#ifdef _WIN32
#include <windows.h>
#endif

// Código da Ilha – Edição Free Fire
// Nível: Mestre
// Este programa simula o gerenciamento avançado de uma mochila com componentes coletados durante a fuga de uma ilha.
// Ele introduz ordenação com critérios e busca binária para otimizar a gestão dos recursos.

#define MAX_ITENS 10
#define TAM_NOME 64
#define TAM_TIPO 32

typedef struct {
	char nome[TAM_NOME];
	char tipo[TAM_TIPO];
	int quantidade;
	int prioridade; // 1 (baixa) a 5 (alta)
} Item;

typedef enum {
	CRIT_NOME = 1,
	CRIT_TIPO = 2,
	CRIT_PRIORIDADE = 3
} CriterioOrdenacao;

static Item mochila[MAX_ITENS];
static int numItens = 0;
static long long comparacoes = 0;
static bool ordenadaPorNome = false;

// limpa a "tela" do terminal imprimindo várias quebras de linha
static void limparTela(void) {
	for (int i = 0; i < 30; i++) {
		printf("\n");
	}
}

// consome caracteres restantes até a próxima quebra de linha (para limpar o buffer)
static void consumirAteQuebraDeLinha(void) {
	int c;
	while ((c = getchar()) != '\n' && c != EOF) {}
}

// lê uma linha de texto do usuário com rótulo e remove o '\n' ao final
static void lerLinha(const char *rotulo, char *destino, size_t tamanho) {
	printf("%s", rotulo);
	if (fgets(destino, (int)tamanho, stdin) == NULL) {
		destino[0] = '\0';
		return;
	}
	// remove \n
	size_t n = strlen(destino);
	if (n > 0 && destino[n - 1] == '\n') destino[n - 1] = '\0';
}

// lê inteiro validando faixa [min, max] e repetindo em caso de erro
static int lerInteiro(const char *rotulo, int min, int max) {
	int valor;
	for (;;) {
		printf("%s", rotulo);
		int lidos = scanf("%d", &valor);
		consumirAteQuebraDeLinha();
		if (lidos == 1 && valor >= min && valor <= max) return valor;
		printf("Entrada inválida. Digite um número entre %d e %d.\n", min, max);
	}
}

// exibe o menu principal com estado resumido da mochila
static void exibirMenu(void) {
	printf("==================== Mochila - Free Fire ====================\n");
	printf("Capacidade: %d | Ocupado: %d\n", MAX_ITENS, numItens);
	printf("Ordenada por nome: %s\n", ordenadaPorNome ? "Sim" : "Não");
	printf("--------------------------------------------------------------\n");
	printf("1. Adicionar um item\n");
	printf("2. Remover um item\n");
	printf("3. Listar todos os itens\n");
	printf("4. Ordenar itens (nome, tipo, prioridade)\n");
	printf("5. Buscar por nome (busca binária)\n");
	printf("0. Sair\n");
}

// lista os itens da mochila em formato de tabela
static void listarItens(void) {
	if (numItens == 0) {
		printf("Mochila vazia.\n");
		return;
	}
	printf("\n%-20s %-12s %-10s %-10s\n", "Nome", "Tipo", "Qtd", "Prioridade");
	printf("%-20s %-12s %-10s %-10s\n", "--------------------", "------------", "----------", "----------");
	for (int i = 0; i < numItens; i++) {
		printf("%-20s %-12s %-10d %-10d\n", mochila[i].nome, mochila[i].tipo, mochila[i].quantidade, mochila[i].prioridade);
	}
}

// coleta dados do usuário e insere um novo item no fim do vetor
static void inserirItem(void) {
	if (numItens >= MAX_ITENS) {
		printf("Não há espaço na mochila.\n");
		return;
	}
	Item novo;
	// leitura de campos do item
	lerLinha("Nome: ", novo.nome, sizeof(novo.nome));
	lerLinha("Tipo: ", novo.tipo, sizeof(novo.tipo));
	novo.quantidade = lerInteiro("Quantidade (>= 0): ", 0, 1000000);
	novo.prioridade = lerInteiro("Prioridade (1-5): ", 1, 5);

	// insere no vetor e invalida ordenação por nome
	mochila[numItens++] = novo;
	ordenadaPorNome = false; // após inserir, não garantimos mais ordenação por nome
	printf("Item inserido.\n");
}

// busca por nome e remove deslocando os elementos subsequentes à esquerda
static void removerItem(void) {
	if (numItens == 0) {
		printf("Mochila vazia.\n");
		return;
	}
	char nome[TAM_NOME];
	lerLinha("Nome do item a remover: ", nome, sizeof(nome));
	int indice = -1;
	// procura o índice do item pelo nome
	for (int i = 0; i < numItens; i++) {
		if (strcmp(mochila[i].nome, nome) == 0) {
			indice = i;
			break;
		}
	}
	if (indice == -1) {
		printf("Item não encontrado.\n");
		return;
	}
	// desloca os elementos para cobrir a lacuna
	for (int i = indice; i < numItens - 1; i++) {
		mochila[i] = mochila[i + 1];
	}
	numItens--;
	ordenadaPorNome = false; // remoções também podem quebrar pressupostos
	printf("Item removido.\n");
}

// compara dois itens conforme o critério; também incrementa o contador de comparações
static int comparar(const Item *a, const Item *b, CriterioOrdenacao crit) {
	comparacoes++;
    if (crit == CRIT_NOME) {
        return strcoll(a->nome, b->nome); // crescente (respeita locale)
    } else if (crit == CRIT_TIPO) {
        return strcoll(a->tipo, b->tipo); // crescente (respeita locale)
	} else { // CRIT_PRIORIDADE
		// prioridade: maior primeiro (decrescente)
		if (a->prioridade == b->prioridade) return 0;
		return (a->prioridade > b->prioridade) ? -1 : 1;
	}
}

// ordena a mochila por Insertion Sort, conforme critério escolhido
static void insertionSort(CriterioOrdenacao crit) {
	comparacoes = 0;
	for (int i = 1; i < numItens; i++) {
		Item chave = mochila[i];
		int j = i - 1;
		// desloca elementos maiores (conforme critério) para a direita
		while (j >= 0 && comparar(&mochila[j], &chave, crit) > 0) {
			mochila[j + 1] = mochila[j];
			j--;
		}
		mochila[j + 1] = chave;
	}
}

// apresenta opções de ordenação, executa sort e atualiza flag de ordenação por nome
static void menuDeOrdenacao(void) {
	if (numItens == 0) {
		printf("Nada para ordenar.\n");
		return;
	}
	printf("Critérios de ordenação:\n");
	printf("1. Nome (A-Z)\n");
	printf("2. Tipo (A-Z)\n");
	printf("3. Prioridade (5 -> 1)\n");
	int opc = lerInteiro("Escolha: ", 1, 3);
	insertionSort((CriterioOrdenacao)opc);
	ordenadaPorNome = (opc == CRIT_NOME);
	printf("Ordenado. Comparações: %lld\n", comparacoes);
}

// executa busca binária por nome; requer a lista previamente ordenada por nome
static void buscaBinariaPorNome(void) {
	if (!ordenadaPorNome) {
		printf("Para usar busca binária, ordene por nome primeiro.\n");
		return;
	}
	if (numItens == 0) {
		printf("Mochila vazia.\n");
		return;
	}
	char alvo[TAM_NOME];
	lerLinha("Nome a buscar: ", alvo, sizeof(alvo));
	int ini = 0, fim = numItens - 1;
	while (ini <= fim) {
		int meio = ini + (fim - ini) / 2;
		int cmp = strcmp(mochila[meio].nome, alvo);
		if (cmp == 0) {
			printf("Encontrado: %s | Tipo: %s | Qtd: %d | Prioridade: %d\n",
				mochila[meio].nome, mochila[meio].tipo, mochila[meio].quantidade, mochila[meio].prioridade);
			return;
		} else if (cmp < 0) {
			ini = meio + 1;
		} else {
			fim = meio - 1;
		}
	}
	printf("Item não encontrado.\n");
}

static void configurarEncoding(void) {
    // tenta configurar pt-BR; se falhar, cai para as alternativas e por fim default
    const char *locales[] = { "pt_BR.UTF-8", "pt_BR", "Portuguese_Brazil.1252", "ptb", "" };
    for (size_t i = 0; i < sizeof(locales)/sizeof(locales[0]); i++) {
        if (setlocale(LC_ALL, locales[i]) != NULL) break;
    }
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

int main() {
	// Menu principal com opções:
	// 1. Adicionar um item
	// 2. Remover um item
	// 3. Listar todos os itens
	// 4. Ordenar os itens por critério (nome, tipo, prioridade)
	// 5. Realizar busca binária por nome
	// 0. Sair

	// A estrutura switch trata cada opção chamando a função correspondente.
	// A ordenação e busca binária exigem que os dados estejam bem organizados.

	configurarEncoding();
	for (;;) { // laço principal do programa até o usuário escolher sair
		limparTela();
		exibirMenu();
		int opcao = lerInteiro("Escolha uma opção: ", 0, 5);
		limparTela();
		switch (opcao) { // despacha a ação conforme a opção selecionada
			case 1:
				inserirItem(); // adiciona um novo item
				break;
			case 2:
				removerItem(); // remove item pelo nome
				break;
			case 3:
				listarItens(); // exibe a tabela de itens
				break;
			case 4:
				menuDeOrdenacao(); // escolhe critério e ordena
				break;
			case 5:
				buscaBinariaPorNome(); // busca item por nome usando binary search
				break;
			case 0:
				printf("Saindo...\n");
				return 0;
		}
		printf("\nPressione Enter para continuar..."); // dá tempo para o usuário ler a saída
		consumirAteQuebraDeLinha();
	}
}