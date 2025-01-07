#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_HISTORY_SIZE 100
char line[10000];
int token_count=0;
int tokens[10000];
int input = 0;
int output = 0;
int append = 0;
int background = 0;
int debug_level = 0;
int status=0;
char pozivnik[1000];
char trenutni_dir[1024];
char procfs_path[256];
long pidss[40000];
int count_pidss=0;
int star_input = STDIN_FILENO;
int star_output = STDOUT_FILENO;

int tokenize(char* linija){


    int dolzina = strlen(linija);

    token_count = 0;
    for(int i = 0; i<dolzina; i++){

        if(linija[i]=='#'){
            linija[i]='\0';
            break;
        }else if(linija[i]!=' ' && linija[i]!='\0' && linija[i]!='\n'){

            if(linija[i]=='"'){
                i++;
                tokens[token_count]=i;
                token_count++;
                while(i<dolzina && linija[i]!='"'){
                i++;
            }
                if(i<dolzina && linija[i]=='"'){
                    linija[i]='\0';
                }
                continue;
            }

            tokens[token_count]=i;
            token_count++;
              
            while(i<dolzina && linija[i]!=' '){
                i++;
            }
            i--;

        }else if(linija[i]==' '){
            linija[i]='\0';
        }
    }
        linija[dolzina-1]='\0';

        strcpy(line,linija);

        return token_count;

}

void ispisi(char* line, int count){

    printf("Input line: '");
    for(int i = 0; i<count; i++){

        if(i==0)
        printf("%s", &line[tokens[i]]);
        else
        printf(" %s", &line[tokens[i]]);
    }
    printf("'\n");

    for(int i = 0; i<count; i++){
            printf("Token %d: '%s'\n",i, &line[tokens[i]]);
        }

}

void print(){

    if(token_count==1){
        return;
    }else{
        for(int i = 1; i<token_count; i++){
        
        if(i==token_count-1)
        printf("%s", &line[tokens[i]]);
        else
        printf("%s ", &line[tokens[i]]);

    }
    }
}

int compare(const void *a, const void *b) {
    return (*(long *)a - *(long *)b);
}

void echo(){

    if(token_count==1){
        printf("\n");
    }else{
        for(int i = 1; i<token_count; i++){
        if(i==token_count-1)
        printf("%s", &line[tokens[i]]);
        else
        printf("%s ", &line[tokens[i]]);
        }
        printf("\n");

    
    }
    status = 0;
}

void len(int vk){
    if(token_count==1){
        printf("0\n");
    }else{
        printf("%d\n",vk-3-token_count);
    }
}

void sum(){
    
    int vsota = 0;
    for (int i = 1; i<token_count; i++){
        vsota+=atoi(&line[tokens[i]]);
    }

    printf("%d\n",vsota);
    
}

void calc(){

    if(token_count<4){
        printf("Not enough arguments\n");
    }else{

        if(strcmp(&line[tokens[2]],"+")==0){
            int x = atoi(&line[tokens[1]]) + atoi(&line[tokens[3]]);
             printf("%d\n",x);
        }
        else if(strcmp(&line[tokens[2]],"-")==0){
            int x = atoi(&line[tokens[1]]) - atoi(&line[tokens[3]]);
             printf("%d\n",x);
        }else if(strcmp(&line[tokens[2]],"*")==0){
            int x = atoi(&line[tokens[1]]) * atoi(&line[tokens[3]]);
             printf("%d\n",x);
        }else if(strcmp(&line[tokens[2]],"/")==0){
            int x = atoi(&line[tokens[1]]) / atoi(&line[tokens[3]]);
             printf("%d\n",x);
        }else if(strcmp(&line[tokens[2]],"%")==0){
            int x = atoi(&line[tokens[1]]) % atoi(&line[tokens[3]]);
             printf("%d\n",x);
        }
    }
}

