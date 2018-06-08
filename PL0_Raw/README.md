# PL/0 编译器 原始代码

本项目仅作为对照组，不进行后续修改更新。

源代码中有一个bug，有可能是故意留的（Unit1.cpp 文件中 line 555）

	case 5: T--; S[T]=S[T] % S[T+1]; break;

应为：

	case 5: T--; S[T]=S[T] / S[T+1]; break;