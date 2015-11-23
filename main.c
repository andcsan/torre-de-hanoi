#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

#define maxTorres 3 // Quantidade máxima de torres (colunas) na matriz
#define maxDiscos 7 // Quantidade máxima de discos (linhas) na matriz

typedef struct // Estrutura para armazenar dados do usuario
{
    float score;
    char nickname[9];
    int tempo;
    int movimentos;
    int discos;
} PLAYER;

typedef struct // Estrutura para armazenar as coordenadas de movimento do jogador
{
    int orig;
    int dest;
} MOV;

void gotoxy(int coluna, int linha);

// Funções simples de impressão na tela
void DesenhaTopo();
void DesenhaHanoi();
void Sobre();
void ImprimeScore();
void ImprimeMovimento(char arq_mov[]);
void Configuracoes(int *n);
void Animacao(int i, int j, int orig, int dest, int topo);
void Menu(char *opcao);
void OrigemDestino(MOV *coord);

// Funções para execução do jogo e criação de arquivos
void IniciaTorre(int discos);
void Jogo(int discos, int modo);
void Score(PLAYER *jog);
int RealizaMovimento(FILE *moves, int modo, char arq_mov[]);
void MovimentoRecursivo(int disco, int orig, int dest, int aux);
void Undo(MOV *coord, char arq_mov[]);
int TrocaDiscos(int orig, int dest);
int AnalisaVitoria(int x);

// Variáveis globais
int hanoi[maxDiscos][maxTorres]; // Matriz de dados da torre de hanoi
int qtdMovimentos;               // Quantidade de movimentos total realizado pelo usuário.
int velocidade;                  // Velocidade da animação

int main()
{
    int qtdDiscos = 5;
    char opcao;
    velocidade = 20; // Atribui um valor padrão à velocidade

    // Loop principal
    while (1)
    {
        qtdMovimentos = 0;
        Menu(&opcao);
        switch (opcao)
        {
        case 'J':
            Jogo(qtdDiscos, 1); // Executa a função para jogar. O argumento 1 indica o modo competicão.
            break;
        case 'T':
            Jogo(qtdDiscos, 0); // Executa a função para jogar. O argumento 0 indica o modo de treino.
            break;
        case 'R':
            IniciaTorre(qtdDiscos);
            DesenhaHanoi();
            gotoxy(6, 18);
            printf("Pressione qualquer tecla para comecar.");
            getch();
            MovimentoRecursivo(qtdDiscos - 1, 0, 2, 1); // Função recursiva responsável por resolver a Torre de Hanói
            gotoxy(6, 18);
            printf("Pressione qualquer tecla para voltar ao menu.");
            getch();
            break;
        case 'C':
            Configuracoes(&qtdDiscos);
            break;
        case 'E':
            ImprimeScore();
            break;
        case 'S':
            Sobre();
            break;
        case 'X':
            exit(1); // Sai do programa
            break;
        }
    };

    return 0;
}

