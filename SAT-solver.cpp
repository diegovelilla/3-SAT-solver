#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
using namespace std;

#define UNDEF -1
#define TRUE 1
#define FALSE 0

uint numVars;
uint numClauses;
vector<vector<int> > clauses;
vector<int> model;
vector<int> modelStack;
uint indexOfNextLitToPropagate;
uint decisionLevel;
vector<vector<vector<int> > > occurlist;
int conflicts = 0;
int conflictstot = 0;

void readClauses( ){
  // Skip comments
  char c = cin.get();
  while (c == 'c') {
    while (c != '\n') c = cin.get();
    c = cin.get();
  }  
  // Read "cnf numVars numClauses"
  string aux;
  cin >> aux >> numVars >> numClauses;
  clauses.resize(numClauses);
  occurlist.resize(numVars + 1);
  for(uint i = 0; i < numVars + 1; ++i){
    occurlist[i].resize(7); 
    occurlist[i][2].push_back(0); // Value of first appearance
    occurlist[i][3].push_back(1); // 1 if always same symbol, -7 -7 -7 -7 would be 1 because its always negative
    occurlist[i][4].push_back(0); // number of appearance
    occurlist[i][5].push_back(0); // positive if there are more positives, negative otherwise
    occurlist[i][6].push_back(0); // number of conflicts generated
  }   
  // Read clauses
  for (uint i = 0; i < numClauses; ++i) {
    int lit;
    while (cin >> lit and lit != 0){
      occurlist[abs(lit)][4][0]++;
      occurlist[abs(lit)][5][0] += lit;
      clauses[i].push_back(lit);
      if (occurlist[abs(lit)][2][0] == 0) occurlist[abs(lit)][2][0] = lit;
      else{
        if(occurlist[abs(lit)][2][0] != lit) occurlist[abs(lit)][3][0] = 0;
      }
      
      if(lit < 0) occurlist[-lit][0].push_back(i); // saves in [lit][0] all clauses where it appears as a negative literal.
      else occurlist[lit][1].push_back(i); // saves in [lit][1] all clauses where it appears as a positive literal.
    }
  }    
}


// returns the value of lit in the current model
int currentValueInModel(int lit){
  if (lit >= 0) return model[lit];
  else {
    if (model[-lit] == UNDEF) return UNDEF;
    else return 1 - model[-lit];
  }
}

// sets some literal to true (lit>0) or false (lit<0), lit can never be 0
void setLiteralToTrue(int lit){
  modelStack.push_back(lit);
  if (lit > 0) model[lit] = TRUE;
  else model[-lit] = FALSE;		
}

// return if there's any conflict with decided literals
bool propagateGivesConflict () {
  while ( indexOfNextLitToPropagate < modelStack.size() ) {
    int nextLitToPropagate = modelStack[indexOfNextLitToPropagate];
    // if positive, only look clauses where it appears as a negative literal
    if (nextLitToPropagate > 0){
      for (uint i = 0; i < occurlist[nextLitToPropagate][0].size(); ++i) {
        bool someLitTrue = false;
        int numUndefs = 0;
        int lastLitUndef = 0;
        for (uint k = 0; not someLitTrue and k < clauses[occurlist[nextLitToPropagate][0][i]].size(); ++k){
          int val = currentValueInModel(clauses[occurlist[nextLitToPropagate][0][i]][k]);
          if (val == TRUE) someLitTrue = true;
          else if (val == UNDEF){ ++numUndefs; lastLitUndef = clauses[occurlist[nextLitToPropagate][0][i]][k]; }
        }
        if (not someLitTrue and numUndefs == 0){ // conflict! all lits false
          // increase the total number of conflicts and the ones the last literal seen causes
          conflicts++;
          conflictstot++;
          occurlist[nextLitToPropagate][6][0]++;
          return true;
        }  
        else if (not someLitTrue and numUndefs == 1) setLiteralToTrue(lastLitUndef);
      }
    }
    // if negative, only look clauses where it appears as a positive literal
    else{
      for (uint i = 0; i < occurlist[-nextLitToPropagate][1].size(); ++i) {
        bool someLitTrue = false;
        int numUndefs = 0;
        int lastLitUndef = 0;
        for (uint k = 0; not someLitTrue and k < clauses[occurlist[-nextLitToPropagate][1][i]].size(); ++k){
          int val = currentValueInModel(clauses[occurlist[-nextLitToPropagate][1][i]][k]);
          if (val == TRUE) someLitTrue = true;
          else if (val == UNDEF){ ++numUndefs; lastLitUndef = clauses[occurlist[-nextLitToPropagate][1][i]][k]; }
        }
        if (not someLitTrue and numUndefs == 0){// conflict! all lits false
          // increase the total number of conflicts and the ones the last literal seen causes
          conflicts++;
          conflictstot++;
          occurlist[-nextLitToPropagate][6][0]++;
          return true;
        }  
        else if (not someLitTrue and numUndefs == 1) setLiteralToTrue(lastLitUndef);
        }
      }
    ++indexOfNextLitToPropagate;
  }
  return false;
}