void basename(int mode, char* linija){

    if(mode==0){
    if(token_count==1){

        status=1;
    }else{
        int kraj = tokens[1] + strlen(&line[tokens[1]]);
        int pocetok=0;

        for (int i = kraj-1; i>=tokens[1]; i--){
            if(line[i]=='/'){
                pocetok=i+1;
                break;
            }
        }

        printf("%s\n",&line[pocetok]); 
    }
    }else{
        int kraj = strlen(linija);
        int pocetok=0;

        for (int i = kraj-1; i>=0; i--){
            if(linija[i]=='/'){
                pocetok=i+1;
                break;
            }
        }

        printf("%s\n",&linija[pocetok]); 
    }
}

void dirname(){

    if(token_count==1){

        status=1;
    }else{

        int kraj = tokens[1] + strlen(&line[tokens[1]]);
        int konec=0;

        for (int i = kraj-1; i>=tokens[1]; i--){
            if(line[i]=='/'){
                konec=i-1;
                break;
            }
        }

        for(int i = tokens[1]; i<=konec; i++){
            printf("%c",line[i]);
        }
        printf("\n");

        
    }

}

void dirch(){

    if(token_count==1){
       if( chdir("/")<0){
        status=errno;
        perror("dirch");
       }else
            status=0;
    }else{

        if(chdir(&line[tokens[1]])<0){
            status = errno;
            perror("dirch");
        }else{
            status=0;
        }
    }
}

void dirwd(){

    getcwd(trenutni_dir, sizeof(trenutni_dir));

    if(strlen(trenutni_dir)==1){
        printf("/\n");
        return;
    }

    if(token_count==1 || (token_count>1 && strcmp(&line[tokens[1]],"base")==0)){
        basename(1,trenutni_dir);
    }else if(strcmp(&line[tokens[1]],"full")==0){

        printf("%s\n",trenutni_dir);
    }
}

void dirmk(){

    if (mkdir(&line[tokens[1]], 0777) < 0) {
        status=errno;
        perror("dirmk");
    }else{
        status = 0;
    }
}

void dirls(){

    DIR* dir;
    if(token_count==1)
        dir = opendir(".");
    else{
        dir = opendir(&line[tokens[1]]);
    }
	struct dirent * entry;
	while ( (entry = readdir(dir)) != 0) {
        printf("%s  ", entry->d_name);
	}
    printf("\n");
	closedir(dir);
}

void dirrm(){

    if(token_count>1)
    if (rmdir(&line[tokens[1]]) < 0) {
        status=errno;
        perror("dirrm");
    }else{
        status=0;
    }
}

void renameMy(){

    if(token_count>2)
    if (rename(&line[tokens[1]],&line[tokens[2]]) < 0) {
        status=errno;
        perror("rename");
    }else{
        status=0;
    }
}

void unlinkMy(){

    if(token_count>1)
    if (unlink(&line[tokens[1]]) < 0) {
        status=errno;
        perror("unlink");
    }else{
        status=0;
    }
}

void removeMy(){

    if(token_count>1)
    if (remove(&line[tokens[1]]) < 0) {
        status=errno;
        perror("remove");
    }else{
        status=0;
    }
}

void linkhard(){

    if(token_count>2)
    if (link(&line[tokens[1]],&line[tokens[2]]) < 0) {
        status=errno;
        perror("linkhard");
    }else{
        status=0;
    }
}

void linksoft(){

    if(token_count>2)
    if (symlink(&line[tokens[1]],&line[tokens[2]]) < 0) {
        status=errno;
        perror("linksoft");
    }else{
        status=0;
    }
}

void linkread(){

    char cilj[1024];
    ssize_t len;
    if(token_count>1){
    len = readlink(&line[tokens[1]], cilj, sizeof(cilj)-1);
    if (len < 0) {
        status=errno;
        perror("linksoft");
    } else {
        cilj[len] = '\0';
        printf("%s\n",cilj);
        status=0;
    }
    }
}

