#include <stdio.h>
#include <string.h>
#include <time.h>

#define ADMINISTRADOR 1
#define GERENTE 2
#define COLABORADOR 3

#define MAX_INGREDIENTES 50
#define MAX_PREPARACOES 10
#define MAX_ING_POR_PREPARACAO 10

/* ==================== USUARIOS E ACESSO ==================== */
char usuarioLogado[30];
int nivelAcesso = 0;

int podeAdministrar(void) {
    return nivelAcesso == ADMINISTRADOR;
}

int podeGerenciarEstoque(void) {
    return nivelAcesso == ADMINISTRADOR || nivelAcesso == GERENTE;
}

int podeVerCustos(void) {
    return nivelAcesso == ADMINISTRADOR || nivelAcesso == GERENTE;
}

void fazerLogin(void) {
    char usuario[30];
    char senha[30];

    printf("\n===== LOGIN - RESTAURANTE MEDEIRIANA =====\n");
    printf("Usuario: ");
    scanf("%29s", usuario);
    printf("Senha: ");
    scanf("%29s", senha);

    if (strcmp(usuario, "admin") == 0 &&
        strcmp(senha, "admin123") == 0) {
        strcpy(usuarioLogado, usuario);
        nivelAcesso = ADMINISTRADOR;
    } else if (strcmp(usuario, "gerente") == 0 &&
               strcmp(senha, "ger123") == 0) {
        strcpy(usuarioLogado, usuario);
        nivelAcesso = GERENTE;
    } else if (strcmp(usuario, "colaborador") == 0 &&
               strcmp(senha, "col123") == 0) {
        strcpy(usuarioLogado, usuario);
        nivelAcesso = COLABORADOR;
    } else {
        nivelAcesso = 0;
        printf("\nUsuario ou senha incorretos.\n");
        return;
    }

    printf("\nLogin realizado com sucesso.\n");
    printf("Usuario: %s | Perfil: ", usuarioLogado);
    if (nivelAcesso == ADMINISTRADOR) printf("Administrador\n");
    else if (nivelAcesso == GERENTE) printf("Gerente\n");
    else printf("Colaborador\n");
}


void encerrarSessao(void) {
    usuarioLogado[0] = '\0';
    nivelAcesso = 0;
    printf("\nSessao encerrada. Os dados cadastrados nesta execucao foram mantidos.\n");
}

/* ==================== DADOS DOS INGREDIENTES ==================== */
char nomes[MAX_INGREDIENTES][100];
char unidades[MAX_INGREDIENTES][10];
float precos[MAX_INGREDIENTES];
float estoqueIngredientes[MAX_INGREDIENTES];
int totalIngredientes = 0;

/* ==================== DADOS DAS PREPARACOES ==================== */
char nomesPreparacoes[MAX_PREPARACOES][100];
int ingredientesPreparacao[MAX_PREPARACOES][MAX_ING_POR_PREPARACAO];
float quantidadesIngredientes[MAX_PREPARACOES][MAX_ING_POR_PREPARACAO];
int qtdIngredientes[MAX_PREPARACOES];
float pesoFinalKg[MAX_PREPARACOES];
int totalPreparacoes = 0;

/* Estoque atual de alimentos prontos e seu custo estimado */
float estoquePreparadoKg[MAX_PREPARACOES];
float valorEstoquePreparado[MAX_PREPARACOES];

/* Historico das producoes realizadas durante a execucao */
#define MAX_HISTORICO_PRODUCAO 500
int historicoPreparacao[MAX_HISTORICO_PRODUCAO];
int historicoLotes[MAX_HISTORICO_PRODUCAO];
float historicoPesoKg[MAX_HISTORICO_PRODUCAO];
float historicoRendimentoEsperadoKg[MAX_HISTORICO_PRODUCAO];
float historicoCusto[MAX_HISTORICO_PRODUCAO];
time_t historicoData[MAX_HISTORICO_PRODUCAO];
int totalRegistrosProducao = 0;

/* ==================== VENDAS E FATURAMENTO ==================== */
/* Totais acumulados durante a execucao do programa */
int totalMarmitasVendidas = 0;
float faturamentoTotal = 0.0f;
float custoAlimentosVendidos = 0.0f;

