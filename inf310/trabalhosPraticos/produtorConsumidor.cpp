/*******************************************************************************
 * Problema do produtor/consumidor com mais de um produtor/consumidor e buffer 
 * limitado utilizando monitor (variáveis condição). 
 * Uso do unique_lock é mais seguro que mux.lock(). Quando o objeto lck é 
 * destruído no final do procedimento (ou em uma falha), o unlock do mutex é 
 * chamado, evitando travamento.
 * Outra novidade é o uso de um parâmetro extra no wait de cada condição para
 * indicar, através de uma função, se a thread pode ser acordada (substitui o
 * loop externo ao wait).
 */
#include<iostream>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<unistd.h>
#include<random>
using namespace std;

class MonitorProdCons {
private:
    int bSize,pin,pout,cont;
    int *buffer;
    mutex mux;
    condition_variable naoVazio,naoCheio; //exige uso do unique_lock

    void printBuffer(){
        cout<<"Buffer = [\t";
        for(int i =0; i<bSize; ++i){
            cout<<buffer[i]<<"\t";
        }
        cout<<"] : "<<cont<<endl;
    }

public:
    MonitorProdCons(int bufferSize){
        bSize=bufferSize;
        buffer = new int[bSize];        
        pin=pout=cont=0;
    }

    ~MonitorProdCons(){
        delete [] buffer;
    }

    void produzir(int msg) {
        unique_lock<mutex> lck(mux); //lck libera mux ao ser destruído (mais seguro)
        naoCheio.wait(lck,[this](){ return cont<bSize; });
        // while (cont==bSize)       //trecho equivalente à linha acima
        //     naoCheio.wait(lck);
        buffer[pin]=msg;
        pin=(pin+1)%bSize;
        ++cont;
        printBuffer();
        if(cont>bSize) cout<<"buffer overrun"<<endl;
        naoVazio.notify_one(); //um consumidor será tirado da fila (se houver) a cada iteração
    }

    void consumir(int &msg) {
        unique_lock<mutex> lck(mux);
        // naoVazio.wait(lck,[this](){ return cont>0; });
        while (cont==0)
            naoVazio.wait(lck);
        msg=buffer[pout];
        buffer[pout]=-1; //limpar o valor consumido (facilitar a visualização do printBuffer)
        pout=(pout+1)%bSize;
        --cont;
        printBuffer();
        if(cont<0) cout<<"buffer underrun"<<endl;
        naoCheio.notify_one();
    }
};

MonitorProdCons prodcons(10);

void produtor(int n) {
    for(int i=0; i<n; i++) {
        prodcons.produzir(i);
        // usleep(10000);  //produção mais lenta que o consumo para ver buffer esvaziando
    }
}

void consumidor(int n){
    // sleep(5);           //dormir para ver o produtor encher o buffer e bloquear
    for(int i=0; i<n; i++){
        prodcons.consumir(i);
        // usleep(1000);
    }
}

int main() {
    srand(time(NULL));
    int n=20;
    thread p(produtor,n);
    thread c(consumidor,n);
    p.join();
    c.join();
    return 0;
}