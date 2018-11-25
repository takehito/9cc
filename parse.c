#include <stdlib.h>
#include <stdio.h>
#include "9cc.h"

// エラーを報告するための関数
void error(char *msg, char *result) {
	fprintf(stderr, msg, result);
	exit(1);
}

Node *new_node(int op, Node *lhs, Node *rhs) {
	Node *node = malloc(sizeof(Node));
	node->ty = op;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = malloc(sizeof(Node));
	node->ty = ND_NUM;
	node->val = val;
	return node;
}

Node *new_node_ident(char name) {
	Node *node = malloc(sizeof(Node));
	node->ty = ND_IDENT;
	node->name = name;
	return node;
}

void printNode(Node *node) {
	if (node == NULL)
		return;
	printNode(node->lhs);
	switch (node->ty) {
		case ND_IDENT:
			fprintf(stderr, "このノードは変数、変数名は%c\n", node->name);
			break;
		case ND_NUM:
			fprintf(stderr, "このノードは数値、値は%d\n", node->val);
			break;
		default:
			fprintf(stderr, "このノードは演算子: %c\n", node->ty);
	}
	printNode(node->rhs);
}

Node *assign();
Node *mul();
Node *term();
Node *expr();

int pos = 0;

Node *code[100];
int code_pos = 0;

void program() {
	code[code_pos++] = assign();

	if (tokens[pos].ty == TK_EOF) {
		code[code_pos] = NULL;
		return;
	}

	program();
}


Node *assign() {
	Node *lhs = expr();
	if (tokens[pos].ty == TK_EOF)
		return lhs;
	if (tokens[pos].ty == ';') {
		pos++;
		return lhs;
	}
	if (tokens[pos].ty == '=') {
		pos++;
		return new_node('=', lhs, assign());
	}
} 

Node *expr() {
	Node *lhs = mul();
	if (tokens[pos].ty == TK_EOF || tokens[pos].ty == ')' || tokens[pos].ty == '=' || tokens[pos].ty == ';')
		return lhs;
	if (tokens[pos].ty == '+') {
		pos++;
		return new_node('+', lhs,expr());
	}
	if (tokens[pos].ty == '-') {
		pos++;
		return new_node('-', lhs,expr());
	}
	error("想定しないトークンです： %s\n", tokens[pos].input);
}

Node *mul() {
	Node *lhs = term();
	if (tokens[pos].ty == TK_EOF || tokens[pos].ty == '+' || tokens[pos].ty == '-' || tokens[pos].ty == ')' || tokens[pos].ty == '=' || tokens[pos].ty == ';')
		return lhs;
	if (tokens[pos].ty == '*') {
		pos++;
		return new_node('*', lhs, mul());
	}
	if (tokens[pos].ty == '/') {
		pos++;
		return new_node('/', lhs, mul());
	}
	error("想定しないトークンです： %s\n", tokens[pos].input);
}

Node *term() {
	if (tokens[pos].ty == TK_NUM)
		return new_node_num(tokens[pos++].val);
	if (tokens[pos].ty == TK_IDENT)
		return new_node_ident(*tokens[pos++].input);
	if (tokens[pos].ty == '(') {
		pos++;
		Node *node = expr();
		if (tokens[pos].ty != ')')
			error("開き括弧に対応する閉じ括弧がありません: %s\n", tokens[pos].input);
		pos++;
		return node;
	}

	error("数値でも開き括弧でもない出ないトークンです: %s\n", tokens[pos].input);
}