// backtrack until last decision
void backtrack(){
  uint i = modelStack.size() -1;
  int lit = 0;
  while (modelStack[i] != 0){ // 0 is the DL mark
    lit = modelStack[i];
    model[abs(lit)] = UNDEF;
    modelStack.pop_back();
    --i;
  }
  // at this point, lit is the last decision
  modelStack.pop_back(); // remove the DL mark
  --decisionLevel;
  indexOfNextLitToPropagate = modelStack.size();
  setLiteralToTrue(-lit);  // reverse last decision
}


// Heuristic for finding the next decision literal:
int getNextDecisionLiteral(){
  int masRepetido = 0;
  if(conflicts > 75000){
    for (uint i = 1; i <= numVars; ++i){ occurlist[i][6][0] /= 2; conflicts = 0;}
  }
  for (uint i = 1; i <= numVars; ++i){
    
    if (occurlist[masRepetido][4][0] + 8*occurlist[masRepetido][6][0] < occurlist[i][4][0] + 8*occurlist[i][6][0]){
      
      if (model[i] == UNDEF) masRepetido = i;
      
    };
  }
  if (masRepetido != 0){
    
    // set the value to the one that would satisfy the most clauses
    if (occurlist[masRepetido][5][0] < 0) return -masRepetido;
    else return masRepetido;
  } 
  return 0; // returns 0 when all literals are defined
}

// checks if the final model is correct
void checkmodel(){
  for (uint i = 0; i < numClauses; ++i){
    bool someTrue = false;
    for (uint j = 0; not someTrue and j < clauses[i].size(); ++j)
      someTrue = (currentValueInModel(clauses[i][j]) == TRUE);
    if (not someTrue) {
      cout << "Error in model, clause is not satisfied:";
      for (uint j = 0; j < clauses[i].size(); ++j) cout << clauses[i][j] << " ";
      cout << endl;
      exit(1);
    }
  }  
}

int main(){ 
  readClauses(); // reads numVars, numClauses and clauses
  model.resize(numVars+1,UNDEF);
  indexOfNextLitToPropagate = 0;  
  decisionLevel = 0;
  
  // Take care of initial unit clauses, if any
  for (uint i = 0; i < numClauses; ++i)
    if (clauses[i].size() == 1) {
      int lit = clauses[i][0];
      int val = currentValueInModel(lit);
      if (val == FALSE) {cout << "UNSATISFIABLE" << conflictstot << endl; return 10;}
      else if (val == UNDEF) setLiteralToTrue(lit);

      
  }

  for(uint i = 1; i < numVars + 1; ++i){
    if(occurlist[i][3][0] == 1) setLiteralToTrue(occurlist[i][2][0]);
  }
  
  // DPLL algorithm
  while (true) {
    while ( propagateGivesConflict() ) {
      if ( decisionLevel == 0) { cout << "UNSATISFIABLE " << conflictstot << " conflicts." << endl; return 10; }
      backtrack();
    }
    int decisionLit = getNextDecisionLiteral();
    
    if (decisionLit == 0) { checkmodel(); cout << "SATISFIABLE " << conflictstot << " conflicts." << endl; return 20; }
    // start new decision level:
    modelStack.push_back(0);  // push mark indicating new DL
    ++indexOfNextLitToPropagate;
    ++decisionLevel;
    setLiteralToTrue(decisionLit);    // now push decisionLit on top of the mark
    
  }
}  
