/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

using namespace std;

#define MESSAGE_SIZE 10000
#define POZITIV "da"
#define NEGATIV "nu"

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;
int sd; // descriptorul de socket

char numeUser[MESSAGE_SIZE]; // pastrare nume pentru aflare id user.

void readCommand(char msg[MESSAGE_SIZE]);
int sendMessage(char msg[MESSAGE_SIZE]);
int receiveMessage(char msg[MESSAGE_SIZE]);
void mainMenu();
void registerAccount();
void loginAccount();
void afterLoginMenu();
void addSong();
void votMelodie();
void afisareTopMelodii();
void afisareDupaGen();
void adminRequest();
void confirmRequest();
void unbanRequest();
void voteBan();
void voteUnban();
void allowVote();
void deleteSong();

int main(int argc, char *argv[])
{

    struct sockaddr_in server; // structura folosita pentru conectare
                               // mesajul trimis
    int nr = 0;

    /* stabilim portul */
    port = 2908;

    /* cream socketul */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare la socket().\n");
        return errno;
    }

    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    /* familia socket-ului */
    server.sin_family = AF_INET;
    /* adresa IP a serverului */
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    /* portul de conectare */
    server.sin_port = htons(port);

    /* ne conectam la server */
    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[client]Eroare la connect().\n");
        return errno;
    }

    mainMenu();

    //printf("[client] Am citit %s\n", buf);

    /* trimiterea mesajului la server */
    //sendMessage(buf);

    /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */
    //receiveMessage(buf);
    /* afisam mesajul primit */
    //printf("[client]Mesajul primit este: %s\n", buf);

    /* inchidem conexiunea, am terminat */
    close(sd);
}

void readCommand(char msg[MESSAGE_SIZE])
{
    fflush(stdout);
    int dim = read(0, msg, MESSAGE_SIZE);
    msg[dim - 1] = '\0';
}

int sendMessage(char msg[MESSAGE_SIZE])
{
    if (write(sd, msg, MESSAGE_SIZE) <= 0)
    {
        perror("[client]Eroare la write() spre server.\n");
        return errno;
    }
}

int receiveMessage(char msg[MESSAGE_SIZE])
{
    if (read(sd, msg, MESSAGE_SIZE) < 0)
    {
        perror("[client]Eroare la read() de la server.\n");
        return errno;
    }
}

void mainMenu()
{

    while (1)
    {
        printf("\033c");
        cout << "Welcome!\n Register sau Login?\n";
        cout << "\n exit pentru iesire.\n\n";
        cout << "Raspunsul dvs:";
        char buf[MESSAGE_SIZE];
        readCommand(buf);
        if (strcmp(buf, "register") == 0)
        {
            sendMessage(buf);
            registerAccount();
        }
        else if (strcmp(buf, "login") == 0)
        {
            sendMessage(buf);
            loginAccount();
            
        }
        else if (strcmp(buf, "exit") == 0)
        {
            sendMessage(buf);
            break;
        }
    }
}

void registerAccount()
{
    printf("\033c");
    char username[MESSAGE_SIZE];
    cout << "Register\n";

    while (1)
    {
        cout << "Username:";
        readCommand(username);
        sendMessage(username);
        char verificare[MESSAGE_SIZE];
        receiveMessage(verificare);
        if (strcmp(verificare, NEGATIV) == 0)
        {
            cout << "Username deja existent! Incercati altul\n";
        }
        else
            break;
    }

    char password[MESSAGE_SIZE];
    cout << "Password:";
    readCommand(password);
    sendMessage(password);
    cout << "inregistreare cu succes\n";
    sleep(2);
}