// Recebe como parâmetro a quantidade de discos e o modo de jogo
void Jogo(int discos, int modo)
{
    time_t tempo_inicial;           // Tempo de jogo
    int min = pow(2, discos) - 1;   // Minimo de movimentos possíveis
    int sair = 0;                   // Controla a saída do jogador
    PLAYER jogador;                 // Dados do usuário
    FILE *arquivo_movimento = NULL; // Declara um ponteiro do tipo FILE para ler e escrever movimentos do usuário
    char nome_arq_jog[18];          // String para armazenar o nome do arquivo de movimentos
    char opc;                       // Coletar a opção do usuário

    IniciaTorre(discos); // Inicia a torre com a respectiva quantidade de discos

    jogador.tempo = 0;
    jogador.movimentos = 0;
    jogador.discos = discos;

    do
    {
        // Caso o modo de jogo seja competitivo, pergunta o nome do jogador e dá um nome ao seu arquivo de movimentos
        if (modo == 1)
        {
            do // Irá executar as instruçoes até que um nome válido seja inserido
            {
                DesenhaTopo();
                if (strlen(jogador.nickname) > 8)
                {
                    gotoxy(6, 7);
                    printf("Nickname ultrapassa 8 caracteres. Escolha outro.");
                }
                else if (arquivo_movimento != NULL)
                {
                    gotoxy(6, 7);
                    printf("Este nickname ja foi registrado. Escolha outro.");
                }

                gotoxy(6, 6);
                printf("Informe seu nickname (max. 8): ");
                gotoxy(37, 6);
                scanf(" %[^\n]s", jogador.nickname); // Coleta o nickname inserido pelo usuário. Aceita espaços
            } while (strlen(jogador.nickname) > 8);

            strcpy(nome_arq_jog, jogador.nickname);
            strcat(nome_arq_jog, "_mov.dat");
            arquivo_movimento = fopen(nome_arq_jog, "rb");
        }
        // Caso o modo de jogo seja de treinamento, dá o nome do arquivo de treino_mov.dat
        else
        {
            strcpy(nome_arq_jog, "treino");
            strcat(nome_arq_jog, "_mov.dat");
            arquivo_movimento = NULL;
        }
    } while (arquivo_movimento != NULL);

    arquivo_movimento = fopen(nome_arq_jog, "wb"); // Abre um arquivo com o nome do jogador ou do treino como escrita

    tempo_inicial = time(NULL); // Atribui à variavel tempo_inicial o tempo do computador no inicio do jogo

    while (AnalisaVitoria(discos))
    {
        DesenhaHanoi();
        gotoxy(8, 17);
        printf("Movimentos : %d", jogador.movimentos);
        gotoxy(8, 18);
        printf("Tempo      : %d", jogador.tempo);
        gotoxy(49, 17);
        printf("Pressione X na origem");
        gotoxy(49, 18);
        printf("para voltar ao menu.");

        // Caso o modo seja treinamento, apresentar algumas informações extras, como a possibilidade de voltar jogadas
        if (modo == 0)
        {
            gotoxy(49, 20);
            printf("Pressione 0 na origem");
            gotoxy(49, 21);
            printf("para voltar uma jogada.");
            gotoxy(8, 20);
            printf("MODO TREINAMENTO");
        }

        // Chama a função responsável por gerenciar os movimentos do usuário
        sair = RealizaMovimento(arquivo_movimento, modo, nome_arq_jog);

        if (sair == 1)
        {
            fclose(arquivo_movimento);
            remove(nome_arq_jog);
            return;
        }
        else
        {
            jogador.movimentos = qtdMovimentos;
            jogador.tempo = time(NULL) - tempo_inicial;
            fclose(arquivo_movimento);                      // Fecha o arquivo de histórico de movimentos para atualizar
            freopen(nome_arq_jog, "ab", arquivo_movimento); // Reabre o historico de movimentos em modo de atualização
        }
    }

    // Saindo do loop principal, termina as operações da função Jogo
    DesenhaHanoi();
    jogador.score = (float)pow(jogador.discos, jogador.discos) / (jogador.tempo + jogador.movimentos); // Calcula o score do jogador
    gotoxy(19, 17);
    printf("PARABENS! Voce concluiu a Torre de Hanoi!", jogador.nickname);
    gotoxy(26, 19);
    printf("Movimentos: %d (minimo %d)", jogador.movimentos, min);
    gotoxy(26, 20);
    printf("Tempo     : %d segundos", jogador.tempo);

    // Caso o jogador esteja em modo competitivo, armazenará o score dele em um arquivo
    if (modo == 1)
    {
        Score(&jogador);
    }

    printf("\n\n      Deseja ver os movimentos que voce fez durante o jogo? [S/N]");

    do
    {
        opc = toupper(getch());
    } while (opc != 'S' && opc != 'N');

    if (opc == 'S')
    {
        ImprimeMovimento(nome_arq_jog);
        printf("\n\n      Pressione qualquer tecla para voltar ao menu.");
        getch();
    }
}

// Recebe como parâmetro a quantidade de discos
void IniciaTorre(int discos)
{
    int i, j, contador = discos; // Variáveis auxiliares

    for (i = maxDiscos - 1; i >= 0; i--)
    {
        for (j = 0; j < maxTorres; j++)
        {
            if (j > 0)
                hanoi[i][j] = 0;
            else if (i > (maxDiscos - 1) - discos)
                hanoi[i][j] = contador;
            else
                hanoi[i][j] = 0;
        }
        contador--;
    }
}

