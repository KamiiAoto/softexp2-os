# ソフトウェア実験Ⅱ・Ⅲ（マルチタスクカーネルの作成）

MC68VZ328（68000 互換）ボードで動く，C 言語のマルチタスクカーネルを作る班リポジトリ。メンバーは 5 人。
ページ番号はテキスト `C-jikken_SW2_3_20241210.pdf` の印刷ページ。

**このリポジトリの一番上がそのまま `mtk` フォルダ**。実験室では `~/mtk` に clone する。

```sh
git clone https://github.com/KamiiAoto/softexp2-os.git ~/mtk   # 初回だけ（~/mtk が既にあれば先に退避する）
cd ~/mtk && git pull                                            # 作業の前に必ず
```

ソフトウェア実験Ⅰ（アセンブラの簡易 OS `simple_os.s`）は別のリポジトリ [`softexp-os`](https://github.com/KamiiAoto/softexp-os) の `sw1/` にある。

| テーマ | 内容 | 作業形態 | 置き場所 |
|---|---|---|---|
| テーマ1 | C 言語ライブラリの移植（`inbyte`/`outbyte`，`test1.c`） | **個人**（相談は可） | `theme1/<名前>/` |
| テーマ2 | マルチタスクカーネル（`mtk_c.c`, `mtk_asm.s`, `test2.c`） | **班で分担** | 一番上 |
| テーマ3 共同開発部 | fd 対応・`fcntl`（`inchrw.s`, `outchr.s`, `csys68k.c`） | **班** | 一番上 |
| テーマ3 個人開発部 | マルチタスクアプリ | **個人** | `user_<名前>.c` |

提出（テキスト p.3–4）: カーネルは班で 1 つ，user タスクは個人ごと。どのファイルが誰のものか分かるように，ファイル名に名前を入れる。

## 日程

| 日付 | 内容 |
|---|---|
| 11/24(火) | 実験開始・テーマ1（1日目: 環境整備・`mon.s` の加工） |
| 11/26 頃 | 2日目: テーマ1 完成・小テスト |
| 11/27(金) | テーマ1 終了・テーマ2 開始（3日目: 分担決定） |
| 12/1(火) | テーマ1 試問の告示 |
| 12/10(木) | テーマ2 終了・テーマ3 開始 |
| **12/16(水)** | **テーマ1 レポート提出締切・テーマ1 試問** |
| 12/22(火) | テーマ2,3 試問の告示 |
| **1/7(木)** | **テーマ2,3 レポート提出締切**・試問（1–6 班 1/7，7–12 班 1/8） |

## ビルドの準備（実験室の PC，初回だけ）

実験室の `/usr/local/lib/soft-jikken/make.conf` のままだと `make` がリンクで失敗する（`cannot find -lgcc`。2026-11-25 確認）。
原因: 実験室の `m68k-elf-gcc` は 11 に更新されたが，`make.conf` が古いまま gcc 5 の場所（`-L/usr/local/lib/gcc/m68k-elf/5`）で `libgcc.a` を探している。
また `.LIS` を作る `gal.py` が Python 2 用のままになっている。

Moodle で配布された修正版（`dist/soft-jikken.tgz`。gcc 11 の場所と `python3 gal3.py` に直した `make.conf`）を `~` に展開する（`dist/soft-jikken_instructions.txt`）。

```sh
tar xzf ~/mtk/dist/soft-jikken.tgz -C ~     # ~/.local/lib/soft-jikken/ ができる
```

一番上の `Makefile` は，Moodle で配布された `LIB_JIKKEN=~/.local/lib/soft-jikken` の版（`dist/Makefile.moodle`）にしてある。
この `Makefile` は `$(MAKEFLAGS)` をそのまま子の make に渡すので，`make test2 LIB_JIKKEN=...` のように上書きすることはできない（`-f` がターゲット扱いになる）。
別の場所を使うときは `make -f Makefile.2 LIB_JIKKEN=...` とする。

## テーマ1（個人）

自分のフォルダ `theme1/<名前>/` を作り，そこを `mtk` フォルダとして使う（`theme1/kamii/` は済み）。
テーマ1 は個人作業。班内で相談してよいが，最後は自分で作る（`06 2日目の目標.txt`）。他人のフォルダは編集しない。

```sh
cd ~/mtk && git pull
mkdir -p theme1/<名前> && cd theme1/<名前>
tar xzf ../../dist/mtk-hina.tgz --strip-components=1   # 配布の mtk フォルダの中身をここに展開
cp ../../dist/Makefile.moodle Makefile                 # 上の「ビルドの準備」の Makefile にする
cp <ソフトウェア実験Ⅰの simple_os.s> mon.s             # 簡易 OS を mon.s にする
```

展開されるファイル: `crt0.s`, `csys68k.c`（`read`/`write`），`inchrw.s`・`outchr.s`（ラベルだけの空ファイル），`Makefile`, `Makefile.1`〜`.3`。
`make` は実験室の PC でしか動かない。

自分で作る・直すファイル:

| ファイル | やること | テキスト |
|---|---|---|
| `mon.s` | `.global monitor_begin` / `.extern start` / `jmp start` を追加。`jmp start` の前で割込みを有効にする | 1.3.2 |
| `inchrw.s` | `inbyte`（GETSTRING で 1 文字読む。失敗したらリトライ） | 1.4.1 |
| `outchr.s` | `outbyte`（PUTSTRING で 1 文字書く。失敗したらリトライ） | 1.4.2 |
| `test1.c` | `main()` の**無限ループの中**にテストを書く（1 回で止まるテストは評価されない） | 1.4.3 |

```sh
make test1        # test1.abs ができる（全部作り直すときは make clean してから）
```

注意: `mon.s` の作業領域は使わない。レジスタは壊すものだけ退避する（`%D0` は戻り値）。

## テーマ2・3（班）

一番上が班で 1 つのカーネル。配布のひな形（`dist/mtk-hina.tgz`）は展開済み。

テーマ2以降は，配布のリファレンスモニタ `mon_r2_230k+sc5v2.o` に差し替えて開発する（2 ポート・230kbps・SKIPMT 対応。`dist/mon_r2_230k+sc5v2.tgz` の readme，`dist/Howto_Replace_mon.txt`）。
`.o` は git に入らないので，clone したら各自 1 回だけ次を打つ。

```sh
cd ~/mtk
tar xzf dist/mon_r2_230k+sc5v2.tgz -O mon_r2_230k+sc5v2.o > mon.o   # モニタを取り出して mon.o にする
make test2
```

担当部分ができたら班で読み合わせ → 統合デバッグ（目安: 8日目）。

### 分担（テーマ2）

3日目（11/27）に分担を決めて，下の表を埋める。全員がどこかの機能の開発責任者になる（`08 3日目の目標.txt`，テキスト図2.6）。

| 機能 | 担当 | 主なファイル・関数 | レビュー |
|---|---|---|---|
| | 神井 | | |
| | 副田 | | |
| | 松岡 | | |
| | 安森 | | |
| | 田邉 | | |

### ファイル

| ファイル | 内容 |
|---|---|
| `mtk_c.h` | カーネルの型・定数・関数の extern 宣言（TCB，セマフォなど） |
| `mtk_c.c` | C 側のカーネル（キュー操作，P/V，スケジューラなど） |
| `mtk_asm.s` | アセンブラ側のカーネル（`swtch`，`first_task`，タイマ割込みなど） |
| `test2.c` | マルチタスクのテスト（main と user タスク） |
| `user_<名前>.c` | テーマ3 個人開発部のアプリ（個人ごと） |

## 配布ファイル（`dist/`。中身は変えない）

2026-11-24 配布:

| ファイル | 内容 |
|---|---|
| `mtk-hina.tgz` | 作業フォルダのひな形（上の「展開されるファイル」） |
| `mon_r2_230k+sc5v2.tgz` | リファレンスモニタ `mon_r2_230k+sc5v2.o` と readme（2024/12/14 版） |
| `Howto_Replace_mon.txt` | モニタの差し替え手順（Shift_JIS。`iconv -f SHIFT_JIS -t UTF-8` で読む） |
| `csys68k_changeBSkey_v2.c` | `csys68k.c` の改良版。BS キーのコードをマクロ（`XTBS` = 0x7f，`ASBS` = 0x08）にしたもの |
| `test1_escseq.c` | xterm のエスケープシーケンス（カーソル移動・画面消去など）を試すサンプル（EUC-JP。`iconv -f EUC-JP -t UTF-8` で読む） |

2026-11-27 Moodle 追加配布:

| ファイル | 内容 |
|---|---|
| `soft-jikken.tgz` | 修正版のビルド設定（`~` に展開すると `~/.local/lib/soft-jikken/`）。`make.conf` は gcc 11・`python3` 用，元の版は `make_org.conf` |
| `Makefile.moodle` | `LIB_JIKKEN=~/.local/lib/soft-jikken` にした `Makefile`（Moodle では `Makefile` という名前） |
| `soft-jikken_instructions.txt` | 上の 2 つの使い方（Moodle では `instructions.txt`。CRLF） |

## git の決まり

- 作業の前に `git pull`。自分のフォルダ・担当ファイル以外は編集しない。
- `.o`, `.abs`, `.map` などの生成物はコミットしない（`.gitignore` 済み）。
- 動いたら `git tag theme1-<名前>` のようにタグを付けておくと後で戻れる。