void loginAccount()
{
    printf("\033c");
    char username[MESSAGE_SIZE];
    char password[MESSAGE_SIZE];
    cout << "Login\n";

    while (1)
    {
        cout << "Username:\n";
        readCommand(username);
        sendMessage(username);
        char verificare[MESSAGE_SIZE];
        receiveMessage(verificare);
        if (strcmp(verificare, NEGATIV) == 0)
        {
            cout << "Numele nu exista in baza de date. Creati un cont nou?\n";
            cout << "Raspuns (da/nu):";
            char raspuns[MESSAGE_SIZE];
            readCommand(raspuns);
            sendMessage(raspuns);
            if (strcmp(raspuns, POZITIV) == 0)
            {
                registerAccount();
                break;
            }
        }
        else
        {
            while (1)
            {
                cout << "Password:\n";
                readCommand(password);
                sendMessage(password);
                char verificare[MESSAGE_SIZE];
                receiveMessage(verificare);
                if (strcmp(verificare, NEGATIV) == 0)
                {
                    cout << "Parola gresita! Mai incearca\n";
                }
                else
                {
                    cout << "Logat cu succes!\n";
                    strcpy(numeUser, username); //salvat global nume user
                    sendMessage(numeUser);
                    afterLoginMenu();
                    break;
                }
            }
            break;
        }
    }
}

void afterLoginMenu()
{
    char raspuns[MESSAGE_SIZE];
    receiveMessage(raspuns);
    while (1)
    {

        if (strcmp(raspuns, "user") == 0)
        {
            //Afisam meniul pentru user
            printf("\033c");
            cout << "Meniu pentru user:\n\n";
            cout << "Adauga o noua melodie.(adauga)\n";
            cout << "Afiseaza melodiile dupa voturi.(afisareTop)\n";
            cout << "Voteaza o melodie.(voteaza)\n";
            cout << "Afiseaza melodiile dupa un gen. (afisareDupaGen)\n";
            cout << "Vrei sa fii admin? Fa request! (request)\n";
            cout << "Vrei sa faci cerere unban pentru voturi?(unban)\n";
            cout << "Delogare.(delogare)\n\n";
            cout << "Raspuns:";
            char comanda[MESSAGE_SIZE];
            readCommand(comanda);
            if (strcmp(comanda, "adauga") == 0)
            {
                sendMessage(comanda);
                addSong();
            }
            else if (strcmp(comanda, "afisareTop") == 0)
            {
                sendMessage(comanda);
                afisareTopMelodii();
            }
            else if (strcmp(comanda, "voteaza") == 0)
            {
                sendMessage(comanda);
                votMelodie();
            }
            else if (strcmp(comanda, "afisareDupaGen") == 0)
            {
                sendMessage(comanda);
                afisareDupaGen();
            }
            else if (strcmp(comanda, "delogare") == 0)
            {
                sendMessage(comanda);
                
                break;
            }
            else if (strcmp(comanda, "request") == 0)
            {
                sendMessage(comanda);
                adminRequest();
            }
            else if (strcmp(comanda, "unban") == 0)
            {
                sendMessage(comanda);
                unbanRequest();
            }
        }
        else if (strcmp(raspuns, "admin") == 0)
        {
            //Afisam meniul pentru admin
            printf("\033c");
            cout << "Meniu pentru admin:\n\n";
            cout << "Adauga o noua melodie.(adauga)\n";
            cout << "Afiseaza melodiile dupa voturi.(afisareTop)\n";
            cout << "Voteaza o melodie.(voteaza)\n";
            cout << "Afiseaza melodiile dupa un gen. (afisareDupaGen)\n";
            cout << "Afiseaza cererile de admin (cereri)\n";
            cout << "Restrictioneaza abilitatea unui user de a vota (restrict)\n";
            cout << "Permite unui user sa voteze (allow)\n";
            cout << "Pentru stergea unei melodii(stergere)\n";
            cout << "Delogare.(delogare)\n\n";
            cout << "Raspuns:";
            char comanda[MESSAGE_SIZE];
            readCommand(comanda);
            if (strcmp(comanda, "adauga") == 0)
            {
                sendMessage(comanda);
                addSong();
            }
            else if (strcmp(comanda, "afisareTop") == 0)
            {
                sendMessage(comanda);
                afisareTopMelodii();
            }
            else if (strcmp(comanda, "voteaza") == 0)
            {
                sendMessage(comanda);
                votMelodie();
            }
            else if (strcmp(comanda, "afisareDupaGen") == 0)
            {
                sendMessage(comanda);
                afisareDupaGen();
            }
            else if (strcmp(comanda, "cereri") == 0)
            {
                sendMessage(comanda);
                confirmRequest();
            }
            else if (strcmp(comanda, "restrict") == 0)
            {
                sendMessage(comanda);
                voteBan();
            }
            else if (strcmp(comanda, "allow") == 0)
            {
                sendMessage(comanda);
                allowVote();
            }
            else if (strcmp(comanda, "stergere") == 0) {
                sendMessage(comanda);
                deleteSong();
            }
            else if (strcmp(comanda, "delogare") == 0)
            {   
                sendMessage(comanda);
                break;
            }
        }
    }
}

