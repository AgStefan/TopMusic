/* servTCPConcTh2.c - Exemplu de server TCP concurent care deserveste clientii
   prin crearea unui thread pentru fiecare client.
   Asteapta un numar de la clienti si intoarce clientilor numarul incrementat.
	Intoarce corect identificatorul din program al thread-ului.
  
   
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sqlite3.h>
#include <pthread.h>
using namespace std;

/* portul folosit */
#define PORT 2908
#define MESSAGE_SIZE 10000
#define POZITIV "da"
#define NEGATIV "nu"

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData
{
    int idThread; //id-ul thread-ului tinut in evidenta de acest program
    int cl;       //descriptorul intors de accept
} thData;

char *dbError = 0;
sqlite3 *database;

char numeUser[MESSAGE_SIZE];

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
void receiveMessage(void *arg, char message[MESSAGE_SIZE]);
void sendMessage(void *arg, char message[MESSAGE_SIZE]);
void registerAccount(void *arg);
void loginAccount(void *arg);
void afterLoginMenu(void *arg);
void addSong(void *arg);
void votMelodie(void *arg);
void afisareTopMelodii(void *arg);
void afisareDupaGen(void *arg);
void adminRequest(void *arg);
void confirmRequest(void *arg);
void unbanRequest(void *arg);
void voteBan(void *arg);
void allowVote(void *arg);
void deleteSong(void *arg);
// void addComment(void *arg);
pthread_t th[100]; //Identificatorii thread-urilor care se vor crea
int main()
{
    struct sockaddr_in server; // structura folosita de server
    struct sockaddr_in from;
    int nr; //mesajul primit de trimis la client
    int sd; //descriptorul de socket
    int pid;
    
    int i = 0;

    if (sqlite3_open("Database.db", &database))
    {
        printf("Could not open database.");
    }

    /* crearea unui socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server]Eroare la socket().\n");
        return errno;
    }
    /* utilizarea optiunii SO_REUSEADDR */
    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    /* pregatirea structurilor de date */
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons(PORT);

    /* atasam socketul */
    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server]Eroare la bind().\n");
        return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen(sd, 2) == -1)
    {
        perror("[server]Eroare la listen().\n");
        return errno;
    }
    /* servim in mod concurent clientii...folosind thread-uri */
    while (1)
    {
        int client;
        thData *td; //parametru functia executata de thread
        unsigned int length = sizeof(from);

        printf("[server]Asteptam la portul %d...\n", PORT);
        fflush(stdout);

        // client= malloc(sizeof(int));
        /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
        if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
        {
            perror("[server]Eroare la accept().\n");
            continue;
        }

        /* s-a realizat conexiunea, se astepta mesajul */

        // int idThread; //id-ul threadului
        // int cl; //descriptorul intors de accept

        td = (struct thData *)malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;

        pthread_create(&th[i], NULL, &treat, td);

    } //while
};
static void *treat(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);
    printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush(stdout);
    pthread_detach(pthread_self());
    raspunde((struct thData *)arg);
    printf("[Thread %d] Clientul s-a deconectat.\n", tdL.idThread);
    /* am terminat cu acest client, inchidem conexiunea */
    close((intptr_t)arg);
    return (NULL);
};

void raspunde(void *arg)
{
    char nr[MESSAGE_SIZE];
    int i = 0;
    struct thData tdL;
    tdL = *((struct thData *)arg);

    while (1)
    {
        receiveMessage(arg, nr);
        printf("[Thread %d]Mesajul a fost receptionat...%s\n", tdL.idThread, nr);
        if (strcmp(nr, "register") == 0)
        {

            registerAccount(arg);
        }
        else if (strcmp(nr, "login") == 0)
        {
            loginAccount(arg);
            
        }
        else if (strcmp(nr, "exit") == 0)
        {
            break;
        }
    }
}

void receiveMessage(void *arg, char message[MESSAGE_SIZE])
{
    struct thData tdL;
    tdL = *((struct thData *)arg);

    if (read(tdL.cl, message, MESSAGE_SIZE) <= 0)
    {
        printf("[Thread %d] ", tdL.idThread);
        perror("Eroare la read() de la client.\n");
        pthread_cancel(th[tdL.idThread]);
    }
    else {
        printf("[Thread %d] Am primit mesajul %s", tdL.idThread, message);
    }
}

void sendMessage(void *arg, char message[MESSAGE_SIZE])
{
    struct thData tdL;
    tdL = *((struct thData *)arg);

    if (write(tdL.cl, message, MESSAGE_SIZE) <= 0)
    {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
        pthread_cancel(th[tdL.idThread]);
    }
    else
        printf("[Thread %d]Mesajul %s a fost transmis cu succes.\n", tdL.idThread, message);
}