// Recebe como parâmetro o arquivo de movimentos, modo de jogo e nome do arquivo
int RealizaMovimento(FILE *moves, int modo, char nome_arq_mov[])
{
    MOV coord;             // Coordenadas informadas pelo jogador
    int mov_valido = 0;    // Verificar se o movimento do jogador é válido
    OrigemDestino(&coord); // Chama a função para coletar os dados de origem e destino do jogador

    // Caso a origem informada pelo jogador seja 0, executará o undo se possível
    if (coord.orig == 0)
    {
        // Para executar o undo, é necessário que o modo seja de treinamento
        if (modo == 0)
        {
            Undo(&coord, nome_arq_mov);

            // Caso a quantidade de movimentos seja 0, não irá trocar os discos
            if (qtdMovimentos < 1)
            {
                gotoxy(6, 23);
                printf("Nao foi possivel realizar a volta! Pressione qualquer tecla.");
                getch();
            }
            // Caso seja maior que 0, indica que poderá realizar a troca de movimentos
            else
            {
                TrocaDiscos((coord.orig) - 1, (coord.dest) - 1);
                qtdMovimentos = qtdMovimentos - 2; // Decrementa a quantidade de movimentos
            }

            fclose(moves);                          // Fecha o arquivo de historico de movimentos para que possa ser removido
            remove(nome_arq_mov);                   // Remove o arquivo com o nome do historico de movimentos
            rename("temporario.dat", nome_arq_mov); // Renomeia o arquivo temporario.dat criado na função Undo para o nome do arquivo antigo
        }
        // Caso o modo de jogo do jogador seja competitivo, não permite que ele execute o Undo
        else
        {
            gotoxy(6, 23);
            printf("Voce nao esta no modo de treino! Pressione qualquer tecla.");
            getch();
        }
        return 0;
    }
    else if (coord.orig == -1)
    {
        return 1;
    }
    // Caso a coordenada de origem seja igual a de destino, nao permite o movimento
    else if (coord.orig == coord.dest)
    {
        mov_valido = 1;
        gotoxy(6, 23);
        printf("Movimento Invalido! Pressione qualquer tecla para continuar...");
        getch();
        return 0;
    }
    // Caso a coordenada de origem seja diferente do destino, permite a troca de discos
    else if (coord.orig != coord.dest)
    {
        mov_valido = TrocaDiscos((coord.orig) - 1, (coord.dest) - 1);

        // Verifica se a troca de discos foi efetuada com sucesso
        if (!mov_valido)
        {
            gotoxy(6, 23);
            printf("Movimento Invalido! Pressione qualquer tecla para continuar...");
            getch();
        }
        else
        {
            fwrite(&coord, sizeof(MOV), 1, moves); // Armazena os movimentos
        }
        return 0;
    }
}

// Recebe como parâmetro o endereço da coordenada
void OrigemDestino(MOV *coord)
{
    // Imprime uma caixa onde será informada a origem e destino
    gotoxy(35, 16);
    printf("%c%c%c%c%c%c%c%c%c", 201, 205, 205, 205, 205, 205, 205, 205, 187);
    gotoxy(35, 17);
    printf("%c   %c   %c", 186, 16, 186);
    gotoxy(35, 18);
    printf("%c%c%c%c%c%c%c%c%c", 200, 205, 205, 205, 205, 205, 205, 205, 188);

    do
    {
        gotoxy(37, 17);
        coord->orig = toupper(getche()) - 48;
        if (coord->orig == 40)
        {
            coord->orig = -1;
        }
        if (coord->orig < -1 || coord->orig > 3)
        {
            gotoxy(6, 23);
            printf("Esta torre de origem nao existe! Informe novamente.\n");
        }
    } while (coord->orig < -1 || coord->orig > 3);

    if (coord->orig != 0 && coord->orig != -1)
    {
        do
        {
            gotoxy(41, 17);
            coord->dest = getche() - 48;
            if (coord->dest < 1 || coord->dest > 3)
            {
                gotoxy(6, 23);
                printf("Esta torre de destino nao existe! Informe novamente.\n");
            }
        } while (coord->dest < 1 || coord->dest > 3);
    }
}

