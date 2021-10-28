# 課題1のヒント

## RTCハードウェアの調査

### QEMUがエミュレートしているマシンは何か？
RTCを読み出すシステムコールを作成するにあたり，まずは使用されているRTCのハードウェアが何であり，それがコンピュータにどのように接続されているかを調べたい．
そのために，xv6がどのような「ハードウェア」上で動作しているのかを知る必要がある．
xv6はQEMUがエミュレートしているRISC-Vコンピュータ上で動作しているので，そのコンピュータの具体的な構成を知るために，QEMUの起動オプションを見てみることにする．
```
$ cd xv6-riscv
$ make qemu
qemu-system-riscv64 -machine virt -bios none -kernel kernel/kernel -m 128M -smp 3 -nographic -drive file=fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

xv6 kernel is booting

hart 2 starting
hart 1 starting
init: starting sh
$ 
```
ここでmakeが実行している `qemu-system-riscv64` がRISC-V版のQEMUの起動コマンドであるが，そのオプションとして `-machine` というのがある．なんとなくエミュレートするマシンを指定していそうである．

[QEMUの公式サイト](https://www.qemu.org)にある[ドキュメント](https://www.qemu.org/docs/master/)の[Invocation](https://www.qemu.org/docs/master/system/invocation.html)セクションによると，確かにこれはエミュレートされるマシンを指定するものであり，`-machine help` でエミュレート可能なマシンのリストが出力されるとある．
試しにやってみると以下のようになった．
```
$ qemu-system-riscv64 -machine help
Supported machines are:
none                 empty machine
sifive_e             RISC-V Board compatible with SiFive E SDK
sifive_u             RISC-V Board compatible with SiFive U SDK
spike                RISC-V Spike Board (default)
spike_v1.10          RISC-V Spike Board (Privileged ISA v1.10)
spike_v1.9.1         RISC-V Spike Board (Privileged ISA v1.9.1)
virt                 RISC-V VirtIO board
```
上の `make qemu` の出力をみると `qemu-system-riscv64` の起動オプションとして `-machine virt` が指定されており，確かにエミュレート可能なマシンのリストにも `virt` が存在する．つまりこの `virt` がエミュレートされているRISC-Vコンピュータなのであろう．ではこれはどのようなマシンなのか．

ちょっとわかりにくいが，ドキュメントにはエミュレートされるアーキテクチャそれぞれについての説明を集めた[QEMU System Emulator Targets](https://www.qemu.org/docs/master/system/targets.html)というセクションがある．
その下の[RISC-V System emulator](https://www.qemu.org/docs/master/system/target-riscv.html)というセクションに[‘virt’ Generic Virtual Platform (virt)](https://www.qemu.org/docs/master/system/riscv/virt.html)セクションがあり，`virt` について説明されている．それによるとこれは実際のハードウェアのエミュレーションをするわけではなく，LinuxなどのOSを仮想化するのに都合の良い構成の仮想ハードウェアを提供しているようである．
このマシンで使用可能なデバイスとして以下が挙げられているが，この中の `1 Google Goldfish RTC` が目的とするものであろう．

* Up to 8 generic RV32GC/RV64GC cores, with optional extensions
* Core Local Interruptor (CLINT)
* Platform-Level Interrupt Controller (PLIC)
* CFI parallel NOR flash memory
* 1 NS16550 compatible UART
* 1 Google Goldfish RTC
* 1 SiFive Test device
* 8 virtio-mmio transport devices
* 1 generic PCIe host bridge
* The fw_cfg device that allows a guest to obtain data from QEMU

### Goldfish RTCとは
世の中にはRTCのデバイスは山ほどあるが，その使い方は統一されているわけではないため，普通はベンダーが提供するドキュメント（データシート）と首っ引きでドライバを書くことになる．
ちょっと面倒だなあと思いつつ検索してみたところ，どうやら Goldfish RTCというハードウェアがあるわけではなく，もともとはAndroidのエミュレータ用に作られた仮想RTCのことらしい．
Googleが公開しているAndroid関連のソースコードに[ドキュメント](https://android.googlesource.com/platform/external/qemu/+/master/docs/GOLDFISH-VIRTUAL-HARDWARE.TXT)がある．
それによるとGoldfish RTCは以下のようなレジスタを持っている．
```
Device properties:
  Name: goldfish_rtc
  Id: -1
  IrqCount: 1
  I/O Registers:
    0x00  TIME_LOW         R: Get current time, then return low-order 32-bits.
    0x04  TIME_HIGH        R: Return high 32-bits, from previous TIME_LOW read.
    0x08  ALARM_LOW        W: Set low 32-bit value or alarm, then arm it.
    0x0c  ALARM_HIGH       W: Set high 32-bit value of alarm.
    0x10  CLEAR_INTERRUPT  W: Lower device's irq level.
```
普通のデバイスは初期化や設定のためのコマンドレジスタがあり，それらを使って適切な設定をしないと動かないのであるが，仮想化専用デバイスのせいかそういったものは見当たらない．
おそらくホストの時計と同期させて使うためであろう．
見た感じでは `TIME_LOW` と `TIME_HIGH` をそれぞれ32ビットデータとして読み，それらを結合した64ビットの値がエポック(1970年1月1日0時0分0秒)からの経過時間をナノ秒単位で表したものになるらしい．
実際，Linuxの[ドライバ](https://github.com/torvalds/linux/blob/master/drivers/rtc/rtc-goldfish.c)でもそうやってアクセスしている．

### RTCのアドレスは？
Goldfish RTCのレジスタの用途はわかったが，それらが `virt` でどのアドレスに割り当てられているのかはまだ不明である．
Goldfish RTCのドキュメントやQEMUのドキュメントにも情報はない．
xv6は `virt` の他のデバイス（UARTやPLIC等）のアドレスをどうやって指定しているのか．
xv6カーネルのヘッダファイル[`memlayout.h`](https://titech-os.github.io/xv6-riscv-html/S/49.html)には以下のようにある．
```
// qemu -machine virt is set up like this,
// based on qemu's hw/riscv/virt.c:
//
// 00001000 -- boot ROM, provided by qemu
// 02000000 -- CLINT
// 0C000000 -- PLIC
// 10000000 -- uart0 
// 10001000 -- virtio disk 
// 80000000 -- boot ROM jumps here in machine mode
//             -kernel loads the kernel here
// unused RAM after 80000000.
```
要は[QEMUのソースコード](https://gitlab.com/qemu-project/qemu)を参照すべきらしい．
確かにソースコードの[`hw/riscv/virt.c`](https://gitlab.com/qemu-project/qemu/-/blob/master/hw/riscv/virt.c)によると，各デバイスのアドレスは以下のようになっていそうである．これは `memlayout.h` で参照されているアドレスとも一致している．
```
static const MemMapEntry virt_memmap[] = {
    [VIRT_DEBUG] =       {        0x0,         0x100 },
    [VIRT_MROM] =        {     0x1000,        0xf000 },
    [VIRT_TEST] =        {   0x100000,        0x1000 },
    [VIRT_RTC] =         {   0x101000,        0x1000 },
    [VIRT_CLINT] =       {  0x2000000,       0x10000 },
    [VIRT_ACLINT_SSWI] = {  0x2F00000,        0x4000 },
    [VIRT_PCIE_PIO] =    {  0x3000000,       0x10000 },
    [VIRT_PLIC] =        {  0xc000000, VIRT_PLIC_SIZE(VIRT_CPUS_MAX * 2) },
    [VIRT_UART0] =       { 0x10000000,         0x100 },
    [VIRT_VIRTIO] =      { 0x10001000,        0x1000 },
    [VIRT_FW_CFG] =      { 0x10100000,          0x18 },
    [VIRT_FLASH] =       { 0x20000000,     0x4000000 },
    [VIRT_PCIE_ECAM] =   { 0x30000000,    0x10000000 },
    [VIRT_PCIE_MMIO] =   { 0x40000000,    0x40000000 },
    [VIRT_DRAM] =        { 0x80000000,           0x0 },
};
```

RTCの場所は0x101000番地のようなので，`memlayout.h` に以下を付け加えることにする．
```
// Goldfish RTC
#define RTC0 0x00101000L
```
このアドレス `RTC0` に `TIME_LOW`(0x00)と`TIME_HIGH`(0x04)を足したアドレスが，現在時刻（エポックからのナノ秒単位での経過時間）の下位32ビットと上位32ビットになるはずである．

以上でxv6で使えるRTCの調査は完了した．

## RTCを読み出すコードを書く

### RTCの設定
RTCが何であるかはわかったが，起動したQEMUがそれを適切な設定で有効にしているかを確認する必要がある．
QEMUドキュメントの[Invocation](https://www.qemu.org/docs/master/system/invocation.html)セクションによると，起動時にRTCを設定するためのコマンドラインオプション `-rtc` があり，その引数に `clock=host,base=utc` と指定することでRTCはホスト（QEMUを動かしている環境）の時計と同期し，協定世界時(UTC)にセットされるとある．
xv6のMakefileではQEMUのコマンドラインオプションを `QEMUOPTS` という変数にまとめているので，この変数を定義しているあたりに以下の行を追加すればよい．
```
QEMUOPTS += -rtc clock=host,base=utc
```

### RTCをアクセス可能にする
RTCにアクセスするためのアドレス（物理アドレス）はわかったが，このアドレスにカーネルスレッドから実際にアクセスできるようにする必要がある．
アドレス空間を設定するコードは[`vm.c`](https://titech-os.github.io/xv6-riscv-html/S/38.html)にまとめられているが，カーネルスレッドの論理アドレス空間はその中の関数 `kvmmake` が設定している．
その中でUARTについて以下のような記述がある．
```
// uart registers
kvmmap(kpgtbl, UART0, UART0, PGSIZE, PTE_R | PTE_W);
```
これは[`memlayout.h`](https://titech-os.github.io/xv6-riscv-html/S/49.html)で定義されているUARTのデバイスアドレス `UART0` から1ページ分を論理アドレス空間の同じアドレス(`UART0`)にマップするよう，カーネルスレッドのページテーブルを設定しているところである．
これを参考にして，`memlayout.h` に付け加えた `RTC0` も論理アドレス空間の同じ位置にマップするようなコードを追加すればよい．

### RTCを読み出す
RTCを読み出すコードを書くために，`uart.c` を参考にして `sysdate.c` に以下のマクロを追加する．
```
#define TIME_LOW  0x00
#define TIME_HIGH 0x04

#define Reg(reg) ((volatile uint32 *)(RTC0 + reg))
#define ReadReg(reg) (*(Reg(reg)))
```
そして `sys_getdate`（あるいはそれから呼ばれる関数）で以下のようにすることで`nsec` にエポックからの経過時間をナノ秒単位で得ることができる．
```
uint64 time_l = ReadReg(TIME_LOW);
uint64 time_h = ReadReg(TIME_HIGH);
uint64 nsec = (time_h << 32) | time_l;
```
あとは `nsec` から `rtcdate` 構造体の各フィールドを計算するコードを書けばよいことになる．

#### 注意
`TIME_HIGH` の読み出しよりも前に `TIME_LOW` の読み出しを行うこと．理由は
QEMUのソースコード中の[Goldfish RTCの実装](https://gitlab.com/qemu-project/qemu/-/blob/master/hw/rtc/goldfish_rtc.c)を読むとわかる．