// Inregistrare cont in baza de date
void registerAccount(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);

    char username[MESSAGE_SIZE];
    char message[MESSAGE_SIZE];
    char query[1000];
    char **result;
    int rows, columns;

    while (1)
    {
        receiveMessage(arg, username);

        strcpy(query, "SELECT * from accounts where username='");
        strcat(query, username);
        strcat(query, "';");

        //Verificare username existent
        sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
        if (rows >= 1)
        {
            strcpy(message, "nu");
            sendMessage(arg, message);
        }
        else
        {
            strcpy(message, "da");
            sendMessage(arg, message);
            break;
        }
    }

    char password[MESSAGE_SIZE];
    receiveMessage(arg, password);

    //Inserare utilizator in baza de date
    strcpy(query, "INSERT INTO accounts (username,password,id_user, can_vote) values (\'");
    strcat(query, username);
    strcat(query, "\',\'");
    strcat(query, password);
    strcat(query, "\', 0,1);");
    sqlite3_exec(database, query, NULL, NULL, &dbError);
    sqlite3_commit_hook(database, NULL, NULL);
}

void loginAccount(void *arg)
{
    
    struct thData tdL;
    tdL = *((struct thData *)arg);

    char username[MESSAGE_SIZE];
    char password[MESSAGE_SIZE];
    char query[1000];
    char **result;
    int rows, columns;
    char message[MESSAGE_SIZE];

    while (1)
    {
        receiveMessage(arg, username);

        strcpy(query, "SELECT * from accounts where username='");
        strcat(query, username);
        strcat(query, "';");

        //Verificare daca username-ul exista in db
        sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);

        if (rows < 1)
        {
            strcpy(message, "nu");
            sendMessage(arg, message);
            receiveMessage(arg, message);
            if (strcmp(message, POZITIV) == 0)
            {
                registerAccount(arg);
                break;
            }
        }
        else
        {
            strcpy(message, "da");
            sendMessage(arg, message);
            while (1)
            {
                //Verificare daca username + password exista in db
                receiveMessage(arg, password);
                strcpy(query, "SELECT * from accounts where username='");
                strcat(query, username);
                strcat(query, "' and password= '");
                strcat(query, password);
                strcat(query, "';");
                sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
                if (rows < 1)
                {
                    strcpy(message, "nu");
                    sendMessage(arg, message);
                }
                else
                {
                    strcpy(message, "da");
                    sendMessage(arg, message);
                    receiveMessage(arg, numeUser);
                    afterLoginMenu(arg);
                    break;
                }
            }
            break;
        }
    }
}

void afterLoginMenu(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);
    char raspuns[MESSAGE_SIZE];
    char query[1000];
    char **result;
    int rows, columns;
    strcpy(query, "SELECT id_user from accounts where username='");
    strcat(query, numeUser);
    strcat(query, "';");
    sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
    if (strcmp(result[1], "0") == 0)
    {
        strcpy(raspuns, "user");
        //Transmitem clientului ca s-a logat un user
        sendMessage(arg, raspuns);
    }
    else
    {
        strcpy(raspuns, "admin");
        //Transmitem clientului ca s-a logat un user
        sendMessage(arg, raspuns);
    }
    while (1)
    {
        if (strcmp(raspuns, "user") == 0)
        {

            char comanda[MESSAGE_SIZE];
            receiveMessage(arg, comanda);
            if (strcmp(comanda, "adauga") == 0)
            {
                addSong(arg);
            }
            else if (strcmp(comanda, "afisareTop") == 0)
            {
                afisareTopMelodii(arg);
            }
            else if (strcmp(comanda, "voteaza") == 0)
            {
                votMelodie(arg);
            }
            else if (strcmp(comanda, "afisareDupaGen") == 0)
            {
                afisareDupaGen(arg);
            }
            else if (strcmp(comanda, "delogare") == 0)
            {
                
                break;
            }
            else if (strcmp(comanda, "request") == 0)
            {
                adminRequest(arg);
            }
            else if (strcmp(comanda, "unban") == 0)
            {
                unbanRequest(arg);
            }
        }
        else if (strcmp(raspuns, "admin") == 0)
        {
            char comanda[MESSAGE_SIZE];
            receiveMessage(arg, comanda);
            if (strcmp(comanda, "adauga") == 0)
            {
                addSong(arg);
            }
            else if (strcmp(comanda, "afisareTop") == 0)
            {
                afisareTopMelodii(arg);
            }
            else if (strcmp(comanda, "voteaza") == 0)
            {
                votMelodie(arg);
            }
            else if (strcmp(comanda, "afisareDupaGen") == 0)
            {
                afisareDupaGen(arg);
            }
            else if (strcmp(comanda, "cereri") == 0)
            {
                confirmRequest(arg);
            }
            else if (strcmp(comanda, "restrict") == 0)
            {
                voteBan(arg);
            }
            else if (strcmp(comanda, "allow") == 0)
            {
                allowVote(arg);
            }
            else if (strcmp(comanda, "stergere") == 0) {
                deleteSong(arg);
            }
            else if (strcmp(comanda, "delogare") == 0)
            {
                
                break;
            }
        }
    }
}