/* ==================== CUSTOS ==================== */
float calcularCustoPreparacao(int indice) {
    int i;
    int ingrediente;
    float custo = 0.0f;

    for (i = 0; i < qtdIngredientes[indice]; i++) {
        ingrediente = ingredientesPreparacao[indice][i];
        custo += precos[ingrediente] * quantidadesIngredientes[indice][i];
    }
    return custo;
}

/* ==================== INGREDIENTES ==================== */
void cadastrarIngrediente(void) {
    int posicao = totalIngredientes;
    float preco;
    float estoqueInicial;

    if (!podeAdministrar()) {
        printf("\nAcesso negado. Somente o administrador pode cadastrar ingredientes.\n");
        return;
    }
    if (totalIngredientes >= MAX_INGREDIENTES) {
        printf("\nLimite de ingredientes atingido.\n");
        return;
    }

    printf("\n=== CADASTRO DE INGREDIENTE ===\n");
    printf("Nome: ");
    scanf(" %99[^\n]", nomes[posicao]);
    printf("Unidade de controle (kg, g, L, un): ");
    scanf(" %9s", unidades[posicao]);
    printf("Preco por unidade informada (R$): ");
    scanf("%f", &preco);
    printf("Estoque inicial nessa unidade: ");
    scanf("%f", &estoqueInicial);

    if (preco < 0 || estoqueInicial < 0) {
        printf("\nErro: preco e estoque nao podem ser negativos. Cadastro cancelado.\n");
        return;
    }

    precos[posicao] = preco;
    estoqueIngredientes[posicao] = estoqueInicial;
    totalIngredientes++;
    printf("\nIngrediente cadastrado com sucesso!\n");
}

void listarIngredientes(void) {
    int i;
    printf("\n=== INGREDIENTES E ESTOQUE ===\n");
    if (totalIngredientes == 0) {
        printf("Nenhum ingrediente cadastrado.\n");
        return;
    }
    for (i = 0; i < totalIngredientes; i++) {
        printf("%d - %s | Estoque: %.3f %s",
               i + 1, nomes[i], estoqueIngredientes[i], unidades[i]);
        if (podeVerCustos()) printf(" | Preco: R$ %.2f por %s", precos[i], unidades[i]);
        printf("\n");
    }
}

void calcularCustoIngrediente(void) {
    int codigo;
    float quantidade;

    if (!podeVerCustos()) {
        printf("\nAcesso negado. Consulta de custos restrita ao administrador e gerente.\n");
        return;
    }
    if (totalIngredientes == 0) {
        printf("\nCadastre ingredientes primeiro.\n");
        return;
    }
    listarIngredientes();
    printf("\nNumero do ingrediente: ");
    scanf("%d", &codigo);
    if (codigo < 1 || codigo > totalIngredientes) {
        printf("\nCodigo invalido.\n");
        return;
    }
    printf("Quantidade utilizada (%s): ", unidades[codigo - 1]);
    scanf("%f", &quantidade);
    if (quantidade < 0) {
        printf("\nA quantidade nao pode ser negativa.\n");
        return;
    }
    printf("Custo calculado: R$ %.2f\n", precos[codigo - 1] * quantidade);
}

void registrarEntradaEstoque(void) {
    int codigo;
    float quantidade;

    if (!podeGerenciarEstoque()) {
        printf("\nAcesso negado. Entrada de estoque restrita ao administrador e gerente.\n");
        return;
    }
    if (totalIngredientes == 0) {
        printf("\nCadastre ingredientes primeiro.\n");
        return;
    }
    listarIngredientes();
    printf("\nNumero do ingrediente recebido: ");
    scanf("%d", &codigo);
    if (codigo < 1 || codigo > totalIngredientes) {
        printf("\nCodigo invalido.\n");
        return;
    }
    printf("Quantidade recebida (%s): ", unidades[codigo - 1]);
    scanf("%f", &quantidade);
    if (quantidade <= 0) {
        printf("\nA quantidade deve ser maior que zero.\n");
        return;
    }
    estoqueIngredientes[codigo - 1] += quantidade;
    printf("\nEntrada registrada. Novo estoque de %s: %.3f %s\n",
           nomes[codigo - 1], estoqueIngredientes[codigo - 1], unidades[codigo - 1]);
}

