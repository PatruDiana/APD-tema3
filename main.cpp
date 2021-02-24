#include <iostream>
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <thread>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cctype>
#include <tuple>
#include <mutex>
#define MASTER 0
#define WORKERHORROR 1
#define WORKERCOMEDY 2
#define WORKERFANTASY 3
#define WORKERSF 4
#define HORROR "horror"
#define COMEDY "comedy"
#define FANTASY "fantasy"
#define SF "science-fiction"
std::vector<std::string> datafromthreads(100000, "");
int nr_line;
std::string modifyparagraphSF(std::string s) {
    // noul string ce va fi returnat
    std::string str = {s[0]};
    int nrofwords = 1;
    for (int i = 1; i < s.size(); i++) {
        // se reseteaza numarul de cuvinte cand trecem pe linie noua
        if(s[i - 1] == '\n'){
            nrofwords = 1;
        }
        // creste numarul de cuvinte de pe linie
        if (s[i - 1] == ' ') {
            nrofwords++;
        }
        // daca numarul e multiplu de 7
        if (nrofwords % 7 == 0) {
            std::string word = "";
            nrofwords = 0;
            // retinem cuvantul
            while(s[i] != '\n' && s[i] != ' ') {
                word += s[i++];
            }
            // il inversam
            reverse(word.begin(), word.end());
            // il adaugam in stringul ce va fi returnat
            str += word;
        }
        str += s[i];
    }
    return str;
}
std::string modifyparagraphFANTASY(std::string s) {
    // noul string ce va fi returnat
    std::string str = "";
    str += toupper(s[0]);
    for (int i = 1; i < s.size(); i++) {
        // daca caracterul este o litera
        if (isalpha(s[i])) {
            // si e primul pe rand
            if (s[i - 1] == '\n') {
                str += toupper(s[i++]);
            }
            // sau e primul dintr-un nou cuvant
            if (s[i - 1] == ' ') {
                str += toupper(s[i++]);
            }
        }
        str += s[i];
    }
    return str;
}
std::string modifyparagraphCOMEDY(std::string s) {
    // noul string ce va fi returnat
    std::string str = "";
    // pozitia caracterelor in cuvantul din care fac parte
    // numerotarea lor incepand de la 0
    int position = 0;
    // daca litera a fost sau nu modificata
    bool modified = false;
    for (int i = 0; i < s.size(); i++) {
        // urmeaza a incepe un nou cuvant
        if (s[i] == ' ') {
            position = -1;
        }
        // urmeaza a incepe un nou cuvant
        if (s[i] == '\n') {
            position = -1;
        }
        // daca se afla pe pozitie impara
        if (position % 2 == 1) {
            if (isalpha(s[i])) {
                modified = true;
            }
        }
        // daca caracterul nu e pe pozitie impara
        if (!modified) {
            str += s[i];
            // daca litera e pe pozitie impara
        } else {
            str += toupper(s[i]);
            modified = false;
        }
        position++;
    }
    return str;
}
std::string modifyparagraphHORROR(std::string s){
    // noul string ce va fi returnat
    std::string str = "";
    for (int i = 0; i < s.size(); i++) {
        str += s[i];
        // daca e liter
        if (isalpha(s[i])) {
            // si nu e vocala
            if (s[i] != 'a' && s[i] != 'e' && s[i] != 'i' && s[i] != 'o' && s[i] != 'u' &&
                s[i] != 'A' && s[i] != 'E' && s[i] != 'I' && s[i] != 'O' && s[i] != 'U') {
                // o transforma in majuscula
                str += tolower(s[i]);
            }
        }
    }
    return str;
}
void readfromfile(std::string arg, std::string type, int destination) {
    MPI_Status status;
    int len;
    std::string data, line;
    std::ifstream indata;
    indata.open(arg);
    int tag = -1;
    // daca fisierul de intrare nu poate fi deschis
    if(!indata) {
        std::cout << "Error: file could not be opened" << std::endl;
        exit(0);
    }
    // cat timp nu am ajuns la sfarsitul fisierului
    while (!indata.eof()) {
        data.clear();
        line.clear();
        // cat timp putem citii cate o linie
        while (std::getline(indata, line)) {
            // daca linia curenta este unul titlul unui paragraf
            if (line.compare(0, 6, HORROR) == 0 || line.compare(0, 6, COMEDY) == 0 ||
                    line.compare(0, 7, FANTASY) == 0 || line.compare(0, 15, SF) == 0) {
                // maresc numarul de paragrafe existente in fisierului de intrare
                tag++;
                // daca paragraful ce urmeaza a fi citit este destinat a fi citit de thread-ul corespunzator
                if(line.compare(0, type.length(), type) == 0) {
                    break;
                }
            }
        }
        // daca a ajuns la sfarsitul fisierului
        if(line.compare(0, type.length(), type) != 0) {
            break;
        }
        std::getline(indata, line);
        data = line;
        // cat timp citesc inca din paragraful curent
        while (std::getline(indata, line) && (line.length() != 0)) {
            data = data + "\n" + line;
        }
        data = data + "\n";
        // trimit catre workerul corespondent, paragraful spre prelucrare
        MPI_Send(data.c_str() ,data.length() + 1, MPI_CHAR, destination, tag, MPI_COMM_WORLD);
        MPI_Probe(destination, tag, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_CHAR, &len);
        char buf[len];
        // primesc de la worker paragraful prelucrar
        MPI_Recv(&buf, len, MPI_CHAR, destination, tag, MPI_COMM_WORLD, &status);
        std::string s = buf;
        // pun in vectorul cu string-uri modificatre pe linia numarului de paragrafe existente
        // pentru a pastra ordinea de scriere la output
        datafromthreads[status.MPI_TAG] = s;
    }
    indata.close();
    data.clear();
    data = "exit";
    // trimit un mesaj de exit pentru a anunta worker-ul ca nu mai are de prelucrat nici un paragraf
    MPI_Send(data.c_str() ,data.length() + 1, MPI_CHAR, destination, 0, MPI_COMM_WORLD);
    nr_line = tag;
}
int main (int argc, char *argv[]) {
    int  numtasks, rank, provided, len;
    MPI_Status status;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    // daca este master
    if (rank == MASTER) {
        // porneste cate un thread pentru fiecare gen pentru citirea in paralel
        std::thread horror(readfromfile, argv[1], HORROR, WORKERHORROR);
        std::thread comedy(readfromfile, argv[1], COMEDY, WORKERCOMEDY);
        std::thread fantasy(readfromfile, argv[1], FANTASY, WORKERFANTASY);
        std::thread sciencefiction(readfromfile, argv[1], SF, WORKERSF);
        // asteapta sincronizarea thread-urilor
        horror.join();
        comedy.join();
        fantasy.join();
        sciencefiction.join();
        // crearea fisierului de iesire
        std::ofstream myfile;
        std::string input = argv[1];
        input[input.length() - 3] = 'o';
        input[input.length() - 2] = 'u';
        input[input.length() - 1] = 't';
        std::string output = input;
        myfile.open(output);
        // daca fisierul de iesire nu poate fi deschis
        if(!myfile) {
            std::cout << "Error: file could not be opened" << std::endl;
            exit(0);
        }
        // scrie in fisierul de iesire textul modificat
        for (int i = 0; i <= nr_line; i++) {
            myfile << datafromthreads[i]  << "\n" ;
        }
    }
    // daca procesul se ocupa de modificarea paragrafelor HORROR
    if (rank ==  WORKERHORROR) {
        while(1) {
            MPI_Probe(MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_CHAR, &len);
            char buf[len];
            // primeste un paragraf de la master
            MPI_Recv(&buf, len, MPI_CHAR, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            std::string s = buf;
            // daca mesajul este exit, inseamna ca nu mai exista sir de prelucrat
            if (s == "exit") {
                break;
            }
            // modifica paragraful
            std::string str = modifyparagraphHORROR(s);
            s.clear();
            // adauga titul
            s = "horror\n";
            str = s + str;
            // trimite catre master paragraful modificat
            MPI_Send(str.c_str() ,str.length() + 1, MPI_CHAR, MASTER, status.MPI_TAG, MPI_COMM_WORLD);
        }
        // daca procesul se ocupa de modificarea paragrafelor COMEDY
    } else if (rank == WORKERCOMEDY) {
        while(1) {
            MPI_Probe(MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_CHAR, &len);
            char buf[len];
            // primeste un paragraf de la master
            MPI_Recv(&buf, len, MPI_CHAR, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            std::string s = buf;
            // daca mesajul este exit, inseamna ca nu mai exista sir de prelucrat
            if (s == "exit") {
                break;
            }
            // modifica paragraful
            std::string str = modifyparagraphCOMEDY(s);
            s.clear();
            // adauga titul
            s = "comedy\n";
            str = s + str;
            // trimite catre master paragraful modificat
            MPI_Send(str.c_str() ,str.length() + 1, MPI_CHAR, MASTER, status.MPI_TAG, MPI_COMM_WORLD);
        }
        // daca procesul se ocupa de modificarea paragrafelor FANTASY
    } else if (rank == WORKERFANTASY) {
        while(1) {
            MPI_Probe(MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_CHAR, &len);
            char buf[len];
            // primeste un paragraf de la master
            MPI_Recv(&buf, len, MPI_CHAR, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            std::string s = buf;
            // daca mesajul este exit, inseamna ca nu mai exista sir de prelucrat
            if (s == "exit") {
                break;
            }
            // modifica paragraful
            std::string str = modifyparagraphFANTASY(s);
            s.clear();
            // adauga titul
            s = "fantasy\n";
            str = s + str;
            // trimite catre master paragraful modificat
            MPI_Send(str.c_str() ,str.length() + 1, MPI_CHAR, MASTER, status.MPI_TAG, MPI_COMM_WORLD);
        }
        // daca procesul se ocupa de modificarea paragrafelor SF
    } else if (rank == WORKERSF) {
        while(1) {
            MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_CHAR, &len);
            char buf[len];
            // primeste un paragraf de la master
            MPI_Recv(&buf, len, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            std::string s = buf;
            // daca mesajul este exit, inseamna ca nu mai exista sir de prelucrat
            if (s == "exit") {
                break;
            }
            // modifica paragraful
            std::string str = modifyparagraphSF(s);
            s.clear();
            // adauga titul
            s = "science-fiction\n";
            str = s + str;
            // trimite catre master paragraful modificat
            MPI_Send(str.c_str() ,str.length() + 1, MPI_CHAR, MASTER, status.MPI_TAG, MPI_COMM_WORLD);
        }
    }
    MPI_Finalize();
    return 0;
}