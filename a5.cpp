// Online C++ compiler to run C++ program online

/*/////// BUGS //////////

1. enter at Last line
2. Regex change register names
3. Word to 8 bit signed int change
4. incorporate 16 bit offsets and immediate addi
5. Search for label names
6. Writeback will get cancelled if DepReg is updated
7. consecutive sw instructions for same address will not run efficiently

/////////////////////*/

#include <iostream>
#include <stdint.h>
#include <fstream>
#include <math.h>
#include <string>
#include <cstring>
#include <regex>
#include <unordered_map>
#include<exception>
#include<vector>

using namespace std;

///// GLOBAL VARIABLES /////////////
/*
int8_t *Memory;
int DepReg[32][] = {}; 
vector<int> RBufferQueue;   
int reg[32][] = {}; 
int RBIKey[1024] = {};
int MaxKey = 0;

*/
///////////////////////////////////

////// NEW Functions //////////////////////////////////////////

class Instruction {
    public :
     int Code;
     int Reg;           // Reg is register number for lw and register value for sw
     int baseRegVal;    // Value of reg for both
     int Offset;        // 16 bit offset
     int key;
     int StartClock;
     int Core;
};

class Heap{
    public :
        vector<Instruction> v;

        bool isEmpty(){
            if (v.size() == 0)
                return true;
            else
                return false;
        }

        Instruction FindMin(){
            if (!isEmpty())
                return v.at(0);
            else{
                Instruction temp;
                return temp;
            }
        }

        void insert(Instruction i){
            v.push_back(i);

            int index = v.size() - 1;
            int Found = 0;

            while (!Found){
                if (index == 0 || v.at(index).key >= v.at(index-1).key){
                    Found =1;
                }
                else{
                    Instruction temp = v.at(index);
                    v.at(index) = v.at(index-1);
                    v.at(index-1) = temp;
                    index--;
                }
            }
        }

        void DeleteMin(){
            v.erase (v.begin());
        }
            
};

int8_t giveReg(string s){
    int8_t ans;

    if (s[0] == 't')                // t0-9 = reg 0 - 9
        ans = (int)s[1] - 48;            
    else if (s[0] == 's' && s[1] == 'p')           // sp = reg 19 
        ans = 19;
    else if (s[0] == 's')                        // s0-8 = reg 10 - 18
        ans = 10 + (int)s[1] - 48;
    else if (s[0] == 'a' && s[0] == 't')        //  at = reg 24
        ans = 24;
    else if (s[0] == 'a')                       // a0-3 = reg 20 - 23
        ans = 20 + (int)s[1] - 48;
    else if (s[0] == 'v')                       // v0-1 = reg 25-26
        ans = 25 + (int)s[1] - 48; 
    else if (s[0] == 'k')                   // k0-1 = reg 27-28
        ans = 27 + (int)s[1] - 48; 
    else if (s[0] == 'r' && s[1] == 'a')    // ra = reg 30
        ans = 30;
    else if (s[0] == 'r' || s[0] == 'z')    // r0 and zero = reg 31
        ans = 31;
    else if (s[0] == 'g')                   // gp = reg 29
        ans = 29; 

    return ans;
}

void printReg(int8_t key){

    if (key < 10)
        cout << "$t" << (int)key;
    else if (key < 19)
        cout << "$s" << key-10;
    else if (key == 19)
        cout << "$sp";
    else if (key < 24)
        cout << "$a" << key - 20;
    else if (key == 24)
        cout << "$at";
    else if (key < 27)
        cout << "$v" << key - 25;
    else if (key < 29)
        cout << "$k" << key - 27;
    else if (key == 29)
        cout << "$gp";
    else if (key == 30)
        cout << "$ra";
    else if (key == 31)
        cout << "$zero" ;
    else
        cout << "Error";

}

int to16bit (int8_t a, int8_t b){
    return (b * 256) + (uint8_t)a;
}

int to32bit (int8_t a, int8_t b, int8_t c, int8_t d){
    return (16777216 * d) + (65536 * (uint8_t)c) + (256 * (uint8_t)b) + (uint8_t)a;
}


