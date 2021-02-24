# APD-tema3
Tema 3 – Algoritmi paraleli in sisteme distribuite (MPI)

Am implementat tema in C++. In cele ce urmeaza, am sa explic logica si functionalitatea solutiei propuse:
* procesul cu rank = 0 va reprezenta master-ul si se va ocupa cu citirea paragrafelor din fisierul de intrare care este primit in linia de comanda si accesat prin "argv[1]" si va scrie la final in fisierul de output paragrafele modificate;
* procesul cu rank = 1 va reprezenta workerul ce va modifica ca in fiecare paragraf de tip horror sa aiba consoanele dublate in fiecare cuvant;
* procesul cu rank = 2 va reprezenta workerul ce va modifica ca in fiecare paragraf de tip comedy sa aiba fiecare litera de pe pozitie para facuta majuscula;
* procesul cu rank = 3 va reprezenta workerul ce va modifica ca in fiecare paragraf de tip fantasy sa aiba prima litera a fiecarui cuvant facuta majuscula;
* procesul cu rank = 4 va reprezenta workerul ce va modifica ca in fiecare paragraf de tip science-fiction sa aiba fiecare al 7 lea cuvant de pe linie inversat;
* citirea paragrafelor din fisierul de intrare este paralelizata, existand cate un thread initiat de master ce va citii doar paragrafe de un singur tip;
* ordinea paragrafelor pentru a putea avea ordinea corecta in fisierul de iesire este realizata prin existenta a cate unei variabile ce va contoriza numarul de paragrafe pentru fiecare thread, urmand ca salvarea unui string primit de la un worker sa fie salvat intr-un vector de string-uri pe pozitia pe pe care o are ca paragraf in fisierului de intrare (numerotarea paragrafelor incepand de la 0);
* in ceea ce priveste workerii, acestia fac primesc paragrafe intr-o bucla infinita deoarece nu stiu exact cate paragrafe vor modifica, pana la primirea unui text "exit" ce ii anunta ca nu mai exista paragrafe de modificat;
* dupa terminarea thread-urilor din master, se va deschide fisierul de iesire prin inlocuirea extensiei .txt in .out si se va parcurge vectorul de string-uri ce a fost populat de cele 4 thread-uri si se va scrie in fisierul de iesire paragrafele modificate.