void addSong(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);

    char numeMelodie[MESSAGE_SIZE];
    char artistMelodie[MESSAGE_SIZE];
    char genMelodie[MESSAGE_SIZE];
    char linkMelodie[MESSAGE_SIZE];
    char descriere[MESSAGE_SIZE];
    char query[1000];
    char query1[1000];
    char **result;
    char **result2;
    int rows, columns;
    char raspuns[MESSAGE_SIZE];
    int m = 1;
    int n = 1;
    char *pch;
    while (m)
    {

        while (n)
        {
            receiveMessage(arg, numeMelodie);
            receiveMessage(arg, artistMelodie);
            receiveMessage(arg, genMelodie);
            pch = strtok(genMelodie, "/");
            receiveMessage(arg, linkMelodie);
            receiveMessage(arg, descriere);
            strcpy(query, "SELECT * from songs where name='");
            strcat(query, numeMelodie);
            strcat(query, "' and artist='");
            strcat(query, artistMelodie);
            strcat(query, "';");
            sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
            if (rows >= 1)
            {
                strcpy(raspuns, "nu");
                sendMessage(arg, raspuns);
                receiveMessage(arg, raspuns);
                if (strcmp(raspuns, NEGATIV) == 0)
                {
                    m = 0;
                    n = 0;
                    break;
                }
            }
            else
            {
                strcpy(raspuns, "da");
                sendMessage(arg, raspuns);
                strcpy(query1, "INSERT INTO songs (name,artist,votes,link, descriere) values (\'");
                strcat(query1, numeMelodie);
                strcat(query1, "\',\'");
                strcat(query1, artistMelodie);
                strcat(query1, "\',0,");
                strcat(query1, "\'");
                strcat(query1, linkMelodie);
                strcat(query1, "\','");
                strcat(query1, descriere);
                strcat(query1, "\');");
                sqlite3_exec(database, query1, NULL, NULL, &dbError);
                cout << "Song added\n";

                //Adaugare gen in baza de date
                while (pch != NULL)
                {
                    //Verificare daca genul exista sau nu baza de date
                    strcpy(query1, "SELECT id from genre where name=\'");
                    strcat(query1, pch);
                    strcat(query1, "\';");
                    sqlite3_get_table(database, query1, &result, &rows, &columns, &dbError);
                    char *genId;
                    //Daca nu exista va fi introdus in db
                    if (rows < 1)
                    {
                        strcpy(query1, "INSERT INTO genre (name) values (\'");
                        strcat(query1, pch);
                        strcat(query1, "\');");
                        sqlite3_exec(database, query1, NULL, NULL, &dbError);
                        sqlite3_commit_hook(database, NULL, NULL);
                        //Cautare id gen
                        strcpy(query1, "SELECT id from genre where name='");
                        strcat(query1, pch);
                        strcat(query1, "';");
                        sqlite3_get_table(database, query1, &result, &rows, &columns, &dbError);
                        genId = result[1];
                    }
                    else
                    {
                        genId = result[1];
                    }

                    //Cautare id song
                    strcpy(query1, "SELECT id from songs where name='");
                    strcat(query1, numeMelodie);
                    strcat(query1, "' and artist='");
                    strcat(query1, artistMelodie);
                    strcat(query1, "';");
                    sqlite3_get_table(database, query1, &result2, &rows, &columns, &dbError);
                    cout << "Query4...";
                    //Insearare in tabelul de lagatura id-ul song si id-ul genului.
                    strcpy(query1, "INSERT INTO songGenres (id_song,id_genre) values (\'");
                    strcat(query1, result2[1]);
                    strcat(query1, "\',\'");
                    strcat(query1, genId);
                    strcat(query1, "\');");
                    sqlite3_exec(database, query1, NULL, NULL, &dbError);
                    sqlite3_commit_hook(database, NULL, NULL);
                    
                    m = 0;
                    n = 0;
                    pch = strtok(NULL, "/");
                }
                strcpy(raspuns, "da");
                    sendMessage(arg, raspuns);
            }
        }
    }
}

