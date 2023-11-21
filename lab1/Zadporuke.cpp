#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <list>
#include <sys/sem.h>
#include <string.h>
#include <semaphore.h>
#include <cstdlib>
#include <sys/wait.h>
#include <signal.h>
#include <sys/shm.h>
#include <unistd.h>
#include <ctime>
#include <iterator>
#include <string>
using namespace std;

int so;
int brojProcesa = 1;
int znamenka = 1;
struct sockaddr_in sa;

int procesID;
int t;
list<int> koVrijeme;

struct poruka
{
    char tip;
    int proces;
    int t;
};
poruka p;
list<poruka> spremljenePoruke;


void otvori() 
{
    so = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

void pripregni() 
{
    sa.sin_family = AF_INET;
    sa.sin_port = htons(10000 + znamenka * 10 + procesID);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(so, (struct sockaddr *) &sa, sizeof(sa));
}

void primi(int *j) 
{
    ssize_t vel = sizeof(sa);
    vel = recvfrom(so, &p, sizeof(p), 0, (struct sockaddr *) &sa, (socklen_t *) &vel);
    if (vel < 0) 
    {
        if (errno == EINTR) 
        {
            return;
        }
    }
    *j = ntohs(sa.sin_port) - 10000 - znamenka * 10;
}

void posalji(int j) 
{
    ssize_t vel;
    sa.sin_port = htons(10000 + znamenka * 10 + j);
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    vel = sendto(so, &p, sizeof(p), 0, (struct sockaddr *) &sa, sizeof(sa));
}

void slanje(char tip, int j)
{
    p.proces = procesID;
    p.t = t;
    p.tip = tip;
    cout<<"P"<<procesID<<" poslao "<<p.tip<<"("<<p.proces<<", "<<p.t<<") k P"<<j<<endl;
    posalji(j);
}
void primanje(int *j)
{
    //interval cekanja
    fd_set readfds;
    struct timeval timer;
    FD_ZERO(&readfds);
    FD_SET(so, &readfds);
    timer.tv_sec = 0;
    timer.tv_usec = 10;
    select(so+1, &readfds, NULL, NULL, &timer);
    if(!FD_ISSET(so, &readfds)) return;

    primi(j);
    cout<<"P"<<procesID<<" primio "<<p.tip<<"("<<p.proces<<", "<<p.t<<") od P"<<*j<<endl;
}
bool sortiraj(const poruka &a, const poruka &b)
{
    if (a.t<b.t) return true;
    if (a.t>b.t) return false;
    if (a.t == b.t && a.proces < b.proces) return true;
    return false;
}
void spremiPoruku(poruka por)
{
    spremljenePoruke.push_back(por);
    spremljenePoruke.sort(sortiraj);
}
void posaljiSvima(char tip)
{
    for (int j=1; j<brojProcesa; j++)
    {
        if (j!=procesID)
        {
            slanje(tip,j);
        }
    }
    if (tip == 'Z')
    {
        spremiPoruku(p);
    }
}
void azurirajT(int noviT)
{
    if (t > noviT)
        t++;
    else
        t = noviT + 1;
    cout<<"T("<<procesID<<")="<<t<<endl;
}
void obrisiPoruku(int j)
{
    list<poruka>::iterator it = spremljenePoruke.begin();

    for (list<poruka>::iterator i=spremljenePoruke.begin(); i!=spremljenePoruke.end(); i++)
    {
       if ((*i).proces == j)
       {
            it = i;
            break;
       }
    }
    spremljenePoruke.erase(it);
}

void KO() 
{
    koVrijeme.pop_front();
    int primljeniO = 0;
    posaljiSvima('Z'); //pošalji k svima ostalima Z(i, T(i));

    do {
        int j = 0;
        primanje(&j);
        if (j != 0 && p.tip=='Z') /*primiš Z(j, T(j) od j*/
        {
            azurirajT(p.t); //ažuriraj T(i);
            spremiPoruku(p); //spremi Z(j, T(j));
            slanje('O', p.proces); //pošalji O(i, T(i)) k j;
        } 
        else if (j != 0 && p.tip=='I') /*primiš I(j, T(j) od j*/
        {
            obrisiPoruku(j); //obriši Z(j, T(j));
        } 
        else if (j != 0 && p.tip=='O') // primio si O(j, T(j)) od j
        { 
            primljeniO++;
            azurirajT(p.t); //ažuriraj T(i);
        }
    } while (primljeniO < brojProcesa - 2 || spremljenePoruke.front().proces != procesID); //nisi primio sve O ili ti vlastiti Z ne prethodi svima drugima
    cout<<"P"<<procesID<<" usao u KO >>>>>>>>>>>>>>>>>>>>>>>>"<<endl;
    sleep(3); // radi svoj KO
    cout<<"P"<<procesID<<" izasao iz KO !!!!!!!!!!!!!!!!!!!!!!!!!!1"<<endl;
    posaljiSvima('I'); //pošalji k svima ostalima I(i, stari T(i));
}

void posao() 
{
    sleep(2);
    while (true) 
    {
        if (!koVrijeme.empty())
            while (koVrijeme.front() <= t) 
            {
                KO();
                if (koVrijeme.empty()) break;
            }
        cout<<"Dogadaj("<<procesID<<")"<<endl;
        sleep(1);
        azurirajT(t+1);
        
        int j = 0;
        primanje(&j);
        if (j != 0 && p.tip=='Z') //
        {
            azurirajT(p.t); //ažuriraj T(i);
            spremiPoruku(p); //spremi Z(j, T(j));
            slanje('O', p.proces); //pošalji O(i, T(i)) k j;
        } 
        else if (j != 0 && p.tip=='I') 
        {
            obrisiPoruku(j); //obriši Z(j, T(j));
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Premalo argumenata!" << endl;
        return 0;
    }
    int i = 1;
    list<int> proces;
    for (; i < argc; i++)
    {
        if (*argv[i] == '@')
        {
            i++;
            break;
        }
        else
        {
            proces.push_back(stoi(argv[i]));
            brojProcesa++;
        }
    }
    list<int> vr;
    int id = 1;
    for (; i < argc; i++)
    {
        if (*argv[i] == '@')
        {
            if (fork() == 0)
            {

                procesID=id;
                t = proces.front();
                koVrijeme = vr;
                otvori();
                pripregni();
                posao();
                exit(0);
            }
            proces.pop_front();
            id++;
            vr.clear();
        }
        else
        {
            vr.push_back(atoi(argv[i]));
            //dodaj procesu vremena
        }
    }
    while (proces.empty() == 0)
    {
        int pr = id, vrm = proces.front();
        proces.pop_front();
        id++;
        if (fork() == 0)
        {
            procesID=pr;
            t = vrm;
            koVrijeme = vr;
            otvori();
            pripregni();
            posao();
            exit(0);
        }
    }
    for (i = 1; i < brojProcesa; i++)
    {
        wait(NULL);
    }
    return 0;
}