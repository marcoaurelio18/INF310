#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <semaphore.h>

using namespace std;

int quantidadeDeJovens = 10;
bool possuiCerveja = true, possuiRefrigerante = true; 
int quantidadeCerveja = 6, quantidadeRefrigerante = 10;
int cont = 0;

sem_t mux;
sem_t jovemServido, precisaMaisBebida;

void funcaoBalconista() {
    sem_wait(&precisaMaisBebida);
    if(!possuiRefrigerante) {
        quantidadeRefrigerante += 10;
        possuiRefrigerante = true;
    }
    else if(!possuiCerveja) {
        quantidadeCerveja += 6;
        possuiCerveja = true;
    }
    sem_post(&jovemServido);
}

void funcaoJovem(int id, int tipoDoJovem) {
    while(cont < 20){
        sem_wait(&mux);
        if(tipoDoJovem == 1) {
            if(!possuiRefrigerante) {
                sem_post(&precisaMaisBebida);
                sem_wait(&jovemServido);
            } else {
                quantidadeRefrigerante--;
                if(quantidadeRefrigerante == 0) possuiRefrigerante = false;
            }
        }
        else if(tipoDoJovem == 2) {
            if(!possuiCerveja) {
                if(!possuiRefrigerante) {
                    sem_post(&precisaMaisBebida);
                    sem_wait(&jovemServido);
                } else {
                    quantidadeRefrigerante--;
                    if(quantidadeRefrigerante == 0) possuiRefrigerante = false;
                }
            }
            else {
                quantidadeCerveja--;
                if(quantidadeCerveja == 0) possuiCerveja = false;
            }
        }
        cont++;
        sem_post(&mux);
    }
}

int main() {
    sem_init(&mux, 0, 1);
    sem_init(&jovemServido, 0, 0);
    sem_init(&precisaMaisBebida, 0, 0);

    thread tBalconista(funcaoBalconista);

    vector<thread> jovens;
    int tipoDoJovem;
    for(int i = 0; i < quantidadeDeJovens; i++) {
        tipoDoJovem = rand()%2 + 1; // 1 refrizeiro, 2 cervejeiro
        thread tJovem(funcaoJovem, i, tipoDoJovem);
        jovens.push_back(move(tJovem));
    }

    for(int i = 0; i < quantidadeDeJovens; i++) jovens[i].join();

    tBalconista.join();

    return 0;
}