void registrarSaidaEstoque(void) {
    int codigo;
    float quantidade;

    if (!podeGerenciarEstoque()) {
        printf("\nAcesso negado. Saida manual de estoque restrita ao administrador e gerente.\n");
        return;
    }
    if (totalIngredientes == 0) {
        printf("\nCadastre ingredientes primeiro.\n");
        return;
    }
    listarIngredientes();
    printf("\nNumero do ingrediente: ");
    scanf("%d", &codigo);
    if (codigo < 1 || codigo > totalIngredientes) {
        printf("\nCodigo invalido.\n");
        return;
    }
    printf("Quantidade a retirar (%s): ", unidades[codigo - 1]);
    scanf("%f", &quantidade);
    if (quantidade <= 0) {
        printf("\nA quantidade deve ser maior que zero.\n");
        return;
    }
    if (quantidade > estoqueIngredientes[codigo - 1]) {
        printf("\nEstoque insuficiente. Saida nao registrada.\n");
        return;
    }
    estoqueIngredientes[codigo - 1] -= quantidade;
    printf("\nSaida registrada. Novo estoque de %s: %.3f %s\n",
           nomes[codigo - 1], estoqueIngredientes[codigo - 1], unidades[codigo - 1]);
}

/* ==================== PREPARACOES ==================== */
void cadastrarPreparacao(void) {
    int posicao;
    int quantidade;
    int codigo;
    int i;
    float peso;

    if (!podeAdministrar()) {
        printf("\nAcesso negado. Somente o administrador pode cadastrar ou alterar receitas.\n");
        return;
    }
    if (totalIngredientes == 0) {
        printf("\nCadastre ingredientes primeiro.\n");
        return;
    }
    if (totalPreparacoes >= MAX_PREPARACOES) {
        printf("\nLimite de preparacoes atingido.\n");
        return;
    }

    posicao = totalPreparacoes;
    printf("\n=== CADASTRO DE PREPARACAO ===\nNome (ex.: Arroz cozido): ");
    scanf(" %99[^\n]", nomesPreparacoes[posicao]);
    printf("Quantidade de ingredientes (1 a %d): ", MAX_ING_POR_PREPARACAO);
    scanf("%d", &quantidade);
    if (quantidade < 1 || quantidade > MAX_ING_POR_PREPARACAO) {
        printf("\nQuantidade invalida.\n");
        return;
    }

    listarIngredientes();
    for (i = 0; i < quantidade; i++) {
        printf("\nIngrediente %d de %d - numero: ", i + 1, quantidade);
        scanf("%d", &codigo);
        if (codigo < 1 || codigo > totalIngredientes) {
            printf("\nCodigo invalido. Cadastro cancelado.\n");
            return;
        }
        ingredientesPreparacao[posicao][i] = codigo - 1;
        printf("Quantidade usada por lote (%s): ", unidades[codigo - 1]);
        scanf("%f", &quantidadesIngredientes[posicao][i]);
        if (quantidadesIngredientes[posicao][i] <= 0) {
            printf("\nA quantidade deve ser maior que zero. Cadastro cancelado.\n");
            return;
        }
    }

    printf("Peso final esperado de um lote pronto, em kg: ");
    scanf("%f", &peso);
    if (peso <= 0) {
        printf("\nO peso final deve ser maior que zero.\n");
        return;
    }

    pesoFinalKg[posicao] = peso;
    qtdIngredientes[posicao] = quantidade;
    estoquePreparadoKg[posicao] = 0.0f;
    valorEstoquePreparado[posicao] = 0.0f;
    totalPreparacoes++;
    printf("\nPreparacao cadastrada com sucesso!\n");
}

void listarPreparacoes(void) {
    int i, j, ingrediente;
    float custo;

    printf("\n=== PREPARACOES CADASTRADAS ===\n");
    if (totalPreparacoes == 0) {
        printf("Nenhuma preparacao cadastrada.\n");
        return;
    }
    for (i = 0; i < totalPreparacoes; i++) {
        printf("\n%d - %s\n", i + 1, nomesPreparacoes[i]);
        printf("Peso final esperado por lote: %.3f kg\n", pesoFinalKg[i]);
        for (j = 0; j < qtdIngredientes[i]; j++) {
            ingrediente = ingredientesPreparacao[i][j];
            printf("  %s: %.3f %s\n", nomes[ingrediente],
                   quantidadesIngredientes[i][j], unidades[ingrediente]);
        }
        if (podeVerCustos()) {
            custo = calcularCustoPreparacao(i);
            printf("Custo estimado por lote: R$ %.2f\n", custo);
            printf("Custo estimado por kg pronto: R$ %.2f\n", custo / pesoFinalKg[i]);
        }
    }
}

