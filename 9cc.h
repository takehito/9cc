void error(char *msg, char *result);

// トークンの型
typedef struct  {
	int ty; // トークンの型
	int val; // tyがTK_NUMの場合、その数値
	char *input; // トークン文字列（エラーメッセージ用）
} Token;

// トークナイズした結果のトークン列はこの配列に保存する
// １００個以上のトークンは来ないものとする
Token tokens[100];

void tokenize(char *msg);

// トークンの方を表す値
enum {
	TK_NUM = 256, // 整数トークン
	TK_IDENT,	// 識別子トークン
	TK_EOF, // 入力の終わりを表すトークン
};

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

void printNode(Node *node);

extern Node *code[100]; 
void program();

void gen(Node *node);

typedef struct {
	void **data;
	int capacity;
	int len;
} Vector;

Vector *new_vector();
void vec_push(Vector *vec, void *elem);

typedef struct {
	Vector *keys;
	Vector *vals;
} Map;

Map *new_map();
void *map_get(Map *map, char* msg);
void map_put(Map *map, char *key, void *val);

void runtest(void);
