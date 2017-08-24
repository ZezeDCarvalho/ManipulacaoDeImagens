/*+-------------------------------------------------------------------------+
  | PROCESSAMENTO DE IMAGEM -                                               |
  | -----------------------                                                 |
  | Programa para processamento de uma imagem em niveis de cinza.           |
  | Criado:.....  Luiz Eduardo da Silva                                     |
  | Manipulado:.  Maria José Silva de Carvalho                              |
  |             funções criadas (gradiente, erosao, dilatacao2, dilatacao,  |
  |             equalizaHistograma, calculaHistograma, mediana, convolucao) |
  +-------------------------------------------------------------------------+*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define TRUE  1
#define FALSE 0
#define CREATOR "# CREATOR: Luiz Eduardo V:1.00 R:MAR/2015\n"

typedef int * imagem;
typedef int histograma [256];

/*+-------------------------------------------------------------------------+
  | Rotinas para ALOCAR e DESALOCAR dinamicamente espaco de memoria para um |
  | vetor monodimensional que armazenara a imagem.                          |
  | PARAMETROS:                                                             |
  | I  = Endereco do vetor (ponteiro de inteiros).                          |
  | nl = Numero de linhas.                                                  |
  | nc = Numero de colunas.                                                 |
  +-------------------------------------------------------------------------+*/
int aloca_memo(imagem *I, int nl, int nc) {
    *I = (int *) malloc(nl * nc * sizeof (int));
    if (*I) return TRUE;
    else return FALSE;
}

int desaloca_memo(imagem *I) {
    free(*I);
}

/*+-------------------------------------------------------------------------+
  | Apresenta informacoes sobre um arquivo de imagem.                       |
  | PARAMETROS:                                                             |
  |     nome = nome fisico do arquivo de imagem.                            |
  |     nl   = numero de linhas da imagem.                                  |
  |     nc   = numero de colunas da imagem.                                 |
  |     mn   = maximo nivel de cinza da imagem.                             |
  +-------------------------------------------------------------------------+*/
void info_imagem(char *nome, int nl, int nc, int mn) {
    printf("\nINFORMACOES SOBRE A IMAGEM:");
    printf("\n--------------------------\n");
    printf("Nome do arquivo.............: %s \n", nome);
    printf("Numero de linhas............: %d \n", nl);
    printf("Numero de colunas...........: %d \n", nc);
    printf("Maximo nivel-de-cinza/cores.: %d \n\n", mn);
}

/*+-------------------------------------------------------------------------+
  | Rotina que faz a leitura de uma imagem em formato .PGM ASCII e armazena |
  | num vetor monodimensional. Um exemplo de imagem .PGM ASCII gravada neste|
  | formato:                                                                |
  |                                                                         |
  | P2                                                                      |
  | # CREATOR: XV Version 3.10a  Rev: 12/29/94                              |
  | 124 122                                                                 |
  | 255                                                                     |
  | 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255     |
  | 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255     |
  | (...)                                                                   |
  |                                                                         |
  | Lin 1: contem P2, o que identifica este arquivo como PGM ASCII.         |
  | Lin 2: contem um comentario qualquer iniciado com #.                    |
  | Lin 3: numero de colunas e numero de linhas da imagem.                  |
  | Lin 4: maximo nivel de cinza na imagem (255 normalmente).               |
  | Lin 5 em diante: valores de cinza da imagem.                            |
  |                                                                         |
  | PARAMETROS:                                                             |
  |     nome = nome do arquivo (entra).                                     |
  |     I    = ponteiro para o vetor imagem (retorna).                      |
  |     nl   = numero de linhas da imagem (retorna).                        |
  |     nc   = numero de colunas da imagem (retorna).                       |
  |     mn   = maximo nivel de cinza (retorna).                             |
  +-------------------------------------------------------------------------+*/
int le_imagem_pgm(char *nome, imagem *I, int *nl, int *nc, int *mn) {
    int i, j, k, LIMX, LIMY, MAX_NIVEL;
    char c, LINHA[100];
    FILE *arq;
    if ((arq = fopen(nome, "r")) == NULL) {
        printf("Erro na ABERTURA do arquivo <%s>\n\n", nome);
        return FALSE;
    }
    /*-- PGM = "P2" -----------*/
    fgets(LINHA, 80, arq);
    if (!strstr(LINHA, "P2")) {
        printf("Erro no FORMATO do arquivo <%s>\n\n", nome);
        return FALSE;
    }
    /*-- Dimensoes da imagem --*/
    fgets(LINHA, 80, arq);
    do {
        fgets(LINHA, 80, arq);
    } while (strchr(LINHA, '#'));

    sscanf(LINHA, "%d %d", &LIMX, &LIMY);
    fscanf(arq, "%d", &MAX_NIVEL);
    if (LIMX == 0 || LIMY == 0 || MAX_NIVEL == 0) {
        printf("Erro nas DIMENSOES do arquivo <%s>\n\n", nome);
        return FALSE;
    }
    if (aloca_memo(I, LIMY, LIMX)) {
        for (i = 0; i < LIMY; i++) {
            for (j = 0; j < LIMX; j++) {
                fscanf(arq, "%d", &k);
                if (k > MAX_NIVEL) {
                    printf("Erro nos DADOS do arquivo <%s>\n", nome);
                    printf("Valor lido: %d   Max Nivel: %d\n\n", k, MAX_NIVEL);
                    return FALSE;
                }
                *(*(I) + i * LIMX + j) = k;
            }
        }
        fclose(arq);
    } else {
        printf("Erro na ALOCACAO DE MEMORIA para o arquivo <%s>\n\n", nome);
        printf("Rotina: le_imagem_pgm\n\n");
        fclose(arq);
        return FALSE;
    }
    *nc = LIMX;
    *nl = LIMY;
    *mn = MAX_NIVEL;
    return TRUE;
}