void calcularCustoMarmita(void) {
    int codigo, indice;
    float gramas, custoKg, custoItem, custoTotal = 0.0f;

    if (!podeVerCustos()) {
        printf("\nAcesso negado. Consulta de custos restrita ao administrador e gerente.\n");
        return;
    }
    if (totalPreparacoes == 0) {
        printf("\nCadastre preparacoes primeiro.\n");
        return;
    }
    printf("\n=== CUSTO DOS ALIMENTOS DA MARMITA ===\n");
    listarPreparacoes();
    printf("\nInforme as preparacoes utilizadas. Digite 0 para finalizar.\n");
    while (1) {
        printf("Numero da preparacao: ");
        scanf("%d", &codigo);
        if (codigo == 0) break;
        if (codigo < 1 || codigo > totalPreparacoes) {
            printf("Codigo invalido.\n");
            continue;
        }
        indice = codigo - 1;
        printf("Quantidade servida em gramas: ");
        scanf("%f", &gramas);
        if (gramas <= 0) {
            printf("A quantidade deve ser maior que zero.\n");
            continue;
        }
        custoKg = calcularCustoPreparacao(indice) / pesoFinalKg[indice];
        custoItem = custoKg * gramas / 1000.0f;
        custoTotal += custoItem;
        printf("Custo de %s: R$ %.2f\n", nomesPreparacoes[indice], custoItem);
    }
    printf("\nCUSTO TOTAL ESTIMADO DOS ALIMENTOS: R$ %.2f\n", custoTotal);
    printf("Observacao: este valor nao inclui embalagem, mao de obra, gas, energia ou impostos.\n");
}

/* ==================== PRODUCAO E SOBRAS ==================== */
void registrarProducao(void) {
    int codigo, indice, lotes, i, ingrediente;
    float pesoProduzido, rendimentoEsperado, custoLote, custoTotal;
    float estoqueNecessario;

    if (totalPreparacoes == 0) {
        printf("\nCadastre receitas primeiro.\n");
        return;
    }
    if (totalRegistrosProducao >= MAX_HISTORICO_PRODUCAO) {
        printf("\nLimite do historico de producao atingido.\n");
        return;
    }

    listarPreparacoes();
    printf("\nNumero da receita produzida: ");
    if (scanf("%d", &codigo) != 1 || codigo < 1 || codigo > totalPreparacoes) {
        printf("Codigo invalido.\n");
        return;
    }
    indice = codigo - 1;

    printf("\nReceita: %s\n", nomesPreparacoes[indice]);
    printf("Rendimento esperado por lote: %.3f kg\n", pesoFinalKg[indice]);

    printf("Quantos lotes completos foram preparados? ");
    if (scanf("%d", &lotes) != 1 || lotes <= 0) {
        printf("Quantidade de lotes invalida.\n");
        return;
    }

    rendimentoEsperado = pesoFinalKg[indice] * lotes;
    printf("Rendimento esperado para %d lote(s): %.3f kg\n",
           lotes, rendimentoEsperado);

    /* Verifica todos os ingredientes antes de alterar qualquer estoque. */
    for (i = 0; i < qtdIngredientes[indice]; i++) {
        ingrediente = ingredientesPreparacao[indice][i];
        estoqueNecessario = quantidadesIngredientes[indice][i] * lotes;
        if (estoqueNecessario > estoqueIngredientes[ingrediente] + 0.0001f) {
            printf("\nEstoque insuficiente de %s. Producao nao registrada.\n",
                   nomes[ingrediente]);
            printf("Necessario: %.3f %s | Disponivel: %.3f %s\n",
                   estoqueNecessario, unidades[ingrediente],
                   estoqueIngredientes[ingrediente], unidades[ingrediente]);
            return;
        }
    }

    printf("Informe o peso REAL obtido apos o preparo, em kg: ");
    if (scanf("%f", &pesoProduzido) != 1 || pesoProduzido <= 0) {
        printf("Peso invalido. Producao nao registrada.\n");
        return;
    }

    custoLote = calcularCustoPreparacao(indice);
    custoTotal = custoLote * lotes;

    /* Consumo de ingredientes baseado na receita x lotes, nunca no peso final. */
    for (i = 0; i < qtdIngredientes[indice]; i++) {
        ingrediente = ingredientesPreparacao[indice][i];
        estoqueIngredientes[ingrediente] -= quantidadesIngredientes[indice][i] * lotes;
    }

    estoquePreparadoKg[indice] += pesoProduzido;
    valorEstoquePreparado[indice] += custoTotal;

    historicoPreparacao[totalRegistrosProducao] = indice;
    historicoLotes[totalRegistrosProducao] = lotes;
    historicoPesoKg[totalRegistrosProducao] = pesoProduzido;
    historicoRendimentoEsperadoKg[totalRegistrosProducao] = rendimentoEsperado;
    historicoCusto[totalRegistrosProducao] = custoTotal;
    historicoData[totalRegistrosProducao] = time(NULL);
    totalRegistrosProducao++;

    printf("\n=== PRODUCAO REGISTRADA ===\n");
    printf("Receita: %s\n", nomesPreparacoes[indice]);
    printf("Lotes preparados: %d\n", lotes);
    printf("Rendimento esperado: %.3f kg\n", rendimentoEsperado);
    printf("Peso real obtido: %.3f kg\n", pesoProduzido);
    printf("Diferenca de rendimento: %+.3f kg\n",
           pesoProduzido - rendimentoEsperado);
    printf("Custo estimado dos ingredientes: R$ %.2f\n", custoTotal);
    printf("Estoque pronto atual: %.3f kg\n", estoquePreparadoKg[indice]);
    printf("Custo medio estimado por kg pronto: R$ %.2f\n",
           valorEstoquePreparado[indice] / estoquePreparadoKg[indice]);
    printf("\nOs ingredientes foram descontados conforme a receita e os lotes.\n");
}

