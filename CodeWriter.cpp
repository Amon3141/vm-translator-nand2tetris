#include <iostream>
#include <fstream>

#include "Parser.h"
#include "CodeWriter.h"

using namespace std;

// @param fileName: *.vm
// opens the file and prepare the output file
CodeWriter::CodeWriter(string fileName) {
  setFileName(fileName);
  ofile << getSPInitializeAssembly() << "\n";
}

// @input filaName: directry/filename.vm
// set the current file name
void CodeWriter::setFileName(string fileName) {
  fileName = fileName.substr(fileName.find('/')+1);
  fileName = fileName.substr(0, fileName.find("."));
  this->fileName = fileName;
  string ofileName = "asm_files/" + fileName + ".asm";
  cout << "output file: " << ofileName << endl;
  // !!! if ofile is currently open, should I close it?
  ofile.open(ofileName);
}

// @input command: arithmetic command ("add", "eq", etc.)
// translate arithmetic code to assembly code
void CodeWriter::writeArithmetic(string command) {
  string assembly;
  if (command == "add" || command == "sub") {
    assembly = getAddSubAssembly(command);
  } else if (command == "neg") {
    assembly = getNegAssembly();
  } else if (command == "and" || command == "or") {
    assembly = getAndOrAssembly(command);
  } else if (command == "not") {
    assembly = getNotAssembly();
  } else if (command == "eq" || command == "gt" || command == "lt") {
    assembly = getEqGtLtAssembly(command);
  } else {
    cout << "invalid arithmetic command" << endl;
  }
  ofile << assembly << "\n";
}

// translate pushcommand to assembly code
void CodeWriter::writePush(string segment, int index) {
  string assembly;
  if (segment == "constant") {
    assembly = getPushConstantAssembly(index);
  } else if (segment == "static") {
    assembly = getPushStaticAssembly(index);
  } else {  // segment == "local", "argument", "this", "that", "pointer", "temp"
    assembly = getPushSegmentAssembly(segment, index);
  }
  ofile << assembly << "\n";
}

// translate pop command to assembly code
void CodeWriter::writePop(string segment, int index) {
  string assembly;
  if (segment == "static") {
    assembly = getPopStaticAssembly(index);
  } else {  // segment == "local", "argument", "this", "that", "pointer", "temp"
    assembly = getPopSegmentAssembly(segment, index);
  }
  ofile << assembly << "\n";
}

// close the streams
void CodeWriter::endWriting() {
  ofile << getEndInfiniteLoopAssembly();
  ofile.close();
}


//--------Prvate--------

string CodeWriter::getSPInitializeAssembly() {
  string assembly =
  "// set SP (RAM[0]) = 256\n"
  "@256\n"
  "D=A\n"
  "@SP\n"
  "M=D\n";
  return assembly;
}

string CodeWriter::getAfterTwoArgsAssembly() {
  string assembly =
  "// SP = SP - 1\n"
  "@SP\n"
  "M=M-1\n"
  "\n"
  "// RAM[SP] = 0\n"
  "@0\n"
  "D=A\n"
  "@SP\n"
  "A=M\n"
  "M=D\n";
  return assembly;
}

string CodeWriter::getEndInfiniteLoopAssembly() {
  string assembly =
  "// infinite loop\n"
  "(END)\n"
  "@END\n"
  "0;JMP\n";
  return assembly;
}

string CodeWriter::getPushConstantAssembly(int x) {
  string assembly = "// push constant x\n";
  assembly += "@" + to_string(x) + "\n";
  assembly +=
  "D=A\n"
  "@SP\n"
  "A=M\n"
  "M=D\n"
  "@SP\n"
  "M=M+1\n";
  return assembly;
}

string CodeWriter::getPushSegmentAssembly(string segment, int x) {
  string assembly = "// push segment x\n";

  if (segment == "local" || segment == "argument" || segment == "this" || segment == "that") {
    assembly += "@" + segToSymbol[segment] + "\n";
    assembly += "D=M\n";
  } else if (segment == "pointer" || segment == "temp") {
    if (segment == "pointer") assembly += "@3\n";
    else assembly += "@5\n";
    assembly += "D=A\n";
  }

  assembly += "@" + to_string(x) + "\n";
  assembly += 
  "A=D+A\n"
  "D=M\n"
  "@SP\n"
  "A=M\n"
  "M=D\n"
  "@SP\n"
  "M=M+1\n";
  return assembly;
}