void linklist(){
    DIR *dir;
    struct dirent *entry;
    struct stat info;

    dir = opendir(".");
    if (dir == NULL) {
        status=errno;
        perror("linklist");
        return;
    }

    if (stat(&line[tokens[1]], &info) == -1) {
        status=errno;
        perror("Unable to get file status");
        closedir(dir);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        struct stat entry_stat;

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if (stat(entry->d_name, &entry_stat) == -1) {
            perror("Unable to get file status");
            continue;
        }

        if (entry_stat.st_ino == info.st_ino && entry_stat.st_dev == info.st_dev) {
            printf("%s  ", entry->d_name);
        }
    }

    closedir(dir);
    printf("\n");
    status=0;
}

void cat(int vir, int cel){
    char buffer[4096];
    ssize_t prebrano, zapisano;

    while ((prebrano = read(vir, buffer, 4096)) > 0) {
        zapisano = write(cel, buffer, prebrano);
        if (zapisano != prebrano) {
            status=errno;
            perror("cpcat");
            return;
        }
    }

    if (prebrano == -1) {
        status = errno;
        perror("cpcat");
    }
}

void cpcat() {

int vhodni, izhodni;

    if (token_count >= 2 && strcmp(&line[tokens[1]], "-") != 0) {
        vhodni = open(&line[tokens[1]], O_RDONLY);
        if (vhodni == -1) {
            status=errno;
            perror("cpcat");
            return;
        }
    } else {
        vhodni = STDIN_FILENO;
    }

    if (token_count >= 3) {
        izhodni = open(&line[tokens[2]], O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (izhodni == -1) {
            status=errno;
            perror("cpcat");
            return;
        }
    } else {
        izhodni = STDOUT_FILENO;
    }

    cat(vhodni, izhodni);

    if (vhodni != STDIN_FILENO) {
        if (close(vhodni) == -1) {
            status=errno;
            perror("cpcat");
            return;
        }
    }
    if (izhodni != STDOUT_FILENO) {
        if (close(izhodni) == -1) {
            status=errno;
            perror("cpcat");
            return;
        }
    }
}

void pid(){
    pid_t pid = getpid();

    printf("%d\n", pid);
}

void ppid(){
    pid_t pid = getppid();

    printf("%d\n", pid);
}

void uid(){

    uid_t uid = getuid();
    printf("%d\n", uid);
}

void euid(){

   uid_t euid = geteuid();
    printf("%d\n", euid);
}

void gid(){

    gid_t gid = getgid();
    
    printf("%d\n", gid);
}

void egid(){

    gid_t egid = getegid();
    
    printf("%d\n", egid);
}

void sysinfo(){

    struct utsname info;
    if (uname(&info) != 0) {
        status=errno;
        perror("uname");
        return;
    }

    printf("Sysname: %s\n", info.sysname);
    printf("Nodename: %s\n", info.nodename);
    printf("Release: %s\n", info.release);
    printf("Version: %s\n", info.version);
    printf("Machine: %s\n", info.machine);

    status=0;
}

void proc() {


    if (token_count<2) {
        printf("%s\n", procfs_path);
        status=0;
        return;
    }

    if (access(&line[tokens[1]], F_OK | R_OK) < 0) {
            status=1;
            return;
    }

    strncpy(procfs_path, &line[tokens[1]], 256);
    status=0;
}

int pids(int x) {

    DIR *dir;
    struct dirent *entry;

    dir = opendir(procfs_path);
    if (dir == NULL) {
        status=errno;
        perror("pids");
        return status;
    }
    int count_pidss=0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char *endptr;
            long pid = strtol(entry->d_name, &endptr, 10);
            if (*endptr == '\0') {
                pidss[count_pidss]=pid;
                count_pidss++;
            }
        }
    }

    closedir(dir);

    qsort(pidss, count_pidss, sizeof(long), compare);
    if(x==0){
    for(int i = 0; i<count_pidss; i++){
        printf("%ld\n", pidss[i]);
    }
    }else{
        return count_pidss;
    }

    return 0;
}


