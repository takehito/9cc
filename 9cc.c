#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void error(char *msg, char *result);

// トークンの方を表す値
enum {
	TK_NUM = 256, // 整数トークン
	TK_IDENT,	// 識別子トークン
	TK_EOF, // 入力の終わりを表すトークン
};

// トークンの型
typedef struct  {
	int ty; // トークンの型
	int val; // tyがTK_NUMの場合、その数値
	char *input; // トークン文字列（エラーメッセージ用）
} Token;

// トークナイズした結果のトークン列はこの配列に保存する
// １００個以上のトークンは来ないものとする
Token tokens[100];

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p) {
	int i = 0;
	while (*p) {
		// 空白文字をスキップ
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (*p == '=' || *p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == ';') {
			tokens[i].ty = *p;
			tokens[i].input = p;
			i++;
			p++;
			continue;
		}

		if (isdigit(*p)) {
			tokens[i].ty = TK_NUM;
			tokens[i].input = p;
			tokens[i].val = strtol(p, &p, 10);
			i++;
			continue;
		}

		if ('a' <= *p && *p <= 'z') {
			tokens[i].ty = TK_IDENT;
			tokens[i].input = p;
			i++;
			p++;
			continue;
		}

		error("トークナイズできません： %s\n", p);
		exit(1);
	}

	tokens[i].ty = TK_EOF;
	tokens[i].input = p;
}

// エラーを報告するための関数
void error(char *msg, char *result) {
	fprintf(stderr, msg, result);
	exit(1);
}

enum {
	ND_NUM = 256,	// 整数のノードの型
	ND_IDENT,	// 変数のノードの型
};

typedef struct Node {
	int ty;			// 演算子かND_NUM
	struct Node *lhs;	// 左辺
	struct Node *rhs;	// 右辺
	int val;		// tyがND_NUMの場合のみ使う
	char name;		// tyがND_IDENTの場合のみ使う
} Node;

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

Node *mul();
Node *term();
Node *expr();

int pos = 0;

Node *assign() {
	Node *lhs = expr();
	if (tokens[pos].ty == TK_EOF)
		return lhs;
	if (tokens[pos].ty == '=') {
		pos++;
		return new_node('=', lhs, assign());
	}

	return assign();
}

Node *expr() {
	Node *lhs = mul();
	if (tokens[pos].ty == TK_EOF || tokens[pos].ty == ')' || tokens[pos].ty == '=')
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
	if (tokens[pos].ty == TK_EOF || tokens[pos].ty == '+' || tokens[pos].ty == '-' || tokens[pos].ty == ')' || tokens[pos].ty == '=')
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

void gen_lval(Node *node) {
	if (node->ty == ND_IDENT) {
		printf("	mov rax, rbp\n");
		printf("	sub rax, %d\n", ('z' - node->name + 1) * 8);
		printf("	push rax\n");
		return;
	}
	fprintf(stderr, "代入の左辺値が変数ではありません");
}

void gen(Node *node) {
	// 式の最初は数でなければならないので、それをチェックして
	if (node->ty == ND_NUM) {
		printf("	push %d\n", node->val);
		return;
	}

	if (node->ty == ND_IDENT) {
		gen_lval(node);
		printf("	pop rax\n");
		printf("	move rax, [rax]\n");
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

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	// トークナイズしてパースする
	tokenize(argv[1]);
	Node* node = assign();

	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	// プロローグ
	// 変数２６個分の領域を確保する
	printf("	push rbp\n");
	printf("	mov rbp, rsp\n");
	printf("	sub rsp, 208\n");

	// 抽象構文木を下りながらコード生成
	gen(node);

	// スタックトップに式全体の値が残っているはずなので
	// それをRAXにロードして関数からの返り値とする
	printf("	pop rax\n");

	// エピローグ
	// 最後の式の結果がRAXに残っているのでそれが返り血になる
	printf("	mov rsp, rbp\n");
	printf("	pop rbp\n");
	printf("	ret\n");
	return 0;
}
