#include<stdio.h>
#include "9cc.h"

void gen_lval(Node *node) {
	if (node->ty == ND_IDENT) {
		printf("	mov rax, rbp\n");
		printf("	sub rax, %d\n", ('z' - node->name + 1) * 8);
		printf("	push rax\n");
		return;
	}
	fprintf(stderr, "代入の左辺値が変数ではありません: %c", node->ty);
}

void echo(Node *node) {
	switch (node->ty) {
		case ND_NUM:
			printf("// %d\n", node->val);
			break;
		case ND_IDENT:
			printf("// %c\n", node->name);
			break;
		case '=':
			printf("// =\n");
			break;
		case '+':
			printf("// +\n");
			break;

		case '-':
			printf("// -\n");
			break;
		case '*':
			printf("// *\n");
			break;
		case '/':
			printf("	sub rax, rdi\n");
	}
	if (node->lhs == NULL) {
		printf("// left NULL\n");
	}
	if (node->rhs == NULL) {
		printf("// right NULL\n");
	}
}

void gen(Node *node) {
	echo(node);
	// 式の最初は数でなければならないので、それをチェックして
	if (node->ty == ND_NUM) {
		printf("	push %d\n", node->val);
		return;
	}

	if (node->ty == ND_IDENT) {
		gen_lval(node);
		printf("	pop rax\n");
		printf("	mov rax, [rax]\n");
		printf("	push rax\n");
		return;
	}

	if (node->ty == '=') {
		gen_lval(node->lhs);
		gen(node->rhs);

		printf("	pop rdi\n");
		printf("	pop rax\n");
		printf("	mov [rax], rdi\n");
		printf("	push rdi\n");
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("	pop rdi\n");
	printf("	pop rax\n");

	switch (node->ty) {
		case '+':
			printf("	add rax, rdi\n");
			break;

		case '-':
			printf("	sub rax, rdi\n");
			break;
		case '*':
			printf("	mul rdi\n");
			break;
		case '/':
			// divは暗黙のうちにRDXとRAXを取って、それを連結したものを128ビット整数とみなして、
			// それを引数のレジスタの64ビットの値で割り、その結果をRAXにセットするという仕様
			printf("	mov rdx, 0\n");
			printf("	div rdi\n");
	}

	printf("	push rax\n");
}