// Recebe como parâmetro as coordenadas e o nome do arquivo de movimento
void Undo(MOV *coord, char nome_arq_mov[])
{
    FILE *undo;        // Declara um ponteiro do tipo FILE para ler os dados do arquivo de movimento
    FILE *tmp;         // Declara um ponteiro do tipo FILE para inserir dados em um arquivo temporário
    MOV aux;           // Variável do tipo MOV para armazenar os dados lidos do arquivo de movimentos
    int i;             // Variavel auxiliar
    long int byte_pos; // Variavel para coletar a posição do byte

    undo = fopen(nome_arq_mov, "rb");    // Abre o arquivo de movimentos em modo de leitura
    tmp = fopen("temporario.dat", "wb"); // Cria um novo arquivo temporario

    // Caso o arquivo de historico de movimentos nao exista, apresenta uma mensagem de erro
    if (undo == NULL)
    {
        gotoxy(6, 21);
        printf("      Historico de movimentos inexistente.\n");
    }
    else
    {
        fseek(undo, sizeof(MOV) * (-1), SEEK_END);
        fread(&coord->dest, sizeof(int), 1, undo); // Atribui à coordenada de destino o valor da coordenada de origem armazenada no arquivo
        fread(&coord->orig, sizeof(int), 1, undo); // Atribui à coordenada de origem o valor da coordenada de destino armazenada no arquivo
    }

    byte_pos = ftell(undo) - sizeof(MOV); // Atribui a byte_pos a posição atual do indicado menos um bloco de bytes de tamanho MOV
    rewind(undo);                         // Posiciona o indicador no começo do arquivo

    // Irá copiar todos os dados do arquivo de historico para o arquivo temporario, exceto a ultima posição utilizada pra retornar a jogada
    for (i = 0; i < byte_pos; i = i + sizeof(MOV))
    {
        fread(&aux, sizeof(MOV), 1, undo);
        fwrite(&aux, sizeof(MOV), 1, tmp);
    }

    fclose(undo);
    fclose(tmp);
}

// Recebe como parâmetro a origem e o destino
int TrocaDiscos(int orig, int dest)
{
    int i = 0, j = 0; // Variaveis auxiliares
    int iaux, jaux;   // Variáveis auxiliares
    int topoOrigem;   // Valor do disco no topo da origem
    int topoDestino;  // Valor do disco no topo do destino

    // Caminha na torre de origem até encontrar o primeiro valor diferente de zero
    while (i < maxDiscos)
    {
        iaux = i;
        if (hanoi[i][orig] == 0)
        {
            i++;
        }
        else
        {
            i = maxDiscos;
        }
    }

    // Caminha na torre de destino até encontrar o primeiro valor diferente de zero
    while (j < maxDiscos)
    {
        jaux = j;
        if (hanoi[j][dest] == 0)
        {
            j++;
        }
        else
        {
            j = maxDiscos;
        }
    }

    topoDestino = hanoi[jaux][dest]; // Atribui à variavel o valor contido no ultimo disco da torre de origem
    topoOrigem = hanoi[iaux][orig];  // Atribui à variavel o valor contido no ultimo disco da torre de destino

    // Caso não haja nenhum disco na torre de origem, retorna 0
    if (topoOrigem == 0)
    {
        return 0;
    }
    // Caso não haja nenhum disco na torre de destino, permite movimentar e retorna 1
    else if (topoDestino == 0)
    {
        Animacao(iaux, jaux, orig, dest, topoOrigem); // Executa a animação dos discos se movimentando
        hanoi[iaux][orig] = 0;                        // Atribui ao valor do topo na torre de origem o valor 0
        hanoi[jaux][dest] = topoOrigem;               // Atribui ao valor do topo na torre de destino o valor do disco que foi movido
        qtdMovimentos++;                              // Incrementa a quantidade de movimentos
        return 1;                                     // Retorna 1, indicando o sucesso na operação
    }
    // Caso o disco no topo da torre de origem seja menor que o disco na torre de destino, permite movimentar
    else if (topoOrigem < topoDestino)
    {
        Animacao(iaux, jaux, orig, dest, topoOrigem); // Executa a animação dos discos se movimentando
        hanoi[iaux][orig] = 0;                        // Atribui ao valor do topo na torre de origem o valor 0
        hanoi[jaux - 1][dest] = topoOrigem;           // Atribui ao valor do topo na torre de destino o valor do disco que foi movido
        qtdMovimentos++;                              // Incrementa a quantidade de movimentos
        return 1;                                     // Retorna 1, indicando o sucesso na operação
    }
    // Caso o disco no topo da torre de origem seja maior que o disco na torre de destino, retorna 0 e nao movimenta
    else if (topoOrigem > topoDestino)
    {
        return 0;
    }

    return 0;
}

