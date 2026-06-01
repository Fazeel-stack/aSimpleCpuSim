
#include <iostream>
#include<fstream>
#include <iomanip>
#include<string>
#include<unordered_map>
#include<array>
#include <bitset>
std::unordered_map<std::string, std::string> mainMap{ {"alu","000"},{"bgt","001"},{"addi","010"},{"store","100"},{"beq","101"},{"jmp","110"},{"mov","111"},{"load","011"}};
std::unordered_map<std::string, std::string> aluMap{ {"add","001"},{"sub","000"},{"and","011"},{"or","010"},{"cmp","100"},{"shr","101"},{"sfl","110"},{"xor","111"} };
std::unordered_map<std::string, std::string> regMap{ {"r0","000"},{"r1","001"},{"r2","010"},{"r3","011"},{"r4","100"},{"r5","101"},{"r6","110"},{"r7","111"} };
std::array<std::string, 8> aluNames{ "add", "sub", "and", "or", "cmp", "shr", "sfl", "xor" };
std::array<std::string, 4> mainNames{"addi", "store","mov", "load" };
std::array<std::string, 3> branchNames{ "bgt", "beq", "jmp"};

std::array<std::string, 8> regNames{ "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7" };
std::unordered_map<std::string, int> labelMapping;
int lineNo = 0;



std::string getAluRegisters(std::string line) {
    std::string rr=line.substr(0,2);
    std::string ra=line.substr(3,2);
    std::string rb=line.substr(6,2);
    bool rrb = false;
    bool rab = false;
    bool rbb = false;

    for (int i = 0;i < regNames.size();i++) {
        if (rr == regNames[i]) {
            rrb = true;
        }
        if (ra == regNames[i]) {
            rab = true;
        }
        if (rb == regNames[i]) {
            rbb = true;
        }

    }
    if (!rrb) {
        std::cout << rr << " Not Found Line a register : " << lineNo<<'\n';
        exit(-1);
    }
    if (!rab) {
        std::cout << ra << " Not Found Line a register : " << lineNo<< '\n';
        exit(-1);
    }
    if (!rbb) {
        std::cout << rb << " Not Found Line a register : " << lineNo<< '\n';
        exit(-1);
    }
    rr = regMap[rr];
    ra = regMap[ra];
    rb = regMap[rb];

    return rr + ra + rb;

}

std::string getImmediate(std::string line) {
    std::string reg = line.substr(0, 2);
    bool regb = false;
    for (int i = 0;i < regNames.size();i++) {
        if (reg == regNames[i]) {
            regb = true;
        }
    }
    if (!regb) {
        std::cerr << "Not Found the register line : " << lineNo << '\n';
        exit(-1);
    }
    int n = 3;
    char c = line[n];
    std::string number;
    while (c != ' ' && c != ';') {
        c = line[n];
        if (c != ' ' && c != ';') {
            number.push_back(c);
        }
        n++;
        
    }
    int numbern = std::stoi(number);
    number = std::bitset<26>(numbern).to_string();
    return regMap[reg]+number;

}

std::string getBranch(std::string line) {
    int n = 0;
    char c = line[n];
    std::string label;
    while (c != ':' && c!=' ') {
        label.push_back(c);
        n++;
        c = line[n];
    }

    if (labelMapping.find(label) == labelMapping.end()) {
        std::cerr << "Label "<<label<<" Not Found line no " << lineNo;
        exit(-1);
    }

    int address = labelMapping[label];
    std::string address_string = std::bitset<29>(address).to_string();
    return address_string;
}



std::string getInstruction(std::string line) {
    int n = 0;
    char c = line[n];
    std::string mainOp;
    std::string resIns ="0";
    //Main instrcution
    while (c != ' ' && c != ';') {
        c = line[n];
        if (c != ' ' && c != ';' && c!=':') {
            mainOp.push_back(c);
        }
        if (c == ':') {
            labelMapping[mainOp] = lineNo;
            return "label";
        }
        n++;
       
    }
    std::string lineSub = line.substr(n, line.size());
    //Alu Instruction
    for (int i = 0;i < aluNames.size();i++) {
        if (mainOp == aluNames[i]) {
            
            resIns = getAluRegisters(lineSub);
            lineNo++;
            return mainMap["alu"] + aluMap[mainOp] + resIns + "00000000000000000";
        }
        
    }
    //Immediate Instructions
    for (int i = 0;i < mainNames.size();i++) {
        if (mainOp == mainNames[i]) {
            resIns = getImmediate(lineSub);
            lineNo++;
            return mainMap[mainOp]+ resIns;
        }
    }
    //branch instruction
    for (int i = 0;i < branchNames.size();i++) {
        if (mainOp == branchNames[i]) {
            resIns = getBranch(lineSub);
            lineNo++;
            return mainMap[mainOp] + resIns;
        }
    }
}

int main()
{

    std::ifstream filo("c.txt");
    std::fstream file("b.txt", std::ios::out | std::ios::trunc);
    if (file.is_open()) {
        file << "v3.0 hex words addressed";
    }
    std::string line;
    int offSetLine = 0;
    while (std::getline(filo, line)) {
        std::string instruction = getInstruction(line);
        std::cout << line << " gap " << instruction << '\n';
        if ((lineNo-1) % 8 == 0 && instruction!="label") {
            
            file << '\n' << std::hex << std::setw(4) << std::setfill('0') << offSetLine << ":";
            offSetLine += 8;
        }

        if (instruction != "label") {
            file << ' ' << std::hex << std::stoul(instruction, nullptr, 2);
        }
       

        // << std::stoi(getInstruction(line)) << getInstruction(line).size();//<<std::hex<< std::stoi(getInstruction(line), nullptr, 2);
    }
}
