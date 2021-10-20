# システムソフトウェア 課題1

現在の日時および時刻を得るためのシステムコール `getdate` を作成する．

xv6-riscv のkernelディレクトリには `date.h` というファイルがあり，そこで 構造体 `rtcdate` （後述）が定義されている．ところがこの構造体はカーネルおよびユーザプログラムのどこにおいても用いられていない．本課題ではせっかくあるこの構造体を活用しようというものである．

## システムコール `getdate`

多くのコンピュータはリアルタイムクロック(RTC)と呼ばれる時計の役割を持つデバイスを備えている．
本課題で作成する `getdate` はRTCから現在の日時の情報を読み出すためのシステムコールで，呼び出した時点の日時が引数が指す `rtcdate` 構造体に格納されるものとする．
```C
int getdate(struct rtcdate *ds);
```
返値はエラーの有無をあらわすものとし，エラーがない場合は0，エラーが生じた場合は -1 とする．

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
`getdate` が完成すれば，xv6上で上記プログラムを実行すると，以下の例のように実行した日時を出力するはずである．
```sh
$ date
2021/10/20 15:20:17
```

## 課題の取り組み方

### 1. 課題用ブランチのチェックアウト
本講義で用意しているxv6-riscvリポジトリ(`https://github.com/titech-os/xv6-riscv`)にこの課題のためのブランチ `kadai1` を用意した．
まずこれを手元の作業ディレクトリ（`xv6-riscv` とする）でチェックアウトする．
```sh
% cd xv6-riscv
% git checkout kadai1
```
チェックアウトの際，作業中のファイルがある場合はコミットするか `git stash` で退避しておく．

上記の代わりに，現在の作業ディレクトリとは別の場所（現在の作業ディレクトリの外とすること）に `kadai1` ブランチの内容をクローンしてもよい．
```sh
% git clone -b kadai1 https://github.com/titech-os/xv6-riscv.git xv6-kadai1
% cd xv6-kadai1
```
ここでは `kadai1` ブランチの内容を `xv6-kadai1` という名前のディレクトリにクローンしている．

### 2. 課題用ブランチの内容
課題用ブランチ `kadai1` には，システムコール `getdate` を実装するためのファイル `kernel/sysdate.c`，およびそのテスト用のユーザプログラム `user/date.c` が用意されている．加えて，システムコールを呼び出せるようにするための細かい変更が加えられている（変更があるファイルは `kernel/syscall.h`, `kernel/syscall.c`, `user/user.h`, `user/usys.pl` および `Makefile`）．

`kernel/sysdate.c` の内容は以下のようになっている．関数 `sys_getdate` がシステムコール `getdate` を実装している．`Fill your code here` とあるコメントを削除して自分のコードを書き込み，`sys_getdate` を完成させること．もちろん必要な関数，変数，マクロ等の定義の本ファイルに付け加えてもかまわない．
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
課題に取り組む前に，ブランチ `kadai1` をチェックアウトした段階でxv6をビルドして実行し，テスト用プログラム `date` を実行し，以下のようになることを確認しておくとよい．
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

### 3. 作業内容

1. QEMUでRTCを使うための方法，およびRTCから情報を得るための方法を調査する．
2. xv6からRTCにアクセスするためのコードを `sysdate.c` 内に書く．また，必要に応じてxv6の他のファイルを変更する．
3. RTCから得られた情報から `rtcdate` 構造体の各フィールドの値を計算するためのコードを `sysdate.c` 内に書き，`sys_getdate` を完成させる．

## 提出方法・提出期限

### 提出物
完成させた `sys_getdate` を含む `sysdate.c` と自分が変更した `sysdate.c` 以外のファイル，およびレポートをアーカイブ(zipあるいはtgz)にまとめ，T2SCHOLAで提出すること．

レポートには以下のことを書くこと．
* 上記の作業内容の1（調査）をどのように行ったか（早期提出の場合）．
参考にした書籍，サイト，アドバイスを受けた人，検索方法等を明記する．
* 作成したプログラム(`sysdate.c`)についての説明（作成方針，使用したアルゴリズム等）
* `sysdate.c` 以外のファイルに加えた変更についての説明（どこを何のためにどう変更したか）

### 早期提出と通常提出
提出期限は以下の2段階とする．早期提出は高得点を狙うことができる．
* 早期提出：上記作業内容の1を自力で行う，プログラムおよびレポートを作成する．期限は10月27日23:59．
* 通常提出：10月28日00:00以降に，本課題を遂行するのに必要なヒント（QEMUでどのようにRTCを使用可能にするか，RTCへのアクセスはどうすればよいか等）を公開するので，それを参考にしてプログラムとレポートを作成する．
