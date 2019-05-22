# TopMusic

1.Packages you need to install:
Make sure you have installed the g++ compiler and makefile.
If you don't have these use:
sudo apt-get install g++ and 
sudo apt-get install make 
to install them.

You also need to install sqlite3. You can use:
sudo apt-get install sqlite3 libsqlite3-dev

2.To compile you only need to use 'make' command and it will do all.

3.How to use it.
First you need to run the server using ./server in terminal.
Then you can connect with an client using ./client .

Client:
All input is key sensitive.
After you entered one command the program will ask you for certain messages.

First menu is Main menu. You can use register to create an accout, login to start the aplication functionability and exit to quit the application.
After you successfully logged in you can use the application.

If you are an admin the following menu will display:
-Adauga melodie (command: adauga): you can add any song you want to the top. The application will ask for certain details about the song and after you gave them the song will be added.
-Afisare Top (command: afisareTop): will display you the current top order by votes. Here you can add comments for a certain song.
-Voteaza o melodie (command: voteaza):all the songs will be displayed and you can vote any song if you have permission. If you don't have permission to vote the application will display a message to warn you. 
-Afisare dupa gen (command: afisareDupaGen): will ask you to choose one genree and then will display the top for that genree, order by votes.
-Cereri de admin (command: cereri): the application will display all the users who made a request to be admin. you can choose which user will be admin by typing his/her ID.
-Restrictie vot (command: restrict): you can restrict one user's right to vote by typing his/her ID.
-Permite vot (command: allow): you can allow a user to vote if his vote right was previously restricted. You can do that by typing his ID.
-Stergere melodie(command: stergere): this command will delete the song you want by typing the ID.
-Delogare (command: delogare): exit this menu and sign out. you will go back to Main menu.

If you are an user the following menu will display:
-Adauga melodie (command: adauga): you can add any song you want to the top. The application will ask for certain details about the song and after you gave them the song will be added.
-Afisare Top (command: afisareTop): will display you the current top order by votes. Here you can add comments for a certain song.
-Voteaza o melodie (command: voteaza):all the songs will be displayed and you can vote any song if you have permission. If you don't have permission to vote the application will display a message to warn you.
-Afisare dupa gen (command: afisareDupaGen): will ask you to choose one genree and then will display the top for that genree, order by votes.
 -Cerere de admin (command: request): you can choose to be an admin if you want. You need to complete a form and an admin will review it. 
-Permite vot (command: unban): you can make a request to vote again if this right was taken from you by an admin. You need to complete a form and an admin will review it.
-Delogare (command: delogare): exit this menu and sign out. you will go back to Main menu.

4.Bibliography:
Server base implementation:
https://profs.info.uaic.ro/~computernetworks/files/NetEx/S12/ServerConcThread/servTcpConcTh2.c
Client base implementation:
https://profs.info.uaic.ro/~computernetworks/files/NetEx/S12/ServerConcThread/cliTcpNr.c
Sqlite3 help:
https://www.sqlite.org/cintro.html
https://www.sqlite.org/capi3ref.html
https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm