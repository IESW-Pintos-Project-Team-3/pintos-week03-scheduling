#include <rbtree.h>

/*
 * 일단 CASE는 무시해도 됩니다
 * CASE 1 : 삼촌 노드가 RED
 * CASE 2 : 삽입 노드가 부모노드의 오른쪽 자식
 * CASE 3 : 삽입 노드가 부모노드의 왼쪽 자식
 */

void
rb_init(struct rb_root* root)
{
    root->rb_node = root->rb_leftmost = root->rb_rightmost = NULL;
}

//트리에서 가장 왼쪽에 있는 노드 = 가장 작은 노드
struct rb_node*
rb_first(const struct rb_root *root)
{
	struct rb_node	*n;

	n = root->rb_node;
	if (!n){
		return NULL;
    }

	while (n->rb_left){
		n = n->rb_left;
    }
	return n;
}

//트리에서 가장 오른쪽에 있는 노드 = 가장 큰 노드
struct rb_node*
rb_last(const struct rb_root *root)
{
	struct rb_node	*n;

	n = root->rb_node;
	if (!n){
		return NULL;
    }

	while (n->rb_right){
		n = n->rb_right;
    }
	return n;
}

//트리에서 자신 다음으로 작은 노드, 자신보다 큰 노드 중 가장 작은 노드
struct rb_node*
rb_next(const struct rb_node *node)
{
	struct rb_node *parent;

	if (RB_EMPTY_NODE(node)){
		return NULL;
    }

	/*
	 * 오른쪽 자식이 있으면 오른쪽 자식부터
	 * 왼쪽으로 갈 수 있을 때까지
	 */
	if (node->rb_right) {
		node = node->rb_right;
		while (node->rb_left){
			node = node->rb_left;
        }
		return (struct rb_node *)node;
	}

	/*
	 * 오른쪽 자식이 없는 경우
     * 올라가는 노드가 왼쪽 자식이 될 때까지 올라감
     * 노드가 부모노드의 왼쪽 자식이면 노드부터 아래까지는
     * 부모노드보다 다 작은 노드들이기 때문
     * 그 위는 부모노드보다 크건 작건 상관없음
	 */
	while ((parent = rb_parent(node)) && node == parent->rb_right){
		node = parent;
    }

	return parent;
}

//트리에서 자신보다 작은 노드 중 가장 큰 노드
struct rb_node*
rb_prev(const struct rb_node *node)
{
	struct rb_node *parent;

	if (RB_EMPTY_NODE(node)){
		return NULL;
    }

	/*
	 * 왼쪽 자식노드가 있는 경우
	 * 왼쪽 노드의 가장 오른쪽에
	 */
	if (node->rb_left) {
		node = node->rb_left;
		while (node->rb_right)
			node = node->rb_right;
		return (struct rb_node *)node;
	}

	/*
	 * 왼쪽 자식이 없는 경우
	 * 노드가 부모노드의 오른쪽 자식이 될 때까지 올라감
     * 노드가 부모노드의 오른쪽 자식이면 노드부터 아래까지는
     * 부모노드보다 다 큰 노드들이기 때문
     * 그 위는 부모노드보다 크건 작건 상관없음
	 */
	while ((parent = rb_parent(node)) && node == parent->rb_left){
		node = parent;
    }

	return parent;
}

static inline void
rb_set_black(struct rb_node* rb)
{
    rb->rb_parent_color += RB_BLACK
}

static inline void
rb_set_red(struct rb_node* rb)
{
    rb->rb_parent_color += RB_RED
}


static inline void
rb_get_parent(struct rb_node* node)
{
    return (struct rb_node*)node->rb_parent_color;
}

static inline void
rb_rotate_set_parents(struct rb_node *old, struct rb_node *new,
			struct rb_root *root, int color)
{
	struct rb_node *parent = rb_parent(old);
	new->__rb_parent_color = old->__rb_parent_color;
	rb_set_parent_color(old, new, color);
	rb_change_child(old, new, parent, root);
}