void pinfo() {
    int broj = pids(10);

    printf("%5s %5s %6s %s\n", "PID", "PPID", "STANJE", "IME");
    for(int i = 0; i < broj; i++) {
        int fd;
        int pid = pidss[i];
        char buffer[1000];
        char path[1024];
        sprintf(path, "%s/%d/stat", procfs_path, pid);
        
        fd = open(path, O_RDONLY);
        if (fd == -1) {
            int status = errno;
            perror("pinfo");
            return;
        }

        ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
        if (bytesRead == -1) {
            int status = errno;
            perror("pinfo");
            close(fd);
            return;
        }
        
        buffer[bytesRead] = '\0';  // Null-terminate the buffer

        int ppid;
        char state;
        char name[100];

        sscanf(buffer, "%*d %s %c %d", name, &state, &ppid);

        int len = strlen(name);
        if (name[0] == '(' && name[len - 1] == ')') {
            memmove(name, name + 1, len - 2);
            name[len - 2] = '\0';
        }
        printf("%5d %5d %6c %s\n", pid, ppid, state, name);

        close(fd);
    }
}
void head(const char *filename, int lines) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    int line_count = 0;

    while (fgets(buffer, sizeof(buffer), file) != NULL && line_count < lines) {
        printf("%s", buffer);
        line_count++;
    }

    fclose(file);
}

void tail(const char *filename, int lines) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    char *tail_lines[lines];
    int line_count = 0;

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        tail_lines[line_count % lines] = strdup(buffer);
        line_count++;
    }

    int start_index = (line_count > lines) ? (line_count % lines) : 0;
    for (int i = 0; i < lines; i++) {
        printf("%s", tail_lines[(start_index + i) % lines]);
    }

    fclose(file);
}


void execute_external(){

    pid_t pid;
    int status_dr;
    fflush(stdin);
    pid = fork();
    if (pid == -1) {
        
        status=errno;
        perror("fork()");
    } else if (pid == 0) {
        char *args[token_count+1];

        for(int i = 0; i<token_count; i++){
        args[i]=&line[tokens[i]];
        }
        args[token_count]=NULL;

        if(execvp(args[0], args)<0){
            status=127;
        }else
         status=0;

         exit(status);
    }
    else {
      waitpid(pid, &status, 0);
    }

    status = status >> 8;
}

void waitone(){

    pid_t otrok;

    if (token_count < 2 ) { 
        otrok = wait(&status);
    } else { 
        otrok = waitpid(atoi(&line[tokens[1]]), &status, 0);
    }

    if (otrok == -1) { 
        status = 0;
    }
    status = status >> 8;
}

void waitall() {
 
    int a;
    while ((a = waitpid(-1, &status, 0)) > 0) {

    }

}
void execute_builtin( int vkupna_dolzina);

