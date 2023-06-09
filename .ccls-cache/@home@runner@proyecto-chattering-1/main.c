
/*
  Ejecución:
  ./Manager -n N -p pipenom
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "structures.h"

void validate_args(int argc, char *argv[], int *N, char *pipeNom);
void auth(Request req, Talker *talkers, int *c_talkers, int N);

int main(int argc, char *argv[]) {

  int N;
  char pipeNom[25];

  validate_args(argc, argv, &N, pipeNom);

  printf("\nManager iniciado con un número máximo de %d usuarios y '%s' como "
         "nombre del pipe pricipal \n\n",
         N, pipeNom);

  Talker *talkers;
  Group *groups;
  int c_talkers = 0, c_groups = 0;
  
  talkers = malloc(N * sizeof(Talker));
  groups = calloc(0, sizeof(Group));

  mode_t fifo_mode = S_IRUSR | S_IWUSR;

  mkfifo(pipeNom, fifo_mode);
  mkfifo("auth", fifo_mode);

  do {
    int fd = open(pipeNom, O_RDONLY);

    if (fd == -1) {
      perror("Error de apertura del FIFO");
      exit(EXIT_FAILURE);
    }

    Request req;

    if (read(fd, &req, sizeof(req)) == -1) {
      perror("Error lectura");
      exit(EXIT_FAILURE);
    }

    printf("\nMe llego esta info:\n\n");
    printf("ID: %d\n", req.ID);
    printf("tipo: '%s'\n", req.type);
    printf("info: '%s'\n", req.args);
    printf("cant_args: %d\n", req.c_args);

    if (strcmp(req.type, "Auth") == 0) {
      auth(req, talkers, &c_talkers, N);
      
    } else if (strcmp(req.type, "List") == 0) {
      
      char pipe_tmp[7], aux[10], envio[100];

      if (c_talkers > 0) {
        
        sprintf(aux, "%d", talkers[0].ID);//casting
        strcpy(envio, aux);
        for (int i = 1; i < c_talkers;i++) {
          strcat(envio, ",");
          sprintf(aux, " %d", talkers[i].ID);//casting
          strcat(envio, aux);
        }
      
      } else {
        
      }

      printf("se envia a %s, la info: %s", pipe_tmp, envio);
      
      sprintf(pipe_tmp, "pipe%d", req.ID);//casting
      int fdtemp = open(pipe_tmp, O_WRONLY);
      
      if (write(fdtemp, &envio, sizeof(envio)) == 0) {
        perror("error en la escritura");
        exit(1);
      }
      close(fdtemp);
        
    } else if (strcmp(req.type, "Group") == 0) {
      printf("cccc info: %s\n", req.args);
    } else if (strcmp(req.type, "Sent") == 0) {
      
      int check = 0;
      char *pipe_tmp;
      strtok(req.args, "\"");
      char *envio = strtok(NULL, "\"");
      strtok(NULL, " ");
      char *id_dest = strtok(NULL, "\0");
      int Ide = (int) *id_dest;
      
      for (int i = 0;i<c_talkers;i++) {
        if (Ide == talkers[i].ID) { check=1; break;}
      }
      if (check==0) {
        sprintf(pipe_tmp, "pipe%d", req.ID);//casting
        int fdtemp = open(pipe_tmp, O_WRONLY);
        char env[] = "El ID seleccionado no existe";
        if (write(fdtemp, &env, sizeof(env)) == 0){
          perror("Error en la escritura");
          exit(1);
        }
      } else {
        sprintf(pipe_tmp, "pipe%d", Ide);//casting
        int fdtemp = open(pipe_tmp, O_WRONLY);
        sprintf(envio, "\"%s\" desde %d", envio, req.ID);
        if(write(fdtemp, &envio, sizeof(envio)) == 0) {
          perror("Error en la escritura");  
          
        //TODO SEÑÑÑÑÑÑAAAAAAAALLLLL
        
        }
      }
      
    } else if (strcmp(req.type, "Salir") == 0) {
      printf("eeee info: %s\n", req.args);
    } else {
      printf("llego una solicitud inválida\n");
    }

    close(fd);
  } while (1);

  free(talkers);

  unlink(pipeNom);
  unlink("auth");

  return 0;
}

void validate_args(int argc, char *argv[], int *N, char *pipeNom) {
  if (argc != 5) {
    perror("El manager requiere solo dos banderas (-n y -p con sus respectivos "
           "valores)");
    exit(1);
  } else {
    for (int i = 1; i < argc; i += 2) {
      if (strcmp(argv[i], "-n") == 0) {
        *N = atoi(argv[i + 1]);

      } else if (strcmp(argv[i], "-p") == 0) {
        strcpy(pipeNom, argv[i + 1]);

      } else {
        printf("\nLa flag '%s' no es válida\n\n", argv[i]);
        exit(1);
      }
    }
  }
}

void auth(Request req, Talker *talkers, int *c_talkers, int N) {
  if (req.c_args != 1) {
     perror("solicitud inválida");
    
  } else if (*c_talkers == N) {
     perror("Limite de usuarios, intentelo más tarde");
    
  } else {
    int exist = 0;
    for (int i = 0; i < *c_talkers;i++) {
      if (req.ID == talkers[i].ID) {
        exist = 1;
        break;
      }
    }
    if (exist) {
      char msg[40] = "Error: el usuario ya existe";

      int fd_tmp = open("auth", O_WRONLY);

      if (write(fd_tmp, &msg, sizeof(msg)) == 0) {
        perror("error en la escritura");
        exit(1);
      }

      close(fd_tmp);
      
    } else {
      char* token = strtok(req.args, " ");
      
      talkers[*c_talkers].ID = req.ID;
      talkers[*c_talkers].pid = atoi(token);

      (*c_talkers)++;
      
      char msg[40] = "El usuario se guardo correctamente";

      int fd_tmp = open("auth", O_WRONLY);

      if (write(fd_tmp, &msg, sizeof(msg)) == 0) {
        perror("error en la escritura");
        exit(1);
      }

      close(fd_tmp);
    }
  }
}