/*+-------------------------------------------------------------------------+
  | Rotina que grava o arquivo da imagem em formato PGM ASCII.              |
  | PARAMETROS:                                                             |
  |     I    = ponteiro para o vetor imagem (entra).                        |
  |     nome = nome do arquivo (entra).                                     |
  |     nl   = numero de linhas (entra).                                    |
  |     nc   = numero de colunas (entra).                                   |
  |     mn   = maximo nivel de cinza (entra).                               |
  +-------------------------------------------------------------------------+*/
void grava_imagem_pgm(imagem I, char *nome, int nl, int nc, int mn) {
    int i, j, x, k, valores_por_linha;
    FILE *arq;
    if ((arq = fopen(nome, "wt")) == NULL) {
        printf("Erro na CRIACAO do arquivo <%s>\n\n", nome);
    } else {
        fputs("P2\n", arq);
        fputs(CREATOR, arq);
        fprintf(arq, "%d  %d\n", nc, nl);
        fprintf(arq, "%d\n", mn);
        valores_por_linha = 16;
        for (i = 0, k = 0; i < nl; i++)
            for (j = 0; j < nc; j++) {
                x = *(I + i * nc + j);
                fprintf(arq, "%3d ", x);
                k++;
                if (k > valores_por_linha) {
                    fprintf(arq, "\n");
                    k = 0;
                }
            }
    }
    fclose(arq);
}

void msg(char *s) {
    printf("\nPROCESSAMENTO de uma imagem qualquer");
    printf("\n-------------------------------");
    printf("\nUSO.:  %s  <IMG.PGM>", s);
    printf("\nONDE:\n");
    printf("    <IMG.PGM>  Arquivo da imagem em formato PGM\n\n");
}

void negativo(imagem I, imagem O, int nl, int nc, int mn) {
    int i, j;
    for (i = 1; i < nl - 1; i++)
        for (j = 1; j < nc - 1; j++) {
            O[i * nc + j] = 255 - I[i * nc + j];
        }
}

void convolucao(imagem I, imagem O, int nl, int nc, int mn) {
    int mask[3][3] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
    int i, j;
    for (i = 1; i < nl - 1; i++)
        for (j = 1; j < nc - 1; j++) {
            int y, x, soma = 0;
            for (y = 0; y < 3; y++)
                for (x = 0; x < 3; x++)
                    soma += mask[y][x] * I[(i + y - 1) * nc + j + x - 1];
            O[i * nc + j] = soma < 0 ? -soma : soma;
            O[i * nc + j] /= 9; // filtros (passaAlta ou passaBaixa)
            O[i * nc + j] = O[i * nc + j] > 20 ? 0 : 255;
        }
}

void mediana(imagem I, imagem O, int nl, int nc) {
    int i, j, MAX = 9, CENTRO = (MAX / 2);
    int vetor [MAX];
    for (i = 1; i <= nl - 1; i++)
        for (j = 1; j <= nc - 1; j++) {
            int y, x, cont = 0, aux, k, m;
            //verifica os vizinhos do ponto analisado e os coloca em um vetor
            for (y = i - 1; y < i + 2; y++)
                for (x = j - 1; x < j + 2; x++)
                    vetor [cont++] = I[y * nc + x];
                
            //for (y = 0; y < 3; y++)
            //    for (x = 0; x < 3; x++)
            //        vetor[cont++] = I[(i + y - 1) * nc + j + x - 1];

            //ordenacao do vetor
            for (k = 0; k < MAX - 1; k++)
                for (m = k + 1; m < MAX; m++)
                    if (vetor[k] > vetor[m]) {
                        aux = vetor[k];
                        vetor[k] = vetor[m];
                        vetor[m] = aux;
                    }
            
            //o ponto processado recebe o ponto médio do vetor
            O[i * nc + j] = vetor[CENTRO];
        }
}

void calculaHistograma(imagem I, histograma H, int nl, int nc, int mn) {
    int i, j;
    for (i = 0; i < mn + 1; i++)
        H[i] = 0;

    for (i = 0; i < nl; i++)
        for (j = 0; j < nc; j++)
            H[I[i * nc + j]]++;
        
}

