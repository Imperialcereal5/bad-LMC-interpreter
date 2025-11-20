// LMC interpreter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <string>
#include <array>
#include <map>
using namespace std;
//typedef array<char, 4> instruction;
typedef pair<short, int> memory; 
/*memory is stored in opcode / operand pairs - 
in LMC, the opcode was a part of the number, because they were only 3 digits and operands went up to 2 digits. 
Since I'm using 32-bit ints, there isn't a simple n-digit representation that is both efficient and covers opcodes 0-9.
Hence, using a short for the opcode makes it easier to handle (also, data has an opcode of zero. A halt command
is represented by an empty piece of data (i.e { 0, 0 })*/
void add(int& acc, array<memory, 1024>& mem, int&  pc, int n) { //ADD operation
	acc += mem[n].second;
}
void sub(int& acc, array<memory, 1024>& mem, int&  pc, int n) { //SUB operation
	acc -= mem[n].second;
}
void lda(int& acc, array<memory, 1024>& mem, int&  pc, int n) { //LDA operation
	acc = mem[n].second;
}
void sta(int& acc, array<memory, 1024>& mem, int&  pc, int n) { //STA operation
	mem[n] = { 0, acc };
}
void bra(int& acc, array<memory, 1024>& mem, int&  pc, int n) { //BRA operation
	pc = n-1;
}
void brp(int& acc, array<memory, 1024>& mem, int&  pc, int n) { //BRP operation
	if (acc >= 0) {
		pc = n-1;
	}
}
void brz(int& acc, array<memory, 1024>& mem, int&  pc, int n) { //BRZ operation
	if (acc == 0) {
		pc = n-1;
	}
}
void inp(int& acc, array<memory, 1024>& mem, int&  pc, int n){ //INP operation
	try {
		cin >> acc;
	}
	catch (exception e){
		cout << e.what();
	}
}
void out(int& acc, array<memory, 1024>& mem, int&  pc, int n) { //OUT operation
	cout << acc << endl;
}
void otc(int& acc, array<memory, 1024>& mem, int&  pc, int n) { //OTC operation
	cout << (char)acc << endl;
}
void assemble(int& acc, array<memory, 1024>& mem, int& pc, map<string, int> opcodes){ //assembles the assembly program into memory.
//as an aside - it would likely be easier to execute the code directly from the assembly (barring maybe DAT statements).
//the reason that I seperate assembly and execution is to remain closer to the original Peter Higgins interpreter.
	map<string, int> named; //named locations
	map<string, array<int, 1024>> unresolvedNames; //calls for locations that haven't been defined yet - resolved at the end.
	string s;
	int dat1 = 0; //opcode
	int dat2 = 0; //operand
	ifstream reader("code.txt"); //assembly source file
	while (getline(reader, s)) { //there are 3 forms of instruction: xxx (i.e HLT); xxx xxx (i.e ADD 5 or end HLT); and xxx xxx xxx (i.e mult LDA 05)
		if (s != "") {
			if (s.find_first_of(" ") != -1) { //returns -1 if it doesn't exist (technically it doesn't but that's what it does when you cast it to int)
				int n = s.find_first_of(" ");
				string s1 = s.substr(0, n); 
				string s2 = s.substr(n + 1, s.length());
				if (s2.find_first_of(" ") != -1) {
					n = s2.find_first_of(" ");
					string s3 = s2.substr(n + 1, s2.length());
					string temp = s2;
					try { //determines whether the data is an address or a name
						dat2 = stoi(s3);
					}
					catch (exception e) { //assigns the value of already defined names and stores undefined names
						if (named.count(s3)) {
							dat2 = named[s3];
						}
						else if (!unresolvedNames.count(s3)){ 
							unresolvedNames[s3][0] = pc;
							dat2 = 0;
						}
						else { //in case names are needed more than once, the relevant addresses are stored in an array
							for (int i = 0; i < 1024;i++) {
								if (unresolvedNames[s3][i] == 0) {
									unresolvedNames[s3][i] = pc;
									break;
								}
							}
							dat2 = 0;
						}
					}
					string s2 = temp.substr(0, n);
					named[s1] = pc; //defines undefined names
					dat1 = opcodes[s2];
				}
				else {
					if (opcodes.count(s1)) {
						dat1 = opcodes[s1];
						try { // same reasoning as above
							dat2 = stoi(s2);
						}
						catch (exception e) {
							if (named.count(s2)) {
								dat2 = named[s2];
							}
							else if (!unresolvedNames.count(s2)) {
								unresolvedNames[s2][0] = pc;
								dat2 = 0;
							}
							else {
								for (int i = 0; i < 1024;i++) {
									if (unresolvedNames[s2][i] == 0) {
										unresolvedNames[s2][i] = pc;
										break;
									}
								}
								dat2 = 0;
							}
						}
					}
					else { //handles single-part operations that get their function from pre-defined operands
						named[s1] = pc;
						if (!s2.compare("HLT")) {
							dat1 = 0; dat2 = 0;
						}
						else if (!s2.compare("INP")) {
							dat1 = 9;
							dat2 = 1;
						}
						else if (!s2.compare("OUT")) {
							dat1 = 9;
							dat2 = 2;
						}
						else if (!s2.compare("OTC")) {
							dat1 = 9;
							dat2 = 22;
						}
					}
				}
			}
			else { //as above
				string s1 = s;
				if (!s1.compare("HLT")) {
					dat1 = 0; dat2 = 0;
				}
				else if (!s1.compare("INP")) {
					dat1 = 9;
					dat2 = 1;
				}
				else if (!s1.compare("OUT")) {
					dat1 = 9;
					dat2 = 2;
				}
				else if (!s1.compare("OTC")) {
					dat1 = 9;
					dat2 = 22;
				}
			}
			mem[pc] = { dat1, dat2 };
			pc++;
		}
	}
	for (auto name: unresolvedNames) { //handles unresolved names
		if (named.count(name.first)) {
			for (int i = 0; i < 1024; i++) {
				if (name.second[i] == 0) {
					break;
				}
				memory temp = mem[name.second[i]];
				mem[name.second[i]] = { temp.first, named[name.first] };
				
			}
		}
		else { //name got in that wasn't ever defined (you're cooked) 
			cout << "oh no bad thing happened";
		}
	}
	pc = 0;
}
int readMem(int addr, int& pc, int& acc, array<memory, 1024>& mem) { //executes instructions
	int dat = mem[addr].second;
	switch (mem[addr].first) {
	case 1:
		add(acc, mem, pc, dat);
		break;
	case 2:
		sub(acc, mem, pc, dat);
		break;
	case 3:
		sta(acc, mem, pc, dat);
		break;
	case 5:
		lda(acc, mem, pc, dat);
		break;
	case 6:
		bra(acc, mem, pc, dat);
		break;
	case 7:
		brp(acc, mem, pc, dat);
		break;
	case 8:
		brz(acc, mem, pc, dat);
	case 9:
		switch (dat) { //differentiates between INP, OUT, and OTC
		case 1:
			inp(acc, mem, pc, dat);
			break;
		case 2:
			out(acc, mem, pc, dat);
			break;
		case 22:
			otc(acc, mem, pc, dat);
			break;
		}
	case 0: //since the core logic checks whether PC is positive, making it a big negative basically halts the program
		if (dat == 0) {
			pc = -1024;
		}
	}
	return dat; //had a use but I scrapped it
	
	
}
int main()
{
	array<memory, 1024> mem; //virtual RAM (can be scaled up if necesarry)
	int acc;
	int pc = 0;
	string instructions[12] = {"HLT", "DAT", "ADD", "SUB", "STA", "LDA", "BRA", "BRP", "BRZ", "INP", "OUT", "OTC"};
	map<string, int> opcodes; //defines opcodes for future reference
	opcodes["HLT"] = opcodes["DAT"] = 0;
	opcodes["ADD"] = 1;
	opcodes["SUB"] = 2;
	opcodes["STA"] = 3;
	opcodes["LDA"] = 5;
	opcodes["BRA"] = 6;
	opcodes["BRP"] = 7;
	opcodes["BRZ"] = 8;
	opcodes["INP"] = opcodes["OUT"] = opcodes["OTC"] = 9;
	string s = "";
	assemble(acc, mem, pc, opcodes);
	while (pc >= 0 && pc < 1024) { //core program loop (kills itself on an HLT)
		readMem(pc,pc,acc, mem);
		pc++;
		
	}
	for (int i = 0; i < 1024; i++) { //outputs memory for debugging
		printf("%d: %d, %d\n", i, mem[i].first, mem[i].second); 
	}

	return 0;
}


/* sample program (definitely works (mostly))
INP
STA 00
INP
STA 01
LDA 02
SUB 02
STA 02
mult LDA 02
ADD 01
STA 02
LDA 00
SUB one
STA 00
BRP mult
LDA 02
SUB 01
OUT
one DAT 1*/