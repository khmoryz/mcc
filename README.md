# MCC
## About
[低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook)を参考にした実装

## Exec

コマンド実行:

```
docker run --rm -v $PWD:/mcc -w /mcc compilerbook {コマンド}
```

シェル起動:

```
docker run --rm -it -v $PWD:/mcc -w /mcc compilerbook bash
```

コアダンプ調査方法:

```
ulimit -c unlimited
./mcc "{引数}"
gdb a.out core.{プロセス番号}
```