// Recebe como parâmetro a quantidade de discos
int AnalisaVitoria(int x)
{
    // Caso a ultima posição na torre 2 ou 3 tenha o disco de valor 1, retorna 0
    if (hanoi[maxDiscos - x][maxTorres - 1] == 1 || hanoi[maxDiscos - x][maxTorres - 2] == 1)
    {
        return 0;
    }
    // Caso contrário, retorna 1
    else
    {
        return 1;
    }
}

// Recebe como parâmetro a estrutura de dados do jogador
void Score(PLAYER *jog)
{
    FILE *arquivo_score; // Declara um ponteiro do tipo FILE para armazenar dados do arquivo de score
    PLAYER aux, aux2;    // Declara duas variavéis do tipo PLAYER para auxiliar na ordenação
    size_t i = 0;        // Controlar o laço for
    size_t last_byte;    // Coletar o último byte do arquivo

    arquivo_score = fopen("score.dat", "ab");      // Abre o arquivo de scores em modo de atualização
    fwrite(jog, sizeof(PLAYER), 1, arquivo_score); // Escreve no arquivo de scores, na ultima posição, o score a ser inserido
    last_byte = ftell(arquivo_score);              // Coleta o último byte do arquivo de scores (igual ao tamanho do arquivo)
    freopen("score.dat", "r+b", arquivo_score);    // Reabe o arquivo de scores em modo de leitura

    if (arquivo_score == NULL)
    {
        printf("Falha ao armazenar o score.\n");
        getch();
    }
    else
    {
        for (i = 0; i < last_byte - sizeof(PLAYER); i = i + sizeof(PLAYER))
        {
            fseek(arquivo_score, -1 * sizeof(PLAYER), SEEK_END); // Posiciona no começo do ultimo bloco
            fread(&aux2, sizeof(PLAYER), 1, arquivo_score);      // Lê o ultimo bloco
            fseek(arquivo_score, i, SEEK_SET);                   // Posiciona no bloco de posição 'i'
            fread(&aux, sizeof(PLAYER), 1, arquivo_score);       // Lê o bloco na posição 'i'

            // Caso o score na posição 'i' seja menor, troca os dois de posição
            if (aux.score < aux2.score)
            {
                fseek(arquivo_score, i, SEEK_SET);                   // Posiciona no o bloco de posição 'i'
                fwrite(&aux2, sizeof(PLAYER), 1, arquivo_score);     // Escreve no bloco de posição 'i' o bloco da ultima posição
                fseek(arquivo_score, -1 * sizeof(PLAYER), SEEK_END); // Posiciona no bloco de última posição
                fwrite(&aux, sizeof(PLAYER), 1, arquivo_score);      // Escreve no bloco de última posição o bloco da posição 'i'
            }
        }
    }

    fclose(arquivo_score); // Fecha o arquivo de scores
}

// Função para imprimir todos os scores e exibir os movimentos
void ImprimeScore()
{
    FILE *le_score;
    PLAYER p;
    int contador = 1;
    int linha = 8;
    size_t last_byte;
    char opc;
    char nome_opc[9];

    le_score = fopen("score.dat", "rb");
    DesenhaTopo();

    if (le_score == NULL)
    {
        gotoxy(6, 6);
        printf("Arquivo de scores inexistente. Jogue pelo menos uma vez para criar.");
    }
    else
    {
        fseek(le_score, 0, SEEK_END);
        last_byte = ftell(le_score);
        rewind(le_score);

        while (ftell(le_score) < last_byte)
        {
            gotoxy(11, 6);
            printf("%-14s%-14s%-14s%-14s%s", "Score", "Nome", "Discos", "Moves", "Tempo");
            gotoxy(6, ++linha);
            printf("%d%c ", contador, 167);
            contador++;
            gotoxy(11, linha);
            fread(&p, sizeof(p), 1, le_score); // Atribui à variavel p o bloco de memoria do score
            printf("%-14.2f%-14s%-14d%-14d%-14d", p.score, p.nickname, p.discos, p.movimentos, p.tempo);
        }

        printf("\n\n      Deseja ver o historico de algum jogador? [S/N]");

        do
        {
            opc = toupper(getch());
        } while (opc != 'S' && opc != 'N');

        if (opc == 'S')
        {
            printf("\n\n      Informe o nome do jogador: ");
            scanf(" %[^\n]s", nome_opc);
            strcat(nome_opc, "_mov.dat");
            ImprimeMovimento(nome_opc);
        }
    }
    fclose(le_score); // Fecha o arquivo de leitura do score dos jogadores
    printf("\n\n      Pressione qualquer tecla para voltar ao menu.");
    getch();
}

