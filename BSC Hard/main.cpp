//BSC - HARD DECODING
#include <iostream>
#include <bits/stdc++.h>

using namespace std;

/*
Hmatrix-0 -> 9x12
Hmatrix-1 -> 3792x5056
Hmatrix-2 -> 3000x5000
Select m,n,dc,dv accordingly
*/

int m = 9;
int n = 12;

int dc = 4;
int dv = 3;

/*
int m = 3792;
int n = 5056;

int dc = 4;
int dv = 3;
*/
/*
int m = 3000;
int n = 5000;

int dc = 5;
int dv = 3;
*/


//The check node takes all the values that have been calculated by its connected variable node and returns the mod2sum.
//So, the value to send to the variable  node is simply calculated as (mod2sum + value_var_node_has_sent)%2
int sum(int **arr,int s){
    int sum_mod_2=0;
    for(int i = 0; i<s; i++)
        sum_mod_2 += *arr[i];
    return sum_mod_2%2;
}

//For the variable node, it helps calculates the number of ones using majority vote,
//and helps in the deciding its value and what it has to send to the check nodes.
int countone(int **arr, int s, int ex, int r){
    int count1 = r;
    for(int i = 0; i<s;i++){
        if(*arr[i] == 1 && i != ex){
            count1++;
        }
    }
    if (count1>((s-1)/2.0))
        return 1;
    return 0;
}

//Standard template for check node
struct checkNode{
    int ind_cr = 0;
    int **c_r = (int**)malloc(sizeof(int**)*dc);

    int *send_vr = (int*)malloc(sizeof(int)*dc);
    int ind_send_vr = 0;
};

//Standard template for variable node
struct varNode{
    int r;
    int value;

    int ind_vr = 0;
    int **v_r = (int**)malloc(sizeof(int**)*dv);

    int *send_cr = (int*)malloc(sizeof(int*)*dv);
    int ind_send_cr = 0;

};

int main()
{
    cout<<"BSC - Hard decoding\n";
    int rows = m, columns = n;
    cout<<rows<<","<<columns<<endl;
    int* H = (int*)malloc(rows*columns*sizeof(int*));
    cout<<"matrix created\n";

    ifstream file("Hmatrix-0.csv");  //Loading Hmatrix-0/1/2
    cout<<"File loaded\n";
    int count1 =0;
    for(int i = 0; i<rows; i++){
        for(int j = 0; j<columns; j++){
            if(!file.eof()){
                string tmp;
                if(j == n-1)
                    getline(file, tmp);
                else
                    getline(file, tmp, ',');
                stringstream geek(tmp);
                int x = 0;
                geek >> x;
                H[i*columns + j]= x;
                if(x == 1)
                    count1++;
            }
        }
    }
    cout<<"Loaded: "<<count1<<endl;

    //Tanner Graph implementation
    checkNode **checkNode_arr = (checkNode**)malloc(sizeof(checkNode**)*m);
    varNode **varNode_arr = (varNode**)malloc(sizeof(varNode**)*n);

    for (int i = 0; i<m; i++){
        checkNode * temp = new checkNode;
        checkNode_arr[i] = temp;
    }

    for (int i = 0; i<n; i++){
        varNode * temp = new varNode;
        varNode_arr[i] = temp;
    }

    //making connections between check node and variable node
    for(int i = 0; i<m; i++){
        for(int j =0; j<n; j++){
            if(H[i*columns + j] == 1){
                checkNode_arr[i]->c_r[checkNode_arr[i]->ind_cr++] = &(varNode_arr[j]->send_cr[varNode_arr[j]->ind_send_cr++]);//if done value wise instead, then i need to repeat it every step
                varNode_arr[j]->v_r[varNode_arr[j]->ind_vr++] = &(checkNode_arr[i]->send_vr[checkNode_arr[i]->ind_send_vr++]);
            }
        }
    }

    cout<<"Adjacency list created!\n";
    double prob[100];
    double p;
    int c[n] = {0};
    int recieved[n];

    float pcorr[11];
    int index = 0;

    //monte carlo simulation
    for(p = 0; p<=1.01; p+=0.05){
        int Nerr = 0;
        for(int Nsim = 0; Nsim<100; Nsim++){

            float noise[n];
            for(int j = 0; j<n; j++){
                float rand_noise = ((float)rand())/RAND_MAX;
                if(rand_noise<p)
                    noise[j] = 1;
                else
                    noise[j] = 0;
            }

            for(int j = 0; j<n; j++){
                if(noise[j] == 1){
                    recieved[j] = !c[j];
                }
                else
                    recieved[j] = c[j];

                varNode_arr[j]->r = recieved[j];
                varNode_arr[j]->value = recieved[j];
            }


            //initialize the variable nodes' send_cr values as recived
            for(int i = 0; i<n; i++){
                for(int j = 0; j<dv; j++){
                    varNode_arr[i]->send_cr[j] = varNode_arr[i]->r;
                }
            }

            int it = 0;
            while(++it<=15){
                //check node calculating values for variable node
                for(int i =0; i<m; i++){
                    int mod2sum = sum(checkNode_arr[i]->c_r, checkNode_arr[i]->ind_cr);//calculate only once 
                    for(int j = 0; j<checkNode_arr[i]->ind_cr; j++){
                        checkNode_arr[i]->send_vr[j] = (mod2sum + *checkNode_arr[i]->c_r[j])%2;
                        // checkNode_arr[i]->c_r[j] points to the value send by some connected vr
                    }
                }

                //variable node calculating values for check node
                for(int i =0; i<n; i++){
                    //varNode_arr[i]->ind_vr = 0;
                    for(int j = 0; j<varNode_arr[i]->ind_vr; j++){
                        varNode_arr[i]->send_cr[j] = countone(varNode_arr[i]->v_r, 3, j,varNode_arr[i]->r);//i.e to calculate max of (recieved + val calulatedby connected checknodes execpt the current checknode)
                    }

                    varNode_arr[i]->value = countone(varNode_arr[i]->v_r,3, -1,0);
                }
            }

            //int key = 1;
            for(int i =0; i<n; i++){
                if(c[i] != varNode_arr[i]->value){
                    Nerr++;
                    break;
                }
            }
        }
        pcorr[index] = 1 - Nerr/100.00;
        prob[index] = p;
        cout<<"p: "<<p<<", pcorr: "<<pcorr[index]<<endl;
        index++;
    }
    return 0;
}
