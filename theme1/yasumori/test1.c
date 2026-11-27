#include <stdio.h>

/* 万が一 main() を抜けた際の暴走防止 (p.19 1.4.6節) */
void exit(int value) {
    *(volatile char *)0x00d00039 = 'H'; /*LED0に'H'を表示 */
    for (;;);                           /* 無限ループで停止 */
}

int main(void) {
    int count = 0;
    int a, b;
    char str[64];

    printf("=== test1 start ===\n");

    while (1) {
        count++;
        printf("\n--- Loop %d ---\n", count);

        /* 1. 文字列入力テスト (scanf/inbyte確認) */
        printf("Input a word: ");
        fflush(stdout);
        scanf("%63s", str);
        printf("Echo: [%s]\n", str);

        /* 2. 数値入力と計算出力テスト (scanf/printfのフォーマット確認) */
        printf("Input two integers (e.g. 10 20): ");
        fflush(stdout);
        scanf("%d %d", &a, &b);
        printf("Result: %d + %d = %d (hex: 0x%x)\n", a, b, a + b, a + b);

        /* 3. 連続出力による送信キュー負荷テスト (outbyteのリトライ確認) */
        printf("Queue load test: ");
        for (int i = 0; i < 10; i++) {
            printf("[%02d:ABCDEFGHIJKLMNOPQRSTUVWXYZ] ", i);
        }
        printf("\n");
    }

    return 0;
}
