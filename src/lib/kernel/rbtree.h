#include <stdbool.h>
#include <stddef.h>

#ifndef __always_inline
#define __always_inline inline __attribute__((always_inline))
#endif

#define READ_ONCE(x) (*(volatile typeof(x)* )&(x))

#define WRITE_ONCE(x, val) \
    do { *(volatile typeof(x) *)&(x) = (val); } while (0)

#define RB_EMPTY_ROOT(root)  (READ_ONCE((root)->rb_node) == NULL)
#define RB_EMPTY_NODE(node)  (READ_ONCE((node)->rb_parent_color) == NULL)

//노드를 가지는 구조체의 주소
#define rb_node_entry(NODE_ELEM, STRUCT, MEMBER)           \
        ((STRUCT* ) ((uint8_t* ) &(NODE_ELEM)->rb_left     \
                     - offsetof (STRUCT, MEMBER.rb_left)))

//노드의 색
#define	RB_RED		0
#define	RB_BLACK	1

//하위 2비트를 0으로한 부모노드의 주소 
#define rb_parent(pc)    ((struct rb_node* )(pc & ~3))

//하위 2비트를 이용한 노드의 색구분
#define rb_color(pc)     ((pc) & 1)
#define rb_is_black(pc)  rb_color(pc)
#define rb_is_red(pc)    (!rb_color(pc))

struct rb_node{
  unsigned long rb_parent_color;
  struct rb_node* rb_left;
  struct rb_node* rb_right;
}__attribute__((aligned(sizeof(long))));

struct rb_root {
	struct rb_node* rb_node;
	struct rb_node* rb_leftmost;
	struct rb_node* rb_rightmost;
};

extern void rb_init(struct rb_root*);
extern struct rb_node* rb_next(const struct rb_node* );
extern struct rb_node* rb_prev(const struct rb_node* );
extern struct rb_node* rb_first(const struct rb_root* );
extern struct rb_node* rb_last(const struct rb_root* );

extern void rb_insert(struct rb_node* node, struct rb_root* root);
extern void rb_erase(struct rb_node* parent, struct rb_root* root);

static inline void
rb_change_child(struct rb_node* old, struct rb_node* new_node,
		  struct rb_node* parent, struct rb_root* root)
{
	if (parent) {
		if (parent->rb_left == old){
			WRITE_ONCE(parent->rb_left, new_node);
		}
		else{
			WRITE_ONCE(parent->rb_right, new_node);
		}
	} else{
		WRITE_ONCE(root->rb_node, new_node);
	}
}

static inline void rb_set_parent(struct rb_node* rb, struct rb_node* p)
{
	//부모노드의 주소값 + rb노드의 색 = rb노드의 부모노드만 바꿈
	rb->rb_parent_color = rb_color(rb->rb_parent_color) + (unsigned long)p;
}

//rb의 부모를 p로 설정하고 색을 color로 설정
static inline void rb_set_parent_color(struct rb_node* rb,
				       struct rb_node* p, int color)
{
	rb->rb_parent_color = (unsigned long)p + color;
}

//부모가 NULL이 아니라면 부모의 자식을 새 노드로 설정
//부모가 NULL == old노드가 루트라면 새 노드를 root로 설정
static inline void
rb_change_child(struct rb_node* old, struct rb_node* new_node,
		  struct rb_node* parent, struct rb_root* root)
{
	if (parent) {
		if (parent->rb_left == old){
			WRITE_ONCE(parent->rb_left, new_node);
		}
		else{
			WRITE_ONCE(parent->rb_right, new_node);
		}
	} else{
		WRITE_ONCE(root->rb_node, new_node);
	}
}