void equalizaHistograma(imagem I, imagem O, histograma H, int nl, int nc, int mn) {
    int i, j, MAX = mn + 1;
    calculaHistograma(I, H, nl, nc, mn);
    double pf[MAX];
    for (i = 0; i < MAX; i++) {
        //divide pelo numero de pixels
        pf[i] = (double) H[i] / (nc * nl);
        if (i) {
            //soma o anterior com o atual
            pf[i] = pf[i] + pf[i - 1];
        }
    }
    //multiplica pelo maior tom de cor
    for (i = 0; i < MAX; i++)
        pf[i] *= mn;
    
    for (i = 0; i < nl; i++)
        for (j = 0; j < nc; j++)
            O[i * nc + j] = (int) pf[I[i * nc + j]];
        
}

void dilatacao(imagem I, imagem O, int nl, int nc, int mn) {
    int i, j;
    for (i = 1; i < nl - 1; i++)
        for (j = 1; j < nc - 1; j++) {
            int y, x, max = -1;
            for (y = -1; y < 2; y++)
                for (x = -1; x < 2; x++)
                    if (max < I[(i + y) * nc + j + x]) {
                        max = I[(i + y) * nc + j + x];
                    }
            O[i * nc + j] = max;
        }
}

void dilatacao2(imagem I, imagem O, int nl, int nc, int mn) {
    int i, j;

    struct {
        int x, y;
    } viz[4] = {
        {-2, 0},
        {-1, 0},
        {1, 0},
        {2, 0}};
    for (i = 1; i < nl - 1; i++)
        for (j = 1; j < nc - 1; j++) {
            int k, max = -1;
            for (k = 0; k < 4; k++)
                if (max < I[(i + viz[k].x) * nc + j + viz[k].y])
                    max = I[(i + viz[k].x) * nc + j + viz[k].y];
            O[i * nc + j] = max;
        }
}

void erosao(imagem I, imagem O, int nl, int nc, int mn) {
    int i, j;
    for (i = 1; i < nl - 1; i++)
        for (j = 1; j < nc - 1; j++) {
            int y, x, min = mn + 1;
            for (y = -1; y < 2; y++)
                for (x = -1; x < 2; x++)
                    if (min > I[(i + y) * nc + j + x])
                        min = I[(i + y) * nc + j + x];
            O[i * nc + j] = min;
        }
}

void gradiente(imagem I, imagem O, int nl, int nc, int mn) {
    int i, j;
    for (i = 1; i < nl - 1; i++)
        for (j = 1; j < nc - 1; j++) {
            int y, x, min = mn + 1, max = -1;
            for (y = -1; y < 2; y++)
                for (x = -1; x < 2; x++) {
                    if (min > I[(i + y) * nc + j + x])
                        min = I[(i + y) * nc + j + x];
                    if (max < I[(i + y) * nc + j + x])
                        max = I[(i + y) * nc + j + x];
                    
                }
            int dif = max - min;
            O[i * nc + j] = dif;
        }
}

/*+------------------------------------------------------+
  |        P R O G R A M A    P R I N C I P A L          |
  +------------------------------------------------------+*/
int main(int argc, char *argv[]) {
    int OK, nc, nl, mn, i, j, maior;
    char nome[100];
    char comando[100];
    histograma Histo;
    imagem In, Out;
    if (argc < 2)
        msg(argv[0]);
    else {
        OK = le_imagem_pgm(argv[1], &In, &nl, &nc, &mn);
        if (OK) {
            printf("\nPROCESSAMENTO DE IMAGEM");
            info_imagem(argv[1], nl, nc, mn);
            aloca_memo(&Out, nl, nc);
            calculaHistograma(In, Histo, nl, nc, mn);
            //tom de maior frequencia
            maior = 0;
            for (i = 0; i < mn + 1; i++)
                if (Histo[i] > Histo[maior]) {
                    maior = i;
                }
            printf("Tom: %d | Frequência: %d\n", maior, Histo[maior]);

//            equalizaHistograma(In, Out, Histo, nl, nc, mn);
//            negativo(In, Out, nl, nc, mn);
//            convolucao(In, Out, nl, nc, mn);
            mediana(In, Out, nl, nc);
//            dilatacao(In, Out, nl, nc, mn);
//            dilatacao2(In, Out, nl, nc, mn);
//            erosao(In, Out, nl, nc, mn);
//            gradiente(In, Out, nl, nc, mn);

            strcpy(nome, argv[1]);
            strcat(nome, "-modificada.pgm");
            grava_imagem_pgm(Out, nome, nl, nc, mn);
            strcpy(comando, "./i_view32 ");
            strcat(comando, nome);
            strcat(comando, " /hide=7");
            system(comando);
            desaloca_memo(&In);
            desaloca_memo(&Out);
        }
    }
    return 0;
}
