#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <list>
#include <fstream>
#define NUM_THREADS 4

using namespace std;

struct playerInfo{
  int player;
  int card;
  bool win;
};

list <int> deck;
int turn;
bool done;
pthread_mutex_t mux;
ofstream myfile;

void shuffleDeck(){
  while(!deck.empty()){
    deck.pop_front();
  }
  int numCards = 0;
  int count[13] = {0};
  while(numCards < 52){
    int random = rand() % 13 + 1;
    if(count[random - 1] < 4){
      count[random - 1]++;
      numCards++;
      deck.push_back(random);
    }
  }
}

void printDeck(){
  cout << "DECK ";
  for(list<int>::iterator it = deck.begin(); it != deck.end(); it++){
    cout << *it << " ";
  }
}

void filePrintDeck(){
  myfile << "DECK: ";
  for(list<int>::iterator it = deck.begin(); it != deck.end(); it++){
    myfile << *it << " ";
  }
}

int compareCard(int c, int player){
  int temp = deck.front();
  myfile << "PLAYER " << player + 1 << ": draws " << temp << "\n";
  deck.pop_front();
  if(c == temp){
    // winner
    myfile << "PLAYER " << player + 1 << ": hand " << c << " " << c << "\n";
    myfile << "PLAYER " << player + 1 << ": wins \n";
    myfile << "PLAYER " << player + 1 << ": exits round \n";
    myfile << "PLAYER " << ((player + 1) % 3) + 1 << ": exits round \n";
    myfile << "PLAYER " << ((player + 2) % 3) + 1 << ": exits round \n";
    myfile << "DEALER: shuffle \n";
    return -1;
  }else{
    if(rand() % 2 == 0){
      deck.push_back(c);
      myfile << "PLAYER " << player + 1 << ": discards " << c << "\n";
      return temp;
    }else{
      deck.push_back(temp);
      myfile << "PLAYER " << player + 1 << ": discards " << temp << "\n";
      return c;
    }
  }
}

void *player(void *arg){
  playerInfo *p = (playerInfo*)arg;
  int temp;
  while(true){
    pthread_mutex_lock(&mux);
    if(done == true){
      pthread_mutex_unlock(&mux);
      break;
    }
    if(turn == p->player){
      myfile << "PLAYER " << p->player + 1 << ": hand " << p->card << "\n";
      temp = compareCard(p->card, p->player);
      if(temp == -1){
        done = true;
        p->win = 1;
        pthread_mutex_unlock(&mux);
        break;
      }else{
        p->card = temp;
        myfile << "PLAYER " << p->player + 1 << ": hand " << p->card << "\n";
        filePrintDeck();
        myfile << "\n";
      }
      
      turn = (turn + 1) % 3;
      pthread_mutex_unlock(&mux);
    }else{
      pthread_mutex_unlock(&mux);
    }
  }
  pthread_exit(NULL);
}

void *dealer(void *arg){
  pthread_t *t = (pthread_t*)arg;
  void* status;
  playerInfo p[3];
  p[0].player = 0;
  p[1].player = 1;
  p[2].player = 2;
  p[0].card = -1;
  p[1].card = -1;
  p[2].card = -1;
  p[0].win = 0;
  p[1].win = 0;
  p[2].win = 0;
  for(int i = 0; i < 3; i++){
    shuffleDeck();
    turn = i;
    done = false;
    p[i].card = deck.front();
    deck.pop_front();
    p[(i + 1) % 3].card = deck.front();
    deck.pop_front();
    p[(i + 2) % 3].card = deck.front();
    deck.pop_front();
    for(int j = 0; j < 3; j++){
      pthread_create(&t[j],NULL,player,(void*)&p[j]);
    }
    while(true){
      pthread_mutex_lock(&mux);
      if(done == true){
        pthread_mutex_unlock(&mux);
        break;
      }
      pthread_mutex_unlock(&mux);
    }
    
    for(int j = 0; j < 3; j++){
      pthread_join(t[j],&status);
      cout << "\nPLAYER " << j+1 << ":";
      if(p[j].win == true){
        cout << "\nHAND " << p[j].card << " " << p[j].card;
        cout << "\nWIN yes";
      }
      else{
        cout << "\nHAND " << p[j].card;
        cout << "\nWIN no";
      }
    }
    p[0].card = -1;
    p[1].card = -1;
    p[2].card = -1;
    p[0].win = 0;
    p[1].win = 0;
    p[2].win = 0;
    cout << "\n";
    printDeck();
    deck.clear();
    cout << "\n";
  }
  
  pthread_exit(NULL);
}

int main(int argc, char *argv[]){
  myfile.open("logFile.txt");
  srand(atoi(argv[1]));
  pthread_t threads[NUM_THREADS];
  pthread_mutex_init(&mux,NULL);

  pthread_create(&threads[3],NULL,dealer,&threads);
  
  pthread_mutex_destroy(&mux);
  pthread_exit(NULL);
  
  myfile.close();
}