//삽입될 위치에 노드를 삽입한 후 호출
void
rb_insert(struct rb_node* node, struct rb_node* root)
{
    struct rb_node* parent = rb_get_parent(node);

    while(true){
        if(!parent){
            /*
             * 삽입한 노드가 루트노드
             * 노드를 검은색으로 칠하고 종료
             */
            rb_set_parent_color(node, NULL, RB_BLACK);
            break;
        }

        if(rb_is_black((unsigned long)parent)){
            //삽입한 노드의 부모가 검은색 노드이면 리밸런싱 필요없음
            break;
        }

        struct rb_node* gparent = rb_get_parent(parent);
        struct rb_node* tmp = gparent->rb_right;
        if(tmp != parent){ //부모노드 != tmp, 부모노드가 조부모의 왼쪽 자식
            if(tmp && rb_is_red(tmp)){
                /*
                 * CASE 1 -> Recoloring
                 */
                rb_set_black(tmp);
                rb_set_black(parent);
                rb_set_red(gparent);
                
                node = gparent;
                parent = rb_get_parent(node);
                continue;
            }
            /*
             * 삼초노드가 NULL이면 검은노드로 간주
             * 그렇기 때문에 위의 조건문의 조건이 거짓이라면
             * 삼촌노드가 검은색이거나 NULL
             */
            tmp = parent->rb_right;
            if(node == tmp){ //삽입노드가 부모의 오른쪽 자식인 경우
                /* 
                 * LR CASE to LL CASE
                 *
                 *        G             G
                 *       / \           / \
                 *      P   U   -->   n   U
                 *       \           /
                 *        n         P
                 */
                tmp = node->rb_left;
                WRITE_ONCE(parent->rb_right, tmp);
				WRITE_ONCE(node->rb_left, parent);
				if (tmp){
					rb_set_parent_color(tmp, parent, RB_BLACK);
                }
				rb_set_parent_color(parent, node, RB_RED);
				//augment_rotate(parent, node);
				parent = node;
				tmp = node->rb_right;
            }
            
            /*
             * LL CASE
             *
             *       G           P
             *      / \         / \
             *     P   U  -->  n   G
             *    /                 \
             *   n                   U
             *  
             */
            WRITE_ONCE(gparent->rb_left, tmp); /* == parent->rb_right */
			WRITE_ONCE(parent->rb_right, gparent);
			if (tmp){
				rb_set_parent_color(tmp, gparent, RB_BLACK);
            }
			rb_rotate_set_parents(gparent, parent, root, RB_RED);
			//augment_rotate(gparent, parent);
			break;

        }
        else{//부모노드 = tmp, 부모노드가 조부모의 오른쪽 자식
            tmp = gparent->rb_left;
            if (tmp && rb_is_red(tmp)) {
				//CASE 1 -> Recoloring
				rb_set_black(tmp);
                rb_set_black(parent);
                rb_set_red(gparent);
                
                node = gparent;
                parent = rb_get_parent(node);
                continue;
			}
            
            tmp = parent->rb_left;
            if(node == tmp){
                /* 
                 * RL CASE to RR CASE
                 *
                 *        G             G
                 *       / \           / \
                 *      U   P   -->   U   n
                 *         /               \
                 *        n                 P
                 */
                tmp = node->rb_right;
                WRITE_ONCE(parent->rb_left, tmp);
				WRITE_ONCE(node->rb_right, parent);
				if (tmp){
					rb_set_parent_color(tmp, parent, RB_BLACK);
                }
				rb_set_parent_color(parent, node, RB_RED);
				//augment_rotate(parent, node);
				parent = node;
				tmp = node->rb_left;
            }

            WRITE_ONCE(gparent->rb_right, tmp); /* == parent->rb_left */
			WRITE_ONCE(parent->rb_left, gparent);
			if (tmp){
				rb_set_parent_color(tmp, gparent, RB_BLACK);
            }
			rb_rotate_set_parents(gparent, parent, root, RB_RED);
			//augment_rotate(gparent, parent);
			break;
        }

    }
}

