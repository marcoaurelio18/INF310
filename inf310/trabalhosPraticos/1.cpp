#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <stack>
#include <map>

using namespace std;


/* Implementacao das classes de impressoras */

class ImpressoraLaser {
    public:
        ImpressoraLaser(int id) {
            this->estaLivre = true;
            this->_id = id;
        };
        bool estaLivre;
        
    private:
        int _id;
};

class ImpressoraJato {
    public:
        ImpressoraJato(int id) {
            this->estaLivre = true; 
            this->_id = id;
        };
        bool estaLivre;
    
    private:
        int _id;
};

/* ---------------------------------------- */

/* Variaveis globais */

mutex muxLaser;
mutex muxJato;
mutex muxAmbas;

int numImpressoras = 2;
int numThreads = 5;

vector<ImpressoraLaser> impressorasLaser;
vector<ImpressoraJato> impressorasJato;

stack<thread::id> threadsImpressoraLaserBarrada;
stack<thread::id> threadsImpressoraJatoBarrada;
stack<thread::id> threadsAmbasImpressoraBarrada;

map<thread::id, int> A;

/* ----------------- */


/* Implementacao de block e wakeup */

void block(mutex &muxblock) {
    bool sair = false;
    
    thread::id eu = this_thread::get_id();
    
    do {
        muxblock.lock();
        if (A[eu] > 0) {
            A[eu]--;
            sair = true;
        }
        muxblock.unlock();
    } while (!sair);
}

void wakeup(mutex &muxblock, thread::id thread_id) {
    muxblock.lock();
    A[thread_id]++;
    muxblock.unlock();
}

/* ------------------------------- */


/* Funcoes uteis */

/* Funcoes de impressora a laser */

void imprimirALaser(const int impressoraLaserID) {
    impressorasLaser[impressoraLaserID].estaLivre = false;
    muxLaser.lock();

    printf("Imprimindo impressora a laser %d no processo %lu\n", impressoraLaserID, (hash<thread::id>{}(this_thread::get_id())));
    
    this_thread::sleep_for(chrono::milliseconds(1000));
    
    muxLaser.unlock();
    impressorasLaser[impressoraLaserID].estaLivre = true;
}

int retornaImpressoraLaserLivre() {
    for(int i = 0; i < impressorasLaser.size(); i++) 
        if(impressorasLaser[i].estaLivre) return i;
    
    return -1;
}

void utilizarImpressoraLaser() {
    int impressoraLaserID = retornaImpressoraLaserLivre();
    if(impressoraLaserID == -1) {
        threadsImpressoraLaserBarrada.push(this_thread::get_id());
        block(muxLaser);
        impressoraLaserID = retornaImpressoraLaserLivre();
    }
    
    imprimirALaser(impressoraLaserID);
    
    this_thread::sleep_for(std::chrono::milliseconds(100));
    
    if(threadsImpressoraLaserBarrada.size() > 0) {
        thread::id threadBarrada = threadsImpressoraLaserBarrada.top();
        wakeup(muxLaser, threadBarrada);
        threadsImpressoraLaserBarrada.pop();
    }
}

/* ----------------------------- */

/* Funcoes de impressora a jato */

void imprimirAJato(const int impressoraJatoID) {
    impressorasJato[impressoraJatoID].estaLivre = false;
    muxJato.lock();
    
    printf("Imprimindo impressora jato %d no processo %lu\n", impressoraJatoID, (hash<thread::id>{}(this_thread::get_id())));
    
    this_thread::sleep_for(chrono::milliseconds(1000));
    
    muxJato.unlock();
    impressorasJato[impressoraJatoID].estaLivre = true;
}

int retornaImpressoraJatoLivre() {
    for(int i = 0; i < impressorasJato.size(); i++) 
        if(impressorasJato[i].estaLivre) return i;
    
    return -1;
}

void utilizarImpressoraJato() {
    int impressoraJatoID = retornaImpressoraJatoLivre();
    if(impressoraJatoID == -1) {
        threadsImpressoraJatoBarrada.push(this_thread::get_id());
        block(muxJato);
        impressoraJatoID = retornaImpressoraJatoLivre();
    }
    
    this_thread::sleep_for(std::chrono::milliseconds(100));
    
    imprimirAJato(impressoraJatoID);
    
    if(threadsImpressoraJatoBarrada.size() > 0) {
        thread::id threadBarrada = threadsImpressoraJatoBarrada.top();
        wakeup(muxJato, threadBarrada);
        threadsImpressoraJatoBarrada.pop();
    }
}

/* Utiliza Ambas impressoras */

void utilizarImpressoraALaserOuAJato() {
    int impressoraLaserID = retornaImpressoraLaserLivre();
    int impressoraJatoID;
    char tipo = 'L';

    if(impressoraLaserID == -1) {
        tipo = 'J';
        impressoraJatoID = retornaImpressoraJatoLivre();
    }

    if(impressoraLaserID == -1 && impressoraJatoID == -1) {
        threadsAmbasImpressoraBarrada.push(this_thread::get_id());
        block(muxAmbas);

        while(true) {
            impressoraLaserID = retornaImpressoraLaserLivre();
            impressoraJatoID = retornaImpressoraJatoLivre();
            if(impressoraLaserID != -1) {
                tipo = 'L';
                break;
            }
            else if(impressoraJatoID != -1) {
                tipo = 'J';
                break;
            }
        }
    }


    if(tipo == 'L') imprimirALaser(impressoraLaserID);
    else imprimirAJato(impressoraJatoID);
    
    this_thread::sleep_for(std::chrono::milliseconds(1000));

    if(threadsAmbasImpressoraBarrada.size() > 0) {
        thread::id threadBarrada = threadsAmbasImpressoraBarrada.top();
        wakeup(muxAmbas, threadBarrada);
        threadsAmbasImpressoraBarrada.pop();
    }
}

/* ------------------------- */


/* ---------------------------- */

/* ------------- */ 

int main() {
    // inicializa impressoras laser
    for(int i = 0; i < numImpressoras; i++) {
        ImpressoraLaser impressora(i);
        impressorasLaser.push_back(impressora);
    }

    // inicializa impressoras jato
    for(int i = 0; i < numImpressoras; i++) {
        ImpressoraJato impressora(i);
        impressorasJato.push_back(impressora);
    }
    

    // inicializa processos de impressoras laser
    vector<thread> threadsImpressoraLaser;
    // inicializa processos de impressora jato
    vector<thread> threadsImpressoraJato;
    // inicializa processos de ambos tipos
    vector<thread> threadsAmbasImpressoras;
    for (int i = 0; i < numThreads; i++) {
        thread tLaser(utilizarImpressoraLaser);
        A[tLaser.get_id()] = 0;
        threadsImpressoraLaser.push_back(move(tLaser));

        thread tJato(utilizarImpressoraJato);
        A[tJato.get_id()] = 0;
        threadsImpressoraJato.push_back(move(tJato));

        thread tAmbas(utilizarImpressoraALaserOuAJato);
        A[tAmbas.get_id()] = 0;
        threadsAmbasImpressoras.push_back(move(tAmbas));
    }


    // finaliza threads 
    for (int i = 0; i < numThreads; ++i) {
        threadsImpressoraLaser[i].join();
        threadsImpressoraJato[i].join();
        threadsAmbasImpressoras[i].join();
    }
    
    return 0;
}