void conditions(){

    for(int i = 1; i<token_count; i=i+2){

        fflush(stdin);
        int pid = fork();
        if(pid==0){

            int dolzina = strlen(&line[tokens[i]]);
            char nova[dolzina+2];
            for(int q =0; q <= dolzina; q++){
                nova[q]=line[tokens[i]+q];
            }
            nova[dolzina]='\n';
            nova[dolzina+1]='\0';
            token_count=tokenize(nova);
            //printf("token brojaco e %d\n",token_count);

            for(int g = 0; g < dolzina+2; g++){
                line[g]=nova[g];
            }
            execute_builtin(dolzina);
            exit(status);

        }else{
            waitpid(pid, &status, 0);
        }

        if(strcmp(&line[tokens[i+1]],"||")==0){
            if(status==0)
            i = i + 2;
        }

        if(strcmp(&line[tokens[i+1]],"&&")==0){
            if(status>0)
            i = i + 2;
        }
    }
}
void execute_background( int vkupna_dolzina);
void builtin_for(){
    if(token_count<3){
        printf("Not enough arguments\n");
        status = 99;
        return;
    }

    int x = atoi(&line[tokens[1]]);
    int y = atoi(&line[tokens[2]]);

    int brojac = token_count;

    if(y<x){
        int d = y;
        y = x;
        x = d;
    }

    for(int m = x; m<y; m++){


        for(int i = 3; i < brojac; i++){
        fflush(stdin);
        fflush(stdout);
        fflush(stderr);

        int pid = fork();
        if(pid==0){

            
            //printf("token brojaco e %d\n",token_count);
            int dolzina = strlen(&line[tokens[i]]);
            char nova[dolzina+2];
            for(int q =0; q <= dolzina; q++){
                nova[q]=line[tokens[i]+q];
            }
            nova[dolzina]='\n';
            nova[dolzina+1]='\0';
            //printf("tokenicirame nova i toa e %s\n",nova);
            token_count=tokenize(nova);
            //printf("token brojaco e %d\n",token_count);

            for(int g = 0; g < dolzina+2; g++){
                line[g]=nova[g];
             }
            execute_builtin(dolzina);
            exit(status);
        }else{
            waitpid(pid, &status, 0);
        }
    }

    }


}



void pipes(){

    int br=token_count;

    int fd[br-2][2];
    for(int i = 0; i<br-2; i++){
        pipe(fd[i]);
    }

    for(int i = 1; i < br; i++){
        fflush(stdin);
        fflush(stdout);
        fflush(stderr);
        if(fork()==0){

            if(i<br-1){
                dup2(fd[i-1][1],1);
                dup2(fd[i-1][1],2);
            }
            if(i>1){
                dup2(fd[i-2][0],0);
            }

            for(int j = 0; j < br-2; j++){
                close(fd[j][0]);
                close(fd[j][1]);
            }
            
            //printf("token brojaco e %d\n",token_count);
            int dolzina = strlen(&line[tokens[i]]);
            char nova[dolzina+2];
            for(int q =0; q <= dolzina; q++){
                nova[q]=line[tokens[i]+q];
            }
            nova[dolzina]='\n';
            nova[dolzina+1]='\0';
            //printf("tokenicirame nova i toa e %s\n",nova);
            token_count=tokenize(nova);
            //printf("token brojaco e %d\n",token_count);

            for(int g = 0; g < dolzina+2; g++){
                line[g]=nova[g];
             }

            input = 0;
            output = 0;
            append = 0;
            background = 0;
            int count_star=token_count;
            //printf("stiga do tuka%s \n",line);

        int x = token_count;        
        if(token_count>=2){
            for(int aa = x-1; aa>=1 && aa>=x-3; aa-- ){
                if(line[tokens[aa]]=='&'){
                    background = tokens[aa];
                    token_count--;
                }
                if(line[tokens[aa]]=='<'){
                    input = tokens[aa]+1;
                    token_count--;
                }
                if(line[tokens[aa]]=='>'){
                    if(line[tokens[aa]+1]=='>'){
                    append = tokens[aa]+2;
                    output=0;
                    
                    }
                    else{
                    output = tokens[aa]+1;
                    append=0;
                    }
                    token_count--;
                }
            }
        }

        //printf("inputo e %s\n",&line[input]);
        if (input > 0) {
        int fd_input = open(&line[input], O_RDONLY);
        
        if (fd_input == -1) {
            perror("Error input");
            exit(EXIT_FAILURE);
        }
        fflush(stdin);
        star_input = dup(STDIN_FILENO);
        dup2(fd_input, STDIN_FILENO);
        close(fd_input);
        }

        if (output > 0) {
        int fd_output = open(&line[output], O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd_output == -1) {
            perror("Error output");
            exit(EXIT_FAILURE);
        }
        fflush(stdout);
        star_output = dup(STDOUT_FILENO);
        dup2(fd_output, STDOUT_FILENO);
        close(fd_output);
        }
        if (append > 0) {
        int fd_append = open(&line[append], O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd_append == -1) {
            perror("Error append");
            exit(EXIT_FAILURE);
        }
        fflush(stdout);
        star_output = dup(STDOUT_FILENO);
        dup2(fd_append, STDOUT_FILENO);
        close(fd_append);
        }
   
        if(background>0){
            execute_background( dolzina);
        }else{
            execute_builtin( dolzina);
            
        }

        if(input>0){
            fflush(stdin);
            dup2(star_input, STDIN_FILENO);
        }
        if(output>0){
            fflush(stdout);
            dup2(star_output, STDOUT_FILENO);
        }
        if(append>0){
            fflush(stdout);
            dup2(star_output, STDOUT_FILENO);
        }
            exit(status);
        }
    }

    for(int j = 0; j < br-2; j++){
        close(fd[j][0]);
        close(fd[j][1]);
    }

    for(int i = 1; i<br; i++){
        wait(NULL);
    }

}