void addSong()
{

    char text[MESSAGE_SIZE];
    char raspuns[MESSAGE_SIZE];
    int m = 1;
    int n = 1;
    while (m)
    {
        printf("\033c");
        cout << "Adaugare melodie\n";
        while (n)
        {
            cout << "Dati nume melodie:";
            readCommand(text);
            sendMessage(text);
            cout << "Dati artistul:";
            readCommand(text);
            sendMessage(text);
            cout << "Dati genul melodiei (daca sunt mai multe separati prin '/'):";
            readCommand(text);
            sendMessage(text);
            cout << "Dati link catre melodie:";
            readCommand(text);
            sendMessage(text);
            cout << "Dati o descriere:";
            readCommand(text);
            sendMessage(text);

            receiveMessage(raspuns);
            if (strcmp(raspuns, NEGATIV) == 0)
            {
                cout << "Melodia exista deja in baza de date!\n";
                cout << "Adaugati alta melodie?(da/nu)\n";
                readCommand(raspuns);
                sendMessage(raspuns);
                if (strcmp(raspuns, NEGATIV) == 0)
                {
                    m = 0;
                    n = 0;
                }
            }
            else
            {
                m = 0;
                receiveMessage(raspuns);
                if (strcmp(raspuns, POZITIV) == 0)
                {
                    cout << "S-a adaugat cu succes!\n";
                    sleep(2);
                    n = 0;
                    break;
                }
                else
                {
                    cout << "Something went wrong!\n";
                    sleep(2);
                    n = 0;
                    break;
                }
                n = 0;
                break;
            }
        }
    }
}

void votMelodie()
{
    while (1)
    {
        printf("\033c");
        char nr[MESSAGE_SIZE];
        int nrMelodii = 0;
        char afisareMelodie[MESSAGE_SIZE];
        char raspuns[MESSAGE_SIZE];
        receiveMessage(raspuns);
        if (strcmp(raspuns, NEGATIV) == 0)
        {
            cout << "Nu ai permisiunea de a vota!\n";
            cout << "Poti face cerere de unban in meniul principal. \n";
            sleep(3);
            break;
        }
        else
        {
            receiveMessage(nr);
            sscanf(nr, "%d", &nrMelodii);
            cout << "Melodii ordonate dupa voturi: ";
            cout << nrMelodii;
            cout << "\n\n";
            for (int i = 1; i <= nrMelodii; i++)
            {
                receiveMessage(afisareMelodie);
                cout << afisareMelodie;
                cout << "\n";
            }
            cout << "Votati o melodie dupa ID.\n";
            cout << "Dati id-ul: ";
            readCommand(nr);
            sendMessage(nr);
            receiveMessage(nr);
            if (strcmp(nr, NEGATIV) == 0)
            {
                cout << "Id-ul nu exista!\n";
                cout << "Va rugam alegeti un ID de mai sus!\n";
                sleep(2);
            }
            else if (strcmp(nr, POZITIV) == 0)
            {

                cout << "S-a votat cu succes! :)\n";
                sleep(2);
                break;
            }
        }
    }
}