void votMelodie(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);

    while (1)
    {
        char query[1000];
        char **result;
        int rows, columns;
        char raspuns[MESSAGE_SIZE];
        char afisareMelodie[MESSAGE_SIZE];
        char idSong[MESSAGE_SIZE];
        strcpy(query, "SELECT can_vote from accounts where username='");
        strcat(query, numeUser);
        strcat(query, "';");
        sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
        if (strcmp(result[1], "0") == 0)
        {
            strcpy(raspuns, "nu");
            sendMessage(arg, raspuns);
            break;
        }
        else
        {
            strcpy(raspuns, "da");
            sendMessage(arg, raspuns);

            strcpy(query, "SELECT * from songs order by votes desc");
            strcat(query, ";");
            sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
            sprintf(raspuns, "%d", rows);
            sendMessage(arg, raspuns);
            for (int i = 1; i <= rows; i++)
            {
                char loc[MESSAGE_SIZE];
                sprintf(loc, "%d", i);
                strcpy(afisareMelodie, "Locul: ");
                strcat(afisareMelodie, loc);
                strcat(afisareMelodie, "\n");
                strcat(afisareMelodie, "ID:");
                strcat(afisareMelodie, result[i * columns]);
                strcat(afisareMelodie, "\n");
                strcat(afisareMelodie, "Nume: ");
                strcat(afisareMelodie, result[i * columns + 1]);
                strcat(afisareMelodie, "\n");
                strcat(afisareMelodie, "Artist: ");
                strcat(afisareMelodie, result[i * columns + 2]);
                strcat(afisareMelodie, "    Votes: ");
                strcat(afisareMelodie, result[i * columns + 3]);
                strcat(afisareMelodie, "\n");
                strcat(afisareMelodie, "Link: ");
                strcat(afisareMelodie, result[i * columns + 4]);
                strcat(afisareMelodie, "\n\n");
                sendMessage(arg, afisareMelodie);
            }
            receiveMessage(arg, idSong);

            //Cautam daca id-ul exista sau nu in db

            strcpy(query, "SELECT * from songs where id=");
            strcat(query, idSong);
            strcat(query, ";");
            sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);

            if (rows < 1)
            {
                strcpy(raspuns, "nu");
                sendMessage(arg, raspuns);
            }
            else
            {
                strcpy(raspuns, "da");
                sendMessage(arg, raspuns);
                strcpy(query, "UPDATE songs SET votes=votes+1 where id=");
                strcat(query, idSong);
                strcat(query, ";");
                sqlite3_exec(database, query, NULL, NULL, &dbError);
                sqlite3_commit_hook(database, NULL, NULL);
                // strcpy(query, "SELECT name, artist from songs where id=");
                // strcat(query,idSong);
                break;
            }
        }
    }
}

