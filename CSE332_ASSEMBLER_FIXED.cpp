/*
 * Course: CSE332
 * Section 2
 * Assignment 02
 * Group no 04
 * Details is written in the report
*/
#include<bits/stdc++.h>
#include<debug.h>
using namespace std;
// global variable definitions
int bit = 3;
int opcode_size = 5;
int instruction_len = 14;
map<string,string>codes;
map<string,string>registors;
map<string, string>immediates;
map<string,pair<string,string>>types;
map<string,int>num_of_types;
map<string, int> jumps;
string line;
int instructon_cnt = 0;
int line_cnt = 0;
string output_file;
string input_file = "assembly.txt";
string hexa_code = "";

// defining functions
bool assembly_to_instruction(string);
void print_error(string,string,int);
vector<string> split_instruction(string);
void stdout_details();
void trim(string&);
void generate_valid_keywords();
void convert_to_hexa_code();
void print_all_instructions();
bool go_through_input_file();
bool write_into_output_file();
bool spacing(char);
bool recheck_for_labels();
bool isComment(string);
char first_char(string);
string binary_to_hexadecimal(string);
string twos_complement(string);
string int_to_binary(int);
string get_label(string);
string offset_to_binary(int);
string int_to_binary_9bit(int);

class instruction{
public:
	string binary_code;
	string assembly_code;
	int line_no;
	int instruction_no;
	string keyword;
	vector<string>words;
	instruction(){}
	instruction(vector<string>word, string bin, string inst, int l_no, int i_no, string key){
		words = word;
		binary_code = bin;
		assembly_code = inst;
		line_no = l_no;
		instruction_no = i_no;
		keyword = key;
	}
};

vector<instruction>instruction_store;
void print_instruction(instruction);
bool complete_instruction(instruction&);

// main function
int main(){	

	io();
	if(not go_through_input_file()){
		return 0;
	}
	if(not recheck_for_labels()){
		return 0;
	}
	convert_to_hexa_code();
	// print_all_instructions();
	if(not write_into_output_file()){
		return 0;
	}
	stdout_details();
}

// converts all instruction into hexadecimal
void convert_to_hexa_code(){
	for(int i = 0, j = 1; i < instruction_store.size(); ++i, ++j){
    	hexa_code += binary_to_hexadecimal(instruction_store[i].binary_code) + '\n';
    }
}

// writes result into output file
bool write_into_output_file(){
	output_file = input_file.substr(0, input_file.size()-4) + "_[machine_code].txt";
	ofstream out_file{
    	output_file
    };
    if(not out_file){
    	cout<< "Couldn't create or open file: "<<endl;
    	return false;
    }
    out_file << "v2.0 raw\n";
    out_file << hexa_code;
    out_file.close();
    return true;
}
void trim(string& str){
	int i = 0; 
	while(spacing(str[i]) and i < str.size()){
		++i;
	};
	if(i >= str.size()){
		str = "";
		return;
	}
	str = str.substr(i, string::npos);
	if(not str.size()){
		return;
	}
	while(spacing(str[str.size()-1])){
		str.pop_back();
		if(str.size() == 0){
			return;
		}
	} 
}
// goes through input file and returns false if any error occurred
bool go_through_input_file(){
	
	generate_valid_keywords();	
	
	ifstream ini_file{
		input_file
	};

    string machine_code = "";
    if (not ini_file) {
    	cout << "Couldn't open file: "<< endl;
       return false;
    }
    while (getline(ini_file, line)) {
		++line_cnt;
		trim(line);
    	if(isComment(line) or not line.size() or line == "\n" or line.size() < 2){
    		continue;
    	}
    	// label starts with a #
    	string label = "";
    	if(first_char(line) == '#'){
    		label = get_label(line);
    		jumps[label] = instructon_cnt;
    	}
    	int i = label.size();
 		while(spacing(line[i++])){}
 		line =  line.substr(i-1, string::npos);
 		// debug(line);
    	if(line.size() < 2){
    		continue;
    	}
    	if(not assembly_to_instruction(line)){
    		ini_file.close();
    		return false;
    	}
    	++instructon_cnt;
    }
    ini_file.close();
    return true;
}
bool recheck_for_labels(){
	for(int i = 0; i < instruction_store.size(); ++i){
 		if(not complete_instruction(instruction_store[i])){
 			return false;
 		}
    }
    return true;
}
string offset_to_binary(int n){
	n -= 1;
	if(abs(n) > 7){
		cout <<"Offset greater than 7 is found, this may cause error\n";
	}
	return int_to_binary(n);
}

