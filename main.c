#include <errno.h> // usado para a constante EEXIST e para a variável errno
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // usado para a função mkdir
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define NUM_LETRAS 26
#define ASCII_A 65
#define NUM_ARQUIVOS 10 // número de arquivos de senha

// função para criptografar
char* encrypt(const char* str, int tamanho) {
    char* str_result = (char*) malloc(sizeof(char)*tamanho);
    for (int i = 0; i < tamanho; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            int chave = str[i] - ASCII_A;
            str_result[i] = (str[i] - ASCII_A + chave) % NUM_LETRAS + ASCII_A;
        }
    }
    return str_result;
}

// função para descriptografar
void brute_force_decrypt(const char* encrypted_password, FILE* output_file) {
    char password[5]; // 4 caracteres + '0'
    for (int c1 = 'A'; c1 <= 'Z'; c1++) {
        for (int c2 = 'A'; c2 <= 'Z'; c2++) {
            for (int c3 = 'A'; c3 <= 'Z'; c3++) {
                for (int c4 = 'A'; c4 <= 'Z'; c4++) {
                    password[0] = c1;
                    password[1] = c2;
                    password[2] = c3;
                    password[3] = c4;
                    password[4] = '\0';

                    char* encrypted_attempt = encrypt(password, 4);

                    if (strcmp(encrypted_attempt, encrypted_password) == 0) {
                        // se a senha é encontrada, escreve no arquivo de saída
                        fprintf(output_file, "%s\n", password);
                        free(encrypted_attempt);
                        return;
                    }

                    free(encrypted_attempt);
                }
            }
        }
    }
}

int main() {
    pid_t original_pid = getpid(); // armazenando o PID do processo original

    // criando a pasta "descriptografado"
    if (mkdir("descriptografado", 0700) != 0 && errno != EEXIST) {
        printf("Erro ao criar a pasta descriptografado.\n");
        return 1;
    }

    // processando cada arquivo de entrada
    for (int i = 0; i < NUM_ARQUIVOS; i++) {
        char filename_in[20];
        char filename_out[32];
        snprintf(filename_in, sizeof(filename_in), "senhas/%d.txt", i);
        snprintf(filename_out, sizeof(filename_out), "descriptografado/quebrado_%d.txt", i);

        FILE *input_file = fopen(filename_in, "r");
        FILE *output_file = fopen(filename_out, "w");

        if (input_file == NULL || output_file == NULL) {
            printf("Erro ao abrir os arquivos.\n");
            return 1;
        }

        char encrypted_password[5];

        // processando cada linha da entrada em um processo separado
        while (fscanf(input_file, "%s", encrypted_password) != EOF) {
            pid_t pid = fork(); // criando um novo processo
            if (pid == 0) { // processo filho
                brute_force_decrypt(encrypted_password, output_file);
                fclose(input_file);
                fclose(output_file);
                exit(0); // fim do processo filho
            }
            // processo pai continua a execução
        }

        fclose(input_file);
        fclose(output_file);
    }

    // processo pai espera todos os processos filhos terminarem
    if (getpid() == original_pid) {
        for (int i = 0; i < NUM_ARQUIVOS; i++) {
            wait(NULL);
        }
    }

    return 0;
}