void afisareTopMelodii(void *arg)
{
    int n = 1;
    while (n)
    {
        struct thData tdL;
        tdL = *((struct thData *)arg);
        char query[1000];
        char **result, **result2;
        int rows, columns, rows2, columns2;
        char raspuns[MESSAGE_SIZE];
        char afisareMelodie[MESSAGE_SIZE];
        strcpy(query, "SELECT * from songs order by votes desc");
        strcat(query, ";");
        sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
        sprintf(raspuns, "%d", rows);
        sendMessage(arg, raspuns);
        for (int i = 1; i <= rows; i++)
        {
            char loc[MESSAGE_SIZE];
            sprintf(loc, "%d", i);
            strcpy(afisareMelodie, "Locul: ");
            strcat(afisareMelodie, loc);
            strcat(afisareMelodie, "\n");
            strcat(afisareMelodie, "ID: ");
            strcat(afisareMelodie, result[i * columns]);
            strcat(afisareMelodie, "\n");
            strcat(afisareMelodie, "Nume: ");
            strcat(afisareMelodie, result[i * columns + 1]);
            strcat(afisareMelodie, "\n");
            strcat(afisareMelodie, "Artist: ");
            strcat(afisareMelodie, result[i * columns + 2]);
            strcat(afisareMelodie, "     Votes: ");
            strcat(afisareMelodie, result[i * columns + 3]);
            strcat(afisareMelodie, "\n");
            strcat(afisareMelodie, "Link: ");
            strcat(afisareMelodie, result[i * columns + 4]);
            strcat(afisareMelodie, "\n");
            strcat(afisareMelodie, "Descriere:");
            strcat(afisareMelodie, result[i * columns + 5]);
            strcat(afisareMelodie, "\n");
            strcat(afisareMelodie, "Comentarii(");

            //Cautam id user si comentariu pentru melodia data dupa id
            strcpy(query, "SELECT id_user, comment from comments where id_song='");
            strcat(query, result[i * columns]);
            strcat(query, "';");
            sqlite3_get_table(database, query, &result2, &rows2, &columns2, &dbError);
            if (rows2 < 1)
            {
                strcat(afisareMelodie, "0)");
                strcat(afisareMelodie, "\n");
                strcat(afisareMelodie, "Nu exista comentarii la aceasta melodie.");
                strcat(afisareMelodie, "\n");
            }
            else
            {
                char randuri[MESSAGE_SIZE]; //Pentru salvarea liniilor ce au comentarii
                sprintf(randuri, "%d", rows2);
                strcat(afisareMelodie, randuri);
                strcat(afisareMelodie, "):");
                strcat(afisareMelodie, "\n");

                for (int j = 1; j <= rows2; j++)
                {
                    strcat(afisareMelodie, "Utilizatorul: ");
                    strcpy(query, "SELECT username from accounts where id='");
                    strcat(query, result2[j * columns2]);
                    strcat(query, "';");
                    int rows3, columns3;
                    char **result3;
                    sqlite3_get_table(database, query, &result3, &rows3, &columns3, &dbError);
                    strcat(afisareMelodie, result3[1]);
                    strcat(afisareMelodie, "\n");
                    strcat(afisareMelodie, " A comentat: ");
                    strcat(afisareMelodie, result2[j * columns2 + 1]);
                    strcat(afisareMelodie, "\n\n");
                }
            }

            sendMessage(arg, afisareMelodie);
        }

        int m = 1;
        while (m)
        {
            receiveMessage(arg, raspuns);
            if (strcmp(raspuns, "exit") == 0)
            {
                n = 0;
                break;
            }
            else if (strcmp(raspuns, "comentariu") == 0)
            {
                char id[MESSAGE_SIZE];
                char raspuns[MESSAGE_SIZE];
                char afisareMelodie[MESSAGE_SIZE];
                char comentariu[MESSAGE_SIZE];
                while (1)
                {
                    receiveMessage(arg, id);

                    //Verificam daca id-ul exista sau nu in db
                    strcpy(query, "SELECT name,artist from songs where id='");
                    strcat(query, id);
                    strcat(query, "';");
                    sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
                    if (rows < 1)
                    {
                        strcpy(raspuns, "nu");
                        sendMessage(arg, raspuns);
                        m = 0;
                        break;
                    }
                    else
                    {
                        strcpy(raspuns, "da");
                        sendMessage(arg, raspuns);
                        strcpy(afisareMelodie, "Nume: ");
                        strcat(afisareMelodie, result[2]);
                        strcat(afisareMelodie, "\n");
                        strcat(afisareMelodie, "  Artist: ");
                        strcat(afisareMelodie, result[3]);
                        strcat(afisareMelodie, "\n");
                        strcat(afisareMelodie, "Comentariul dvs:");
                        sendMessage(arg, afisareMelodie);
                        receiveMessage(arg, comentariu);

                        //Cautam id user
                        strcpy(query, "SELECT id from accounts where username ='");
                        strcat(query, numeUser);
                        strcat(query, "';");
                        sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);

                        // Inseram in tabela comments
                        strcpy(query, "INSERT INTO comments (id_song,id_user,comment) values ('");
                        strcat(query, id);
                        strcat(query, "','");
                        strcat(query, result[1]);
                        strcat(query, "','");
                        strcat(query, comentariu);
                        strcat(query, "');");
                        sqlite3_exec(database, query, NULL, NULL, &dbError);
                        sqlite3_commit_hook(database, NULL, NULL);
                        m = 0;
                        break;
                    }
                }
            }
            else
            {
                m = 0;
                break;
            }
        }
    }
}