bool complete_instruction(instruction& I){
	num_of_types[types.find(I.words[0]) -> second.first]++;
	I.binary_code = "";
	if(I.words[0] == "JUMP" or I.words[0] == "EQL" or I.words[0] == "NEQL" or I.words[0] == "INPT" or I.words[0] == "OUT") {
		if(I.words[0] == "JUMP"){
			if(jumps.find(I.words[1]) == jumps.end()){
				string error =  "JUMP destination not found! ";
				print_error(I.assembly_code, error, I.line_no);
				return false;
			}
			I.binary_code += codes[I.words[0]] + int_to_binary_9bit(jumps[I.words[1]]);
			return true;
		}
		else if(I.words[0] == "EQL" or I.words[0] == "NEQL" ){
			for(int i = 0; i < I.words.size()-1; ++i){
				I.binary_code += codes[I.words[i]];
			}
			if(jumps.find(I.words[3]) == jumps.end()){
			string error =  "Branch Destination not found !!";
			print_error(I.assembly_code, error, I.line_no);
			return false;
			}
			string offset = to_string(jumps[I.words[3]] - I.instruction_no - 1);
			if(immediates.find(offset) == immediates.end()){
				string error =  "Illegal offset '"+ offset+"' to reach " + (I.words[3]);
				print_error(I.assembly_code, error, I.line_no);
				return false;
			}
			I.binary_code += codes[offset];
			return true;
		}
		else{
			I.binary_code += codes[I.words[0]];
			I.binary_code += string(bit, '0');
			I.binary_code += codes[I.words[1]];
			I.binary_code += string(bit, '0');
			return true;
		}
		
	}

	for(int i = 0; i < I.words.size(); ++i){
		I.binary_code += codes[I.words[i]];
	}
	return true;
}

void print_instruction(instruction I){
	cout << "words: ";
	for(int i = 0; i < I.words.size(); ++i){
		cout << I.words[i] <<" ";
	}
	cout << endl;
	cout << "Binary code: " << I.binary_code <<endl;
	cout << "Assembly code: " << I.assembly_code <<endl;
	cout << "Line no: " << I.line_no <<endl;
	cout << "Instruction no: " << I.instruction_no <<endl;
	cout << "keyword: " << I.keyword<<endl;
}
void print_all_instructions(){
	cout  << endl;
	for(int i = 0; i < instruction_store.size(); ++i){
		cout <<"i: "<<i << "\n";
		print_instruction(instruction_store[i]);
		cout<<endl;
	}
	cout << endl;
}

bool valid_keyword(string str){
	return codes.find(str) != codes.end();
}