void ImprimeMovimento(char arq_mov[])
{
    FILE *le_mov;
    MOV mov_jogador;
    size_t last_byte;

    le_mov = fopen(arq_mov, "rb");

    if (le_mov == NULL)
    {
        printf("\n      Este jogador nao existe!");
    }
    else
    {
        DesenhaTopo();
        fseek(le_mov, 0, SEEK_END); // Posiciona o indicador do arquivo de historicos no final
        last_byte = ftell(le_mov);  // Coleta a posição do último byte
        rewind(le_mov);             // Posiciona o indicador no começo do arquivo
        gotoxy(31, 5);
        printf("Origem - Destino\n");

        while (ftell(le_mov) < last_byte)
        {
            fread(&mov_jogador, sizeof(MOV), 1, le_mov);
            printf("\n\t\t\t\t   %d  -  %d\n", mov_jogador.orig, mov_jogador.dest);
        }
    }

    fclose(le_mov); // Fecha o arquivo de leitura do historico de movimentos
}

// Recebe como parâmetro os discos, a origem, o destino e um auxiliar
void MovimentoRecursivo(int disco, int orig, int dest, int aux)
{
    // Caso a variavel disco seja 0
    if (disco == 0)
    {
        TrocaDiscos(orig, dest); // Coloca o disco da origem no destino
        DesenhaHanoi();
        gotoxy(33, 18);
        printf("Movimentos: %d\n", qtdMovimentos);
        Sleep(velocidade); // Pausa o programa
    }
    else
    {
        MovimentoRecursivo(disco - 1, orig, aux, dest); // Chama a própria função, trocando o destino com o auxiliar e decrementando o disco
        TrocaDiscos(orig, dest);                        // Coloca o disco da origem no destino
        DesenhaHanoi();
        gotoxy(33, 18);
        printf("Movimentos: %d\n", qtdMovimentos);
        Sleep(velocidade);
        MovimentoRecursivo(disco - 1, aux, dest, orig); // Chama a própria função, trocando a origem com auxiliar e destino
    }
}

// Recebe como parâmetro o endereço da variavel que armazena a quantidade de discos
void Configuracoes(int *n)
{
    char op;

    DesenhaTopo();
    gotoxy(0, 6);
    printf("      A - Configurar quantidade de discos\n\n");
    printf("      B - Configurar intervalo da animacao\n\n\n");
    printf("      X - Cancelar\n\n");

    do
    {
        op = toupper(getch());
    } while (op != 'A' && op != 'B' && op != 'X');

    // Opção A, configura a quantidade de discos
    if (op == 'A')
    {
        DesenhaTopo();
        gotoxy(6, 6);
        printf("Quantidade de discos (max %d): ", maxDiscos);

        do
        {
            scanf("%d", n); // Recebe o novo valor da quantidade de discos

            if (*n < 1 || *n > maxDiscos)
            {
                gotoxy(6, 6);
                printf("Quantidade de discos invalida, informe novamente (max %d): ", maxDiscos);
            }
        } while (*n < 1 || *n > maxDiscos);
    }
    // Opção B, configura a velocidade da animação
    else if (op == 'B')
    {
        do
        {
            DesenhaTopo();
            gotoxy(6, 6);

            if (velocidade < 10 || velocidade > 150)
            {
                gotoxy(6, 7);
                printf("Velocidade invalida, informe novamente.");
            }

            gotoxy(6, 6);
            printf("Velocidade da animacao (em ms) [Min: 10, Max: 150]: ");
            scanf("%d", &velocidade); // Recebe o novo valor da velocidade da animação
        } while (velocidade < 10 || velocidade > 150);
    }
}