void relatorioProducao(void) {
    int i, indice;
    char dataHora[30];
    struct tm *infoTempo;

    printf("\n=== HISTORICO DE PRODUCAO E ESTOQUE PRONTO ===\n");
    if (totalRegistrosProducao == 0) {
        printf("Nenhuma producao registrada.\n");
        return;
    }

    printf("\n--- Historico de producoes ---\n");
    for (i = 0; i < totalRegistrosProducao; i++) {
        indice = historicoPreparacao[i];
        infoTempo = localtime(&historicoData[i]);
        if (infoTempo != NULL) {
            strftime(dataHora, sizeof(dataHora), "%d/%m/%Y %H:%M:%S", infoTempo);
        } else {
            strcpy(dataHora, "Data indisponivel");
        }
        printf("\nRegistro %d - %s\n", i + 1, nomesPreparacoes[indice]);
        printf("Data e hora: %s\n", dataHora);
        printf("Lotes preparados: %d\n", historicoLotes[i]);
        printf("Rendimento esperado: %.3f kg\n", historicoRendimentoEsperadoKg[i]);
        printf("Peso real obtido: %.3f kg\n", historicoPesoKg[i]);
        printf("Diferenca de rendimento: %+.3f kg\n",
               historicoPesoKg[i] - historicoRendimentoEsperadoKg[i]);
        if (podeVerCustos()) {
            printf("Custo estimado dos ingredientes: R$ %.2f\n", historicoCusto[i]);
        }
    }

    printf("\n--- Estoque atual de preparacoes prontas (sobras disponiveis) ---\n");
    for (i = 0; i < totalPreparacoes; i++) {
        printf("%s: %.3f kg disponiveis\n",
               nomesPreparacoes[i], estoquePreparadoKg[i]);
        if (podeVerCustos() && estoquePreparadoKg[i] > 0.0001f) {
            printf("  Custo estimado do estoque: R$ %.2f\n",
                   valorEstoquePreparado[i]);
        }
    }
    printf("\nO estoque pronto restante nao significa necessariamente desperdicio.\n");
}

void relatorioEstoque(void) {
    int i;
    if (!podeGerenciarEstoque()) {
        printf("\nAcesso negado. Relatorio de estoque restrito ao administrador e gerente.\n");
        return;
    }
    printf("\n=== RELATORIO DE ESTOQUE ===\n");
    if (totalIngredientes == 0) {
        printf("Nenhum ingrediente cadastrado.\n");
        return;
    }
    for (i = 0; i < totalIngredientes; i++) {
        printf("%s: %.3f %s | Valor estimado: R$ %.2f\n",
               nomes[i], estoqueIngredientes[i], unidades[i],
               estoqueIngredientes[i] * precos[i]);
    }
}