bool spacing(char c){
	return c == ' ' or c == 13 or c == '\n';
}
void print_error(string instruction, string error, int l_cnt){
		cout<< "Error at line " << l_cnt << ":\n" << instruction << endl << error << endl;
}
bool later(char c){
	return ('A' <= c and c <= 'Z')
	or ('a' <= c and c <= 'z')
	or ('0' <= c and c <= '9')
	or c == '_'
	or c == '#';

}
bool syntax_is_valid(string str){
	int n = str.size();
	bool f_r = false;
	for(int i = 0; i < n; ++i){
		if(not f_r and str[i] == ','){
			return false;
		}
		if(str[i] == ' '){
			str = str.substr(i, string::npos);
			break;
		}
	}
	// debug(str);
	n = str.size();
	for(int i = 0; i < n; ++i){
		if(str[i] == ',')
		{
			int j = i-1;
			bool found_leter = false; 
			while(j >= 0 ){
				if(later(str[j])){
					found_leter = true;
					break;
				}
				if(str[j] == ','){
					return false;
				}
				--j;
			}
			if(not found_leter){
				return false;
			}
			j = i+1;
			found_leter = false; 
			while(j < n ){
				if(later(str[j])){
					found_leter = true;
					break;
				}
				if(str[j] == ','){
					return false;
				}
				++j;
			}
			if(not found_leter){
				return false;
			}
		}
	}
	return true;
}
vector<string> split_instruction(string str){
	vector<string> words;
	trim(str);
	int n = str.size();
	int i = 0;
	string operation = "";
	while(str[i] != ' ' and i < n){
		operation += str[i++];
	}
	trim(operation);
	words.push_back(operation);
	// debug(operation);
	for(i = i; i < n; ++i){
		string operand = "";
		while(str[i] != ',' and i < n){
			operand += str[i];
			++i;
		}
		trim(operand);
		words.push_back(operand);
		// debug(operand);	
	}
	return words;
}
bool check_operands(vector<string> words){
	if(not words.size()){
		string error =  "NULL instruction";
		print_error(line, error, line_cnt);
		return false;
	}
	if(types.find(words[0]) == types.end()){
		string error =  "Invalid instruction: " + words[0];
		print_error(line, error, line_cnt);
		return false;
	}
	
	if(words[0] == "JUMP" or words[0] == "INPT" or words[0] == "OUT"){
		if(words.size() < 2){
			string error =   "Too few arguments for " + words[0];
			print_error(line, error, line_cnt);
			return false;
		}
		else if(words.size() > 2){
			string error =   "Too many arguments for " + words[0];
			print_error(line, error, line_cnt);
			return false;
		}
		if(words[0] != "JUMP" and registors.find(words[1]) == registors.end()){
			string error =   "Invalid argument '" + words[1] +"' for " + words[0];
			print_error(line, error, line_cnt);
			return false;
		}
		else if( words[0] == "JUMP" and words[words.size()-1][0] != '#'){
			string error =   "Invalid Label format '" + words[1] +"' for " + words[0];
			print_error(line, error, line_cnt);
			return false;
		}
		return true;
	}
	// debug(words);
	// debug((int)words.size());
	
	int i = words.size()-1;
	string format = types.find(words[0])->second.second;
	if(format == "R" and registors.find(words[i]) == registors.end() or words[i] == "ZERO"){
		string error =   "Invalid argument '" + words[i] +"' for " + words[0];
		print_error(line, error, line_cnt);
		return false;
	}
	else if(format == "I" and immediates.find(words[i]) == immediates.end() and not (words[0] == "EQL" or words[0] == "NEQL")){
		string error =   "Invalid argument '" + words[i] +"' for " + words[0];
		print_error(line, error, line_cnt);
		return false;
	}
	// debug(words);
	cerr << ((int)words.size()) << endl;;
	if(words[0] == "EQL" or words[0] == "NEQL"){
		if(words[words.size()-1][0] != '#'){
			string error =   "Invalid Label format '" + words[i] +"' for " + words[0];
			print_error(line, error, line_cnt);
			return false;
		}
	}
	for(i = 1; i < words.size()-1; ++i){
		if(registors.find(words[i]) == registors.end()){
			string error =   "Invalid argument '" + words[i] +"' for " + words[0];
			print_error(line, error, line_cnt);
			return false;
		}
	}
	if(words.size() < 4){
		string error =   "Too few arguments for " + words[0];
		print_error(line, error, line_cnt);
		return false;
	}
	else if(words.size() > 4){
		string error =   "Too many arguments for " + words[0];
		print_error(line, error, line_cnt);
		return false;
	}
	return true;
}
bool assembly_to_instruction(string str){
	if(not syntax_is_valid(str)){
		string error =  "Syntax Error !!!";
		print_error(line, error, line_cnt);
		return false;
	}
	vector<string> words = split_instruction(str);
	// debug(words);
	if(not check_operands(words)){
		return false;
	}
	instruction_store.push_back(instruction(words, "", line, line_cnt, instructon_cnt, ""));
	return true;
}

bool isComment(string str){
	return first_char(str) == ';';
}
char first_char(string str){
	for(int i = 0; i < str.size(); ++i){
		if(str[i] != ' '){
			return str[i];
		}
	}
	return '\0';
}
string get_label(string str){
	for(int i = 0; i < str.size(); ++i){
		string flag = "";
		while(!spacing(str[i]) and i < str.size()){
			flag += str[i];
			++i;
		}
		if(flag.size()){
			return flag;
		}
	}
	return "";
}
string int_to_binary(int n){
	string str = "";
	while(n>0){
		str += ((n%2) + '0');
		n = n >> 1;
	}
	n = str.size();
	for(int j = 0; j < bit - n; ++j){
		str += '0';
	}
	reverse(str.begin(), str.end());
	return str;
}
string int_to_binary_9bit(int n){
	string str = "";
	while(n>0){
		str += ((n%2) + '0');
		n = n >> 1;
	}
	n = str.size();
	for(int j = 0; j < 9 - n; ++j){
		str += '0';
	}
	reverse(str.begin(), str.end());
	return str;
}

