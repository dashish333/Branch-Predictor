/* CS assignment 4 : Desgining Branch Predictors 

Intructor: Dr.Manu Awasthi
Institute : IIT Gandhinagar

By Ashish Dwivedi 

*/
#include<iostream>
#include<fstream>
#include<cstring>
#include<string>
#include<bitset>
#include<algorithm>
#include<vector>

using namespace std;

string type; // stores the type of predcitor we will be dealing

int main(int argc,char* argv[])
{
  int m,n,lsb; //m => Global register width
               // n=> BHT entry width
			   //lsb => to store the first LSB's branchPC   
  long long branchPC; //  stores the hex address from trace file
  int PC;             // stores the PC which maps to BHT location
  char isTaken;       // stores the branch T/NT from trace file
  int prediction;     // store 1 for T and 0 for NT
  long misPred=0;     // count number of misses
  long totalPC=0;     // stores total number of instruction
  bitset<16> bs;     

  
ifstream file(argv[1]);
type=argv[2];
char* token;

// 2nd command line input:seperating m & n;
  char* str = new char[type.length()];
  strcpy(str,type.c_str());  
  token=strtok(str,",");
  int m_n[2],i=0;
  
  while(token!=NULL)
  { 
    m_n[i]=atoi(token);
    i++;
    token=strtok(NULL,",");
  } 
  m=m_n[0]; // Global History register width
  n=m_n[1]; // BHT entry width
  
//************************************************************
cout<<"Please enter the max number of LSB's from branchPC: ";
cin>>lsb;

if(m >= 16 || n > 16 || lsb > 16 )
{ cerr<<"Wrong Input, please try again:Exiting!";
  return 0;
}

// Actual Task begins here***********************************
else
{ 
if(m==0)
    { 
	  unsigned long size=pow(2,lsb); //size of BHT 
	  
	  long ulimit=pow(2,n);  // maximum numer of value that can be represnted by n bits
	  
	  long ubound = (ulimit/2)-1; // upper bound for maximu where "NT" exist
	  long lbound = ulimit/2;     //lower bound for last "T" exist 
	  
	  
	  long low=0;           // lowest value using n bits : unsigned
	  long high=ulimit-1;   // highest value that can be represented using n bits
	  
	  int* BHT=new int[size](); // size of BHT on the basis of lsb of PC
      int* access = new int[size](); // to check which all entries visited atleast once in BHT
      string temp;
      
      while(!file.eof())
  
       {
  		file >> hex >> branchPC;
  		if(file.fail()){break;}
  		
  		file >> isTaken;
		if(file.fail()){break;}  
		
        //**********Scaling down to required number of bits from branchPC*********************
		bs=branchPC;
        temp=bs.to_string();
        temp=temp.substr(16-lsb,lsb);
        bitset<16> btemp(temp);
		        
        //*********************Scaling Ends and we ahve first n LSB's of PC*********************************************
		
		PC=btemp.to_ulong();
        if(isTaken == 'T')
          { 
		    long val=BHT[PC]; 
            access[PC]+=1;
            
            if( lbound<=val && val <= high) //hit: correction prediction
			    {  
			       BHT[PC]=std::min(high,(val + 1));
			 	}
            else //miss: wrong Prediction: Branch actually taken but predicted as not taken
            { BHT[PC]=std::min(high,val + 1);
              misPred+=1;
			  access[PC]+=1; 
			}
            
		  }
        else if(isTaken == 'N') 
  			{ 
  			long val=BHT[PC]; 
            access[PC]+=1;
            
            if( val >= low && val <=ubound) //hit: Prediction -> NT , Branch Behaviour->NT
			    { 
			       BHT[PC]=std::max(low,val-1);
			 	}
            else //miss: Prediction -> T , Branch Behaviour->NT
            	{ BHT[PC]=std::max(low,val-1);
              	  misPred+=1;
              	  access[PC]+=1;
				} 				
  				
			}
		  totalPC++;
		  
		  }
		  //**********************calculatine the number of entry used*************************
		   int util=0;
		   for(int i=0;i<1024;i++)
		     { if(access[i] > 0){ util++;}
			 }
		  //*********************************end of calculating util*********************************	 
		  cout<<"MisPrediction rate for ("<<m<<","<<n<<") predictor & for file "<<argv[1]<<":"<<(float)misPred/totalPC<<endl;
		  //cout<<"Number of Entries Accessed: "<<util;
          delete[]BHT;
		  delete[]access;
	}
  // Predictor with history
  else
    {  
	  //********************finding the actual size of BHT*****************
	   unsigned long size=pow(2,(lsb+m));
	  
	  // ***************BHT entry range calculation**********************************************
	  long ulimit=pow(2,n);  // maximum no of value can be represented by n bits
	  
	  long ubound = (ulimit/2)-1; // for largest NT prediction 
	  long lbound = ulimit/2;     //for lowest T prediction
	  
	  long low=0;
	  long high=ulimit-1;
	  
	  int* BHT=new int[size](); // size of BHT on the basis of lsb of PC
      int* access = new int[size](); 
      string temp;
      bitset<15>gHistory=0;
      
      while(!file.eof())
  
       {
  		file >> hex >> branchPC;
  		if(file.fail()){break;}
  		
  		file >> isTaken;
		if(file.fail()){break;}  
		
        //**********Scaling down to required number of bits from branchPC*********************
		bs=branchPC;
        temp=bs.to_string();
        temp=temp.substr(16-lsb,lsb);
        
        //**********Scaling down to required number of bits from branchPC**********************
        
		string gtemp=gHistory.to_string();
        gtemp=gtemp.substr(15-m,m);
                
        bitset<31> btemp(temp+gtemp);
        
        //*********************Scaling Ends and we have first n LSB's of PC concatenated with m bits of global register*********************************************
		
		PC=btemp.to_ulong();
		
        if(isTaken == 'T')
          { gHistory<<=1;
		    gHistory.set(0,1); // modifying global history
		    long val=BHT[PC];  // taking the prediction at PC int BHT 
            access[PC]+=1;     // the above PC has been accessed and thus access++
            
            if( lbound<=val && val <= high ) //hit
			    {  
			       BHT[PC]=std::min(high,(val + 1));
			 	}
            else //miss
            { BHT[PC]=std::min(high,val + 1);
              misPred+=1;
			  access[PC]+=1; 
			}
            
		  }
        else if(isTaken == 'N')
  			{ gHistory<<=1;
			  gHistory.set(0,0); 
  			  long val=BHT[PC]; 
              access[PC]+=1;
            
            if( val >= low && val <=ubound)
			    { 
			       BHT[PC]=std::max(low,val-1);
			 	}
            else
            	{ BHT[PC]=std::max(low,val-1);
              	  misPred+=1;
              	  access[PC]+=1;
				} 				
  				
			}
		  totalPC++;
		  
		  }
		  //**********************calculatine the number of entry used*************************
		int util=0;
		for(int i=0;i<1024;i++)
		     { if(access[i] > 0){ util++;}
			 }
		  //*********************************end of calculating util*********************************	 
		  cout<<"MisPrediction rate for ("<<m<<","<<n<<") predictor & for file "<<argv[1]<<":"<<(float)misPred/totalPC<<endl;
		  //cout<<"Number of Entries Accessed: "<<util;
		  delete[]BHT;
		  delete[]access;
    
    }
	file.close();	
}  
	
	
}

//**************************************END*********************************
