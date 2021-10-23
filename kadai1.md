# 課題1：システムコール　getdate の作成

現在の日時および時刻を得るためのシステムコール `getdate` を作成する．

xv6-riscvのkernelディレクトリには `date.h` というファイルがあり，その中で `rtcdate` という構造体が定義されている．ところがこの構造体はカーネルおよびユーザプログラムのどこにおいても用いられていない．本課題ではせっかくあるこの構造体を活用しようというものである．

## システムコール `getdate`
多くのコンピュータはリアルタイムクロック(RTC)と呼ばれる時計の役割を持つデバイスを備えている．画面に表示される時計やファイルのタイムスタンプ（読み書きした時刻）など，さまざまな場面でRTCから得られる時刻が用いられる．
本課題で作成する `getdate` は現在時刻をRTCから読みだし，引数が指す `rtcdate` 構造体に格納する．格納される時刻は協定世界時(UTC)とする．
```C
int getdate(struct rtcdate *ds);
```
本システムコールの返値はエラーの有無を表す．その値はエラーがない場合は0，エラーが生じた場合は-1とする．

ここで用いられている `rtcdate` 構造体は `kernel/date.h` において以下のように定義されている．
```C
struct rtcdate {
  uint second;
  uint minute;
  uint hour;
  uint day;
  uint month;
  uint year;
};
```

### 使用例
例えば以下のようなユーザプログラム(`user/date.c`)を実行すると現在の日時が表示される．
```C
#include "kernel/types.h"
#include "kernel/date.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  struct rtcdate ds;
  if (getdate(&ds) < 0) {
    fprintf(2, "date: failed to get date\n");
    exit(1);
  }

  printf("%d/%d/%d %d:%d:%d\n",
         ds.year, ds.month, ds.day,
         ds.hour, ds.minute, ds.second);

  exit(0);
}
```
このプログラムは本課題用のgitリポジトリ `kadai1` （下記参照）に含まれている．`getdate` が完成すれば，xv6上で上記プログラムを実行すると，以下の例のように実行した日時を出力するはずである．
```sh
$ date
2021/10/20 15:20:17
```

## 課題への取り組み方
### (1)課題用ブランチのチェックアウト
本講義で用意しているxv6-riscvリポジトリ(`https://github.com/titech-os/xv6-riscv`)にこの課題のためのブランチ `kadai1` を用意した．
まずこれを手元の作業ディレクトリ（`xv6-riscv` とする）でチェックアウトする．
```sh
% cd xv6-riscv
% git checkout kadai1
```
チェックアウトの際，作業中のファイルがある場合はコミットするか `git stash` で退避しておくとよい．

上記の代わりに，現在の作業ディレクトリとは別の場所（現在の作業ディレクトリの外とすること）に `kadai1` ブランチの内容をクローンしてもよい．
```sh
% git clone -b kadai1 https://github.com/titech-os/xv6-riscv.git xv6-kadai1
% cd xv6-kadai1
```
ここでは `kadai1` ブランチの内容を `xv6-kadai1` という名前のディレクトリにクローンしている．

### (2)課題用ブランチの内容
課題用ブランチ `kadai1` には，システムコール `getdate` を実装するためのファイル `kernel/sysdate.c`，およびそのテスト用のユーザプログラム `user/date.c` が用意されている．加えて，システムコールを呼び出せるようにするための細かい変更が加えられている（変更があるファイルは `kernel/syscall.h`, `kernel/syscall.c`, `user/user.h`, `user/usys.pl` および `Makefile`）．

`kernel/sysdate.c` の内容は以下のようになっている．他のシステムコールと同様に，ユーザプログラムでシステムコール `getdate` を呼び出すとカーネル内で関数 `sys_getdate` が呼び出されるようになっている．`Fill your code here` とあるコメントを削除して自分のコードを書き込み，`sys_getdate` を完成させること．もちろん`sys_getdate` の外に必要な関数，変数，マクロ等の定義を付け加えてもかまわない．
```C
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "date.h"

uint64
sys_getdate(void)
{
  /*
   * Fill your code here
   */
  return 0;
}
```
課題に取り組む前に，ブランチ `kadai1` をチェックアウトした段階でxv6をビルドして起動し，テスト用プログラム `date` を実行できることを確認しておくとよい．
```sh
% make
% make qemu
.....
xv6 kernel is booting

hart 2 starting
hart 1 starting
init: starting sh
$ date
0/0/0 0:0:0
$ 
```
この時点ではシステムコール `getdate` が未完成であるため，正しい時刻は表示されない．

### (3)作業内容
1. QEMUでRTCを使うための方法，およびRTCから情報を得るための方法を調査する．
2. xv6からRTCにアクセスするためのコードを `sysdate.c` 内に書く．また，必要に応じてxv6の他のファイルを変更する．
3. RTCから得られた情報から `rtcdate` 構造体の各フィールドの値を計算するためのコードを `sysdate.c` 内に書き，`sys_getdate` を完成させる．
4. 作成したコードを含むxv6をビルドし，テストプログラム `date` を実行して正しい出力が得られることを確認する．

### (4)年月日の計算についての補足
RTCの仕様は様々であり，現在の年月日時分秒をそれぞれ読み出せるものや，ある特定の日時（例えば1970年1月1日00:00）からの経過時間のみを読み出せるものなどがある．前者の場合は構造体 `rtcdate` の各フィールドに読み出した値をそのまま入れればよいが，後者の場合は閏年等を考慮して年月日時分秒を計算するコードを用意する必要がある．
もし今回使用するRTCが後者である場合は，閏年や閏秒等の扱いにあまり凝る必要はない（そこがこの課題の目的ではないため）．例えば「西暦2000年から2099年まで動作」のような注釈をつけて手を抜いてかまわない．

## 提出物と提出期限
### 提出物
完成させた `sys_getdate` を含む `sysdate.c` と自分が変更した `sysdate.c` 以外のファイル，およびレポートをアーカイブ(zipあるいはtgz)にまとめ，T2SCHOLAで提出すること．

レポートには以下を明記すること．
* 作成したプログラム(`sysdate.c`)について，作成方針，使用したアルゴリズム等
* `sysdate.c` 以外のファイルに変更を加えた場合，どのファイルを何のためにどう変更したか
* プログラムの作成やテストに用いたPC環境（CPU，メモリ，ホストOS，クロスコンパイラのバージョン等．本講義で用意したDockerコンテナを使った場合はそのことも明記する）
* 参考にした書籍やサイト(URL)，アドバイスをもらった人とアドバイスの内容（もしいれば）
* （早期提出の場合のみ）本課題を達成するための調査の方法と内容．参考にしたサイトやソースコード，ドキュメント等のありか（URL等）
* 本課題によって得られた知見

### 提出期限
提出期限は以下の2段階とする．どちらかを選んで提出すること．早期提出は高得点を狙うことができる．
* __早期提出__：上記作業内容の1を自力で行う，プログラムおよびレポートを作成する．期限は10月27日23:59 (JST)．
* __通常提出__：10月28日00:00以降に，本課題に必要な情報（QEMUでどのようにRTCを使用可能にするか，RTCへのアクセスはどうすればよいか等）をヒントとして公開するので，それを参考にしてプログラムとレポートを作成する．期限は11月3日23:59 (JST)．
