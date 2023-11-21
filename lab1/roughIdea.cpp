#include <netinet/in.h>
#include <sys/socket.h>
#include <cstdio>
#include <cerrno>



int so;

struct sockaddr_in sa;

void otvori() {
        so = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (so < 0) {
                pogrjeska(1);
        }
}

void pripregni() {
        sa.sin_family = AF_INET;
        sa.sin_port = htons(10000 + znamenka * 10 + i);
        sa.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(so, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
                pogrjeska(2);
        }
}

void primi(int *j) {
        ssize_t vel = sizeof(sa);

        vel = recvfrom(so, &p, sizeof(p), 0, (struct sockaddr *) &sa, (socklen_t *) &vel);
        if (vel < 0) {
                if (errno == EINTR) {
                        return;
                }
                pogrjeska(3);
        }
        if (vel < sizeof(p)) {
                pogrjeska(4);
        }
        *j = ntohs(sa.sin_port) - 10000 - znamenka * 10;
}

void posalji(int j) {
        ssize_t vel;

        sa.sin_port = htons(10000 + znamenka * 10 + j);
        sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        vel = sendto(so, &p, sizeof(p), 0, (struct sockaddr *) &sa, sizeof(sa));
        if (vel < 0) {
                pogrjeska(5);
        }
        if (vel < sizeof(p)) {
                pogrjeska(6);
        }
}


KO() {
        pošalji k svima ostalima Z(i, T(i));
        čini {
                ako (primiš Z(j, T(j) od j) {
                        ažuriraj T(i);
                        spremi Z(j, T(j));
                        pošalji O(i, T(i)) k j;
                } inače ako (primiš I(j, T(j) od j) {
                        obriši Z(j, T(j));
                } inače { // primio si O(j, T(j)) od j
                        ažuriraj T(i);
                }
        } dokle (nisi primio sve O ili ti vlastiti Z ne prethodi svima drugima);
        ... // radi svoj KO
        pošalji k svima ostalima I(i, stari T(i));
}

posao() {
        dokle (istina) {
                dokle (treba u KO) {
                        KO();
                }
                radi pa ako (primiš Z(j, T(j) od j) {
                        ažuriraj T(i);
                        spremi Z(j, T(j));
                        pošalji O(i, T(i)) k j;
                } inače ako (primiš I(j, T(j) od j) {
                        obriši Z(j, T(j));
                }
        }
}

int main(){
    
}