/*

void ExecCont(int &EC, int Cycle, int End){
    while( (int)Memory[EC+2] != DepReg && (int)Memory[EC+1] != DepReg && (int)Memory[EC] != 11 && Cycle != End){
        if(Memory[EC] == 0 && (int)Memory[EC+3] != DepReg) // add
        {
            reg[Memory[EC+1]] = reg[Memory[EC+2]] + reg[Memory[EC+3]];
            

            Cycle++;
            cout << endl <<"Cycle " << Cycle << " -> add $r"<< (int)Memory[EC+1] << ", $r" << (int)Memory[EC+2] << ", $r" << (int)Memory[EC+3] << "\t executed" << endl;
            cout << "Register $r" <<  (int)Memory[EC+1] << " updated to " << reg[Memory[EC+1]] << endl; 
        
            EC+=4;
        }

        else if(Memory[EC] == 10) // addi
        {
            reg[Memory[EC+1]] = reg[Memory[EC+2]] + Memory[EC+3];
            
            Cycle++;
            cout << endl << "Cycle " << Cycle << " -> addi $r"<< (int)Memory[EC+1] << ", $r" << (int)Memory[EC+2] << ", " << (int)Memory[EC+3] << "\t executed" << endl;
            cout << "Register $r" <<  (int)Memory[EC+1] << " updated to " << reg[Memory[EC+1]] << endl; 


            EC+=4;
        }
        else
            break;
    }
}

*/



void pause()
{
	string temp;
    cout<<"\nenter anything to continue";
    cin>>temp;
}

void removeSpaces(string& str) 
{ 
    int count = 0; 
  
    for (int i = 0; str[i] != '\0'; i++) 
        if (str[i] != ' ' && str[i] != '\t') 
            str[count++] = str[i]; // here count is 
                                   // incremented 
    str[count] = '\0'; 
    str.resize(count);

}

void printMemory(int8_t *Memory, int lastPC)
{
    cout<<"\nPC\t@PC\t@PC+1\t@PC+2\t@PC+3";
	for(int PC=0; PC <= lastPC; PC+=4)
	{
		cout<<"\n"<<PC<<"\t"<<int(Memory[PC])<<"\t"<<int(Memory[PC+1])<<"\t"<<int(Memory[PC+2])<<"\t"<<int(Memory[PC+3]);
	}
}


void Invalid(){
    cout << endl << "Invalid syntax" << endl;

}

/*
string FindKey(unordered_map<string, int8_t> u , int8_t k)
{
    int Found = 0;

    unordered_map<string, int8_t>:: iterator itr; 
    
    for (itr = u.begin() ; itr != u.end() && !Found ; itr++)
        if(itr->second == k)
            Found = 1;
    
    itr--;

    if(!Found)
        return "Label not Found";
    else
        return itr->first; 
}
*/