void afisareDupaGen(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);
    char gen[MESSAGE_SIZE];
    while (1)
    {

        //Verificam daca genul exista in db
        char query[1000];
        string query2;
        char **result;
        int rows, columns;
        int idGen = 0;
        char raspuns[MESSAGE_SIZE];
        char afisareMelodie[MESSAGE_SIZE];
        char genuri[MESSAGE_SIZE];
        strcpy(query, "SELECT name from genre;");
        sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
        strcpy(genuri, result[1]);
        strcat(genuri, ", ");
        for (int i = 2; i <= rows; i++)
        {
            strcat(genuri, result[i * columns]);
            strcat(genuri, ", ");
        }
        sendMessage(arg, genuri);
        receiveMessage(arg, gen);
        strcpy(query, "SELECT * from genre where name='");
        strcat(query, gen);
        strcat(query, "';");
        sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
        if (rows < 1)
        {
            strcpy(raspuns, "nu");
            sendMessage(arg, raspuns);
        }
        else
        {
            strcpy(raspuns, "da");
            sendMessage(arg, raspuns);
            strcpy(query, "SELECT id from genre where name='");
            strcat(query, gen);
            strcat(query, "';");
            sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);

            strcpy(query, "SELECT name,artist,votes,link from songs join songgenres on id=id_song where id_genre=");
            strcat(query, result[1]);
            strcat(query, " order by votes desc;");
            sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
            if (rows < 1) {
                strcpy(raspuns, "nu");
                sendMessage(arg, raspuns);
                break;
            }
            else {
                strcpy(raspuns, "da");
                sendMessage(arg, raspuns);
            //Trimitem numarul de linii catre client
            sprintf(raspuns, "%d", rows);
            sendMessage(arg, raspuns);
            for (int i = 1; i <= rows; i++)
            {
                char loc[MESSAGE_SIZE];
                sprintf(loc, "%d", i);
                strcpy(afisareMelodie, "Locul: ");
                strcat(afisareMelodie, loc);
                strcat(afisareMelodie, "\n");
                strcat(afisareMelodie, "Nume: ");
                strcat(afisareMelodie, result[i * columns]);
                strcat(afisareMelodie, "\n");
                strcat(afisareMelodie, "Artist: ");
                strcat(afisareMelodie, result[i * columns + 1]);
                strcat(afisareMelodie, "     Votes: ");
                strcat(afisareMelodie, result[i * columns + 2]);
                strcat(afisareMelodie, "\n");
                strcat(afisareMelodie, "Link: ");
                strcat(afisareMelodie, result[i * columns + 3]);
                strcat(afisareMelodie, "\n");
                sendMessage(arg, afisareMelodie);
            }

            receiveMessage(arg, raspuns);
            break;
        }
        
        }
    }
}

void adminRequest(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);
    char raspuns[MESSAGE_SIZE];
    char numeComplet[MESSAGE_SIZE];
    char email[MESSAGE_SIZE];
    char query[1000];
    char **result, **result2;
    int rows, columns, rows2, columns2;
    while (1)
    {
        receiveMessage(arg, numeComplet);
        receiveMessage(arg, email);

        //Verificam daca nu cumva userul are deja o cerere
        strcpy(query, "SELECT id from accounts where username='");
        strcat(query, numeUser);
        ;
        strcat(query, "';");
        sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
        strcpy(query, "SELECT id_user from requests where id_user='");
        strcat(query, result[1]);
        strcat(query, "';");
        cout << result[1];
        sqlite3_get_table(database, query, &result2, &rows2, &columns2, &dbError);

        if (rows2 >= 1)
        {

            strcpy(raspuns, "nu");
            sendMessage(arg, raspuns);
            break;
        }
        else
        {
            strcpy(raspuns, "da");
            sendMessage(arg, raspuns);
            strcpy(query, "INSERT INTO requests (id_user,name,email) values (\'");
            strcat(query, result[1]);
            strcat(query, "\',\'");
            strcat(query, numeComplet);
            strcat(query, "\','");
            strcat(query, email);
            strcat(query, "\');");
            sqlite3_exec(database, query, NULL, NULL, &dbError);
            sqlite3_commit_hook(database, NULL, NULL);
            break;
        }
    }
}