string binary_to_hexadecimal(string str){
	reverse(str.begin(), str.end());
	string result = "";
	int n = str.size();
	for(int i = 0; i < n; i += 4){
		int hex = 0;
		for(int j = i, mul= 1; j < i+4 and j < n; ++j,  mul = mul << 1){
			hex += ((str[j] - '0')*mul);
		}
		if(hex > 9){
			result += (hex -10) + 'A';
		}
		else{
			result += hex + '0';
		}
	}
	reverse(result.begin(), result.end());
	return result;
}
void stdout_details(){
	cout << "Translation completed" << endl;
	cout << "Total instructions: " << instructon_cnt << endl;
	for(auto it : num_of_types){
		cout << it.first <<": " << it.second<<endl;
	}
}
void generate_valid_keywords(){

	types.insert({"ADD",{"Arithmetic", "R"}});
	types.insert({"ADDi",{"Arithmetic", "I"}});
	types.insert({"SUB",{"Arithmetic", "R"}});
	types.insert({"SUBi",{"Arithmetic", "I"}});
	types.insert({"AND",{"Logical", "R"}});
	types.insert({"OR",{"Logical", "R"}});
	types.insert({"XOR",{"Logical", "R"}});
	types.insert({"SLL",{"Logical", "I"}});
	types.insert({"SRL",{"Logical", "I"}});
	types.insert({"COMP",{"Conditional", "R"}});
	types.insert({"COMPi",{"Conditional", "I"}});
	types.insert({"EQL",{"Conditional", "I"}});
	types.insert({"NEQL",{"Conditional", "I"}});
	types.insert({"JUMP",{"Unconditional", "J"}});
	types.insert({"LOAD",{"Data Transfer", "I"}});
	types.insert({"STOR",{"Data Transfer", "I"}});
	types.insert({"INPT",{"I/O", "I"}});
	types.insert({"OUT",{"I/O", "I"}});

	/* OPCODES */
	codes.insert({"ADD","00000"});
	codes.insert({"ADDi","00001"});
	codes.insert({"SUB","00010"});
	codes.insert({"SUBi","00011"});
	codes.insert({"AND","00100"});
	codes.insert({"OR","00101"});
	codes.insert({"XOR","00110"});
	codes.insert({"SLL","00111"});
	codes.insert({"SRL","01000"});
	codes.insert({"COMP","01001"});
	codes.insert({"COMPi","01010"});
	codes.insert({"EQL","01011"});
	codes.insert({"NEQL","01100"});
	codes.insert({"JUMP","01101"});
	codes.insert({"LOAD","01110"});
	codes.insert({"STOR","01111"});
	codes.insert({"INPT","10000"});
	codes.insert({"OUT","10001"});
	/* Registers */
	codes.insert({"ZERO","000"});
	codes.insert({"R1","001"});
	codes.insert({"R2","010"});
	codes.insert({"R3","011"});
	codes.insert({"R4","100"});
	codes.insert({"R5","101"});
	codes.insert({"R6","110"});
	codes.insert({"R7","111"});

	//
	registors.insert({"ZERO","000"});
	registors.insert({"R1","001"});
	registors.insert({"R2","010"});
	registors.insert({"R3","011"});
	registors.insert({"R4","100"});
	registors.insert({"R5","101"});
	registors.insert({"R6","110"});
	registors.insert({"R7","111"});

	/* Numeric */
	codes.insert({"0", string(bit,'0')});
	immediates.insert({"0", string(bit,'0')});
	int max_val = round(pow(2,bit)) -1;
	for(int i = 1; i <= max_val; ++i){
		codes.insert({to_string(i), int_to_binary(i)});
		immediates.insert({to_string(i), int_to_binary(i)});
	}
}
// END OF CODE