#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>

// Guardar o path da pasta para guardar os ficheiros
char * path_folder;

// Descritores
int rd_fifo = 0;
int wr_fifo = 0;

// Estrutura para armazenar informações sobre as tarefas
struct Task {
    int id;
    int duration_ms;
    char program[100];
    char args[200];
};

void setup(char * p_folder) {
    // Criar o fifo para o cliente comunicar com o servidor
    if (mkfifo("tmp/fifo", 0666) < 0) {
        perror("mkfifo\n");
    }
    // Abrir o descritor para ler do fifo
    rd_fifo = open("tmp/fifo", O_RDONLY);
    if (rd_fifo < 0) { 
        perror("open fifo");
    }
    // Abrir o descritor para escrever no fifo, apenas de modo a não fechar o servidor
    wr_fifo = open("tmp/fifo", O_WRONLY);
    if (wr_fifo < 0) { 
        perror("open fifo");
    }
    // Guardar o path para a pasta onde vão ser guardados os ficheiros
    path_folder = p_folder;
}

void str_to_args(char buffer[], int n_bytes, char * args[]) {
    
    int j = 0;
    int n = 0;
    args[n] = malloc(sizeof(char) * n_bytes);

    //Dividir a string que está no buffer num vetor de argumentos
    for(int i = 0; i < n_bytes; i++) {
        if(buffer[i] == '\n') {
            args[n][j] = '\0';
            n++;
            args[n] = malloc(sizeof(char) * n_bytes);
            j = 0;
        } else {
            args[n][j] = buffer[i];
            j++;
        }
    }
}

void enviarSolicitacaoExecucaoTarefa(int duracao_ms, const char *programa, const char *argumentos) {
    // Criar e preencher os detalhes da tarefa
    struct Task tarefa;
    tarefa.duration_ms = duracao_ms;
    strcpy(tarefa.program, programa);
    strcpy(tarefa.args, argumentos);

    // Abrir o pipe nomeado para escrita
    int wr_fifo = open("tmp/fifo", O_WRONLY);
    if (wr_fifo < 0) {
        perror("open fifo for write");
        return;
    }

    // Escrever os detalhes da tarefa no pipe nomeado
    if (write(wr_fifo, &tarefa, sizeof(tarefa)) < 0) {
        perror("write to fifo");
        close(wr_fifo);
        return;
    }

    // Fechar o pipe nomeado após a escrita
    close(wr_fifo);
}

void consultarStatusTarefas() {
    // Abrir o pipe nomeado para escrita
    int wr_fifo = open("tmp/fifo", O_WRONLY);
    if (wr_fifo < 0) {
        perror("open fifo for write");
        return;
    }

    // Escrever a solicitação de consulta de status no pipe nomeado
    const char *solicitacao = "status";
    if (write(wr_fifo, solicitacao, strlen(solicitacao) + 1) < 0) {
        perror("write to fifo");
        close(wr_fifo);
        return;
    }

    // Fechar o pipe nomeado após a escrita
    close(wr_fifo);

    // Abrir o pipe nomeado para leitura
    int rd_fifo = open("tmp/fifo", O_RDONLY);
    if (rd_fifo < 0) {
        perror("open fifo for read");
        return;
    }

    // Ler os detalhes das tarefas do pipe nomeado de leitura
    // e preencher a estrutura apropriada
    // (implemente de acordo com a resposta do servidor)

    // Fechar o pipe nomeado após a leitura
    close(rd_fifo);
}

void processarRespostaServidor() {
    // Abrir o pipe nomeado para leitura
    int rd_fifo = open("tmp/fifo", O_RDONLY);
    if (rd_fifo < 0) {
        perror("open fifo for read");
        return;
    }

    // Ler a resposta do servidor do pipe nomeado de leitura
    char buffer[1024];
    int bytes_lidos = read(rd_fifo, buffer, sizeof(buffer));
    if (bytes_lidos < 0) {
        perror("read from fifo");
        close(rd_fifo);
        return;
    }

    // Interpretar a resposta do servidor
    // (depende do formato das respostas definido entre cliente e servidor)
    // Neste exemplo, simplesmente imprimimos a resposta
    printf("Resposta do servidor: %s\n", buffer);

    // Fechar o pipe nomeado após a leitura
    close(rd_fifo);
}

int main() {
    int opcao;
    int duracao_ms;
    char programa[100];
    char argumentos[200];

    // Loop principal para interagir com o usuário
    while (1) {
        // Exibir o menu de opções
        printf("\n1. Executar tarefa\n");
        printf("2. Consultar status das tarefas\n");
        printf("3. Sair\n");
        printf("Escolha uma opcao: ");

        // Ler a opção do usuário
        scanf("%d", &opcao);

        // Executar a opção selecionada
        switch (opcao) {
            case 1:
                // Ler os detalhes da tarefa a ser executada
                printf("Digite a duracao em milissegundos: ");
                scanf("%d", &duracao_ms);
                printf("Digite o nome do programa: ");
                scanf("%s", programa);
                printf("Digite os argumentos do programa: ");
                scanf("%s", argumentos);

                // Enviar solicitação ao servidor para executar a tarefa
                enviarSolicitacaoExecucaoTarefa(duracao_ms, programa, argumentos);
                break;
            case 2:
                // Consultar o status das tarefas
                consultarStatusTarefas();
                break;
            case 3:
                // Encerrar o programa
                exit(EXIT_SUCCESS);
            default:
                printf("Opcao invalida.\n");
        }

        // Processar e exibir a resposta do servidor
        processarRespostaServidor();
    }

    return 0;
}