int main() {

    int M;

    cout << "Enter number of clock cycles to simulate: ";
    cin >> M;

    cout << "Enter number of Cores: " ;

    int N;
    cin >> N;

    int8_t *Memory;
    int DepReg[32][N]; 

    for (int j =0 ; j < N; j++)
        for (int k = 0; k < 32; k++)
            DepReg[k][j] = 0;
    

    vector<int> RBufferQueue;   
    
    int reg[32][N]; 

    for (int j =0 ; j < N; j++)
        for (int k = 0; k < 32; k++)
            reg[k][j] = 0;
    
    int RBIKey[1024] = {};
    int MaxKey = 0;

    Heap InsQueue;

    int Clock;

    int Break = 0;

    int size = pow(2,20);

    Memory = new int8_t[size]; 

    int ReservedMemory = N*1000;

    int PartitionLength = (size - ReservedMemory) / N;

    int RBIndex = -1024;     // Row Buffer Index
    int RBUpdate = 0;        // Number of Row buffer updates
    
    int RowAD;            // Row Access Delay
    int ColAD;            // Col access delay

    int PC = 0;             // Position of Program Counter (bytewise)

    //bool DRAMAccess = 0;    // Is DRAM being accessed
    
    

    int8_t LabelNumber = 0;     //Number of labels, used to map labels to numbers
    string filename;
    
    string instruction;

    unordered_map<string , int8_t> lmap[N];    // Maps labels to integers
    unordered_map<int8_t , int> PCmap[N];     // Maps the integers to PC
    
    
    cout<< "Enter the ROW_ACCESS_DELAY: ";
    cin >> RowAD;

    cout<< "Enter the COL_ACCESS_DELAY: ";
    cin >> ColAD;
    
    for (int i =0; i < N; i++){

        cout << "Enter name of file " << i +1 <<" : ";
        cin >> filename;
        cout << endl;
        
        ifstream file;
        file.open(filename);
        
        while (file){
            getline(file, instruction);

            
            removeSpaces(instruction);
            
            
            string str = instruction;
            smatch m;
            //regex re_add("add\\$r([0-9][0-9]?),\\$r([0-9][0-9]?),\\$r([0-9][0-9]?)");
            regex re_add("add\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero)(|#.*)");
            regex re_sub("sub\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero)(|#.*)");
            regex re_mul("mul\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero)(|#.*)");
            regex re_muli("mul\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),([\\+-]?[0-9]+)(|#.*)");
            regex re_beq("beq\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),([a-z]+)(|#.*)");
            regex re_bne("bne\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),([a-z]+)(|#.*)");
            regex re_slt("slt\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero)(|#.*)");
            regex re_j("j([a-z]+)(|#.*)");
            regex re_label("([a-z]+):(|#.*)");
            regex re_lw("lw\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),([\\+-]?[0-9]+)\\(\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero)\\)(|#.*)");
            regex re_sw("sw\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),([\\+-]?[0-9]+)\\(\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero)\\)(|#.*)");
            regex re_addi("addi\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),\\$(t[0-9]|s[p0-8]|v[01]|a[t0-3]|r[a0]|k[01]|gp|zero),([\\+-]?[0-9]+)(|#.*)");
            regex re_comment("#.*");

        
            if(regex_match(str,m,re_label))
            {

                if (lmap[i].find(string(m[1])) == lmap[i].end()){

                    lmap[i][string(m[1])] = LabelNumber;
                    PCmap[i][LabelNumber] = PC;
                    LabelNumber++;
                }
                else{
                    PCmap[i][lmap[i].at(string(m[1]))] = PC;
                }

            }
                
            else if(regex_match(str,m,re_add)){
                Memory[PC] = 0;
                Memory[PC+1] = giveReg(string(m[1]));
                Memory[PC+2] = giveReg(string(m[2]));
                Memory[PC+3] = giveReg(string(m[3]));
                PC += 4;
            }
                
            else if(regex_match(str,m,re_sub)){
                Memory[PC] = 1;
                Memory[PC+1] = giveReg(string(m[1]));
                Memory[PC+2] = giveReg(string(m[2]));
                Memory[PC+3] = giveReg(string(m[3]));
                   
                PC += 4;
            }

            else if(regex_match(str,m,re_mul)){
                Memory[PC] = 2;
                Memory[PC+1] = giveReg(string(m[1]));
                Memory[PC+2] = giveReg(string(m[2]));
                Memory[PC+3] = giveReg(string(m[3]));
                
                PC += 4;
            }

            else if(regex_match(str,m,re_muli)){
                Memory[PC] = 3;
                Memory[PC+1] = giveReg(string(m[1]));
                Memory[PC+2] = giveReg(string(m[2]));
                Memory[PC+3] = stoi(string(m[3]));
                
                PC += 4;
            }

            else if(regex_match(str,m,re_beq)){
                Memory[PC] = 4;
                Memory[PC+1] = giveReg(string(m[1]));
                Memory[PC+2] = giveReg(string(m[2]));



                if (lmap[i].find(string(m[3])) == lmap[i].end()){                
                    lmap[i][string(m[3])] = LabelNumber;
                    Memory[PC + 3] = LabelNumber;
                    LabelNumber++;
                }
                else
                    Memory[PC + 3] = lmap[i].at(string(m[3]));
                
                PC += 4;
            }
            else if(regex_match(str,m,re_bne)){
                Memory[PC] = 5;
                Memory[PC+1] = giveReg(string(m[1]));
                Memory[PC+2] = giveReg(string(m[2]));


                if (lmap[i].find(string(m[3])) == lmap[i].end()){                
                    lmap[i][string(m[3])] = LabelNumber;
                    Memory[PC + 3] = LabelNumber;
                    LabelNumber++;
                }
                else
                    Memory[PC + 3] = lmap[i].at(string(m[3]));
                
                PC += 4;
            }
            else if(regex_match(str,m,re_slt)){
                Memory[PC] = 6;
                Memory[PC+1] = giveReg(string(m[1]));
                Memory[PC+2] = giveReg(string(m[2]));
                Memory[PC+3] = giveReg(string(m[3]));
                PC += 4;
                
            }
            else if(regex_match(str,m,re_j)){
                Memory[PC] = 7;
                
                if (lmap[i].find(string(m[1])) == lmap[i].end()){                
                    lmap[i][string(m[1])] = LabelNumber;
                    Memory[PC + 1] = LabelNumber;
                    LabelNumber++;
                }
                else
                    Memory[PC + 1] = lmap[i].at(string(m[1]));

                PC += 4;
                
            }
            
            else if(regex_match(str,m,re_lw)){
                Memory[PC] = 8;
                Memory[PC+1] = giveReg(string(m[1]));          // register 1
                Memory[PC+2] = giveReg(string(m[3]));          // offset register
                Memory[PC+3] = stoi(string(m[2]));          // signed 8 bit integer offset
                Memory[PC+4] = (stoi(string(m[2])) - (uint8_t)Memory[PC+3] ) / 256;

                PC += 5;
                
            }
            else if(regex_match(str,m,re_sw)){
                Memory[PC] = 9;
                Memory[PC+1] = giveReg(string(m[1]));          // register 1
                Memory[PC+2] = giveReg(string(m[3]));          // offset register
                Memory[PC+3] = stoi(string(m[2]));          // signed 8 bit integer offset
                Memory[PC+4] = (stoi(string(m[2])) - (uint8_t)Memory[PC+3] ) / 256;
                
                PC += 5;
            }
            else if(regex_match(str,m,re_addi)){
                Memory[PC] = 10;
                Memory[PC+1] = giveReg(string(m[1]));          
                Memory[PC+2] = giveReg(string(m[2]));          
                Memory[PC+3] = stoi(string(m[3]));
                Memory[PC+4] = (stoi(string(m[3])) - (uint8_t)Memory[PC+3] ) / 256;          
                
                PC += 5;
                
            }
            else if(regex_match(str,m,re_comment)){
                ;
            }
            else if(str.empty())
                cout << "";
            else{
                Invalid();
                Break = 1;
                break;                   
            }
        }
        int lastPC = PC;
        Memory[lastPC] = 11; // end of program
        PC++;
    }
    // Output memory
	//cout<<"\nMEMORY (BEFORE EXECUTION)";
    //printMemory(Memory,lastPC);
    //pause();

    // Declaring variables for statistics
    int stat[12] = {}; // total 12 codes from 0 to 11
    int stat_tot = 0;



    // EXECUTION STARTS HERE
    int EC[N];
    for (int i = 0; i<N; i++){
        try
        {
            EC[i] = PCmap[i].at(lmap[i].at("main"));
        }
        catch(exception& e)
        {
            cout<<"Error. main: not found" << endl;
            return 0;
        }
    }
    
    //cout << "done!" << endl;

    //cout<<"\n\n\nREGISTERS (DURING EXECUTION)\n";

    while( (!Break  || !InsQueue.isEmpty()) && stat_tot < M)
    {

        Break = 1;
        //cout << "Loop here" << endl;
        //cout << (int)InsQueue.isEmpty() << endl;
    	// Update stats
    	//stat_tot++;
    	//stat[Memory[EC]]++;
        stat_tot++;

    	// Actual execution

        for (int i=0; i<N; i++){

        	if(Memory[EC[i]] == 0 && (DepReg[Memory[EC[i]+2]][i] == 0) && (DepReg[Memory[EC[i]+3]][i] == 0 )) // add
        	{
        		reg[Memory[EC[i]+1]][i] = reg[Memory[EC[i]+2]][i] + reg[Memory[EC[i]+3]][i];
        		   
                cout << "Core " << i +1<< ": "<< endl;
                cout << "Cycle " << stat_tot << " -> add ";
                printReg(Memory[EC[i] + 1]);
                cout <<", ";
                printReg(Memory[EC[i] + 2]);
                cout <<", ";
                printReg(Memory[EC[i] + 3]);

                cout << "\t executed" << endl;
                cout << "Register " ;
                printReg(Memory[EC[i]+1]);
                cout << " updated to " << reg[Memory[EC[i]+1]][i] << endl << endl; 

                if (DepReg[Memory[EC[i] + 1]][i] != 0)
                    DepReg[Memory[EC[i] +1]][i]++;
        	
                EC[i]+=4;
            }
        	else if(Memory[EC[i]] == 1 && (DepReg[Memory[EC[i]+2]][i] == 0) && (DepReg[Memory[EC[i]+3]][i] == 0 )) // sub
        	{
        		reg[Memory[EC[i]+1]][i] = reg[Memory[EC[i]+2]][i] - reg[Memory[EC[i]+3]][i];
        		

                cout << "Core " << i +1 << ": "<< endl;
                cout << "Cycle " << stat_tot << " -> sub ";
                printReg(Memory[EC[i] + 1]);
                cout <<", ";
                printReg(Memory[EC[i] + 2]);
                cout <<", ";
                printReg(Memory[EC[i] + 3]);

                cout << "\t executed" << endl;
                cout << "Register " ;
                printReg(Memory[EC[i]+1]);
                cout << " updated to " << reg[Memory[EC[i]+1]][i] << endl << endl; 

                if (DepReg[Memory[EC[i] + 1]][i] != 0)
                    DepReg[Memory[EC[i] +1]][i]++;
            
                EC[i]+=4;
        	}
        	else if(Memory[EC[i]] == 2 && (DepReg[Memory[EC[i]+2]][i] == 0) && (DepReg[Memory[EC[i]+3]][i] == 0 )) // mul
        	{
        		reg[Memory[EC[i]+1]][i] = reg[Memory[EC[i]+2]][i] * reg[Memory[EC[i]+3]][i];
        		

                cout << "Core " << i+1 << ": "<< endl;
                cout << "Cycle " << stat_tot << " -> mul ";
                printReg(Memory[EC[i] + 1]);
                cout <<", ";
                printReg(Memory[EC[i] + 2]);
                cout <<", ";
                printReg(Memory[EC[i] + 3]);

                cout << "\t executed" << endl;
                cout << "Register " ;
                printReg(Memory[EC[i] +1]);
                cout << " updated to " << reg[Memory[EC[i] +1]][i] << endl<< endl; 

                if (DepReg[Memory[EC[i] + 1]][i] != 0)
                    DepReg[Memory[EC[i] +1]][i]++;
            
                EC[i]+=4;
        	}
        	else if(Memory[EC[i]] == 3 && (DepReg[Memory[EC[i]+2]][i]  == 0)) // muli
        	{
        		reg[Memory[EC[i]+1]][i] = reg[Memory[EC[i]+2]][i] * Memory[EC[i]+3];


                cout << "Core " << i+1 << ": "<< endl;
                cout << "Cycle " << stat_tot << " -> mul ";
                printReg(Memory[EC[i] + 1]);
                cout <<", ";
                printReg(Memory[EC[i] + 2]);
                cout <<", ";

                cout << (int)Memory[EC[i] + 3] << "\t executed" << endl;
                cout << "Register " ;
                printReg(Memory[EC[i] +1]);
                cout << " updated to " << reg[Memory[EC[i] +1]][i] << endl<< endl; 

                if (DepReg[Memory[EC[i] + 1]][i] != 0)
                    DepReg[Memory[EC[i] +1]][i]++;
            
                EC[i]+=4;

        	}
        	else if(Memory[EC[i]] == 4 && (DepReg[Memory[EC[i]+1]][i] == 0) && (DepReg[Memory[EC[i]+2]][i] == 0 )) // beq
        	{

                cout << "Core " << i+1 << ": "<< endl;
                cout << "Cycle " << stat_tot << " -> beq ";

                printReg(Memory[EC[i] + 1]);
                cout <<", ";
                printReg(Memory[EC[i] + 2]);


                cout << ", label ID: " << (int)Memory[EC[i]+3] << "\t executing" << endl;
     

        		if(reg[Memory[EC[i]+1]][i] == reg[Memory[EC[i]+2]][i])
                {
                    try
                    {
                        cout << "Jumping to Label ID: " << (int)Memory[EC[i]+3] << endl<< endl;
                        EC[i] = PCmap[i].at(Memory[EC[i]+3]);
                    }
                    catch(exception& e)
                    {
                        cout<<"Error. Label not found";
                        return 0;
                    }
                    
                }
        		else{
        			cout << "No branching occured." << endl<< endl;
                    EC[i]+=4;
                }
        	}
        	else if(Memory[EC[i]] == 5 && (DepReg[Memory[EC[i]+1]][i] == 0) && (DepReg[Memory[EC[i]+2]][i] == 0 )) // bne
        	{

                cout << "Core " << i+1 << ": "<< endl;
                cout << "Cycle " << stat_tot << " -> bne ";

                printReg(Memory[EC[i] + 1]);
                cout <<", ";
                printReg(Memory[EC[i] + 2]);

                
                cout << ", label ID: " << (int)Memory[EC[i]+3] << "\t executing" << endl;
     

                if(reg[Memory[EC[i]+1]][i] != reg[Memory[EC[i]+2]][i])
                {
                    try
                    {
                        cout << "Jumping to Label ID: " << (int)Memory[EC[i]+3] << endl<< endl;
                        EC[i] = PCmap[i].at(Memory[EC[i]+3]);
                    }
                    catch(exception& e)
                    {
                        cout<<"Error. Label not found";
                        return 0;
                    }

                }
                else{
                    cout << "No branching occured." << endl<< endl;
                    EC[i]+=4;
                }
        	}
        	else if(Memory[EC[i]] == 6 && (DepReg[Memory[EC[i]+2]][i] == 0) && (DepReg[Memory[EC[i]+3]][i] == 0 )) // slt
        	{
        		if(reg[Memory[EC[i]+2]][i] < reg[Memory[EC[i]+3]][i])
        			reg[Memory[EC[i]+1]][i] = 1;
        		else
        			reg[Memory[EC[i]+1]][i] = 0;


                cout << "Core " << i+1 << ": "<< endl;
                cout << "Cycle " << stat_tot << " -> slt ";
                printReg(Memory[EC[i] + 1]);
                cout <<", ";
                printReg(Memory[EC[i] + 2]);
                cout <<", ";
                printReg(Memory[EC[i] + 3]);

                cout << "\t executed" << endl;
                cout << "Register " ;
                printReg(Memory[EC[i]+1]);
                cout << " updated to " << reg[Memory[EC[i]+1]][i] << endl<< endl; 

                if (DepReg[Memory[EC[i] + 1]][i] != 0)
                    DepReg[Memory[EC[i]+1]][i]++;
            
                EC[i]+=4;
        	}
        	else if(Memory[EC[i]] == 7) // j
        	{

                cout << "Core " << i+1 << ": "<< endl;
                cout << "Cycle " << stat_tot << " -> j label ID: "<< (int)Memory[EC[i]+1] << "\t executing" << endl;
                try
                {
                    cout << "Jumping to Label ID: " << (int)Memory[EC[i]+1] << endl<< endl;
                    EC[i] = PCmap[i].at(Memory[EC[i]+1]);    
                }
        		catch(exception& e)
                {
                    cout<<"Error. Label not found";
                    return 0;
                }
        	}
        	else if(Memory[EC[i]] == 8 && DepReg[Memory[EC[i]+2]][i] == 0 ) // lw
        	{

                Instruction Temp;

                Temp.Code = 1;
                Temp.Reg = Memory[EC[i]+1];
                Temp.Core = i;
                Temp.baseRegVal = reg[Memory[EC[i]+2]][i];
                Temp.Offset = to16bit(Memory[EC[i]+3], Memory[EC[i]+4]) + ReservedMemory + i*PartitionLength;

                if ( (Temp.Offset + Temp.baseRegVal) > ReservedMemory + (i+1)*PartitionLength){
                    Invalid();
                    cout << "Overstepping memory partition bounds" << endl;
                }

                //cout << "RBI Key at index 1: " << RBIKey[1] << endl;
                //cout << "Temp.Offset: " << Temp.Offset << endl;
                //cout << "Temp.baseRegVal: " << Temp.baseRegVal << endl; 
                //cout << "RBIKey[Temp.Offset + Temp.baseRegVal / 1024] : " << RBIKey[(Temp.Offset + Temp.baseRegVal) / 1024] << endl;

                if (RBIKey[(Temp.Offset + Temp.baseRegVal) / 1024] == 0){
                //    cout << "Key is maxkey" << endl;
                    Temp.key = MaxKey+1;
                    MaxKey++;
                    RBIKey[(Temp.Offset + Temp.baseRegVal) / 1024] = MaxKey;
                }
                else 
                    Temp.key = RBIKey[(Temp.Offset + Temp.baseRegVal) / 1024];

                InsQueue.insert(Temp);
                //cout << "Key inserted: " << Temp.key << endl;


                cout << "Core " << i+1 << ": "<< endl;
                cout << "Cycle " << stat_tot << " -> lw " ;
                printReg(Memory[EC[i] + 1]);
                cout <<", " << to16bit(Memory[EC[i]+3], Memory[EC[i]+4]) << "(" ;
                printReg(Memory[EC[i] + 2]);
                cout << ")\t executing" << endl;
                cout << "DRAM request queued." << endl<< endl;

                DepReg[Memory[EC[i] + 1]][i]++; 
                EC[i] += 5;
        	}
        	else if(Memory[EC[i]] == 9 && DepReg[Memory[EC[i]+2]][i] == 0 && DepReg[Memory[EC[i]+1]][i] == 0) // sw
        	{

                Instruction Temp;

                Temp.Code =2;
                Temp.Core = i;
                Temp.Reg = reg [Memory[EC[i]+1]][i];
                Temp.baseRegVal = reg [Memory[EC[i]+2]][i];
                Temp.Offset = to16bit(Memory[EC[i]+3], Memory[EC[i]+4]) + ReservedMemory + i * PartitionLength;

                if ( (Temp.Offset + Temp.baseRegVal) > ReservedMemory + (i+1)*PartitionLength){
                    Invalid();
                    cout << "Overstepping memory partition bounds" << endl;
                }

                if (RBIKey[ (Temp.Offset + Temp.baseRegVal) / 1024] == 0){
                    Temp.key = MaxKey+1;
                    MaxKey++;
                    RBIKey[(Temp.Offset + Temp.baseRegVal) / 1024] = MaxKey;
                }
                else 
                    Temp.key = RBIKey[(Temp.Offset + Temp.baseRegVal) / 1024];

                InsQueue.insert(Temp);
                //cout << "Key inserted: " << Temp.key << endl;
                
                cout << "Core " << i+1 << ": "<< endl;
                cout << "Cycle " << stat_tot << " -> sw " ;
                printReg(Memory[EC[i] + 1]);
                cout <<", " << to16bit(Memory[EC[i]+3], Memory[EC[i]+4]) << "(" ;
                printReg(Memory[EC[i] + 2]);
                cout << ")\t executing" << endl;
                cout << "DRAM request queued." << endl << endl;
                
                EC[i] += 5;

        	}
        	else if(Memory[EC[i]] == 10 && DepReg[Memory[EC[i]+2]][i] == 0) // addi
        	{
        		reg[Memory[EC[i]+1]][i] = reg[Memory[EC[i]+2]][i] + to16bit(Memory[EC[i]+3], Memory[EC[i]+4]);
        		
                cout << "Core " << i+1 << ": "<< endl;
                cout << "Cycle " << stat_tot << " -> addi ";
                printReg(Memory[EC[i] + 1]);
                cout <<", ";
                printReg(Memory[EC[i] + 2]);
                cout <<", ";

                cout << to16bit(Memory[EC[i]+3], Memory[EC[i]+4]);

                cout << "\t executed" << endl;
                cout << "Register " ;
                printReg(Memory[EC[i]+1]);
                cout << " updated to " << reg[Memory[EC[i]+1]][i] << endl<< endl;

                if (DepReg[Memory[EC[i]+1]][i] != 0)
                    DepReg[Memory[EC[i]+1]][i]++;


                EC[i]+=5;
        	}

            if (Memory[EC[i]] != 11){
                Break =0;
            }

        }
        //cout << "Outside for loop" << endl;
    	// CHANGED else ; HERE

        Clock = stat_tot;

        Instruction Temp = InsQueue.FindMin();
        //cout << Temp.key << Temp.Reg << Temp.Offset << Temp.baseRegVal << Temp.StartClock << endl;
        //cout << "DepReg[Temp.Reg] is "<< DepReg[Temp.Reg] << endl;
        while(!InsQueue.isEmpty() && Temp.Code != 2 && DepReg[Temp.Reg][Temp.Core] != 1) {
            //cout << "Reducing" << endl;
            DepReg[Temp.Reg][Temp.Core]--;            
            InsQueue.DeleteMin(); 

            // This will only work if all instructions with a particular key are together
            if ((InsQueue.FindMin().Offset + InsQueue.FindMin().baseRegVal)/ 1024 != (Temp.Offset + Temp.baseRegVal)/ 1024)
                RBIKey[(Temp.Offset + Temp.baseRegVal)/ 1024] = 0;           
            Temp = InsQueue.FindMin();
        }

        if (!InsQueue.isEmpty() && Temp.StartClock == 0){
            Temp.StartClock = Clock;
            InsQueue.v.at(0).StartClock = Clock;
            cout << "Cycle " << Clock << " -> DRAM request initiated" << endl << endl;
        }

        if( !InsQueue.isEmpty() && Temp.Code == 1){     // LW code

            //cout << "Inside";

            int location = Temp.Offset + Temp.baseRegVal;

            if(Temp.Offset + Temp.baseRegVal <= RBIndex*1024 + 1023 && Temp.Offset + Temp.baseRegVal >= RBIndex*1024){
                //cout << "same Rb" << endl;
                if (Temp.StartClock + ColAD == Clock){

                    reg[Temp.Reg][Temp.Core] = to32bit(Memory[location], Memory[location + 1], Memory [location +2], Memory[location + 3]);
                    
                    cout <<endl<<"Cycle " << Clock + 1 - ColAD << " - " << Clock << " -> Reading from Memory address "<< location << endl << endl;
                    cout << "Core " << Temp.Core+1 << " :" << endl;
                    cout <<"Register " ;
                    printReg(Temp.Reg);
                    cout << " updated to " << reg[Temp.Reg][Temp.Core] << endl << endl;

                    InsQueue.DeleteMin();
                    DepReg[Temp.Reg][Temp.Core]--;
                }   
            }

            else if (RBIndex < 0){
                //cout << "RB is 0" << endl;
                if (Temp.StartClock + RowAD == Clock){
                    RBIndex = location / 1024;
                    cout <<endl<< "Cycle " << Clock - RowAD + 1 << " - " << Clock  << " -> Activating Row index "<< RBIndex << endl << endl;
                    InsQueue.v.at(0).StartClock = Clock;
                    Temp.StartClock = Clock;
                    RBUpdate++;
                }
            }

            else {
                // Assumes that all instructions with same RBIndex will be grouped together (maybe?)
                RBIKey[RBIndex] = 0;
                if (Temp.StartClock + RowAD == Clock){
                    cout << endl << "Cycle " << Clock - RowAD + 1 << " - " << Clock << " -> Writeback Row Buffer to DRAM" << endl << endl;
                     
                }
                else if (Temp.StartClock + 2*RowAD == Clock){
                    RBIndex = location / 1024;
                    cout <<endl<< "Cycle " << Clock - RowAD + 1 << " - " << Clock  << " -> Activating Row index "<< RBIndex << endl << endl; 
                    InsQueue.v.at(0).StartClock = Clock;   
                    Temp.StartClock = Clock;               
                    RBUpdate++;
                }
            }
        } 

        else if (!InsQueue.isEmpty() && Temp.Code == 2){    // SW Code

            //cout << "Executing sw" << endl;

            int location = Temp.Offset + Temp.baseRegVal;

            if (location <= RBIndex * 1024 + 1023 && location >= RBIndex *1024 ){
                if ( Temp.StartClock + ColAD == Clock){
                    Memory[location] = Temp.Reg;
                    Memory[location+1] = (Temp.Reg- (uint8_t)Memory[location] ) / 256;
                    Memory[location+2] = (Temp.Reg - (256 * (uint8_t)Memory[location + 1]) - (uint8_t)Memory[location]) / 65536;
                    Memory[location+3] = (Temp.Reg - (65536 * (uint8_t)Memory[location + 2]) -  (256 * (uint8_t)Memory[location + 1]) - (uint8_t)Memory[location]) / 16777216;

                    cout <<endl <<"Cycle " << Clock + 1 - ColAD << " - " << Clock  << " -> Memory address "<< location << " updated to "<< to32bit(Memory[location],Memory[location+1],Memory[location+2],Memory[location+3]) << endl << endl;
                    
                    InsQueue.DeleteMin();
                    RBUpdate++;
                }
            }
            else if (RBIndex < 0){
                //cout << "RBINDex < 0" << endl; 
                if (Temp.StartClock + RowAD == Clock){
                //    cout << "inside" << endl;
                    RBIndex = (location) / 1024;
                    cout <<endl<< "Cycle " << Clock - RowAD + 1 << " - " << Clock  << " -> Activating Row index "<< RBIndex << endl << endl;
                    InsQueue.v.at(0).StartClock = Clock;
                    Temp.StartClock = Clock;
                    RBUpdate++;
                }
            }

            else {
                RBIKey[RBIndex] = 0;
                if (Temp.StartClock + RowAD == Clock){
                    cout << endl << "Cycle " << Clock - RowAD + 1 << " - " << Clock << " -> Writeback Row Buffer to DRAM" << endl << endl; 
                    
                }
                else if (Temp.StartClock + 2*RowAD == Clock){
                    RBIndex = location / 1024;
                    cout <<endl<< "Cycle " << Clock - RowAD + 1 << " - " << Clock  << " -> Activating Row index "<< RBIndex << endl << endl;
                    InsQueue.v.at(0).StartClock = Clock;
                    Temp.StartClock = Clock;
                    RBUpdate++;
                }

            }
        }

        if (InsQueue.isEmpty()){
            //cout << "InsQueue is empty " << endl;
            MaxKey = 0;
        }

        //cout << "Outside" << endl;

    	//cout<<"\n\n";
    }
    
    //pause();


    // Output memory
	//cout<<"\nMEMORY (AFTER EXECUTION)";
    //printMemory(Memory,lastPC);

    
    if (RBIndex >= 0 && stat_tot+RowAD <= M){
        stat_tot++;

        cout << "Cycle " << stat_tot << " - " << stat_tot + RowAD -1 <<" -> Writeback Row buffer to Memory. (End of Execution)" << endl;

        stat_tot = stat_tot + RowAD -1;
    }

    // Output stats
    cout<<"\n\n\nTotal number of Clock Cycles: "<<stat_tot << endl;
    cout << "Total number of row buffer updates (new buffer row activation + modification of buffer row contents): " << RBUpdate << endl; 

    /*
    if (Break)
        cout << "\nSyntax Error in input file. Please rectify\n";
    */


    cout << endl; 

    return 0;
}