#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// プロトタイプ宣言
void error(char *msg, char *input);

// トークンの方を表す値
enum {
	TK_NUM = 256, // 整数トークン
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

		if (*p == '+' || *p == '-') {
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

		fprintf(stderr, "トークナイズできません： %s\n", p);
		exit(1);
	}

	tokens[i].ty = TK_EOF;
	tokens[i].input = p;
}

// エラーを報告するための関数
void error(char *msg, char *input) {
	fprintf(stderr, msg, input);
	exit(1);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	// トークナイズする
	tokenize(argv[1]);

	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	// 式の最初は数でなければならないので、それをチェックして
	// 最初のMOV命令を出力
	if (tokens[0].ty != TK_NUM)
		error("予期せぬトークンです：%s\n", tokens[0].input);
	printf("	mov rax, %d\n", tokens[0].val);

	// `+ <数>`あるいは`- <数>`というトークンの並びを消費しつつ
	// 左遷ぶりを出力
	int i = 1;
	while (tokens[i].ty != TK_EOF) {
		if (tokens[i].ty == '+') {
			i++;
			if (tokens[i].ty != TK_NUM)
				error("予期せぬトークンです：%s\n", tokens[i].input);
			printf("	add rax, %d\n", tokens[i].val);
			i++;
			continue;
		}

		if (tokens[i].ty == '-') {
			i++;
			if (tokens[i].ty != TK_NUM)
				error("予期せぬトークンです：%s\n", tokens[i].input);
			printf("	sub rax, %d\n", tokens[i].val);
			i++;
			continue;
		}

		error("予期せぬトークンです：%s\n", tokens[i].input);
	}

	printf("	ret\n");
	return 0;
}

enum {
	ND_NUM = 256, // 整数のノードの型
};

typedef struct Node {
	int ty; // 演算子かND_NUM
	struct Node *lhs; // 左辺
	struct Node *rhs; // 右辺
	int val; // tyがND_NUMの場合のみ使う
} Node;

Node *expr();
Node *mul();
Node *term();

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

int pos = 0;

// mul | mul "+" expr | mul "-" expr
Node *expr() {
	Node *lhs = mul();
	if (tokens[pos].ty == TK_EOF)
		return lhs;
	if (tokens[pos].ty == '+') {
		pos++;
		return new_node('+', lhs, expr());
	}
	if (tokens[pos].ty == '-') {
		pos++;
		return new_node('-', lhs, expr());
	}
	error("想定しないトークンです: %s", tokens[pos].input);
}

Node *mul() {
	Node *lhs = term();
	if (tokens[pos].ty == TK_EOF)
		return lhs;
	if (tokens[pos].ty == '*') {
		pos++;
		return new_node('*', lhs, mul());
	}
	if (tokens[pos].ty == '/') {
		pos++;
		return new_node('/', lhs, mul());
	}
	error("想定しないトークンです: %s", tokens[pos].input);
}