void execute_builtin( int vkupna_dolzina){

    if(strcmp(&line[0],"debug")==0){
        if(debug_level>0){
            ispisi(line,token_count); 
            if(background==0){
                printf("Executing builtin 'debug' in foreground\n");
            }else{
                printf("Executing builtin 'debug' in background\n");
            }
        }
        if(token_count>1){
            debug_level=atoi(&line[tokens[1]]);
        }else{
            printf("%d\n",debug_level);
        }

    }else if(strcmp(&line[0],"prompt")==0){
        if(token_count==1){
            printf("%s\n",pozivnik);
            status=0;
        }else{
            if(strlen(&line[tokens[1]])>8){
                status = 1;
                strcpy(pozivnik,"mysh\0");
            }else{
                status = 0;
                strcpy(pozivnik,&line[tokens[1]]);
            }
        }

    }else if(strcmp(&line[0],"status")==0){
        printf("%d\n",status);

    }else if(strcmp(&line[0],"exit")==0){
        if (token_count>1){
            int izlez = atoi(&line[tokens[1]]);
            exit(izlez);
        }else{
            exit(status);
        }

    }else if(strcmp(&line[0],"help")==0){
        printf("Ukaz: debug level\n");
        printf("     * Opcijski stevilcni argument level podaja nivo razhroscevanja (debug level).\n");
        printf("     * Ce argument ni podan, se izpise trenutni nivo razhroscevanja.\n");
        printf("     * Ce pa je podan, se nastavi nivo razhroscevanja na podani level. Ce uporabnik poda stevilo v napacni obliki, potem se privzame 0.\n");
        printf("     * Ce je nivo razhroscevanja vecji od 0, izpise vhodne vrstice, simbolov in zaznanih posebnosti izvedbe.\n");
        printf("     * Zacetni nivo razhroscevanja je 0.\n");
        printf("\n");
        printf("Ukaz: prompt poziv\n");
        printf("     * S tem ukazovom izpisemo ali nastavimo izpis pozivnika (prompt).\n     * Ce argument ni podan, potem se izpise trenutni poziv.\n");
        printf("     * Ce je podan, pa se nastavi nov pozivnik. Podano ime lahko vsebuje do 8 znakov, v primeru daljsega imena, se vrne izhodni status 1.\n");
        printf("\n");
        printf("Ukaz: status\n");
        printf("     * Izpise se izhodni status zadnjega izvedenega ukaza.\n     * Ta ukaz izjemoma status pusti nespremenjen.\n");
        printf("\n");
        printf("Ukaz: exit status\n");
        printf("     * Izvajanje lupine se zakljuci s podanim izhodnim statusom.\n");
        printf("     * Ce argument ni podane, se lupina konca s statusom zadnjega izvedenega ukaza.\n");
        printf("\n");

    }else if(strcmp(&line[0],"print")==0 ){
            print();
    }else if(strcmp(&line[0],"echo")==0){
        echo();
    }else if(strcmp(&line[0],"len")==0){
        len(vkupna_dolzina);
    }else if(strcmp(&line[0],"sum")==0){
        sum();
    }else if(strcmp(&line[0],"calc")==0){
        calc();
    }else if(strcmp(&line[0],"basename")==0){
        basename(0,line);
    }else if(strcmp(&line[0],"dirname")==0){
        dirname();
    }else if(strcmp(&line[0],"dirch")==0){
        dirch();
    }else if(strcmp(&line[0],"dirwd")==0){
        dirwd();
    }else if(strcmp(&line[0],"dirmk")==0){
            dirmk();
    }else if(strcmp(&line[0],"dirls")==0){
        dirls();
    }else if(strcmp(&line[0],"dirrm")==0){
        dirrm();
    }else if(strcmp(&line[0],"rename")==0){
        renameMy();
    }else if(strcmp(&line[0],"unlink")==0){
        unlinkMy();
    }else if(strcmp(&line[0],"remove")==0){
        removeMy();
    }else if(strcmp(&line[0],"linkhard")==0){
        linkhard();
    }else if(strcmp(&line[0],"linksoft")==0){
        linksoft();
    }else if(strcmp(&line[0],"linkread")==0){
        linkread();
    }else if(strcmp(&line[0],"linklist")==0){
        linklist();
    }else if(strcmp(&line[0],"cpcat")==0){
        cpcat();
    }else if(strcmp(&line[0],"pid")==0){
        pid();
    }else if(strcmp(&line[0],"ppid")==0){
        ppid();
    }else if(strcmp(&line[0],"uid")==0){
        uid();
    }else if(strcmp(&line[0],"euid")==0){
        euid();
    }else if(strcmp(&line[0],"gid")==0){
        gid();
    }else if(strcmp(&line[0],"egid")==0){
        egid();
    }else if(strcmp(&line[0],"sysinfo")==0){
        sysinfo();
    }else if(strcmp(&line[0],"proc")==0){
        proc();
    }else if(strcmp(&line[0],"pids")==0){
        int broj = pids(0);
    }else if(strcmp(&line[0],"pinfo")==0){
        pinfo();
    }else if(strcmp(&line[0],"waitone")==0){
        waitone();
    }else if(strcmp(&line[0],"waitall")==0){
        waitall();
    }else if(strcmp(&line[0],"pipes")==0){
        pipes();
    }else if(strcmp(&line[0],"for")==0){
        builtin_for();
    }else if(strcmp(&line[0],"head")==0){

        if(token_count == 3){
            int broj = atoi(&line[tokens[1]]);
            head(&line[tokens[2]],broj);
        }else if(token_count == 2){
            head(&line[tokens[1]], 1);
        }else{
            printf("head: Not correct number of arguments\n");
        }
    }else if(strcmp(&line[0],"tail")==0){

        if(token_count == 3){
            int broj = atoi(&line[tokens[1]]);
            tail(&line[tokens[2]],broj);
        }else if(token_count == 2){
            tail(&line[tokens[1]], 1);
        }else{
            printf("head: Not correct number of arguments\n");
        }
    }else if(strcmp(&line[0],"condition")==0){
        conditions();
    }
    else{
        if(token_count == 0){
            return;
        }
        execute_external();
    }

}

