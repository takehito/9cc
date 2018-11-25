typedef struct Node {
	int ty;			// 演算子かND_NUM
	struct Node *lhs;	// 左辺
	struct Node *rhs;	// 右辺
	int val;		// tyがND_NUMの場合のみ使う
	char name;		// tyがND_IDENTの場合のみ使う
} Node;

enum {
	ND_NUM = 256,	// 整数のノードの型
	ND_IDENT,	// 変数のノードの型
}; 

void gen(Node *node);
