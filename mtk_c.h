/***************************************************************
 * mtk_c.h : カーネルの定数・型・大域変数・関数宣言
 *           (テキスト 2.5.2 節 p.32-35, 図 2.7 p.34)
 *   担当: データ構造（          ）
 *
 *   mtk_c.c と test2.c の両方から #include する．
 *   大域変数は mtk_c.c でだけ実体を置き，ほかのファイルでは extern にする
 *   (p.31: #ifdef〜#else〜#endif で切り替える)．
 *     mtk_c.c :  #define MTK_C_BODY  してから  #include "mtk_c.h"
 *     test2.c :  そのまま #include "mtk_c.h"
 *
 *   【注意】ここを変えるときは班に一声かける（全員のコードに効く）．
 *           TCB_TYPE のフィールドの順番・型は mtk_asm.s の
 *           オフセット (TCB_SIZE, TCB_STACK_PTR) と必ず合わせる．
 ***************************************************************/
#ifndef MTK_C_H
#define MTK_C_H

#ifdef MTK_C_BODY
#define EXTERN                  /* mtk_c.c: 実体を置く */
#else
#define EXTERN extern           /* ほかのファイル: extern 宣言 */
#endif

/***************
 * 定数 (p.35)
 ***************/
#define NULLTASKID   0          /* キューの終端．タスク ID には使わない */
#define NUMTASK      5          /* 最大タスク数 (p.35: 5 くらいで十分) */
#define NUMSEMAPHORE 5          /* TODO: 2.5.7 のテストが入る数に決める */
#define STKSIZE      1024       /* スタック 1 本の大きさ (p.35: 1K バイトほど) */

/* TCB の使用状態 (status) の記号定数 (p.35)．TODO: 班で決める */
#define UNDEFINED    0          /* 未使用 */
/* #define USED      1 */
/* #define FINISHED  2 */

/***************
 * 型 (p.33-35)
 ***************/
typedef int TASK_ID_TYPE;

/* TCB (Task Control Block)．添字 = タスク ID (0 番は使わない) */
typedef struct {
    void (*task_addr)();        /* タスク関数の先頭番地 */
    void *stack_ptr;            /* 中断時の SSP (asm から +4 で参照) */
    int priority;               /* 優先度 (テーマ2 では使わない) */
    int status;                 /* 使用状態 (UNDEFINED など) */
    TASK_ID_TYPE next;          /* 同じキューで次に並ぶタスクの ID */
} TCB_TYPE;

/* セマフォ (p.33, 2.5.7 p.43) */
typedef struct {
    int count;                  /* 1 以上: 空き / 0: 使用中 / 負: 待ちあり */
    int nst;                    /* 予約 (2.5.8 の waitP で使う) */
    TASK_ID_TYPE task_list;     /* このセマフォを待つタスクのキュー (先頭 ID) */
} SEMAPHORE_TYPE;

/* タスクスタック．ID が id のタスクは stacks[id-1] を使う (p.32) */
typedef struct {
    char ustack[STKSIZE];       /* ユーザスタック */
    char sstack[STKSIZE];       /* スーパバイザスタック */
} STACK_TYPE;

/***************
 * 大域変数 (p.32)．curr_task, next_task, task_tab は asm からも見る
 ***************/
EXTERN TASK_ID_TYPE   curr_task;                /* 実行中のタスク */
EXTERN TASK_ID_TYPE   new_task;                 /* 登録作業中のタスク */
EXTERN TASK_ID_TYPE   next_task;                /* 次に実行するタスク */
EXTERN TASK_ID_TYPE   ready;                    /* ready キュー (先頭 ID) */
EXTERN SEMAPHORE_TYPE semaphore[NUMSEMAPHORE];
EXTERN TCB_TYPE       task_tab[NUMTASK + 1];
EXTERN STACK_TYPE     stacks[NUMTASK];

/***************
 * mtk_c.c の関数 (2.5.2 節)
 ***************/
void init_kernel(void);
void set_task(void (*func)());
void *init_stack(TASK_ID_TYPE id);
void begin_sch(void);
void addq(TASK_ID_TYPE *q, TASK_ID_TYPE id);
TASK_ID_TYPE removeq(TASK_ID_TYPE *q);
void sched(void);
void sleep(int ch);
void wakeup(int ch);
void p_body(int id);
void v_body(int id);

/***************
 * mtk_asm.s のルーチン (2.5.3 節)
 ***************/
extern void first_task(void);
extern void pv_handler(void);
extern void P(int id);
extern void V(int id);
extern void swtch(void);
extern void hard_clock(void);
extern void init_timer(void);
/* extern void skipmt(void); */ /* オプション 2.5.6 */

#endif /* MTK_C_H */