//posibilitatea unui admin de a face un alt user admin.
void confirmRequest(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);
    char raspuns[MESSAGE_SIZE];
    char cerere[MESSAGE_SIZE];
    char idUser[MESSAGE_SIZE];
    char query[1000];
    char **result;
    int rows, columns;
    int n = 1;
    while (n)
    {
        strcpy(query, "SELECT * from requests;");
        sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
        sprintf(raspuns, "%d", rows);
        sendMessage(arg, raspuns);
        if (strcmp(raspuns, "0") == 0)
        {
            n = 0;
        }
        else
        {
            for (int i = 1; i <= rows; i++)
            {
                strcpy(cerere, "ID user: ");
                strcat(cerere, result[i * columns]);
                strcat(cerere, "\n");
                strcat(cerere, "Nume: ");
                strcat(cerere, result[i * columns + 1]);
                strcat(cerere, "\n");
                strcat(cerere, "Email: ");
                strcat(cerere, result[i * columns + 2]);
                sendMessage(arg, cerere);
            }
            while (1)
            {
                receiveMessage(arg, raspuns);
                if (strcmp(raspuns, "exit") == 0)
                {
                    n = 0;
                    break;
                }
                else
                {
                    strcpy(idUser, raspuns);
                    cout << idUser;
                    strcpy(query, "SELECT name from requests where id_user='");
                    strcat(query, idUser);
                    strcat(query, "';");
                    sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
                    if (rows < 1)
                    {
                        strcpy(raspuns, "nu");
                        sendMessage(arg, raspuns);
                    }
                    else
                    {
                        strcpy(raspuns, "da");
                        sendMessage(arg, raspuns);
                        strcpy(query, "UPDATE accounts SET id_user = 1");
                        sqlite3_exec(database, query, NULL, NULL, &dbError);
                        sqlite3_commit_hook(database, NULL, NULL);
                        strcpy(query, "DELETE from requests where id_user = '");
                        strcat(query, idUser);
                        strcat(query, "';");
                        sqlite3_exec(database, query, NULL, NULL, &dbError);
                        sqlite3_commit_hook(database, NULL, NULL);
                        sendMessage(arg, result[1]);
                        break;
                    }
                }
            }
        }
    }
}

//posibilitatea unui admin de a bloca un user din a vota.
void voteBan(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);
    char raspuns[MESSAGE_SIZE];
    char useri[MESSAGE_SIZE];
    char query[1000];
    char **result;
    int rows, columns;
    while (1)
    {
        //Afisam toti userii din db care au abilitatea de a vota
        strcpy(query, "SELECT id, username from accounts where can_vote=1 and id_user=0;");
        sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
        sprintf(raspuns, "%d", rows);
        strcpy(useri, "");
        for (int i = 1; i <= rows; i++)
        {
            strcat(useri, "ID: ");
            strcat(useri, result[i * columns]);
            strcat(useri, "\n");
            strcat(useri, "Nume utilizator: ");
            strcat(useri, result[i * columns + 1]);
            strcat(useri, "\n\n");
        }
        sendMessage(arg, useri);
        receiveMessage(arg, raspuns);
        if (strcmp(raspuns, "exit") == 0)
        {
            break;
        }
        else
        {
            strcpy(query, "SELECT id, username from accounts where id=");
            strcat(query, raspuns);
            strcat(query, ";");
            sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
            if (rows < 1)
            {
                strcpy(raspuns, "nu");
                sendMessage(arg, raspuns);
                break;
            }
            else
            {
                strcpy(raspuns, "da");
                sendMessage(arg, raspuns);
                strcpy(query, "UPDATE accounts SET can_vote=0 where id='");
                strcat(query, result[2]);
                strcat(query, "';");
                sendMessage(arg, result[3]);
                sqlite3_exec(database, query, NULL, NULL, &dbError);
                sqlite3_commit_hook(database, NULL, NULL);
                break;
            }
        }
    }
}

//user--cerere de a vota iar
void unbanRequest(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);
    char raspuns[MESSAGE_SIZE];
    char query[1000];
    char **result, **result2;
    int rows, columns, rows2, columns2;
    while (1)
    {
        strcpy(query, "SELECT id,can_vote from accounts where username='");
        strcat(query, numeUser);
        strcat(query, "';");
        sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
        if (strcmp(result[3], "1") == 0)
        {
            //userul poate vota, deci nu poate face cerere
            strcpy(raspuns, "nu");
            sendMessage(arg, raspuns);
            break;
        }
        else
        {
            strcpy(raspuns, "da");
            sendMessage(arg, raspuns);
            strcpy(query, "SELECT * from unbanrequests where name='");
            strcat(query, numeUser);
            strcat(query, "';");
            sqlite3_get_table(database, query, &result2, &rows2, &columns2, &dbError);
            if (rows2 >= 1)
            {
                strcpy(raspuns, "nu");
                sendMessage(arg, raspuns);
                break;
            }
            else
            {
                strcpy(raspuns, "da");
                sendMessage(arg, raspuns);
                strcpy(query, "INSERT into unbanrequests(id_user, name) values ('");
                strcat(query, result[2]);
                strcat(query, "','");
                strcat(query, numeUser);
                strcat(query, "');");
                sqlite3_exec(database, query, NULL, NULL, &dbError);
                sqlite3_commit_hook(database, NULL, NULL);
                break;
            }
        }
    }
}

