#include <iostream>
#include <vector>
#include <set>
#include <string>

class Lexem {
public:
	int id;
	int line_id;
	std::string lex_type;
	std::string value;

	Lexem(int _id, int _line_id, std::string _lex_type, std::string _value) {
		id = _id;
		line_id = _line_id;
		lex_type = _lex_type;
		value = _value;
	}
};

class SyntaxA {
private:
	std::vector<std::string> Words = { "program", "var", "integer", "real", "bool", "string", "begin", "end", "if", "then", "else", "while", "do", "true", "false" };
	std::vector<std::string> Delimiter = { ".", ";", ",", "(", ")" };
	std::vector<std::string> Operators = { "+", "-", "*", "/", "=", ">", "<" };
	std::vector<std::string> DoubleOperators = { "++", "--", "+=", "-=", "*=", "/=", "==", ">=", "<=" };
	std::vector<Lexem> Lexemes;

	std::string buf = "";
	int num_buf = 0, double_counter = 1;
	double double_buf = 0;
	int lex_id = 0;
	int line_id = 0;
	int line_prev_value = 0;
	enum States { S, NUM, DOUBLE, STR, OPER, DLM, FIN, ID, ER, ASGN, SINGLE_COM, MULTI_COM };
	enum Types {INTEGER, REAL, STRING, IDENTIFIER, WORDS, COMMENT, OPERATOR, DELIMITER, END};
	std::string TypesName[9] = { "Integer", "Real", "String", "Identifier", "Reserved word", "Comment", "Operator", "Delimiter", "End"};
	int state = S;

	std::pair<int, std::string> SearchLexem(std::vector<std::string> lexes) {
		auto ptr = std::find(lexes.begin(), lexes.end(), buf);
		if (ptr != lexes.end())
			return std::make_pair(std::distance(lexes.begin(), ptr), buf);
		else
			return std::make_pair(-1, "");
	}

public:

	void Analyze(std::string input) {
		int i = 0;
		while (state != FIN) {
			switch (state) {
			case S: {
				lex_id = i;
				if (input[i] == ' ' || input[i] == '\t' || input[i] == '\0' || input[i] == '\r')
					i++;
				else if (input[i] == '\n') {
					line_id++;
					i++;
					line_prev_value = i;
				}
				else if (isalpha(input[i])) {
					buf = "";
					buf += input[i];
					state = ID;
					i++;
				}
				else if (isdigit(input[i])) {
					state = NUM;
					num_buf = (int)(input[i] - '0');
					i++;
				}
				else if (input[i] == '{') {
					state = MULTI_COM;
					i++;
				}
				else if (input[i] == '/' && input[i + 1] == '/') {
					state = SINGLE_COM;
					i += 2;
				}
				else if (input[i] == ':') {
					state = ASGN;
					buf = "";
					buf += input[i];
					i++;
				}
				else if (input[i] == '.') {
					state = FIN;
					buf = ".";
					Lexem* lex = new Lexem(lex_id - line_prev_value, line_id, TypesName[END], buf);
					Lexemes.push_back(*lex);

				}
				else if (input[i] == '\'') {
					state = STR;
					i++;
					buf = "";
				}
				else {
					state = DLM;
				}
				break;
			}
			case ID: {
				if (isalnum(input[i])) {
					buf += input[i];
					i++;
				}
				else {
					state = S;
					std::pair<int, std::string> p = SearchLexem(Words);
					if (p.first == -1) {
						Lexem* lex = new Lexem(lex_id - line_prev_value, line_id, TypesName[IDENTIFIER], buf);
						Lexemes.push_back(*lex);
					}
					else {
						Lexem* lex = new Lexem(lex_id - line_prev_value, line_id, TypesName[WORDS], buf);
						Lexemes.push_back(*lex);
					}
				}
				break;
			}
			case NUM: {
				if (isdigit(input[i])) {
					num_buf = num_buf * 10 + (int)(input[i] - '0');
					i++;
				}
				else if (input[i] == '.') {
					double_buf = num_buf;
					i++;
					state = DOUBLE;
					double_counter = 10;
				}
				else if (input[i] == ' ' || input[i] == '\n' || input[i] == ';' || input[i] == ',' || input[i] == ')') {
					state = S;
					Lexem* lex = new Lexem(lex_id - line_prev_value, line_id, TypesName[INTEGER], std::to_string(num_buf));
					Lexemes.push_back(*lex);
				}
				else {
					state = ER;
				}
				break;
			}
			case DOUBLE: {
				if (isdigit(input[i])) {
					double_buf = double_buf + (double)(input[i] - '0') / double_counter;
					double_counter *= 10;
					i++;
				}
				else if (input[i] == ' ' || input[i] == ';' || input[i] == ',' || input[i] == ')') {
					state = S;
					Lexem* lex = new Lexem(lex_id - line_prev_value, line_id, TypesName[REAL], std::to_string(double_buf));
					Lexemes.push_back(*lex);
				}
				else if (input[i] == '\n') {
					state = S;
					Lexem* lex = new Lexem(lex_id - line_prev_value, line_id, TypesName[REAL], std::to_string(double_buf));
					Lexemes.push_back(*lex);
					line_prev_value = i + 1;
				}
				else {
					state = ER;
				}
				break;
			}
			case STR: {
				while (input[i] != '\'') {
					buf += input[i];
					i++;
				}
				Lexem* lex = new Lexem(lex_id - line_prev_value, line_id, TypesName[STRING], buf);
				Lexemes.push_back(*lex);
				i++;
				state = S;
			}
			case DLM: {
				state = S;
				buf = "";
				buf += input[i];
				i++;
				std::pair<int, std::string> p_del = SearchLexem(Delimiter);
				std::pair<int, std::string> p_oper = SearchLexem(Operators);
				if (p_del.first != -1) {
					Lexem* lex = new Lexem(lex_id - line_prev_value, line_id, TypesName[DELIMITER], buf);
					Lexemes.push_back(*lex);
				}
				else if (p_oper.first != -1) {
					state = OPER;
				}
				else {
					state = ER;
				}
				break;
			}
			case OPER: {
				std::string old_buf = buf;
				buf += input[i];
				std::pair<int, std::string> p_oper = SearchLexem(DoubleOperators);
				if (p_oper.first != -1) {
					Lexem* lex = new Lexem(lex_id - line_prev_value, line_id, TypesName[OPERATOR], buf);
					Lexemes.push_back(*lex);
					i++;
				}
				else {
					buf = old_buf;
					Lexem* lex = new Lexem(lex_id - line_prev_value, line_id, TypesName[OPERATOR], buf);
					Lexemes.push_back(*lex);
				}
				state = S;
				break;
			}
			case ASGN: {
				if (input[i] == '=') {
					buf = ":=";
					i++;
				}
				Lexem* lex = new Lexem(lex_id - line_prev_value, line_id, TypesName[OPERATOR], buf);
				Lexemes.push_back(*lex);
				state = S;
				break;
			}
			case SINGLE_COM: {
				if (input[i] == '\n')
				{
					state = S;
					line_id++;
				}
				i++;
				break;
			}
			case MULTI_COM: {
				if (input[i] == '}')
					state = S;
				if (input[i] == '\n')
				{
					line_id++;
					line_prev_value = i + 1;
				}
				i++;
				break;
			}
			case ER: {
				state = FIN;
				std::cout << std::endl << "Error at line " << line_id << " symbol " << lex_id << std::endl;
				
			}
			case FIN: {
				break;
			}
			}
		}
		for (int i = 0; i < Lexemes.size(); i++) std::cout << Lexemes[i].line_id << "	" << Lexemes[i].id << "	" << Lexemes[i].lex_type << "	" << Lexemes[i].value << std::endl;
	}
};

int main(int argc, char* argv[]) {
	SyntaxA s;
	s.Analyze(R"(program test;
	var a, b, c : integer;
begin
	c := a - b + 15;
	c++;
	str := 'abcawdawfawrSTRING))';
end.)");
	return 0;
}