// Recebe como parametro variaveis que indicam as posições dos discos a serem movimentados
void Animacao(int i, int j, int orig, int dest, int topo)
{
    int p; // Variável auxiliar
    int k; // Variável auxiliar

    // Levantando o disco na torre a partir da sua origem
    for (p = i + 1; p > 1; p--)
    {
        gotoxy((orig * 20) + 13, p + 7);
        printf("      %c      ", 179);
        gotoxy((orig * 20) + 20 - topo, p + 6);
        for (k = (topo * 2) - 1; k > 0; k--)
            printf("%c", 254);
        Sleep(velocidade);
    }

    gotoxy((orig * 20) + 13, p + 7); // Limpando o ultimo disco presente na origem
    printf("      %c      ", 179);

    // Movendo o disco horizontalmente até seu destino
    if (orig < dest)
    {
        for (p = (orig * 20) + 20 - topo; p <= (dest * 20) + 20 - topo; p++)
        {
            gotoxy(p - 7, 7);
            printf("       ");
            gotoxy(p, 7);
            for (k = (topo * 2) - 1; k > 0; k--)
                printf("%c", 254);
            Sleep(velocidade);
        }
    }
    else
    {
        for (p = (orig * 20) + 20 - topo; p >= (dest * 20) + 20 - topo; p--)
        {
            gotoxy(p, 7);
            printf("          ");
            gotoxy(p, 7);
            for (k = (topo * 2) - 1; k > 0; k--)
                printf("%c", 254);
            Sleep(velocidade);
        }
    }

    // Movendo o disco para baixo na torre de destino até encontrar a sua posição na torre
    for (p = 0; p < j; p++)
    {
        gotoxy((dest * 20) + 13, p + 7);
        if (p == 0)
        {
            printf("              ", 179);
        }
        else
            printf("      %c      ", 179);
        gotoxy((dest * 20) + 20 - topo, p + 8);
        for (k = (topo * 2) - 1; k > 0; k--)
            printf("%c", 254);
        Sleep(velocidade);
    }
}

// Função que imprime as informações sobre o jogo
void Sobre()
{
    FILE *arquivo_info;
    char ch[100];

    arquivo_info = fopen("info.txt", "r");

    DesenhaTopo();

    if (arquivo_info == NULL)
    {
        gotoxy(0, 6);
        printf("      Arquivo de informacoes inexistente.\n");
    }
    else
    {
        gotoxy(0, 6);
        while (fgets(ch, 100, arquivo_info) != NULL)
        {
            printf("   %s", ch);
        }
    }
    fclose(arquivo_info); // Fecha o arquivo de informações
    printf("\n\n      Pressione qualquer tecla para voltar.");
    getch();
}

// Função para imprimir na tela a Torre de Hanoi
void DesenhaHanoi()
{
    int i, j, k; // Variáveis auxiliares
    int x = 13, y = 8;

    DesenhaTopo();
    gotoxy(5, 5);
    printf("              1                   2                   3");

    for (i = 0; i < maxDiscos; i++)
    {
        for (j = 0; j < maxTorres; j++)
        {
            gotoxy(x + j * 20, y + i);
            if (hanoi[i][j] != 0)
            {
                for (k = hanoi[i][j]; k < maxDiscos; k++)
                {
                    printf(" ");
                }
            }
            else
            {
                for (k = hanoi[i][j]; k < maxDiscos - 1; k++)
                    printf(" ");
                printf("%c", 179);
            }
            for (k = hanoi[i][j]; k > 0; k--)
                printf("%c", 254);
            for (k = hanoi[i][j]; k > 1; k--)
                printf("%c", 254);
            for (k = hanoi[i][j]; k < maxDiscos; k++)
                printf(" ");
        }
        printf("\n");
    }
    printf("   \\o--------------------------------------------------------------------o/");
}

// Recebe como parametro o endereço de uma variavel de opção
void Menu(char *opcao)
{
    DesenhaTopo();
    gotoxy(0, 6);
    printf("\t\t\t\tJ - %-12s\n\n", "Jogar");
    printf("\t\t\t\tT - %-12s\n\n", "Treinar");
    printf("\t\t\t\tR - %-12s\n\n", "Resolver");
    printf("\t\t\t\tC - %-12s\n\n", "Configurar");
    printf("\t\t\t\tE - %-12s\n\n", "Estatistica");
    printf("\t\t\t\tS - %-12s\n\n\n", "Sobre");
    printf("\t\t\t\tX - %-12s", "Sair");
    *opcao = toupper(getch()); // O conteudo do ponteiro recebe o valor da tecla digitada em maiúsculo
}

// Função que imprime na tela um cabeçalho do jogo.
void DesenhaTopo()
{
    system("cls");
    printf("\n");
    printf("    o--------------------------------------------------------------------o\n");
    printf("   //                           Torre de Hanoi                           \\\\\n");
    printf("   o----------------------------------------------------------------------o");
}

// Função gotoxy para posicionar o cursor na tela
void gotoxy(int coluna, int linha)
{
    COORD point;
    point.X = coluna;
    point.Y = linha;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), point);
}
