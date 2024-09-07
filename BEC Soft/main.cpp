//BEC soft decoding
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
//change Nsim
*/

/*
int m = 3000;
int n = 5000;

int dc = 5;
int dv = 3;
//Change Nsim
*/


//returns a binary string for a integer i of length n used to calculate p(1) and p(0).
string binaryConversion(int n, int length){
    string ans = "";
    while (n>0){
        if (n%2)
            ans = '1' + ans;
        else
            ans = '0' + ans;
        n /= 2;
    }
    int size = ans.size();
    while (size < length){
        ans = '0' + ans;
        size++;
    }
    return ans;
}

//at the check node it calculates the odds in favour of 1 for a given variable node to which
//it is connected with help of the rest of variable nodes it is connected to.
float lambda_1_calc(float **arr, int n, int exclude){

    float res0 = 0;
    float res1 = 0;
    float calc_tmp;

    //for string with odd ones
    int max = pow(2,n);
    int i=0;
    while (i<max){
        if (__builtin_popcount(i)%2){//__builtin_popcount(i) calculates number of ones in binary of i
            //here only if number of ones is odd. i.e. calc for p(exclude == 1);
            string tmp = binaryConversion(i, n);
            calc_tmp = 1;
            int it_ind = 0;
            for(int j =0; j<n+1; j++){
                if(j == exclude){
                    continue;
                }

                if(tmp[it_ind] == '1')
                    calc_tmp *= ((*arr[j]));
                else
                    calc_tmp *= (1 - (*arr[j]));
                it_ind++;
            }
            res1 += calc_tmp;
        }
        else {
            string tmp = binaryConversion(i, n);
            calc_tmp = 1;
            int it_ind = 0;
            for(int j =0; j<n+1; j++){
                if(j == exclude){
                    continue;
                }

                if(tmp[it_ind] == '1')
                    calc_tmp *= ((*arr[j]));
                else
                    calc_tmp *= (1 - (*arr[j]));
                it_ind++;
            }
            res0 += calc_tmp;
        }
        i++;
    }

    return res1/res0;
}

//Standard template for check node
struct checkNode{
    int ind_cr = 0;
    float **c_r = (float**)malloc(sizeof(float**)*dc);

    float *send_vr = (float*)malloc(sizeof(float)*dc);
    int ind_send_vr = 0;
};

//Standard template for variable node
struct varNode{
    float r;
    int value;
    float ld1;

    int ind_vr = 0;
    float **v_r = (float**)malloc(sizeof(float**)*dv);

    float *send_cr = (float*)malloc(sizeof(float*)*dv);
    int ind_send_cr = 0;

};


int main()
{
    cout<<"BEC - SOFT DECODING\n";
    int rows = m, columns = n;
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

    double prob[100];
    double p;
    int c[n] = {0};
    int recieved[n];
    float pcorr[11];

    //Tanner graph implementation
    checkNode **checkNode_arr = (checkNode**)malloc(sizeof(checkNode**)*m);//check **
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
                checkNode_arr[i]->c_r[checkNode_arr[i]->ind_cr++] = &(varNode_arr[j]->send_cr[varNode_arr[j]->ind_send_cr++]);
                varNode_arr[j]->v_r[varNode_arr[j]->ind_vr++] = &(checkNode_arr[i]->send_vr[checkNode_arr[i]->ind_send_vr++]);
            }
        }
    }

    cout<<"Memory allocated\n";

    int index = 0;
    //Monte carlo simulation
    for(p = 0; p<=1.01; p+=0.1){
        int Nerr = 0;
        for(int Nsim = 0; Nsim<1000; Nsim++){

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
                    recieved[j] = -1;
                }
                else
                    recieved[j] = c[j];
            }


            for (int i = 0; i<n; i++){
                if(recieved[i] == 1){
                    varNode_arr[i]->r = 0.99999;
                }

                else if(recieved[i] == 0){
                    varNode_arr[i]->r = 0.00001;//means, if i recieve a 0 then probability that it is one is 0.00001
                }
                else{
                    varNode_arr[i]->r = 0.5;//if noise, it is equally likely to be any of 1 or 0
                }
                varNode_arr[i]->value = recieved[i];
            }

            //initialize the check node values
            for(int i = 0; i<n; i++){
                for(int j = 0; j<dv; j++){
                    varNode_arr[i]->send_cr[j] = varNode_arr[i]->r;
                }
            }
            float calc;
            float alpha1;
            float alpha2;
            int it = 0;
            float tmp_arr[n];
            while(++it<=10){

                //to set initial value required for break check
                for(int i = 0; i<n; i++){
                    tmp_arr[i] = varNode_arr[i]->value;
                }

                for(int i =0; i<m; i++){
                    for(int j = 0; j<checkNode_arr[i]->ind_cr; j++){
                        checkNode_arr[i]->send_vr[j] = lambda_1_calc(checkNode_arr[i]->c_r,dc-1, j);
                    }
                }

                for(int i =0; i<n; i++){
                    varNode_arr[i]->ld1 = (varNode_arr[i]->r)/(1-varNode_arr[i]->r);//used to calulate odds in favour of 1

                    for(int k = 0; k< dv; k++){
                        varNode_arr[i]->ld1 *= *(varNode_arr[i]->v_r[k]);//v_r points to odds in favour sent by checknodes 
                    }

                    for(int j = 0; j<varNode_arr[i]->ind_vr; j++){
                        calc = varNode_arr[i]->ld1/(*(varNode_arr[i]->v_r[j]));//odds excluding current odds
                        varNode_arr[i]->send_cr[j] = calc/(1+calc);//convert odss to probability of 1
                    }

                    if(varNode_arr[i]->ld1 > 1)
                        varNode_arr[i]->value = 1;
                    else if(varNode_arr[i]->ld1 < 1)
                        varNode_arr[i]->value = 0;
                }

                //break condition: if no improvement
                for(int i = 0; i<n; i++){
                    if(tmp_arr[i] != varNode_arr[i]->value){
                        break;
                    }
                }
            }

            for(int i =0; i<n; i++){
                if(c[i] != varNode_arr[i]->value){
                    Nerr++;
                    break;
                }
            }
        }

        pcorr[index] = 1 - (Nerr/1000.00);
        prob[index] = p;
        cout<<"p: "<<p<<", pcorr: "<<pcorr[index]<<endl;
        index++;
    }
    return 0;
}