//admin--aprobare cerere unban
void allowVote(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);
    char raspuns[MESSAGE_SIZE];
    char query[1000];
    char **result;
    int rows, columns;
    char useri[MESSAGE_SIZE];
    while (1)
    {
        strcpy(query, "SELECT * from unbanrequests;");
        sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
        if (rows < 1)
        {
            strcpy(raspuns, "nu");
            sendMessage(arg, raspuns);
            break;
        }
        else
        {
            strcpy(raspuns, "da");
            sendMessage(arg, raspuns);
            sprintf(raspuns, "%d", rows);
            sendMessage(arg, raspuns);
            strcpy(useri, "");
            for (int i = 1; i <= rows; i++)
            {
                strcat(useri, "ID:");
                strcat(useri, result[i * columns]);
                strcat(useri, "  Nume:");
                strcat(useri, result[i * columns + 1]);
                strcat(useri, "\n");
            }
            strcat(useri, "\n");
            sendMessage(arg, useri);
            receiveMessage(arg, raspuns);
            if (strcmp(raspuns, "exit") == 0)
            {
                break;
            }
            else
            {
                strcpy(query, "SELECT id_user from unbanrequests where id_user=");
                strcat(query, raspuns);
                sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
                if (rows < 1)
                {
                    //Daca admin-ul introduce un id invalid se va relua comanda
                    strcpy(raspuns, "nu");
                    sendMessage(arg, raspuns);
                }
                else
                {
                    strcpy(raspuns, "da");
                    sendMessage(arg, raspuns);
                    strcpy(query, "DELETE from unbanrequests where id_user='");
                    strcat(query, result[1]);
                    strcat(query, "';");
                    sqlite3_exec(database, query, NULL, NULL, &dbError);
                    sqlite3_commit_hook(database, NULL, NULL);
                    strcpy(query, "UPDATE accounts SET can_vote=1;");
                    sqlite3_exec(database, query, NULL, NULL, &dbError);
                    sqlite3_commit_hook(database, NULL, NULL);
                    break;
                }
            }
        }
    }
}

void deleteSong(void *arg) {
    struct thData tdL;
    tdL = *((struct thData *)arg);
    char raspuns[MESSAGE_SIZE];
    char query[1000];
    char **result;
    int rows, columns;
    char songs[MESSAGE_SIZE];
    char idSong[MESSAGE_SIZE];

    while(1) {
        cout << "DA\n";
    strcpy(query, "SELECT id, name, artist from songs;");
    sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
    if (rows < 1) {
        strcpy(raspuns, "nu");
        sendMessage(arg, raspuns);
        break;
    }
    else {
        strcpy(raspuns, "da");
        sendMessage(arg, raspuns);
        strcpy(songs, "");
        for (int i=1; i<=rows; i++) {
            strcat(songs, "ID: ");
            strcat(songs, result[i * columns]);
            strcat(songs, "  Nume: ");
            strcat(songs, result[i * columns +1]);
            strcat(songs, "\n");
            strcat(songs, "Artist: ");
            strcat(songs, result[i *columns +2]);
            strcat(songs, "\n");
        }
        strcat(songs, "\n");
        sendMessage(arg, songs);
        receiveMessage(arg, raspuns);
        strcpy(idSong, raspuns);
        strcpy(query, "SELECT id from songs where id='");
        strcat(query, idSong);
        strcat(query, "';");
        sqlite3_get_table(database, query, &result, &rows, &columns, &dbError);
        if (rows <1) {
            strcpy(raspuns, "nu");
            sendMessage(arg, raspuns);
            break;
        }
        else {
            strcpy(raspuns, "da");
            sendMessage(arg, raspuns);
            //Stergem melodia
            strcpy(query, "DELETE from songs WHERE id='");
            strcat(query, idSong);
            strcat(query, "';");
            sqlite3_exec(database, query, NULL, NULL, &dbError);
            sqlite3_commit_hook(database, NULL, NULL);
            //Stergem legatura cu genul
            strcpy(query, "DELETE from songgenres WHERE id_song='");
            strcat(query, idSong);
            strcat(query, "';");
            sqlite3_exec(database, query, NULL, NULL, &dbError);
            sqlite3_commit_hook(database, NULL, NULL);
            //Stergem si comentariile asociate melodiei
            strcpy(query, "DELETE from comments WHERE id_song='");
            strcat(query, idSong);
            strcat(query, "';");
            sqlite3_exec(database, query, NULL, NULL, &dbError);
            sqlite3_commit_hook(database, NULL, NULL);
            break;
        }
    }
    }
}