static inline void
rb_erase_color(struct rb_node* parent, struct rb_root* root)
{
	struct rb_node* node = NULL;
	struct rb_node* sibling;
	struct rb_ndoe* tmp1;
	struct rb_ndoe* tmp2;

	while (true) {
		sibling = parent->rb_right;
		if (node != sibling) {
			if (rb_is_red((unsigned long) sibling)) {
				/*
				 * 삭제된 노드의 형제노드가 오른쪽 노드 && RED
				 * 삭제된 노드의 형제노드를 BLACK으로 만들어줌
                 * 
				 *     P               S
				 *    / \             / \
				 *   N   S    -->    P  SR
				 *      / \         / \
				 *     SL  SR      N  SL
				 */
				tmp1 = sibling->rb_left;
				WRITE_ONCE(parent->rb_right, tmp1);
				WRITE_ONCE(sibling->rb_left, parent);
				rb_set_parent_color(tmp1, parent, RB_BLACK);
				rb_rotate_set_parents(parent, sibling, root, RB_RED);
				//augment_rotate(parent, sibling);
				sibling = tmp1;
			}
			tmp1 = sibling->rb_right;
			if (!tmp1 || rb_is_black(tmp1)) { //형제노드의 오른쪽 자식이 BLACK
				tmp2 = sibling->rb_left;
				if (!tmp2 || rb_is_black(tmp2)) { 
					/*
					 * 형제노드의 자식이 둘다 BLACK
					 * 형제노드를 RED로 만들고 부모노드를 BLACK 칠함
					 *
					 *     P             P
					 *    / \           / \
					 *   N   B    -->  N   R
					 *      / \           / \
					 *     B   B         B   B
					 *
					 */
					rb_set_parent_color(sibling, parent, RB_RED);
                    /* 부모노드가 RED면 형제노드 경로의 BLACK을 공유하게 바꾼 것이기 때문에
                     * 부모만 BLACK으로 칠하면 rebalancing 종료
                     * 부모노드가 원래 BLACK이였다면 문제를 부모노드에게 전가한 것이기 때문에
                     * 부모노드를 삭제노드로 간주하고 다시 rebalancing
                     */
					if (rb_is_red(parent))
						rb_set_black(parent);
					else {
						node = parent;
						parent = rb_parent(node);
						if (parent){
							continue;
                        }
					}
					break;
				}
				/*
				 * 형제노드의 왼쪽 자식은 RED인 경우 
				 * 형제노드를 RED로 칠하고 오른쪽으로 회전
                 * (형제노드의 오른쪽 자식이 RED인 경우로 만들기)
                 * RED, RED는 밑에서 해결
				 *
				 *    P             P               P 
				 *   / \           / \             / \
				 *  N   S    -->  N   R      -->  N   S
				 *     / \             \               \ 
				 *    R   B             R              SR
				 *                       \
				 *                        B
				 *
				 *
				 *    P              SL
				 *   / \            /  \
				 *  N   SL   -->   P    S
				 *       \        /      \
				 *        S      N       SR
				 *         \
				 *         SR
				 */
				tmp1 = tmp2->rb_right;
				WRITE_ONCE(sibling->rb_left, tmp1);
				WRITE_ONCE(tmp2->rb_right, sibling);
				WRITE_ONCE(parent->rb_right, tmp2);
				if (tmp1){
					rb_set_parent_color(tmp1, sibling, RB_BLACK);
                }
				//augment_rotate(sibling, tmp2);
				tmp1 = sibling;
				sibling = tmp2;
			}
			/*
			 * 형제노드를 기준으로 부모노드 방향으로 회전
			 * 부모노드, 형제노드의 오른쪽 자식 노드를 BLACK으로 칠함
			 */
			tmp2 = sibling->rb_left;
			WRITE_ONCE(parent->rb_right, tmp2);
			WRITE_ONCE(sibling->rb_left, parent);
			rb_set_parent_color(tmp1, sibling, RB_BLACK);
			if (tmp2){
				rb_set_parent(tmp2, parent);
            }
			rb_rotate_set_parents(parent, sibling, root, RB_BLACK);
			//augment_rotate(parent, sibling);
			break;
		} else { //형제노드가 부모노드의 왼쪽 자식
			sibling = parent->rb_left;
			if (rb_is_red((unsigned long)sibling)) {
                //위에서 한 rebalancing 좌우반전?
				tmp1 = sibling->rb_right;
				WRITE_ONCE(parent->rb_left, tmp1);
				WRITE_ONCE(sibling->rb_right, parent);
				rb_set_parent_color(tmp1, parent, RB_BLACK);
				rb_rotate_set_parents(parent, sibling, root, RB_RED);
				//augment_rotate(parent, sibling);
				sibling = tmp1;
			}
			tmp1 = sibling->rb_left;
			if (!tmp1 || rb_is_black(tmp1)) {
				tmp2 = sibling->rb_right;
				if (!tmp2 || rb_is_black(tmp2)) {
                    //위에서 한 rebalancing 좌우반전?
					rb_set_parent_color(sibling, parent, RB_RED);
					if (rb_is_red(parent)){
						rb_set_black(parent);
                    }
					else {
						node = parent;
						parent = rb_parent(node);
						if (parent){
							continue;
                        }
					}
					break;
				}
				//위에서 한 rebalancing 좌우반전?
				tmp1 = tmp2->rb_left;
				WRITE_ONCE(sibling->rb_right, tmp1);
				WRITE_ONCE(tmp2->rb_left, sibling);
				WRITE_ONCE(parent->rb_left, tmp2);
				if (tmp1){
					rb_set_parent_color(tmp1, sibling, RB_BLACK);
                }
				//augment_rotate(sibling, tmp2);
				tmp1 = sibling;
				sibling = tmp2;
			}
			//위에서 한 rebalancing 좌우반전?
			tmp2 = sibling->rb_right;
			WRITE_ONCE(parent->rb_left, tmp2);
			WRITE_ONCE(sibling->rb_right, parent);
			rb_set_parent_color(tmp1, sibling, RB_BLACK);
			if (tmp2){
				rb_set_parent(tmp2, parent);
            }
			rb_rotate_set_parents(parent, sibling, root, RB_BLACK);
			//augment_rotate(parent, sibling);
			break;
		}
	}
}

