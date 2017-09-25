#include <stdio.h>  
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <deque>
#include <cstdlib>


using namespace std;

const int MAX_SIZE = 10;
const int MAX_DIFF = 10;
int t1, t2, t3;
int quit = 0;
int pausing = 0;
deque<char> buffer, output, optimes;
deque<int> count;
char products[3] = {'A', 'B', 'C'};
int tools, operators, recipiescount;
char rules[3];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int cycle = 1;

void pausefunction() {
    while(pausing == 1) {
        sleep(1);
    }
}

void * gen_thread(void * data) {
    char product = *((char*) data);
    cout << "Generator created with product: " << product << endl;
  
    while(true) {
        if(product == products[0]) {
            sleep(t1);
        }
        else if(product == products[1]) {
            sleep(t2);
        }
        else {
            sleep(t3);
        }

        pausefunction();

        if(buffer.size() > MAX_SIZE) {
            cout << "Buffer is full generator going to sleep for 5 seconds" << endl;
            sleep(5);
        }
        else {
            pthread_mutex_lock(&mutex);
            buffer.push_back(product);
            pthread_mutex_unlock(&mutex);

            if(buffer.size() == 0) {
                cout << "Updated buffer is empty";
            } else {
                cout << "Updated buffer is: |";
                int count;
                for(count = 0; count <= buffer.size() - 1; count++) {
                    cout << buffer.at(count) << "|";
                } 
            }
        cout << endl;
        }
    }
}

void * producer_thread(void * data) {
    int position = *((int *)data);
    int counter = 0;
    int diffrence = 0;

    char materialA = rules[0];
    char materialB = rules[1];
    char product = rules[2];
    cout << "Created operator with rule " << materialA 
    << " + " << materialB  << " = " << product << endl;

    int prodtime = 0;
    if(!optimes.empty()) {
        prodtime = optimes.at(0);
        optimes.pop_front();
    }

    while(true) {
        sleep(1);

        if(buffer.size() >= 2 && tools >= 2) {
            char fst = buffer.at(0);
            char snd = buffer.at(1);
            if((materialA == fst && materialB == snd) ||
                (materialB == fst && materialA == snd)) {

                int total = count.at(position);
                int diffcount = 0;
                for(int i = 0; i < count.size(); i++) {
                    diffcount = total - count.at(i);
                    if(i != position && diffcount > 10) {
                        diffrence = 1;
                    } else {
                        diffrence = 0;
                    }
                }

                if(diffrence == 0) {
                    tools -= 2;
                    cout << "The amount of unused tools is: ";
                    cout << tools;
                    cout << endl;

                    sleep(prodtime);

                    pausefunction();

                    //cout << buffer.size() << endl;
                    pthread_mutex_lock(&mutex);
                    buffer.pop_front();
                    buffer.pop_front();
                    output.push_back(product);
                    pthread_mutex_unlock(&mutex);

                    counter = count.at(position);
                    counter++;
                    count.at(position) = counter;

                    cout << "Producing " << product 
                    << " from " << materialA << " and " << materialB << endl;
                    if(output.size() > 0 || !output.empty()) {
                        cout << "Output buffer is: |";
                        int count;
                        for(count = 0; count < output.size(); count++) {
                        cout << output.at(count) << "|";
                        }
                    } else {
                        cout << "Output buffer is empty";
                    }
            
                    cout << endl;

                    tools += 2;
                    cout << "Tools are released" << endl;
                }
            }
        }
        else
            sleep(1);
    }
}

void * keyboard_thread(void * arguments) {
    cout << "Input thread initialized" << endl;
    char input;

    while(true) {
        sleep(1);
        cin >> input;
        if(pausing == 0 && input == 'p') {
          cout << "Paused" << endl;
          pausing = 1;
          input = 'q';
        }
        if(pausing == 1 && input == 'p') {
          cout << "Unpaused" << endl;
          pausing = 0;
          input = 'q';
        }
    }

}

void generator(char *recipies) {
    int i = 0;
    for(i = 0; i <= 2; i++) {
        pthread_t g[i];
        pthread_create(&g[i], NULL, gen_thread, (void *) &products[i]);
        //pthread_join(g[i], NULL);
    }

    int j = 0;
    for(j = 0; j < operators; j++) {
        rules[0] = recipies[j*3];
        rules[1] = recipies[j*3 + 1];
        rules[2] = recipies[j*3 + 2];

        int *number = (int *)malloc(sizeof(*number));
        *number = j;
        count.push_front(0);

        pthread_t p[j];
        pthread_create(&p[j], NULL, producer_thread, (void *) number);
        sleep(1);
        //pthread_join(p[i], NULL);
    }
    
    pthread_t k1;
    pthread_create(&k1, NULL, keyboard_thread, NULL);
}


int main() {
    char input;

    cout << "Welcome to the generator" << endl; 
    cout << endl;
    cout << "Type in q for exit and p for pausing and p again to unpausing" << endl;
    cout << endl;
    cout << "Would you like to use a predefined preset (3 operators)? Y/N: ";
    cin >> input;
    cout << endl;
    if(input == 'N') {
        cout << "How many tools "; 
        cin >> tools;
        cout << "What is the delay in the generator 1: ";
        cin >> t1;
        cout << "What is the delay in the generator 2: ";
        cin >> t2;
        cout << "What is the delay in the generator 3: ";
        cin >> t3;;
        cout << "How many operators: ";
        cin >> operators;
        cout << endl;

        char recipies[3 * operators];
        recipiescount = 3 * operators;
        int i;
        int number;
        char input;

        for(i = 0; i < operators; i ++) {
            cout << "Please enter material one: ";
            cin >> input;
            recipies[i*3] = input;
            cout << "Please enter material two: ";
            cin >> input;
            recipies[i*3 + 1] = input;
            cout << "Please enter product of material one and two: ";
            cin >> input;
            recipies[i*3 + 2] = input;
            cout << "Please enter product time for operator: ";
            cin >> input;
            number = input - '0';
            optimes.push_back(number);
            cout << endl;
        }

        cout << "Recipies are as follows:" << endl;
        for(i = 0; i < operators; i++) {
            cout << recipies[i*3] << " + " << recipies[i*3 + 1] 
            << " = " << recipies[i*3 + 2] << endl;
        }
        cout << endl;

        generator(recipies);
        while(true) {
            sleep(1);
        }

    } else {
        char recipies[9];
        recipiescount = 9;

        cout << "Predefined preset chosen. It has 2 tools with" << endl;
        cout << "generators delays 4,6,8 seconds. With the following" << endl;
        cout << "rules A + B = D with delay 1, B + C = E with delay 3," << endl;
        cout << "A + C = F with delay 5." << endl;
        cout << endl;

        tools = 4;
        t1 = 4;
        t2 = 6;
        t3 = 8;
        operators = 3;
        recipies[0] = 'A';
        recipies[1] = 'B';
        recipies[2] = 'D';
        recipies[3] = 'B';
        recipies[4] = 'C';
        recipies[5] = 'E';
        recipies[6] = 'A';
        recipies[7] = 'C';
        recipies[8] = 'F';
        optimes.push_back(1);
        optimes.push_back(3);
        optimes.push_back(5);
        generator(recipies);

        while(true) {
            sleep(1);
        }
    } 
    return 0;
}