void afisareTopMelodii()
{
    int m = 1;
    while (m)
    {
        printf("\033c");
        char nr[MESSAGE_SIZE];
        int nrMelodii = 0;
        char afisareMelodie[MESSAGE_SIZE];
        receiveMessage(nr);
        sscanf(nr, "%d", &nrMelodii);
        cout << "Melodii ordonate dupa voturi: ";
        cout << nrMelodii;
        cout << "\n";
        for (int i = 1; i <= nrMelodii; i++)
        {
            receiveMessage(afisareMelodie);
            cout << afisareMelodie;
            cout << "\n\n";
        }
        cout << "Tastati exit pentru iesire.\n";
        cout << "Tastati comentariu pentru a adauga un comentariu la o melodie!\n";
        cout << "Raspunsul dvs:";
        int n = 1;
        while (n)
        {
            readCommand(nr);
            sendMessage(nr);
            if (strcmp(nr, "exit") == 0)
            {

                m = 0;
                break;
            }
            else if (strcmp(nr, "comentariu") == 0)
            {
                while (1)
                {

                    cout << "Adaugare comentariu!\n";
                    cout << "Tastati ID-ul melodiei pentru a adauga comentariu:";
                    char id[MESSAGE_SIZE];
                    char raspuns[MESSAGE_SIZE];
                    char comentariu[MESSAGE_SIZE];
                    readCommand(id);
                    sendMessage(id);
                    receiveMessage(raspuns);
                    if (strcmp(raspuns, NEGATIV) == 0)
                    {
                        cout << "ID-ul nu exista in baza de date!\n";
                        cout << "Dati un ID de mai sus!\n";
                        sleep(2);
                        n = 0;
                        break;
                    }
                    else
                    {
                        receiveMessage(afisareMelodie);
                        cout << afisareMelodie;
                        readCommand(comentariu);
                        sendMessage(comentariu);

                        cout << "Comentariu adaugat cu succes!\n";
                        sleep(2);
                        n = 0;
                        break;
                    }
                }
            }
            else
            {
                cout << "Aceasta comanda nu exista!\n";
                sleep(2);
                n = 0;
                break;
            }
        }
    }
}

void afisareDupaGen()
{
    char raspuns[MESSAGE_SIZE];
    char gen[MESSAGE_SIZE];
    int nrMelodii = 0;
    char afisareMelodie[MESSAGE_SIZE];
    char genuri[MESSAGE_SIZE];

    while (1)
    {
        printf("\033c");
        cout << "Aveti de ales din urmatoarele genuri muzicale: ";
        receiveMessage(genuri);
        cout << genuri;
        cout << "\n";
        cout << "Dati genul:";
        readCommand(gen);
        sendMessage(gen);
        receiveMessage(raspuns);
        if (strcmp(raspuns, "nu") == 0)
        {
            cout << "Genul nu exista in baza de date!\n";
            cout << "Incercati altul!\n";
            sleep(2);
        }
        else
        {   
            receiveMessage(raspuns);
            if (strcmp(raspuns, NEGATIV) == 0) {
                cout << "Nu exista melodii pentru acest gen.\n";
                cout << "Puteti adauga o melodie pentru acest gen din meniul principal.\n";
                sleep(3);
                break;
            }
            else {
            cout << "Ordonare dupa genul: \n";
            cout << gen;
            cout << "\n\n";
            //Primim numarul de melodii dupa genul dat
            receiveMessage(raspuns);
            sscanf(raspuns, "%d", &nrMelodii);
            for (int i = 1; i <= nrMelodii; i++)
            {
                receiveMessage(afisareMelodie);
                cout << afisareMelodie;
                cout << "\n";
            }
            cout << "\n Press any key to exit.";
            readCommand(raspuns);
            sendMessage(raspuns);
            break;
        }
        
        
        }
    }
}