void
rb_erase(struct rb_node *node, struct rb_root *root)
{
	if (root->rb_leftmost == node){
		root->rb_leftmost = rb_next(node);
	}
	
	if (root->rb_rightmost == node){
		root->rb_leftmost = rb_prev(node);
	}

	struct rb_node* child = node->rb_right;
	struct rb_node* tmp = node->rb_left;
	struct rb_node* parent;
	struct rb_node* rebalance;
	unsigned long pc;

	if (!tmp) {
		/*
		 * 왼쪽 자식이 없는 경우
		 * 오른쪽 자식도 없거나 오른쪽 자식은 있음
		 */
		pc = node->rb_parent_color;
		parent = rb_parent(pc);
		rb_change_child(node, child, parent, root);
		if (child) {
			/*
			 * 오른쪽 자식이 있으면 오른쪽 자식은 RED, 본인은 BLACK
			 * RED가 BLACK의 색을 물려받기 때문에 rebalancing 필요없음
			 */
			child->rb_parent_color = pc;
			rebalance = NULL;
		} else
		    /*
			 * BLACK이 하나 없어지는 것이기 때문에 rebalancing 필요
			 * 이때 삭제되는 노드가 BLACK이라는 것은 형제노드가 NIL이 무조건 아니라는 것
			 * 그렇게 때문에 parent는 무조건 한쪽 자식이 있음
			 */
			rebalance = rb_is_black(pc) ? parent : NULL;
	} else if (!child) {
		/* 
		 * 오른쪽 자식은 없고 왼쪽 자식만 있는 경우
		 * 왼쪽 자식이 삭제노드의 색, 부모노드를 물려받음
		 */
		tmp->rb_parent_color = pc = node->rb_parent_color;
		parent = rb_parent(pc);
		rb_change_child(node, tmp, parent, root);
		rebalance = NULL;
	} else {
		/*
		 * 자식이 둘다 있는 경우
		 * 오른쪽 서브트리에서 가장 왼쪽에 있는 노드를 선택
		 * 자식이 둘다 있으면 가능한 경우
		 *    B    |     B    |    B    |    R
		 *   / \   |    / \   |   / \   |   / \
		 *  B   B  |   R   B  |  B   R  |  B   B
 		 */
		struct rb_node* successor = child;
		struct rb_node* child2;

		tmp = child->rb_left;
		if (!tmp) {
			/* 오른쪽 자식이 왼쪽 자식이 없는 경우
			 * 
			 *
			 *    (n)          (s)
			 *    / \          / \
			 *  (x) (s)  ->  (x) (c)
			 *        \
			 *        (c)
			 */
			parent = successor;
			child2 = successor->rb_right;

			//augment->copy(node, successor);
		} else {
			/*
			 * 오른쪽 자식이 왼쪽 자식이 있는 경우
			 * 가장 왼쪽 아래에 있는 노드를 선택
			 *
			 *    (n)          (s)
			 *    / \          / \
			 *  (x) (y)  ->  (x) (y)
			 *      /            /
			 *    (p)          (p)
			 *    /            /
			 *  (s)          (c)
			 *    \
			 *    (c)
			 */
			do {
				parent = successor;
				successor = tmp;
				tmp = tmp->rb_left;
			} while (tmp);
			child2 = successor->rb_right;
			WRITE_ONCE(parent->rb_left, child2);
			WRITE_ONCE(successor->rb_right, child);
			rb_set_parent(child, successor);

			//augment->copy(node, successor);
			//augment->propagate(parent, successor);
		}

		tmp = node->rb_left;
		WRITE_ONCE(successor->rb_left, tmp);
		rb_set_parent(tmp, successor);

		pc = node->rb_parent_color;
		tmp = rb_parent(pc);
		rb_change_child(node, successor, tmp, root);
        
		if (child2) {
			/* 
			 * 가장 왼쪽 아래 노드(successor)가 오른쪽 자식노드가 있으면
			 * child2의 부모를 설정하고 노드의 색을 검은색으로 설정
			 * successor의 왼쪽 노드는 NULL(BLACK)
			 * successor의 오른쪽 자식이 BLACK이면 BLACK높이가 맞을 수 없음 -> 무조건 RED
			 * successor가 RED면 이중 RED -> succesor는 무조건 BLACK
			 * RED가 BLACK을 물려받으니 rebalancing 필요없음
			 */
			rb_set_parent_color(child2, parent, RB_BLACK);
			rebalance = NULL;
		} else {
			/*
			 * successor를 대체할 RED노드 없음
			 * successor가 BLACK이면 BLACK이 하나 없어지는 것
			 * successor의 부모부터 rebalancing 필요
			 */
			rebalance = rb_is_black((unsigned long)successor) ? parent : NULL;
		}
		successor->rb_parent_color = pc;
	}

	if (rebalance)
		rb_erase_color(rebalance, root);
}

/*
노드 대체를 쉽게 할 수 있을 까해서 일단 넣음
void
rb_replace_node(struct rb_node *victim, struct rb_node* new, struct rb_root* root)
{
	struct rb_node *parent = rb_parent(victim);

	//Copy the pointers/colour from the victim to the replacement 
	*new = *victim;

	// Set the surrounding nodes to point to the replacement
	if (victim->rb_left){
		rb_set_parent(victim->rb_left, new);
    }
	if (victim->rb_right){
		rb_set_parent(victim->rb_right, new);
    }
	rb_change_child(victim, new, parent, root);
}
*/
