//BEC - HARD DECODING
#include <iostream>
#include <bits/stdc++.h>
#include<fstream>

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

//iterates over the values that a check node receives from the variable node, and if there are no erasures it
//returns the mod2sum of all the values it goes over and stores in the array otherwise it returns an erasure.
int sum(int **arr,int s, int ex){
    int sum_mod_2=0;
    for(int i = 0; i<s; i++){
        if(i != ex && arr[i]!=NULL){
            if(*arr[i] == -1)
                return -1;
            else if(*arr[i] == 1 || *arr[i] ==0)
                sum_mod_2 += *arr[i];
        }
    }
    return sum_mod_2%2;
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
    cout<<"BEC - HARD DECODING\n";
    int rows = m, columns = n;//m: check node; n: variable node
    cout<<rows<<","<<columns<<endl;
    int* H = (int*)malloc(rows*columns*sizeof(int*));
    cout<<"matrix created\n";

    ifstream file("Hmatrix-0.csv"); //loading Hmatrix-0/1/2
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
    double prob[11];
    double p;

    int c[n] = {0};//message signal
    int recieved[n];
    float pcorr[11];//stores answer from p=0:0.1:10
    int index = 0;

    //Tanner Graph implementation
    checkNode **checkNode_arr = (checkNode**)malloc(sizeof(checkNode**)*m);//array of checknode pointers
    varNode **varNode_arr = (varNode**)malloc(sizeof(varNode**)*n);//array of varnode pointers

    for (int i = 0; i<m; i++){//create m checknodes
        checkNode * temp = new checkNode;
        checkNode_arr[i] = temp;
    }

    for (int i = 0; i<n; i++){//create n varnodes
        varNode * temp = new varNode;
        varNode_arr[i] = temp;
    }

    //making connections from check node to variable node
    for(int i = 0; i<m; i++){
        for(int j =0; j<n; j++){
            if(H[i*columns + j] == 1){
                checkNode_arr[i]->c_r[checkNode_arr[i]->ind_cr++] = &(varNode_arr[j]->send_cr[varNode_arr[j]->ind_send_cr++]);
                varNode_arr[j]->v_r[varNode_arr[j]->ind_vr++] = &(checkNode_arr[i]->send_vr[checkNode_arr[i]->ind_send_vr++]);
            }
        }
    }

    cout<<"Adjacency list created!\n";
    //Monte carlo for each p
    for(p = 0; p<1.01; p+=0.1){
        int Nerr = 0;
        for(int Nsim = 0; Nsim<1000; Nsim++){
            
            //create a noise array
            float noise[n];
            for(int j = 0; j<n; j++){
                float rand_noise = ((float)rand())/RAND_MAX;
                if(rand_noise<p)
                    noise[j] = 1;
                else
                    noise[j] = 0;
            }

            //recieved taken up, errased if noise is 1
            for(int j = 0; j<n; j++){
                if(noise[j] == 1){
                    recieved[j] = -1;
                }
                else
                    recieved[j] = c[j];
            }

            //initialize the check node values
            for(int i = 0; i<n; i++){
                varNode_arr[i]->r = recieved[i];
                for(int j = 0; j<dv; j++){
                    varNode_arr[i]->send_cr[j] = recieved[i];
                }
            }

            int it = 0;//running for 15 iterations to try to recover
            while(++it<=15){
                //iterate over all check nodes and deicde what to send to each varnode
                for(int i =0; i<m ; i++){
                    for(int j = 0; j<dc; j++){
                        int mod2sum = sum(checkNode_arr[i]->c_r, dc, j);
                        if(mod2sum!= -1)
                            checkNode_arr[i]->send_vr[j] = mod2sum;
                        else
                            checkNode_arr[i]->send_vr[j] = -1;
                    }
                }
                for(int i =0; i<n; i++){
                    //finding if non -1 value exists for variable node
                    int correct = -1;
                    if(varNode_arr[i]->r!= -1){
                        correct = varNode_arr[i]->r;
                    }

                    else{
                        for(int j = 0; j<dv; j++){
                            if(*varNode_arr[i]->v_r[j]!= -1){
                                correct = *varNode_arr[i]->v_r[j];
                                break;
                            }
                        }
                    }

                    for(int j = 0; j<dv; j++){
                        varNode_arr[i]->send_cr[j] = correct;
                    }
                    varNode_arr[i]->value = correct;
                }
            }

            for(int i =0; i<n; i++){
                if(c[i] != varNode_arr[i]->value){
                    Nerr++;
                    break;
                }
            }
        }
        pcorr[index] = 1-(Nerr/1000.00);
        prob[index] = p;
        cout<<"p: "<<p<<", pcorr: "<<pcorr[index]<<endl;
        index++;
    }
    return 0;
}