//Un user face cerere de a deveni admin
void adminRequest()
{
    char raspuns[MESSAGE_SIZE];
    char numeComplet[MESSAGE_SIZE];
    char email[MESSAGE_SIZE];
    while (1)
    {
        printf("\033c");
        cout << "Cerere admin:\n";
        cout << "Nume complet:";
        readCommand(numeComplet);
        sendMessage(numeComplet);
        cout << "Adresa de email:";
        readCommand(email);
        sendMessage(email);
        receiveMessage(raspuns);
        if (strcmp(raspuns, NEGATIV) == 0)
        {
            cout << "Aveti deja o cerere! Va rugam asteptati ca un admin sa o verifice!\n";
            sleep(3);
            break;
        }
        else if (strcmp(raspuns, POZITIV) == 0)
        {
            cout << "Cererea s-a realizat cu succes!\n";
            cout << "Un admin o va verifica in curand!\n";
            sleep(3);
            break;
        }
    }
}

//posibilitatea unui admin de a face un alt user admin.
void confirmRequest()
{
    char raspuns[MESSAGE_SIZE];
    int nrCereri = 0;
    char cerere[MESSAGE_SIZE];
    int n = 1;
    while (n)
    {
        printf("\033c");
        receiveMessage(raspuns);
        sscanf(raspuns, "%d", &nrCereri);
        if (nrCereri == 0)
        {
            cout << "Momentan nu aveti nici o cerere!\n";
            cout << "Redirectionare catre meniu principal...\n";
            sleep(4);
            n = 0;
        }
        else
        {
            cout << "Aveti un total de ";
            cout << nrCereri;
            cout << " cereri!\n\n";
            for (int i = 1; i <= nrCereri; i++)
            {
                receiveMessage(cerere);
                cout << cerere;
                cout << "\n";
            }
            while (1)
            {
                cout << "\n";
                cout << "exit pentru iesire\n";
                cout << "Scrieti id-ul utilizatorului caruia ii aprobati cererea.\n";
                cout << "Raspunsul dvs: ";
                readCommand(raspuns);
                if (strcmp(raspuns, "exit") == 0)
                {
                    sendMessage(raspuns);
                    n = 0;
                    break;
                }
                else
                {
                    sendMessage(raspuns);
                    receiveMessage(raspuns);
                    if (strcmp(raspuns, NEGATIV) == 0)
                    {
                        cout << "ID-ul este invalid!\n";
                        cout << "Introduceti un id de mai sus!\n";
                    }
                    else if (strcmp(raspuns, POZITIV) == 0)
                    {
                        cout << "Utilizatorul ";
                        //Numele utilizatorului
                        receiveMessage(raspuns);
                        cout << raspuns;
                        cout << " a fost facut admin!\n";
                        sleep(4);
                        break;
                    }
                }
            }
        }
    }
}

//posibilitatea unui admin de a bloca un user din a vota.
void voteBan()
{
    char raspuns[MESSAGE_SIZE];
    char useri[MESSAGE_SIZE];
    while (1)
    {
        printf("\033c");
        cout << "Puteti restrictiona abilitatea de vot a urmatorilor utlizatori: \n\n";
        //Afisam toti userii din db care au abilitatea de a vota
        receiveMessage(useri);
        cout << useri;
        cout << "\n";
        cout << "Tastati ID-ul utilizatorului care va primi vote ban.\n";
        cout << "exit pentru iesire.\n";
        cout << "Raspunsul dvs:";
        readCommand(raspuns);
        sendMessage(raspuns);
        if (strcmp(raspuns, "exit") == 0)
        {
            break;
        }
        else
        {
            receiveMessage(raspuns);
            if (strcmp(raspuns, NEGATIV) == 0)
            {
                cout << "ID invalid!";
                cout << "Redirectionare la meniul principal...\n";
                sleep(3);
                break;
            }
            else
            {

                cout << "Utilizatorul ";
                receiveMessage(raspuns);
                cout << raspuns;
                cout << " nu mai poate vota!\n";
                cout << "Redirectionare la meniul principal...\n";
                sleep(3);
                break;
            }
        }
    }
}

