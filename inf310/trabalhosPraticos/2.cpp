#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

const int NUM_TOTAL_CLIENTES = 25;
const int NUM_CADEIRAS_DE_ESPERA = 5;

bool todosClientesUsaramABarbearia = false;
int numClientesUsaramABarbearia = 0;

class MonitorBarbearia {
    public:
        void clienteChega(const int clienteId) {
            unique_lock<mutex> lock(mux);
            
            numClientesUsaramABarbearia++;
            
            if(this->numCadeirasDeEsperaOcupadas == NUM_CADEIRAS_DE_ESPERA) {
                printf("O cliente %d chegou e nao haviam lugares livres, ele teve que ir embora :(\n", clienteId);
                return;
            }

            this->filaDeClientes.push(clienteId);
            this->numCadeirasDeEsperaOcupadas++;
            
            printf("O cliente %d chegou e sentou na fila de esepra\n", clienteId);
            if(this->filaDeClientes.size() == 1) barbeiroEsperando.notify_one();  

        }

        void barbeiroFazCorte() {
            unique_lock<mutex> lock(mux);

            if(numClientesUsaramABarbearia == NUM_TOTAL_CLIENTES && this->filaDeClientes.empty()) {
                todosClientesUsaramABarbearia = true;
                return;
            }
            
            if(this->filaDeClientes.empty()) {
                printf("O barbeiro esta dormindo... zZz\n");
                barbeiroEsperando.wait(lock);
            }
            
            this->cadeiraDoBarbeiroOcupada = true;

            int cliente = this->filaDeClientes.front();
            this->filaDeClientes.pop();
	    this->numCadeirasDeEsperaOcupadas--;

            printf("O barbeiro esta cortando o cabelo do cliente %d!\n", cliente);

            this->cadeiraDoBarbeiroOcupada = false;
        }

    private:
        queue<int> filaDeClientes;
        int numCadeirasDeEsperaOcupadas;
        bool cadeiraDoBarbeiroOcupada = false;
        mutex mux;
        condition_variable_any barbeiroEsperando;
};

MonitorBarbearia barbearia;

void cliente() {
    int threadId = (hash<thread::id>{}(this_thread::get_id()));
    barbearia.clienteChega(threadId);
}

void barbeiro() {
    while(!todosClientesUsaramABarbearia) {
        barbearia.barbeiroFazCorte();
    }
}


int main() {    
    vector<thread> clientes;
    thread threadBarbeiro(barbeiro);
    
    for(int i = 0; i < NUM_TOTAL_CLIENTES; i++) {
        thread threadCliente(cliente);
        clientes.push_back(move(threadCliente));
    }
    
    for(int i = 0; i < clientes.size(); i++) {
        clientes[i].join();
    }
    threadBarbeiro.join();


    return 0;
}
