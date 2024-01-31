#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

using namespace std;

int tamBuffer = 10;
int n = 100;

class MonitorProdutorConsumidor {
    public:
        MonitorProdutorConsumidor() {
            for(int i = 0; i < tamBuffer; i++) buffer.push_back(-1);
        }

        int consumir() {
            mux.lock();
            
            if(buffer[posicaoRemover] == -1) bufferVazio.wait(mux);
            
            int item = buffer[posicaoRemover];
            buffer[posicaoRemover] = -1;
            bufferCheio.notify_one();
            
            if(posicaoRemover == (tamBuffer - 1)) posicaoRemover = 0;
            else posicaoRemover++;

            mux.unlock();

            return item;
        }
        
        void produzir(int itemAProduzir) {
            mux.lock();

            buffer[posicaoInserir] = itemAProduzir;
            bufferVazio.notify_one();
            
            if(posicaoInserir == (tamBuffer - 1)) posicaoInserir = 0;
            else posicaoInserir++;
            
            int proximaPosicao = posicaoInserir;
            if(buffer[proximaPosicao] != -1) bufferCheio.wait(mux);

            mux.unlock();
        }

    private:
        int posicaoInserir = 0, posicaoRemover = 0;
        vector<int> buffer;
        mutex mux;
        condition_variable_any bufferCheio, bufferVazio;
};

MonitorProdutorConsumidor produtorConsumidor;

void funcaoThreadProdutor() {
    for(int i = 0; i < n; i++) produtorConsumidor.produzir(i);
}

void funcaoThreadConsumidor() {
    int item;
    for (int i = 0; i < n; i++) {
        item = produtorConsumidor.consumir();
        printf("consumido o produto %d\n", item);
    }
}

int main() {
    thread threadProdutor(funcaoThreadProdutor);
    thread threadConsumidor(funcaoThreadConsumidor);

    threadConsumidor.join();
    threadProdutor.join();

    return 0;
}