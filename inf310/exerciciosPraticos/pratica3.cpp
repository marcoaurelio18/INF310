#include <semaphore.h>
#include <iomanip>
#include <thread>
#include <vector>
#include <map>
#include <stack>

using namespace std;

int tamPopulacao = 20;

vector<int> vacinasProduzidas;
stack<int> vacinaTipoUm, vacinaTipoDois, vacinaTipoTres;

int vacinasTipoUmProduzidas = 0, vacinasTipoDoisProduzidas = 0, vacinasTipoTresProduzidas = 0;
int vacinasTipoUmConsumidas = 0, vacinasTipoDoisConsumidas = 0, vacinasTipoTresConsumidas = 0;

sem_t vacinaQualquerTipoDisponivel;
sem_t vacinaTipoUmDisponivel, vacinaTipoDoisDisponivel, vacinaTipoTresDisponivel;
sem_t muxPrimeiraDose, muxSegundaDose;

void produzirSegundaDose(int tipoVacina) {
    if(tipoVacina == 1) {
        vacinaTipoUm.push(tipoVacina);
        sem_post(&vacinaTipoUmDisponivel);
    }
    else if(tipoVacina == 2) {
        vacinaTipoDois.push(tipoVacina);
        sem_post(&vacinaTipoDoisDisponivel);
    }
    else if(tipoVacina == 3) {
        vacinaTipoTres.push(tipoVacina);
        sem_post(&vacinaTipoTresDisponivel);
    }

    printf("O produtor produziu uma dose da vacina %d\n", tipoVacina);
}

int produzirPrimeiraDose() {
    int tipoVacinaProduzido = rand()% 3 + 1;
    if(tipoVacinaProduzido == 1) {
        vacinasTipoUmProduzidas++;
        vacinaTipoUm.push(tipoVacinaProduzido);
    }
    else if(tipoVacinaProduzido == 2) {
        vacinasTipoDoisProduzidas++;
        vacinaTipoDois.push(tipoVacinaProduzido);
    }
    else if(tipoVacinaProduzido == 3) {
        vacinasTipoTresProduzidas++;
        vacinaTipoTres.push(tipoVacinaProduzido);
    }
    
    printf("O produtor produziu uma dose da vacina %d\n", tipoVacinaProduzido);

    sem_post(&vacinaQualquerTipoDisponivel);
    return tipoVacinaProduzido;
}

void funcaoProdutor() {
    for(int i = 0; i < tamPopulacao; i++) {
        int tipoVacina = produzirPrimeiraDose();
        vacinasProduzidas.push_back(tipoVacina);
    }

    for(auto &vacina:vacinasProduzidas) {
        produzirSegundaDose(vacina);
    }
}


void consumirSegundaDose(int id, int tipoVacina) {
    sem_wait(&muxSegundaDose);
    if(tipoVacina == 1) {
        sem_wait(&vacinaTipoUmDisponivel);
        vacinaTipoUm.pop();
    }
    else if(tipoVacina == 2) {
        sem_wait(&vacinaTipoDoisDisponivel);
        vacinaTipoDois.pop();
    }
    else if(tipoVacina == 3) {
        sem_wait(&vacinaTipoTresDisponivel);
        vacinaTipoTres.pop();
    }

    printf("O consumidor %d consumiu a segunda dose da vacina %d\n", id, tipoVacina);
    sem_post(&muxSegundaDose);
}

int consumirPrimeiraDose(int id) {
    sem_wait(&muxPrimeiraDose);
    sem_wait(&vacinaQualquerTipoDisponivel);

    int tipoVacinaCosumido;
    if(!vacinaTipoUm.empty() && (vacinasTipoUmConsumidas < vacinasTipoUmProduzidas)) {
        vacinasTipoUmConsumidas++;
        tipoVacinaCosumido = vacinaTipoUm.top();
        vacinaTipoUm.pop();
    }
    else if(!vacinaTipoDois.empty() && (vacinasTipoDoisConsumidas < vacinasTipoDoisProduzidas)) {
        vacinasTipoDoisConsumidas++;
        tipoVacinaCosumido = vacinaTipoDois.top();
        vacinaTipoDois.pop();
    }
    else if(!vacinaTipoTres.empty() && (vacinasTipoTresConsumidas < vacinasTipoTresProduzidas)) {
        vacinasTipoTresConsumidas++;
        tipoVacinaCosumido = vacinaTipoTres.top();
        vacinaTipoTres.pop();
    }

    printf("O consumidor %d consumiu a primeira dose da vacina %d\n", id, tipoVacinaCosumido);
    sem_post(&muxPrimeiraDose);

    return tipoVacinaCosumido;
}

void funcaoConsumidor(int id) {
    this_thread::sleep_for(chrono::seconds(3));
    int tipoVacina = consumirPrimeiraDose(id);
    this_thread::sleep_for(chrono::seconds(3));
    consumirSegundaDose(id, tipoVacina);
}


int main() {
    sem_init(&vacinaQualquerTipoDisponivel, 0, 0);
    sem_init(&vacinaTipoUmDisponivel, 0, 0);
    sem_init(&vacinaTipoDoisDisponivel, 0, 0);
    sem_init(&vacinaTipoTresDisponivel, 0, 0);

    sem_init(&muxPrimeiraDose, 0, 1);
    sem_init(&muxSegundaDose, 0, 1);

    thread produtor(funcaoProdutor);
    
    vector<thread> consumidores;
    for(int i = 0; i < tamPopulacao; i++) {
        thread consumidor(funcaoConsumidor, i);
        consumidores.push_back(move(consumidor));
    }

    for(int i = 0; i < tamPopulacao; i++) consumidores[i].join();
    produtor.join();

    return 0;
}