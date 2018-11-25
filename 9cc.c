#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "9cc.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	// トークナイズしてパースする
	tokenize(argv[1]);
	program();

	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	// プロローグ
	// 変数２６個分の領域を確保する
	printf("	push rbp\n");
	printf("	mov rbp, rsp\n");
	printf("	sub rsp, 208\n");

	for (int i = 0; code[i]; i++) {
		fprintf(stderr, "code: %d\n", i);
		printNode(code[i]);
		fprintf(stderr, "\n\n");

		// 抽象構文木を下りながらコード生成
		gen(code[i]);

		// スタックトップに式全体の値が残っているはずなので
		// それをRAXにロードして関数からの返り値とする
		printf("	pop rax\n");
	}

	// エピローグ
	// 最後の式の結果がRAXに残っているのでそれが返り血になる
	printf("	mov rsp, rbp\n");
	printf("	pop rbp\n");
	printf("	ret\n");
	return 0;
}