string CodeWriter::getPopSegmentAssembly(string segment, int x) {
  string assembly = "// pop segment x\n";

  if (segment == "local" || segment == "argument" || segment == "this" || segment == "that") {
    assembly += "@" + segToSymbol[segment] + "\n";
    assembly += "D=M\n";
  } else if (segment == "pointer" || segment == "temp") {
    if (segment == "pointer") assembly += "@3\n";
    else assembly += "@5\n";
    assembly += "D=A\n";
  }

  assembly += "@" + to_string(x) + "\n";
  assembly +=
  "D=D+A\n"
  "@R13\n"
  "M=D\n"
  "\n"
  "@SP\n"
  "A=M-1\n"
  "D=M\n"
  "\n"
  "@R13\n"
  "A=M\n"
  "M=D\n";
  assembly += "\n" + getAfterTwoArgsAssembly();
  return assembly;
}

string CodeWriter::getPushStaticAssembly(int x) {
  string assembly = "// push static x\n";
  assembly += "@" + this->fileName + "." + to_string(x) + "\n";
  assembly +=
  "D=M\n"
  "@SP\n"
  "A=M\n"
  "M=D\n"
  "@SP\n"
  "M=M+1\n";
  return assembly;
}

string CodeWriter::getPopStaticAssembly(int x) {
  string assembly = "// pop static x\n";
  assembly +=
  "@SP\n"
  "A=M-1\n"
  "D=M\n";
  assembly += "@" + this->fileName + "." + to_string(x) + "\n";
  assembly += "M=D\n";
  assembly += "\n" + getAfterTwoArgsAssembly();
  return assembly;
}

// @require (command == "add" || command == "sub")
string CodeWriter::getAddSubAssembly(string command) {
  if (!(command == "add" || command == "sub")) {
    cout << "command should be ADD or SUB" << endl;
    return "";
  }

  string assembly;
  if (command == "add") assembly += "//add\n";
  else if (command == "sub") assembly += "//sub\n";

  assembly +=
  "@SP\n"
  "A=M\n"
  "A=A-1\n"
  "D=M\n"
  "A=A-1\n";

  if (command == "add") assembly += "D=M+D\n";
  else if (command == "sub") assembly += "D=M-D\n";
  else return "";

  assembly += "M=D\n";
  assembly += "\n" + getAfterTwoArgsAssembly();
  return assembly;
}

string CodeWriter::getNegAssembly() {
  string assembly =
  "// neg\n"
  "@SP\n"
  "A=M\n"
  "A=A-1\n"
  "M=-M\n";
  return assembly;
}

// @require (command == "eq" || command == "gt" || command == "lt")
string CodeWriter::getEqGtLtAssembly(string command) {
  if (!(command == "eq" || command == "gt" || command == "lt")) {
    cout << "command should be EQ or GT or LT" << endl;
    return "";
  }

  string assembly;
  if (command == "eq") assembly += "//eq\n";
  else if (command == "gt") assembly += "//gt\n";
  else if (command == "lt") assembly += "//lt\n";

  assembly +=
  "@SP\n"
  "A=M\n"
  "A=A-1\n"
  "D=M\n"
  "A=A-1\n"
  "D=M-D\n"
  "@" + this->fileName + ".TRUE" + to_string(symbolRound) + "\n";

  if (command == "eq") assembly += "D;JEQ\n";
  else if (command == "gt") assembly += "D;JGT\n";
  else if (command == "lt") assembly += "D;JLT\n";
  else return "";
  
  assembly +=
  "@" + this->fileName + ".FALSE" + to_string(symbolRound) + "\n"
  "0;JMP\n"
  "(" + this->fileName + ".TRUE" + to_string(symbolRound) + ")\n"
  "@0\n"
  "D=!A\n"
  "@" + this->fileName + ".ENDIF" + to_string(symbolRound) + "\n"
  "0;JMP\n"
  "(" + this->fileName + ".FALSE" + to_string(symbolRound) + ")\n"
  "@0\n"
  "D=A\n"
  "("+ this->fileName + ".ENDIF" + to_string(symbolRound) + ")\n"
  "@SP\n"
  "A=M\n"
  "A=A-1\n"
  "A=A-1\n"
  "M=D\n";
  assembly += "\n" + getAfterTwoArgsAssembly();

  symbolRound++;
  return assembly;
}

// @require (command == "and" || command == "or")
string CodeWriter::getAndOrAssembly(string command) {
  if (!(command == "and" || command == "or")) {
    cout << "command should be AND or OR" << endl;
    return "";
  }

  string assembly;
  if (command == "and") assembly += "//and\n";
  else if (command == "or") assembly += "//or\n";

  assembly +=
  "@SP\n"
  "A=M\n"
  "A=A-1\n"
  "D=M\n"
  "A=A-1\n";

  if (command == "and") assembly += "D=D&M\n";
  else if (command == "or") assembly += "D=D|M\n";

  assembly += "M=D\n";
  assembly += "\n" + getAfterTwoArgsAssembly();
  return assembly;
}

string CodeWriter::getNotAssembly() {
  string assembly =
  "//not\n"
  "@SP\n"
  "A=M\n"
  "A=A-1\n"
  "M=!M\n";
  return assembly;
}