//user--cerere de a vota iar
void unbanRequest()
{
    char raspuns[MESSAGE_SIZE];
    while (1)
    {
        printf("\033c");
        cout << "Cerere unban votes!\n\n";
        receiveMessage(raspuns);
        if (strcmp(raspuns, NEGATIV) == 0)
        {
            cout << "Deja poti vota! Nu poti face cerere unban!\n";
            cout << "Redirectionare catre meniul principal...\n";
            sleep(3);
            break;
        }
        else
        {
            receiveMessage(raspuns);
            if (strcmp(raspuns, NEGATIV) == 0)
            {
                cout << "Aveti deja o cerere in curs!\n";
                cout << "Asteptati ca un admin sa o verifice!\n";
                cout << "Redirectionare catre meniul principal...\n";
                sleep(4);
                break;
            }
            else
            {
                cout << "Cererea s-a realizat cu succes!\n";
                cout << "Un admin o va verifica in curand!\n";
                cout << "Redirectionare catre meniul principal...\n";
                sleep(4);
                break;
            }
        }
    }
}

//admin--aprobare cerere unban
void allowVote()
{
    char raspuns[MESSAGE_SIZE];
    char useri[MESSAGE_SIZE];
    while (1)
    {
        printf("\033c");
        receiveMessage(raspuns);
        if (strcmp(raspuns, NEGATIV) == 0)
        {
            cout << "Nu aveti cereri de unban!\n";
            cout << "Redirectionare catre meniul principal...\n";
            sleep(3);
            break;
        }
        else
        {
            receiveMessage(raspuns);
            cout << "Aveti ";
            cout << raspuns;
            cout << " cereri!\n\n";

            //Afisam toti userii (ID+nume)
            receiveMessage(useri);
            cout << useri;
            cout << "\n";
            cout << "Dati ID-ul utilizatorului ce va primi unban.\n";
            cout << "exit pentru iesire\n";
            cout << "Raspunsul dvs:";
            readCommand(raspuns);
            sendMessage(raspuns);
            if (strcmp(raspuns, "exit") == 0)
            {
                break;
            }
            else
            {
                receiveMessage(raspuns);
                //Daca admin-ul introduce un id invalid se va relua comanda
                if (strcmp(raspuns, NEGATIV) == 0)
                {
                    cout << "ID invalid\n";
                    cout << "Introduceti un ID de mai sus!\n";
                    sleep(3);
                }
                else
                {
                    cout << "Utilizatorul va putea acum sa voteze!\n";
                    cout << "Redirectionare catre meniul principal...\n";
                    sleep(3);
                    break;
                }
            }
        }
    }
}

void deleteSong() {
    char raspuns[MESSAGE_SIZE];
    char songs[MESSAGE_SIZE];
    while(1) {
    cout << "Stergere melodie\n\n";
    receiveMessage(raspuns);
    if (strcmp(raspuns, NEGATIV) == 0) {
        cout << "Nu sunt melodii in baza de date!\n";
        cout << "Redirectionare catre meniul principal\n;";
        sleep(3);
        break;
    }
    else {
        receiveMessage(songs);
        cout << songs;
        cout << "Dati id-ul melodiei pe care vreti sa o stergeti\n";
        cout << "exit pentru iesire\n";
        cout << "Raspunsul dvs:";
        readCommand(raspuns);
        sendMessage(raspuns);
        receiveMessage(raspuns);
        if (strcmp(raspuns, NEGATIV) == 0) {
            cout << "ID invalid!!!\n";
            cout << "Redirectionare catre meniul principal...\n";
            sleep(3);
            break;
        }
        else {
            cout << "Melodia a fost stearsa cu succes!\n";
            cout << "Redirectionare catre meniul principal...\n";
            sleep(3);
            break;
        }
    }   
    }
}
