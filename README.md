# ソフトウェア実験Ⅱ・Ⅲ：マルチタスクカーネル

MC68VZ328（68000 互換）ボードで動くマルチタスクカーネルを C 言語で作る、5 人の班のリポジトリ。

## はじめに

**このリポジトリの一番上がそのまま `mtk` フォルダ**。実験室の PC では `~/mtk` に clone する。

> [!WARNING]
> **`~/mtk` にはテーマ1 のファイル（自分の `mon.s`, `inchrw.s`, `outchr.s`, `test1.c` など）が残っているはず。** clone の前に名前を変えて退避する（`~/mtk` があると clone できない）。
> ```sh
> mv ~/mtk ~/mtk.theme1
> ```
> 退避したファイルは、clone した後に自分の `theme1/<名前>/` へコピーしてコミットしておく（[テーマ1](#テーマ1個人)）。

```sh
git clone https://github.com/KamiiAoto/softexp2-os.git ~/mtk   # 初回だけ（先に上の退避をする）
cd ~/mtk && git pull                                            # 作業の前に必ず
make test2                                                      # test2.abs ができる
m68k-term                                                       # C-\ c → xm test2.abs → c → 何かキー
```

- 準備のコマンドは要らない。`make test1`〜`test3` が必要なものを自動で用意する（[ビルドの仕組み](#ビルドの仕組み)）。
- `make` は実験室の PC でしか動かない。
- テキストのページ番号は `C-jikken_SW2_3_20241210.pdf` の印刷ページ。

## フォルダ構成

```
~/mtk/
├── mtk_c.h, mtk_c.c, mtk_asm.s, test2.c   テーマ2・3 のカーネル（班で 1 つ）
├── crt0.s, csys68k.c, inchrw.s, outchr.s  配布のひな形（テーマ3 で直す）
├── Makefile, Makefile.1〜3                ビルド設定
├── theme1/<名前>/                         テーマ1（個人）
└── dist/                                  配布ファイル（中身は変えない）
```

## テーマと担当

| テーマ | 内容 | 作業 | 置き場所 |
|---|---|---|---|
| テーマ1 | C ライブラリの移植（`inbyte`/`outbyte`） | 個人 | `theme1/<名前>/` |
| テーマ2 | マルチタスクカーネル | 班で分担 | 一番上 |
| テーマ3 共同開発部 | fd 対応・`fcntl` | 班 | 一番上 |
| テーマ3 個人開発部 | マルチタスクアプリ | 個人 | `user_<名前>.c` |

提出（p.3–4）: カーネルは班で 1 つ、user タスクは個人ごと。誰のファイルか分かるように、ファイル名に名前を入れる。

## 日程

| 日付 | 内容 |
|---|---|
| 11/27(金) | テーマ1 終了・テーマ2 開始（分担決定） |
| 12/10(木) | テーマ2 終了・テーマ3 開始 |
| **12/16(水)** | **テーマ1 レポート締切・試問** |
| 12/22(火) | テーマ2,3 試問の告示 |
| **1/7(木)** | **テーマ2,3 レポート締切・試問**（1–6 班 1/7、7–12 班 1/8） |

## テーマ2：何を作るか

### ゴール

複数の C の関数（**ユーザタスク**）を、切り替えながら同時に動かす**カーネル**を作る（テキスト 2.1 p.20）。
タスクが切り替わるきっかけは 2 つ。

1. **タイマ割込み**: 一定時間たったら次のタスクへ（時分割）
2. **セマフォの P**: 資源が取れなかったタスクは休眠し、V で起こされるまで待つ

完成すると、`test2.c` の 2 つ以上のタスクが交互に `printf` するようになる。

### しくみ（3 つの部品）

| 層 | 中身 |
|---|---|
| ユーザ（`test2.c`） | `main()`: `init_kernel` → `set_task` × n → `begin_sch`。タスク `task1()`, `task2()`, … は `P()` / `V()` を呼ぶ |
| カーネル：タスク管理 | TCB、ready キュー、タスクごとのスタック |
| カーネル：タイマ | `hard_clock`: 割込みのたびに今のタスクを ready の末尾へ戻し、次のタスクへ `swtch` |
| カーネル：セマフォ | `pv_handler` → `p_body` / `v_body` → `sleep` / `wakeup` → `swtch` |
| モニタ（配布の `mon.o`） | `trap #0` の入出力（GETSTRING / PUTSTRING）とタイマ設定（SET_TIMER） |

- **TCB**（タスクごとの管理表）に、中断したときのスタックの位置（SSP）を覚えておく
- **ready キュー**に、実行を待っているタスクが並ぶ。セマフォごとにも待ちキューがある
- タスクを切り替えるとき（`swtch`）は、今のタスクのレジスタを全部スタックに積み、次のタスクのスタックから全部戻す

### 作る関数（`mtk_c.c` と `mtk_asm.s` に雛形あり）

| 部品 | 関数 | 言語 | 何をするか | テキスト |
|---|---|---|---|---|
| データ構造 | `mtk_c.h` | C | TCB・セマフォ・スタックの型、定数、大域変数 | p.32–35 |
| キュー | `addq` / `removeq` | C | キューの末尾に入れる／先頭から取り出す | p.37 |
| | `sched` | C | ready の先頭を次に走るタスクにする | p.38 |
| タスクの登録と開始 | `init_kernel` | C | TCB・キューを空にし、`pv_handler` を `trap #1` に登録 | p.35 |
| | `set_task` | C | タスクを TCB に登録して ready につなぐ | p.36 |
| | `init_stack` | C | 初めて走るタスク用に、スタックを「中断した形」に作る | p.36–37 |
| | `begin_sch` | C | 最初のタスクを選び、タイマを始めて起動する | p.36 |
| | `first_task` | asm | 最初のタスクのレジスタをスタックから戻して起動する | p.38–39 |
| タスク切替え | `swtch` | asm | 今のタスクを保存して、`next_task` を再開する | p.40–41 |
| タイマ | `init_timer` | asm | モニタの `trap #0` でタイマ割込みを設定する | p.41 |
| | `hard_clock` | asm | タイマ割込みのたびに、今のタスクを ready に戻して切り替える | p.41 |
| セマフォ | `P` / `V` | asm | `trap #1` でカーネルを呼ぶ（P = 0 番、V = 1 番） | p.39 |
| | `pv_handler` | asm | `trap #1` の入口。割込みを止めて `p_body` / `v_body` を呼ぶ | p.39–40 |
| | `p_body` / `v_body` | C | セマフォの値を減らす／増やす。必要なら休眠・起床 | p.37–38 |
| | `sleep` / `wakeup` | C | セマフォのキューで休眠させる／ready に戻す | p.38 |
| テスト | `test2.c` | C | `main` と、交互に動くことを確かめる user タスク | p.42–48 |

**合わせるのが大事なところ**（担当どうしで話し合う）:
- `init_stack` が積む順番 ⇔ `first_task`・`swtch` が戻す順番（図 2.8 p.37）
- TCB のフィールドの並び ⇔ asm 側のオフセット（`mtk_asm.s` の `TCB_SIZE`・`TCB_STACK_PTR`）

### 動作確認の段階（テキスト p.42）

| 段階 | やること | 主に確かめる関数 |
|---|---|---|
| D1 | タイマを切った状態で、最初のタスクが動く | `set_task`, `init_stack`, `begin_sch`, `first_task` |
| D2 | タイマを入れても（`hard_clock` は何もしない）、最初のタスクが動き続ける | `init_timer` |
| D3 | タイマでタスクが切り替わり、2 巡目が来る | `hard_clock`, `swtch`, キュー |
| D4 | セマフォの P・V が正しく動く | `P`, `V`, `pv_handler`, `p_body`, `v_body` |
| D5 | セマフォでタスクの順番を制御できる（2.5.7 のテスト） | 全体 |

## テーマ2・3：分担と進め方

### 分担

**機能ごとに分ける**（テキスト図 2.6）。全員がどれか 1 つの機能の開発責任者になる（`08 3日目の目標.txt`）。

| # | 機能 | 関数・仕事 | 言語 | 担当 |
|---|---|---|---|---|
| 1 | データ構造・キュー・デバッグ表示 | `mtk_c.h`、`addq`, `removeq`, `sched`, `sleep`, `wakeup`、ready キューとセマフォのキューを表示する関数（D3・D4 の確認用） | C | 松岡 |
| 2 | タスクの登録と開始 | `init_kernel`, `set_task`, `init_stack`, `begin_sch`、`test2.c` の `main` | C | 副田 |
| 3 | タスクの切替え | `first_task`, `swtch` | asm | 安森 |
| 4 | タイマ・最終テスト | `init_timer`, `hard_clock`, `skipmt`、カーネル内のタイマ割込みの扱い、2.5.7 のテストプログラム（D5） | asm・C | 神井 |
| 5 | セマフォ | `P`, `V`, `pv_handler`（asm）、`p_body`, `v_body`（C） | asm・C | 田邉 |

負担がそろうように分けてある。3 は書く量は少ないが一番難しいので、D1・D3 のデバッグは 1・2 も手伝う。

**機能のあいだで合わせること**（最初に話し合う）:

| 合わせること | 関係する機能 |
|---|---|
| TCB のフィールドの並び ⇔ asm 側の `TCB_SIZE`・`TCB_STACK_PTR` | 1 ↔ 3 |
| `init_stack` が積む順番 ⇔ `first_task`・`swtch` が戻す順番（図 2.8 p.37） | 2 ↔ 3 |
| `addq`・`removeq` の引数と、空のキューで返す値 | 1 ↔ 2, 4, 5 |
| `sleep`・`wakeup` の引数（セマフォ ID）、`swtch` を呼ぶ前に `next_task` を決めておくこと | 1 ↔ 3, 5 |
| `swtch` が積む SR、`hard_clock` の戻り方（RTS）と退避するレジスタ | 3 ↔ 4 |
| `p_body`・`v_body` の引数の渡し方（スタックに 4 バイト） | 5 |
| `inchrw.s`・`outchr.s` を誰のテーマ1 から入れるか | 全員 |

### 進め方

1. 担当の関数を書く（`mtk_c.h` を変えるときは班に一声かける）
2. 担当部分ができたら班で読み合わせ
3. 統合して、D1 から順にデバッグ（目安: 8日目）

- テーマ2 以降はリファレンスモニタ `mon_r2_230k+sc5v2.o` を使う。`make` が自動で `mon.o` にするので、**一番上には `mon.s` を置かない**。
- 一番上の `inchrw.s`・`outchr.s` はまだ空。誰かのテーマ1 の `inbyte`/`outbyte` を入れないと `printf` が出ない。
- テーマ3 個人開発部のアプリは `user_<名前>.c` に書く。

## テーマ1（個人）

自分のフォルダを作って、そこで作業する（`theme1/kamii/` は済み）。他人のフォルダは編集しない。

```sh
cd ~/mtk && git pull
mkdir -p theme1/<名前> && cd theme1/<名前>
tar xzf ../../dist/mtk-hina.tgz --strip-components=1   # 配布のひな形を展開
cp ../kamii/Makefile .                                 # 自動準備つきの Makefile

# テーマ1 を ~/mtk で進めていた人（退避した ~/mtk.theme1 から持ってくる）
cp ~/mtk.theme1/{mon.s,inchrw.s,outchr.s,test1.c} .
# これから始める人
cp <ソフトウェア実験Ⅰの simple_os.s> mon.s             # 簡易 OS を mon.s にする

make test1
git add . && git commit -m "テーマ1（<名前>）" && git push   # 生成物は .gitignore で除かれる
```

| 作る・直すファイル | やること | テキスト |
|---|---|---|
| `mon.s` | `.global monitor_begin` / `.extern start` / `jmp start` を追加。`jmp start` の前で割込みを有効にする | 1.3.2 |
| `inchrw.s` | `inbyte`：GETSTRING で 1 文字読む。失敗したらリトライ | 1.4.1 |
| `outchr.s` | `outbyte`：PUTSTRING で 1 文字書く。失敗したらリトライ | 1.4.2 |
| `test1.c` | `main()` の**無限ループの中**にテストを書く | 1.4.3 |

注意: `mon.s` の作業領域は使わない。レジスタは壊すものだけ退避する（`%D0` は戻り値）。

## git の決まり

- 作業の前に `git pull`
- 自分のフォルダ・担当の関数以外は編集しない
- 生成物（`.o`, `.abs`, `.map` など）はコミットしない（`.gitignore` 済み）
- 動いたらタグを付けておく（例: `git tag theme1-<名前>`）

## 配布ファイル（`dist/`）

| ファイル | 中身 | 配布日 |
|---|---|---|
| `mtk-hina.tgz` | 作業フォルダのひな形 | 11/24 |
| `mon_r2_230k+sc5v2.tgz` | リファレンスモニタと readme（2024/12/14 版） | 11/24 |
| `Howto_Replace_mon.txt` | モニタの差し替え手順（Shift_JIS） | 11/24 |
| `csys68k_changeBSkey_v2.c` | BS キーのコードをマクロにした `csys68k.c` | 11/24 |
| `test1_escseq.c` | xterm のエスケープシーケンスのサンプル（EUC-JP） | 11/24 |
| `soft-jikken.tgz` | 修正版のビルド設定（gcc 11・`python3` 用の `make.conf`） | 11/27 |
| `Makefile.moodle` | `LIB_JIKKEN=~/.local/lib/soft-jikken` にした `Makefile` | 11/27 |
| `soft-jikken_instructions.txt` | 上の 2 つの使い方（Moodle では `instructions.txt`） | 11/27 |

Shift_JIS・EUC-JP のファイルは `iconv -f SHIFT_JIS -t UTF-8 ファイル名` のようにして読む。

## ビルドの仕組み

`make test1`〜`test3` は、足りないものを自動で用意してからビルドする。

| 足りないもの | 自動でやること | 書いてある場所 |
|---|---|---|
| `~/.local/lib/soft-jikken/make.conf` | `dist/soft-jikken.tgz` を `~` に展開（初回だけ） | `Makefile` |
| `mon.o` | `dist/mon_r2_230k+sc5v2.tgz` から取り出す | `Makefile.1`〜`.3`（一番上だけ） |
| `mon.glis` | 空のファイルを作る | 〃 |

`make clean` で `mon.o` などが消えても、次の `make` でまた用意される。

<details>
<summary>なぜ必要か（詳しく）</summary>

- **`soft-jikken.tgz`**: 実験室の `/usr/local/lib/soft-jikken/make.conf` は gcc 5 の場所（`-L/usr/local/lib/gcc/m68k-elf/5`）を見ているが、実験室の gcc は 11 で、`5/` には `libgcc.a` がない。そのままだとリンクで `cannot find -lgcc` になる（2026-11-27 確認）。また `.LIS` を作る `gal.py` が Python 2 用のまま。Moodle の `soft-jikken.tgz` はこの 2 点を直した版（元の版は中の `make_org.conf`）。
- **`mon.o`**: `.o` は git に入らないので、clone したばかりのフォルダにはない。
- **`mon.glis`**: `testN.LIS` を作るのに `mon.glis`（`mon.s` をアセンブルするとできるリスティング）が要る。`mon.s` を置かないのでできず、`make` が止まってしまう。空のファイルでも `gal3.py` は問題なく読む。
- `Makefile` は Moodle 版をもとに、`LIB_JIKKEN ?= $(HOME)/.local/lib/soft-jikken` にし、子の make に `$(MAKEFLAGS)` を渡す行を消した。なので `make test2 LIB_JIKKEN=...` で別の場所も指せる。

</details>

## 関連

- ソフトウェア実験Ⅰ（アセンブラの簡易 OS `simple_os.s`）: [`softexp-os`](https://github.com/KamiiAoto/softexp-os) の `sw1/`
