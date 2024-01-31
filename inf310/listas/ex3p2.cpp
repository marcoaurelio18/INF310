#include <iostream>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>

using namespace std;
int cont = 0;

class MonitorFumante {
    public:
        void servir(int ingrediente1, int ingrediente2) {
            printf("%d", ingrediente1);
            printf(" %d\n", ingrediente2);

            ++n;                
            
            if((ingrediente1 + ingrediente2) == 5) fumante1TemTodos.notify_one();
            else if((ingrediente1 + ingrediente2) == 4) fumante2TemTodos.notify_one();
            else if((ingrediente1 + ingrediente2) == 3) fumante3TemTodos.notify_one();
        }

        void fumar(int numIngrediente) {
            mux.lock();

            if(numIngrediente == 1 && n<10) fumante1TemTodos.wait(mux);
            else if(numIngrediente == 2) fumante2TemTodos.wait(mux);
            else if(numIngrediente == 3) fumante3TemTodos.wait(mux);

            mux.unlock();
            if(n==10)
                return false;
            else
                printf("O fumante %d tem todos ingredientes e subiu fumaca\n", numIngrediente);
                return true;

        }
    private:
        int n=0;
        mutex mux;
        condition_variable_any fumante1TemTodos, fumante2TemTodos, fumante3TemTodos;
};

MonitorFumante fumante;

void funcaoFornecedor() {
    while(cont < 10) {
        int ingrediente1 = rand() % 3 + 1;
        int ingrediente2 = ingrediente1 % 3 + 1;
        fumante.servir(ingrediente1, ingrediente2);

        this_thread::sleep_for(chrono::seconds(1));     
        cont++;
    }

}

void funcaoUsuario(int temIngrediente) {
    while(cont < 10) {
        fumante.fumar(temIngrediente);
    }
}

int main() {
    thread fornecedor(funcaoFornecedor);

    vector<thread> usuarios;
    for(int i = 0; i < 3; i++) {
        thread usuario(funcaoUsuario, (i+1));
        usuarios.push_back(move(usuario));
    }

    fornecedor.join();
    for(int i = 0; i < 3; i++) usuarios[i].join();

    return 0;
}