/* ==================== VENDAS E FATURAMENTO ==================== */
void registrarVenda(void) {
    int quantidadeMarmitas;
    int i;
    float precoVenda;
    float gramas[MAX_PREPARACOES];
    float quantidadeNecessaria[MAX_PREPARACOES];
    float custoPorMarmita = 0.0f;
    float custoKg;
    float faturamentoVenda;
    float custoVenda;
    int temPreparacao = 0;

    if (!podeVerCustos()) {
        printf("\nAcesso negado. Registro de vendas restrito ao administrador e gerente.\n");
        return;
    }
    if (totalPreparacoes == 0) {
        printf("\nCadastre preparacoes antes de registrar vendas.\n");
        return;
    }

    printf("\n=== REGISTRO DE VENDA ===\n");
    printf("Quantidade de marmitas vendidas: ");
    if (scanf("%d", &quantidadeMarmitas) != 1 || quantidadeMarmitas <= 0) {
        printf("Quantidade invalida. Venda nao registrada.\n");
        return;
    }
    printf("Preco de venda por marmita (R$): ");
    if (scanf("%f", &precoVenda) != 1 || precoVenda <= 0) {
        printf("Preco invalido. Venda nao registrada.\n");
        return;
    }

    printf("\nInforme os gramas de cada preparacao em UMA marmita.\n");
    printf("Use 0 quando a preparacao nao fizer parte da marmita.\n");
    listarPreparacoes();

    /* Primeiro coleta e valida a composicao; nenhum estoque e alterado ainda. */
    for (i = 0; i < totalPreparacoes; i++) {
        printf("\n%s - gramas por marmita: ", nomesPreparacoes[i]);
        if (scanf("%f", &gramas[i]) != 1 || gramas[i] < 0) {
            printf("Quantidade invalida. Venda cancelada.\n");
            return;
        }
        if (gramas[i] > 0) temPreparacao = 1;
        quantidadeNecessaria[i] = gramas[i] * quantidadeMarmitas / 1000.0f;
        if (quantidadeNecessaria[i] > estoquePreparadoKg[i] + 0.0001f) {
            printf("\nEstoque pronto insuficiente de %s.\n", nomesPreparacoes[i]);
            printf("Necessario: %.3f kg | Disponivel: %.3f kg\n",
                   quantidadeNecessaria[i], estoquePreparadoKg[i]);
            printf("Venda cancelada; nenhum estoque foi alterado.\n");
            return;
        }
        if (gramas[i] > 0) {
            if (estoquePreparadoKg[i] <= 0.0001f) {
                printf("\nNao ha estoque pronto de %s. Venda cancelada.\n",
                       nomesPreparacoes[i]);
                return;
            }
            custoKg = valorEstoquePreparado[i] / estoquePreparadoKg[i];
            custoPorMarmita += custoKg * gramas[i] / 1000.0f;
        }
    }

    if (!temPreparacao) {
        printf("\nA venda precisa incluir ao menos uma preparacao. Venda cancelada.\n");
        return;
    }

    faturamentoVenda = quantidadeMarmitas * precoVenda;
    custoVenda = quantidadeMarmitas * custoPorMarmita;

    /* Baixa os alimentos prontos e seus custos somente apos validar tudo. */
    for (i = 0; i < totalPreparacoes; i++) {
        if (quantidadeNecessaria[i] > 0) {
            custoKg = valorEstoquePreparado[i] / estoquePreparadoKg[i];
            valorEstoquePreparado[i] -= custoKg * quantidadeNecessaria[i];
            estoquePreparadoKg[i] -= quantidadeNecessaria[i];

            if (estoquePreparadoKg[i] < 0.0001f) {
                estoquePreparadoKg[i] = 0.0f;
                valorEstoquePreparado[i] = 0.0f;
            }
        }
    }

    totalMarmitasVendidas += quantidadeMarmitas;
    faturamentoTotal += faturamentoVenda;
    custoAlimentosVendidos += custoVenda;

    printf("\n=== VENDA REGISTRADA ===\n");
    printf("Marmitas vendidas nesta operacao: %d\n", quantidadeMarmitas);
    printf("Preco por marmita: R$ %.2f\n", precoVenda);
    printf("Faturamento desta venda: R$ %.2f\n", faturamentoVenda);
    printf("Custo estimado dos alimentos por marmita: R$ %.2f\n", custoPorMarmita);
    printf("Custo estimado dos alimentos vendidos: R$ %.2f\n", custoVenda);
    printf("Resultado apos custo dos alimentos: R$ %.2f\n", faturamentoVenda - custoVenda);
    printf("\nO estoque de alimentos prontos foi atualizado.\n");
    printf("Este resultado nao desconta embalagem, mao de obra, gas, energia ou impostos.\n");
}

