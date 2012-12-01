#include <iostream>
 
using namespace std;
 
int main()
{
    string t[11];
    int r, tc = 179;
 
    for (r = 0; r < 11; r++)  //declara no vetor t[] as partes do quadrado.
    {
        t[r] = tc;
                tc++;
                if(tc==181)tc=' ';
                if(tc==198)tc='_';
        }
 
    cout<<t[10]<<t[7]<<t[7]<<t[7]<<t[5]<<t[7]<<t[7]<<t[7]<<t[5]<<t[7]<<t[7]<<t[7]<<t[2]<<endl;
        cout<<t[0]<<" 7 "<<t[0]<<" 8 "<<t[0]<<" 9 "<<t[0]<<endl;
        cout<<t[6]<<t[7]<<t[7]<<t[7]<<t[8]<<t[7]<<t[7]<<t[7]<<t[8]<<t[7]<<t[7]<<t[7]<<t[1]<<endl;
        cout<<t[0]<<" 4 "<<t[0]<<" 5 "<<t[0]<<" 6 "<<t[0]<<endl;
        cout<<t[6]<<t[7]<<t[7]<<t[7]<<t[8]<<t[7]<<t[7]<<t[7]<<t[8]<<t[7]<<t[7]<<t[7]<<t[1]<<endl;
        cout<<t[0]<<" 1 "<<t[0]<<" 2 "<<t[0]<<" 3 "<<t[0]<<endl;
        cout<<t[3]<<t[7]<<t[7]<<t[7]<<t[4]<<t[7]<<t[7]<<t[7]<<t[4]<<t[7]<<t[7]<<t[7]<<t[9]<<endl;
 
 
    return 0;
}