void execute_background( int vkupna_dolzina){

    
    fflush(stdin);
    int pid = fork();
    if (pid> 0) {
        //waitpid(pid, &status, 0);
    } else {
        execute_builtin(vkupna_dolzina);
        exit(status);
    }
   
}


int main(int argc, char *argv[]){

    strcpy(pozivnik,"mysh>\0");
    strcpy(procfs_path,"/proc");
    // if(isatty(STDIN_FILENO)){
    //     printf("%s>",pozivnik);
    // }

    char *history[MAX_HISTORY_SIZE];
    int history_index = 0;
    
    
   // while ( fgets(line, sizeof(line), stdin)>0 ) {
    while(1){
        char* vlez = readline(pozivnik);
        if(!vlez){
            break;
        }

        add_history(vlez);

        strcpy(line, vlez);

        bool nema = true;

        if(line[0]=='!'){
            int x;
            if(line[1]=='#')
            x = atoi(&line[2]);
            if(line[1]=='!')
            x = history_index;
            strcpy(line,history[x-1]);
            nema = false;
        }
        int vkupna_dolzina = strlen(line);
        line[vkupna_dolzina] = '\n';
        line[vkupna_dolzina+1] = '\0';
        vkupna_dolzina++;
        
        token_count=0;
        fflush(stderr);
        fflush(stdout);
        int x=tokenize(line);
        input = 0;
        output = 0;
        background = 0;
        int count_star=token_count;
        //printf("stiga do tuka%s \n",line);


        if(token_count>=2){
            for(int i = x-1; i>=1 && i>x-3; i-- ){
                if(line[tokens[i]]=='&'){
                    background = tokens[i];
                    token_count--;
                }
                if(line[tokens[i]]=='<'){
                    input = tokens[i]+1;
                    token_count--;
                }
                if(line[tokens[i]]=='>'){
                    if(line[tokens[i]+1]=='>'){
                    append = tokens[i]+2;
                    output=0;
                    
                    }
                    else{
                    output = tokens[i]+1;
                    append=0;
                    }
                    token_count--;
                }
            }
        }

        if(token_count<=0){
            // if(isatty(STDIN_FILENO))
            // printf("%s>",pozivnik);
            continue;
        }



        if (strcmp(&line[tokens[0]], "history") == 0) {
            for (int i = 0; i < history_index; i++) {
                printf("%d: %s\n", i + 1, history[i]);
            }
            free(vlez);
            continue;
        }else if(strcmp(&line[tokens[0]], "delete_history") == 0){
            history_index=0;
            free(vlez);
            continue;
        }

        if(nema)
        if (history_index < MAX_HISTORY_SIZE) {
            history[history_index++] = strdup(vlez);
        } else {
            free(history[0]);
            for (int i = 0; i < MAX_HISTORY_SIZE - 1; i++) {
                history[i] = history[i + 1];
            }
            history[MAX_HISTORY_SIZE - 1] = strdup(vlez);
        }

        if (input > 0) {
        int fd_input = open(&line[input], O_RDONLY);
        
        if (fd_input == -1) {
            perror("Error input");
            exit(EXIT_FAILURE);
        }
        fflush(stdin);
        star_input = dup(STDIN_FILENO);
        dup2(fd_input, STDIN_FILENO);
        close(fd_input);
        }

        if (output > 0) {
        int fd_output = open(&line[output], O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd_output == -1) {
            perror("Error output");
            exit(EXIT_FAILURE);
        }
        fflush(stdout);
        star_output = dup(STDOUT_FILENO);
        dup2(fd_output, STDOUT_FILENO);
        close(fd_output);
        }

        if (append > 0) {
        int fd_append = open(&line[append], O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd_append == -1) {
            perror("Error append");
            exit(EXIT_FAILURE);
        }
        fflush(stdout);
        star_output = dup(STDOUT_FILENO);
        dup2(fd_append, STDOUT_FILENO);
        close(fd_append);
        }

   
        if(background>0){
            execute_background( vkupna_dolzina);
        }else{
            execute_builtin( vkupna_dolzina);
            
        }

        if(input>0){
            fflush(stdin);
            dup2(star_input, STDIN_FILENO);
        }
        if(output>0){
            fflush(stdout);
            dup2(star_output, STDOUT_FILENO);
        }
        if(append>0){
            fflush(stdout);
            dup2(star_output, STDOUT_FILENO);
        }
        // if(isatty(STDIN_FILENO)){
        // printf("%s>",pozivnik);
        // }
        free(vlez);

    }

    for (int i = 0; i < history_index; i++) {
        free(history[i]);
    }

    return status;
        
    
}