void relatorioFinanceiro(void) {
    float resultado;

    if (!podeVerCustos()) {
        printf("\nAcesso negado. Relatorio financeiro restrito ao administrador e gerente.\n");
        return;
    }

    resultado = faturamentoTotal - custoAlimentosVendidos;

    printf("\n=== RELATORIO FINANCEIRO ===\n");
    printf("Total de marmitas vendidas: %d\n", totalMarmitasVendidas);
    printf("Faturamento acumulado: R$ %.2f\n", faturamentoTotal);
    printf("Custo estimado dos alimentos vendidos: R$ %.2f\n", custoAlimentosVendidos);
    printf("Resultado apos custo dos alimentos: R$ %.2f\n", resultado);
    printf("\nImportante: este resultado nao e lucro liquido.\n");
    printf("Nao inclui embalagem, mao de obra, gas, energia, impostos e outras despesas.\n");
}

/* ==================== MENU ==================== */
void mostrarMenu(void) {
    printf("\n=== RESTAURANTE MEDEIRIANA ===\n");
    printf("Usuario: %s | Perfil: ", usuarioLogado);
    if (nivelAcesso == ADMINISTRADOR) printf("Administrador\n");
    else if (nivelAcesso == GERENTE) printf("Gerente\n");
    else printf("Colaborador\n");

    /* As opcoes sao exibidas em ordem numerica. */
    if (podeAdministrar()) printf("1 - Cadastrar ingrediente\n");
    printf("2 - Listar ingredientes\n");
    if (podeVerCustos()) printf("3 - Calcular custo de ingrediente\n");
    if (podeAdministrar()) printf("4 - Cadastrar preparacao/receita\n");
    printf("5 - Listar preparacoes\n");
    if (podeVerCustos()) printf("6 - Calcular custo dos alimentos da marmita\n");
    printf("7 - Registrar producao e sobras\n");
    printf("8 - Consultar relatorio de producao e sobras\n");
    if (podeGerenciarEstoque()) {
        printf("9 - Registrar entrada de estoque\n");
        printf("10 - Registrar saida manual de estoque\n");
        printf("11 - Consultar relatorio de estoque\n");
    }
    if (podeVerCustos()) {
        printf("12 - Registrar venda e faturamento\n");
        printf("13 - Consultar relatorio financeiro\n");
    }
    printf("14 - Trocar usuario / Fazer logout\n");
    printf("0 - Sair\n");
    printf("Escolha uma opcao: ");
}

int main(void) {
    int opcao;
    int encerrando = 0;

    while (!encerrando) {
        while (nivelAcesso == 0) {
            fazerLogin();
            if (nivelAcesso == 0) {
                char tentarNovamente;
                printf("Deseja tentar novamente? (s/n): ");
                scanf(" %c", &tentarNovamente);
                if (tentarNovamente != 's' && tentarNovamente != 'S') {
                    printf("Encerrando o programa.\n");
                    return 1;
                }
            }
        }

        mostrarMenu();
        if (scanf("%d", &opcao) != 1) {
            printf("\nEntrada invalida. Encerrando o programa.\n");
            return 1;
        }

        switch (opcao) {
            case 1: cadastrarIngrediente(); break;
            case 2: listarIngredientes(); break;
            case 3: calcularCustoIngrediente(); break;
            case 4: cadastrarPreparacao(); break;
            case 5: listarPreparacoes(); break;
            case 6: calcularCustoMarmita(); break;
            case 7: registrarProducao(); break;
            case 8: relatorioProducao(); break;
            case 9: registrarEntradaEstoque(); break;
            case 10: registrarSaidaEstoque(); break;
            case 11: relatorioEstoque(); break;
            case 12: registrarVenda(); break;
            case 13: relatorioFinanceiro(); break;
            case 14: encerrarSessao(); break;
            case 0:
                printf("\nEncerrando o programa.\n");
                encerrando = 1;
                break;
            default:
                printf("\nOpcao invalida ou indisponivel para seu perfil.\n");
        }
    }

    return 0;
}
